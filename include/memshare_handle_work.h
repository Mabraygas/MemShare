#ifndef MEMSHARE_HANDLE_WORK_H
#define MEMSHARE_HANDLE_WORK_H
/**
 * @file memshare_handle_work.h
 * @brief 内存共享模块处理线程
 * @author maguangxu
 * @version 1.0
 * @date 2015-12-02
 */
#include <string>
#include <stdint.h>
#include <eagle_thread.h>

#include <log.h>
#include <urcu/uatomic.h>
#include <GetFinger.h>
#include "memshare_global.h"
#include "memshare_send_work.h"

namespace MEMSHARE
{

using namespace std;
using namespace eagle;

//共享内存接收方的具体处理函数
typedef int (*HandleFunc)(char* buffer, int len);

class MemShareHandleWork : public Thread {
//构造与析构
public:
	MemShareHandleWork(int tid);
	~MemShareHandleWork();

	//初始化函数
	static void Init(HandleFunc handle_func_ptr);

protected:
	//线程的启动栈帧
	virtual void run();

public:
	//本类线程组
	static MemShareHandleWork* s_memshare_handle_work_arr[MemShareGlobal::skMemShareHandleWorkNum];
	//静态函数指针, 由用户自行定义并需在Init函数中初始化
	static HandleFunc _handle_func_ptr;

private:
	//线程号
	int _tid;
	//回复包体缓冲区
	char _send_buf[MemShareGlobal::skMemShareMaxLen];
};

}

#endif //MEMSHARE_HANDLE_WORK_H

// vim: ts=4 sw=4 nu

