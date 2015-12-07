#ifndef MEMSHARE_SEND_WORK_H
#define MEMSHARE_SEND_WORK_H
/**
 * @file 	memshare_send_work.h
 * @brief 	�ڴ湲��ģ�鷢���߳�
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
	//����������
	 MemShareSendWork() {}
	~MemShareSendWork() {}

public:
	/**
	 * @brief Ȼ��index���ڵ�������buf�й����ظ���, �����.
	 * ע��: �����������������Դ.
	 *
	 * @param [buf]  : ������, ���ڱ���ظ���.
	 * @param [index]: ������ȫ�������е��±�
	 * @param [status]:״̬��.
	 *
	 */
	static void SendLastBuf(char *buf, int index, int status);

private:
	//����ظ���
	static int ConstructResp(char *buf, int index, int status);

public:
	//���������
	static EpollServerPtr s_server;
};

}

#endif //MEMSHARE_SEND_WORK_H

// vim: ts=4 sw=4 nu

