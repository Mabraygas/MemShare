#ifndef MEMSHARE_GLOBAL_H
#define MEMSHARE_GLOBAL_H
/**
 * @file 	memshare_global.h
 * @brief 	内存共享全局变量
 * @author 	maguangxu
 * @version 1.0
 * @date 	2015-12-01
 */
#include <string>
#include <stdint.h>
#include <urcu/uatomic.h>
#include <eagle_thread_queue.h>

namespace MEMSHARE
{

using namespace std;
using namespace eagle;

//MemShare全局类
class MemShareGlobal {

public:
	//内存共享的集群机器数量最大值
	static const int skClusterMaxNum = 100;
	//单次内存共享的最大共享量(1MB)
	static const int skMemShareMaxLen = 1024 * 1024;

public:
	//传输内容在共享模块内部的资源结构体
	struct Resource {
		//发送时作为目的机器编号, 接收时作为客户端标记
		uint32_t dest_id;
		//单个内存单元长度
		uint32_t unit_len;
		//传输的内存单元个数
		uint32_t unit_num;
		//传输的buffer指纹(用于校验数据)
		uint64_t finger;
		//具体的传输内容
		char   	 buffer[skMemShareMaxLen];
	}__attribute__ ((packed));

	//发送方资源结构体数组
	static Resource* s_send_data;
	//接收方资源结构体数组
	static Resource* s_recv_data;

	//发送方资源数组长度
	static const int skSendResourceArrLen = 300;
	//接收方资源数组长度
	static const int skRecvResourceArrLen = 300;


//服务器配置处
public:
	//内存共享模块的默认接收端口
	static const int skMemSharePort = 25252;
	//共享内存Socket超时时间
	static const int skMemShareSocketTimeout = 30000 /* ms */;
	
	//网络通信秘钥(客户端req, 服务端recv)
	static const char* skMemShareReqKey;
	//网络通信版本号(客户端req, 服务端recv)
	static const uint8_t skMemShareReqVer = 0x01;
	//网络通信秘钥(客户端recv, 服务端resp)
	static const char* skMemShareRespKey;
	//网络通信版本号(客户端recv, 服务端resp)
	static const uint8_t skMemShareRespVer = 0x01;

//线程配置处
public:
	//接收线程数
	static const int skMemShareRecvWorkNum = 80;
	//处理线程数
	static const int skMemShareHandleWorkNum = 80;
	//客户端发送线程数
	static const int skMemShareShareWorkNum = 17;


public:
	//发送方资源号队列
	static ThreadQueue<int> s_send_resource_queue;
	//接收方资源号队列
	static ThreadQueue<int> s_recv_resource_queue;
	
	//server side:
	//receive队列
	//static ThreadQueue<int> s_memshare_receive_queue;
	//handle队列
	static ThreadQueue<int> s_memshare_handle_queue;

	//client side:
	//share队列
	static ThreadQueue<int> s_memshare_share_queue;

	//回收发送资源
	static void ReclaimSend(int index);
	//回收接收资源
	static void ReclaimRecv(int index);

protected:
	//禁止从类的外部生成类的实例
	MemShareGlobal();
	~MemShareGlobal();
	//禁止复制
	MemShareGlobal(const MemShareGlobal&);
	//禁止赋值
	MemShareGlobal& operator = (const MemShareGlobal&);
};

}

#endif //MEMSHARE_GLOBAL_H

// vim: ts=4 sw=4 nu

