#include "memshare_share_work.h"

namespace MEMSHARE
{

MemShareShareWork::MemShareShareWork(int tid) {
	//线程号赋值
	_tid = tid;
}

MemShareShareWork::~MemShareShareWork()
{ }

void MemShareShareWork::Init(MemShareGlobal* global, const char* ip, int Port, const char** cluster_ip, int cluster_num) {
	//本机ip及端口号
	_ip = string(ip);
	_port = Port;

	//全局定义实例指针
	Global = global;

	//检查cluster_num合法性
	if(cluster_num < 0 || cluster_num > MemShareGlobal::skClusterMaxNum) {
		ERROR("MEMSHARE: Cluster Num Init Invalid In Share Thread! Cluster Num = " << cluster_num);
		return;
	}

	//初始化集群信息
	int next_switch_no = GetNextSwitchNo();
	s_cluster_num[next_switch_no] = cluster_num;
	for(int id = 0; id < cluster_num; id ++ ) {
		s_cluster_ip_arr[next_switch_no][id] = string(cluster_ip[id]);
		//检查cluster_ip合法性
		if(s_cluster_ip_arr[next_switch_no][id].length() <= 0) {
			ERROR("MEMSHARE: Cluster Ip Init Invalid In Share Thread! Cluster Ip = " << s_cluster_ip_arr[next_switch_no][id]);
			return;
		}

		//不建立本机的socket链接
		if(s_cluster_ip_arr[next_switch_no][id] == _ip) {
			continue;
		}
		
		//TCP客户端初始化
		_sock_arr[next_switch_no][id].init(s_cluster_ip_arr[next_switch_no][id], _port, \
										//MemShareGlobal::skMemShareSocketTimeout, MemShareGlobal::skMemShareSocketTimeout);
										MemShareGlobal::skMemShareSocketTimeout);
		//关闭Nagle
		_sock_arr[next_switch_no][id].set_nodelay(true);
	}
	
	//切换
	Switch();
}

void MemShareShareWork::ReInit(const char** cluster_ip, int cluster_num) {

	//检查cluster_num合法性
	if(cluster_num < 0 || cluster_num > MemShareGlobal::skClusterMaxNum) {
		ERROR("MEMSHARE: Cluster Num ReInit Invalid In Share Thread! Cluster Num = " << cluster_num);
		return;
	}

	//初始化集群信息
	int next_switch_no = GetNextSwitchNo();
	s_cluster_num[next_switch_no] = cluster_num;

	//日志
	INFO("MEMSHARE: Share Work ReInit with cluster_num = " << cluster_num);
	
	for(int id = 0; id < cluster_num; id ++ ) {
		s_cluster_ip_arr[next_switch_no][id] = string(cluster_ip[id]);
		//检查cluster_ip合法性
		if(s_cluster_ip_arr[next_switch_no][id].length() <= 0) {
			ERROR("MEMSHARE: Cluster Ip ReInit Invalid In Share Thread! Cluster Ip = " << s_cluster_ip_arr[next_switch_no][id]);
			return;
		}

		INFO("MEMSHARE: Share Work ReInit with cluster_ip[" << id << "] = " << s_cluster_ip_arr[next_switch_no][id]);
		//不建立本机的socket链接
		if(s_cluster_ip_arr[next_switch_no][id] == _ip) {
			continue;
		}

		//TCP客户端初始化
		_sock_arr[next_switch_no][id].init(s_cluster_ip_arr[next_switch_no][id], _port, \
										//MemShareGlobal::skMemShareSocketTimeout, MemShareGlobal::skMemShareSocketTimeout);
										MemShareGlobal::skMemShareSocketTimeout);
		//关闭Nagle
		_sock_arr[next_switch_no][id].set_nodelay(true);
	}
	
	//TCP Client切换

	//切换
	Switch();

}

int MemShareShareWork::Share(char* buf, uint32_t unit_len, uint32_t unit_num, uint32_t destination_id) {
	//检查buf的长度是否合法
	if(unit_num * unit_len <= 0 || unit_num * unit_len > static_cast<uint32_t>(MemShareGlobal::skMemShareMaxLen)) {
		WARN("MEMSHARE: MemShare Share Length Invalid! Len = " << unit_num * unit_len);
		return -1;
	}
	//检查目的机器号是否合法
	if(destination_id >= (uint32_t)s_cluster_num[GetCurrSwitchNo()]) {
		ERROR("MEMSHARE: Destination_Id Invalid In Process Share! Stop Sharing!");
		return -2;
	}

	int  index;
	bool bexist;
	//从资源号队列中获取资源号
	do {
		bexist = Global->s_send_resource_queue.pop_front(index, -1);
	}while(!bexist); //无限制等待

	//赋值资源结构体
	Global->s_send_data[index].dest_id = destination_id;
	Global->s_send_data[index].unit_len = unit_len;
	Global->s_send_data[index].unit_num = unit_num;

	//求buf的finger, 用来接收方校验数据
	unsigned __int64 finger1, finger2;
	Finger_Buf((unsigned char *)buf, unit_len * unit_num, finger1, finger2);
	Global->s_send_data[index].finger = static_cast<uint64_t>(finger1);

	//buffer内容的拷贝
	memcpy(Global->s_send_data[index].buffer, buf, unit_len * unit_num);
	
	//将资源号推送至share队列
	Global->s_memshare_share_queue.push_back(index);

	//推送成功
	return 0;
}

void MemShareShareWork::run() {
	//share队列取出资源号
	int  index;
	bool bexist;
	
	while(1) {
		//无限制等待
		bexist = Global->s_memshare_share_queue.pop_front(index, -1);
		if(!bexist) { continue; } //不存在

		//检查destination_id是否是本机
		uint32_t destination_id = Global->s_send_data[index].dest_id;
		if(_ip == MemShareShareWork::s_cluster_ip_arr[GetCurrSwitchNo()][destination_id]) {
			WARN("MEMSHARE: Do Not Allow To MemShare With the Server Itself!");
			//回收资源并返回
			Global->ReclaimSend(index);
			continue;
		}

		//网络通信收发逻辑
		int iRet = Do(index, destination_id);
		if(0 != iRet) { //收发失败
			WARN("MEMSHARE: MemShare Share a Data Fail from " << _ip << " to " << s_cluster_ip_arr[GetCurrSwitchNo()][destination_id]);
		}

		//回收资源号
		Global->ReclaimSend(index);
	}
}

#define SOCK (_sock_arr[curr_switch_no][destination_id])
int MemShareShareWork::Do(const int index, const uint32_t destination_id) {
	//获取当前切换号
	int curr_switch_no = GetCurrSwitchNo();

	//发送数据缓冲区: 21 = key(8) + ver(1) + len(4) + finger(8)
	char req_buf[MemShareGlobal::skMemShareMaxLen + 21];
	//回复数据缓冲区: 13 = key(8) + ver(1) + status(4)
	char resp_buf[13];
	//网络通信状态变量
	int ret;
	string err;

	//拼发送报文
	size_t packet_len = ConstructReq(req_buf, index);

	//尝试次数
	int try_num = 0;
	while(++try_num <= 3) { //为保证数据的完整性, 共尝试3次
		//发送
		ret = SOCK.send(req_buf, packet_len, err);
		//发送失败
		if(0 != ret) {
			ERROR("MEMSHARE: " << SOCK.getIp() << ":" << SOCK.getPort() << " Share Memory failed! try_num: " << try_num);
			continue;
		}

		//接收数据
		memset(resp_buf, 0x00, 13);
		ret = SOCK.recvLength(resp_buf, 13, err);

		//接收失败
		if(0 != ret) {
			ERROR("MEMSHARE: " << SOCK.getIp() << ":" << SOCK.getPort() << " recv Shared Memory Resp failed! try_num: " << try_num);
			continue;
		}

		//判断秘钥
		if(*(uint64_t *)MemShareGlobal::skMemShareRespKey != *(uint64_t *)resp_buf) {
			ERROR("MEMSHARE: " << SOCK.getIp() << ":" << SOCK.getPort() << " key err: " << *(uint64_t *)resp_buf);
			SOCK.close();
			try_num = 3;
			break;
		}

		//判断版本号
		if(MemShareGlobal::skMemShareRespVer != *(uint8_t *)(resp_buf + 8)) {
			ERROR("MEMSHARE: " << SOCK.getIp() << ":" << SOCK.getPort() << " ver err: " << *(uint8_t *)(resp_buf + 8));
			SOCK.close();
			break;
		}

		//收发成功
		break;
	}
	//尝试3次都失败
	if(try_num > 3) {
		return -1;
	}

	//成功
	return 0;
}

size_t MemShareShareWork::ConstructReq(char* req_buf, const int index) {
	
	size_t offset = 0;
	size_t buffer_len = Global->s_send_data[index].unit_len * Global->s_send_data[index].unit_num;

	*(uint64_t *)req_buf = *(uint64_t *)MemShareGlobal::skMemShareReqKey;
	offset += 8;

	*(uint8_t *)(req_buf + offset) = MemShareGlobal::skMemShareReqVer;
	offset += 1;

	*(uint32_t *)(req_buf + offset) = 8 + buffer_len;
	offset += 4;

	*(uint64_t *)(req_buf + offset) = Global->s_send_data[index].finger;
	offset += 8;

	memcpy(req_buf + offset, Global->s_send_data[index].buffer, \
		   buffer_len);
	offset += buffer_len;

	return offset;
}

}

// vim: ts=4 sw=4 nu

