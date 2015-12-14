#ifndef MEMSHARE_RECV_WORK_H
#define MEMSHARE_RECV_WORK_H
/**
 * @file    memshare_recv_work.h
 * @brief   �ڴ湲��ģ������߳�
 * @author  maguangxu
 * @version 1.0
 * @date    2015-12-02
 */
#include <string>
#include <log.h>
#include <eagle_epoll_server.h>

#include "memshare_global.h"

namespace MEMSHARE
{

using namespace std;
using namespace eagle;

/**
 * raft�����߳�
 */
class MemShareReceiveWork : public Work
{
public :
    /**
     * @brief Ĭ�Ϲ��캯��
     */
    MemShareReceiveWork(void* para = NULL) {
		if(NULL != para) {
			Global = (MemShareGlobal*)para;
		}
	}

    /**
     * @brief ��������
     */
    ~MemShareReceiveWork() {}

	//ȫ�ֶ���ʵ��ָ��
	MemShareGlobal* Global;

protected:
    /** 
     * @brief �߳�����ǰ�ĳ�ʼ��.
     */
    virtual void initialize() {}

    /**
     * @brief �߳�����ʱ����ѭ������ǰ�Ļص�����.
     */
    virtual void startHandle() {}

    /** 
     * @brief �߼�������
     *
     * @param [recv] : Adapter���ն����е�Ԫ��
     */
    virtual void work(const RecvData &recv);

	/**
     * @brief �߼��������(Ҳ��������ѭ�������)ʱ�Ļص�����
     * �ȴ�Broker�߳��˳�.
     */
    virtual void stopHandle() {}

    virtual void WorkOverload(const RecvData &recv)
    {
		WARN("overload");
    }

    virtual void WorkTimeout(const RecvData &recv)
    {
		WARN("timeout");
    }
};

}

#endif //MEMSHARE_RECV_WORK_H

// vim: ts=4 sw=4 nu

