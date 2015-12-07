#ifndef MEMSHARE_SEND_WORK_H
#define MEMSHARE_SEND_WORK_H
/**
 * @file 	memshare_send_work.h
 * @brief 	内存共享模块发送线程
 * @author 	maguangxu
 * @version ver 1.0
 * @date 	2015-12-02
 */
#include <eagle_epoll_server.h>
#include <log.h>
#include <string>

#include <log.h>
#include "memshare_global.h"

namespace MEMSHARE
{

using namespace std;
using namespace eagle;

class MemShareSendWork
{
public:
	//构造与析构
	 MemShareSendWork() {}
	~MemShareSendWork() {}

public:
	/**
	 * @brief 然后将index所在的数据在buf中构建回复包, 最后发送.
	 * 注意: 本函数本不会回收资源.
	 *
	 * @param [buf]  : 缓冲区, 用于保存回复包.
	 * @param [index]: 数据在全局数组中的下标
	 * @param [status]:状态码.
	 *
	 */
	static void SendLastBuf(char *buf, int index, int status);

private:
	//构造回复包
	static int ConstructResp(char *buf, int index, int status);

public:
	//服务器句柄
	static EpollServerPtr s_server;
};

}

#endif //MEMSHARE_SEND_WORK_H

// vim: ts=4 sw=4 nu

