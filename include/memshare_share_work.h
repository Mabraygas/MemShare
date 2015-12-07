#ifndef MEMSHARE_SHARE_WORK_H
#define MEMSHARE_SHARE_WORK_H
/**
 * @file 	memshare_share_work.h
 * @brief 	内存共享模块共享线程
 * @author 	maguangxu
 * @version 1.0
 * @date 	2015-12-01
 */
#include <string>
#include <stdint.h>
#include <eagle_thread.h>
#include <eagle_simple_clientsocket.h>

#include <log.h>
#include <urcu/uatomic.h>
#include <GetFinger.h>
#include "memshare_global.h"
#include "memshare.h"

namespace MEMSHARE
{

using namespace std;
using namespace eagle;

class MemShareShareWork : public Thread {
//构造与析构
public:
	MemShareShareWork(int tid);
	~MemShareShareWork();

	//初始化函数
	static void Init(const char* ip, int Port, const char** cluster_ip, int cluster_num);

	//重新初始化函数, 用于集群ip、角色发生变化时使用
	static void ReInit(const char** cluster_ip, int cluster_num);

protected:
	//线程的启动栈帧
	virtual void run();

public:
	//本类线程组
	static MemShareShareWork* s_memshare_share_work_arr[MemShareGlobal::skMemShareShareWorkNum];
	/**
	 * @brief 客户端调用的共享函数实际指向的位置
	 *
	 * @param [buf]: 共享内存起始地址
	 *		  [unit_len]: 单个内存单元的长度
	 *		  [unit_num]: 本次传输的内存单元个数
	 *		  [destination_id]: 传输目的机器的编号
	 *
	 * @ret	  [0]: 成功
	 *		  [-1]:长度不合法
	 *		  [-2]:目的机器号不合法
	 */
	static int Share(char* buf, uint32_t unit_len, uint32_t unit_num, uint32_t destination_id);

private:
	//发送接收逻辑
	int Do(const int index, const uint32_t destination_id);
	//拼网络报文函数
	size_t ConstructReq(char* req_buf, const int index);

private:
	//本机ip
	static std::string _ip;
	//内存共享端口号
	static int _port;
	
	//共享内存集群信息切换号
	static int s_curr_switch_no;
	//集群内机器数量
	static int s_cluster_num[2];
	//集群内机器ip
	static std::string s_cluster_ip_arr[2][MemShareGlobal::skClusterMaxNum];

public:
	//获取当前切换号
	static int GetCurrSwitchNo() { return uatomic_read(&s_curr_switch_no); }
	//获取下次切换号
	static int GetNextSwitchNo() { return !s_curr_switch_no; }
	//集群信息切换
	static void Switch() { uatomic_set(&s_curr_switch_no, !s_curr_switch_no); }

private:
	//线程号
	int _tid;
	//TCP客户端
	static TCPSimpleClient _sock_arr[2][MemShareGlobal::skClusterMaxNum];
};

}

#endif //MEMSHARE_SHARE_WORK_H

// vim: ts=4 sw=4 nu

