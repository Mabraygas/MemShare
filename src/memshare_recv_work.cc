#include "memshare_recv_work.h"

namespace MEMSHARE
{

void MemShareReceiveWork::work(const RecvData &recv) {
	int  index;
	bool bexist;

	//从接收资源号队列获取资源号
	do {
		bexist = Global->s_recv_resource_queue.pop_front(index, -1);
	}while(!bexist); //无限制等待

	//客户端标记
	Global->s_recv_data[index].dest_id = recv.uid;
	//buffer长度
	Global->s_recv_data[index].unit_len = *(uint32_t *)(recv.buffer.c_str()) - 8;
	Global->s_recv_data[index].unit_num = 1;
	//buffer指纹
	Global->s_recv_data[index].finger = *(uint64_t *)(recv.buffer.c_str() + 4);
	//buffer
	memcpy(Global->s_recv_data[index].buffer, recv.buffer.c_str() + 12, Global->s_recv_data[index].unit_len);

	//推送到处理线程
	Global->s_memshare_handle_queue.push_back(index);
}

}

// vim: ts=4 sw=4 nu

