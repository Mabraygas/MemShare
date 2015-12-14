#ifndef MEMSHARE_H
#define MEMSHARE_H
/**
 * @file 	memshare.h
 * @brief 	内存共享模块
 * @author 	maguangxu
 * @version 1.0
 * @date 	2015-12-01
 */
#include <string>
#include <stdint.h>
#include <vector>
#include <eagle_thread.h>
#include <eagle_epoll_server.h>

#include <log.h>
#include "memshare_global.h"
#include "memshare_recv_work.h"
#include "memshare_handle_work.h"
#include "memshare_send_work.h"
#include "memshare_share_work.h"

namespace MEMSHARE
{

using namespace std;
using namespace eagle;

class MemShare : public Thread {

//构造与析构
public:
	MemShare();
	~MemShare();

public:
	//初始化函数
	void Init(HandleFunc handle_func_ptr, const char* ip, int Port = MemShareGlobal::skMemSharePort, \
					 const char** cluster_ip = NULL, int cluster_num = 0);
	//重新初始化函数, 用于集群ip变更时
	void ReInit(const char** cluster_ip = NULL, int cluster_num = 0);

	//全局定义实例
	MemShareGlobal* Global;

protected:
	//线程的启动栈帧
	virtual void run();

public:
	//epoll_server报文解析函数
	static int Parse(string& buffer, string& o);

	//分配内存
	void AllocateBuffer();

	//启动各工作线程
	int StartThread();

	//客户端调用的共享内存函数
	int Share(char* buf, uint32_t unit_len, uint32_t unit_num, uint32_t destination_id);

private:
	//share线程号offset
	int share_offset;
	//全局服务器变量
	EpollServerPtr g_server;

	//本机ip地址
    std::string _ip;
    //内存共享端口号
    int _port;

	//handle线程向量
	std::vector<MemShareHandleWork*> _handle_vec;
	//share线程向量
	std::vector<MemShareShareWork*> _share_vec;
};

}

#endif //MEMSHARE_H

// vim: ts=4 sw=4 nu

