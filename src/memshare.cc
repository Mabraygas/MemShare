#include "memshare.h"

namespace MEMSHARE
{

MemShare::MemShare()
{ }

MemShare::~MemShare()
{ }

void MemShare::Init(HandleFunc handle_func_ptr, const char* ip, int Port, const char** cluster_ip, int cluster_num) {
	
	//ip地址及端口号
	_ip = string(ip);
	_port = Port;

	share_offset = 0;

	//创建全局定义实例
	if(Global != NULL) {
		delete Global;
	}
	Global = new MemShareGlobal;

	//服务器变量
	g_server = new EpollServer();

	//工作线程的初始化
	_handle_vec.clear();
	_share_vec.clear();
	//调用Share线程(客户端)的初始化函数
	for(int gid = 0; gid < MemShareGlobal::skMemShareHandleWorkNum; gid ++ ) {
		MemShareHandleWork* handle_work = new MemShareHandleWork(gid);
		_handle_vec.push_back(handle_work);
		handle_work->Init(g_server, Global, handle_func_ptr);
	}
	//初始化handle线程的解析函数指针
	for(int gid = 0; gid < MemShareGlobal::skMemShareShareWorkNum; gid ++ ) {
		MemShareShareWork* share_work = new MemShareShareWork(gid);
		_share_vec.push_back(share_work);
		share_work->Init(Global, ip, Port, cluster_ip, cluster_num);
	}

	//分配内存
	AllocateBuffer();
}

void MemShare::ReInit(const char** cluster_ip, int cluster_num) {
	//调用Share线程(客户端)的重新初始化函数
	for(int gid = 0; gid < (int)_share_vec.size(); gid ++ ) {
		_share_vec[gid]->ReInit(cluster_ip, cluster_num);
	}
}

void MemShare::AllocateBuffer() {
	//清空各种工作队列
	Global->s_send_resource_queue.clear();
	Global->s_recv_resource_queue.clear();
	Global->s_memshare_handle_queue.clear();
	Global->s_memshare_share_queue.clear();


	//资源数组
	Global->s_send_data = new MemShareGlobal::Resource[MemShareGlobal::skSendResourceArrLen];
	Global->s_recv_data = new MemShareGlobal::Resource[MemShareGlobal::skRecvResourceArrLen];
	
	//资源号队列
	for(int i = 0; i < MemShareGlobal::skSendResourceArrLen; i ++ ) {
		Global->s_send_resource_queue.push_back(i);
	}
	for(int i = 0; i < MemShareGlobal::skRecvResourceArrLen; i ++ ) {
		Global->s_recv_resource_queue.push_back(i);
	}
}

void MemShare::run() {

	//适配器
	BindAdapterPtr adapter = new BindAdapter(g_server);

	if(_port <= 0 || _port > 65535) {
		ERROR("MEMSHARE: MemShare Port Info Error! Port = " << _port << ". Stop MemShare!");
		return;
	}

	//ip, 端口, 超时(300s), tcp
	Endpoint local(_ip, _port, 300000, true);

	adapter->setEndpoint(local);

	//报文解析协议
	adapter->setProtocol(Parse);

	//创建epollserver线程组, 并将适配器加入服务器
	g_server->CreateWorkGroup<MemShareReceiveWork>("MemShareReceiveWork", MemShareGlobal::skMemShareRecvWorkNum, adapter, (void *)Global);

	//启动各工作线程
	if(0 != StartThread()) {
		ERROR("MEMSHARE: Start Thread Error! Stop MemShare!");
		return;
	}

	//启动服务器
	g_server->EnterMainLoop();
}

int MemShare::Parse(string& buffer, string& o) {
	
	//判断包体长度(key(8) + ver(1) + len(4) = 13)
	if(buffer.length() < 13) {
		return PACKET_LESS;
	}

	//判断秘钥、版本号
	if(*(uint64_t *)MemShareGlobal::skMemShareReqKey != *(uint64_t *)buffer.c_str() || \
	   MemShareGlobal::skMemShareReqVer != *(uint8_t *)(buffer.c_str() + 8)) {
		buffer = buffer.substr(9, buffer.length() - 9);
		return PACKET_ERR;
	}

	//读取包体长度
	uint32_t packet_len = *(uint32_t *)(buffer.c_str() + 9);
	//判断包体长度合法性
	if(packet_len == 0 || packet_len > MemShareGlobal::skMemShareMaxLen + 8) {
		buffer = buffer.substr(13, buffer.length() - 13);
		return PACKET_ERR;
	}

	//再次判断包体长度
	if(buffer.length() < 13 + packet_len) {
		return PACKET_LESS;
	}

	//包体完整
	o = buffer.substr(9, 4 + packet_len);
	buffer = buffer.substr(13 + packet_len, buffer.length() - 13 - packet_len);

	return PACKET_FULL;
}

int MemShare::StartThread() {
	//启动各个工作线程
	try {
		//启动MemShare处理线程
		for(int gid = 0; gid < (int)_handle_vec.size(); gid ++ ) {
			_handle_vec[gid]->start();
		}
		//启动MemShare客户端发送线程
		for(int gid = 0; gid < (int)_share_vec.size(); gid ++ ) {
			_share_vec[gid]->start();
		}

	}catch(EagleException& ex) {
		fprintf(stderr, "Start Thread Error!\n");
		return -1;
	}

	//启动成功
	return 0;
}

int MemShare::Share(char* buf, uint32_t unit_len, uint32_t unit_num, uint32_t destination_id) {
	share_offset ++ ;
	if(share_offset >= 100 * MemShareGlobal::skMemShareShareWorkNum) {
		share_offset = 0;
	}
	//调用Share线程的同名函数
	return _share_vec[share_offset % MemShareGlobal::skMemShareShareWorkNum]->Share(buf, unit_len, unit_num, destination_id);
}

}

// vim: ts=4 sw=4 nu

