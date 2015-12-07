#include "memshare_handle_work.h"

namespace MEMSHARE
{

MemShareHandleWork* MemShareHandleWork::s_memshare_handle_work_arr[MemShareGlobal::skMemShareHandleWorkNum];
HandleFunc MemShareHandleWork::_handle_func_ptr;

MemShareHandleWork::MemShareHandleWork(int tid) {
	//线程号赋值
	_tid = tid;
}

MemShareHandleWork::~MemShareHandleWork()
{ }

void MemShareHandleWork::Init(HandleFunc handle_func_ptr) {
	//具体处理逻辑函数指针
	_handle_func_ptr = handle_func_ptr;
}

void MemShareHandleWork::run() {
	
	int  index;
	bool bexist;

	//数据校验用到的临时变量
	unsigned __int64 finger1, finger2;

	while(1) {
		//从处理队列中取出资源号
		bexist = MemShareGlobal::s_memshare_handle_queue.pop_front(index, -1);
		if(!bexist) { continue; } //无限制等待

		//接收数据状态码
		int status = 0;

		//进行数据校验
		Finger_Buf((unsigned char*)MemShareGlobal::s_recv_data[index].buffer, MemShareGlobal::s_recv_data[index].unit_len, finger1, finger2);
		if(MemShareGlobal::s_recv_data[index].finger != static_cast<uint64_t>(finger1)) {
			WARN("MEMSHARE: Finger Different [" << MemShareGlobal::s_recv_data[index].finger << "," << static_cast<uint64_t>(finger1) << "] when Recv SharedMemory! Give Up the Data!");
			//数据校验出错
			status = -11;
		}

		//若数据校验无误, 调用用户定义的逻辑函数
		if(0 == status) {
			status = _handle_func_ptr(MemShareGlobal::s_recv_data[index].buffer, MemShareGlobal::s_recv_data[index].unit_len);
		}

		//调用发送函数
		MemShareSendWork::SendLastBuf(_send_buf, index, status);
		//回收资源号
		MemShareGlobal::ReclaimRecv(index);
	}
}

}

// vim: ts=4 sw=4 nu

