#include "memshare_global.h"

namespace MEMSHARE
{

MemShareGlobal::Resource* MemShareGlobal::s_send_data;
MemShareGlobal::Resource* MemShareGlobal::s_recv_data;
const char* MemShareGlobal::skMemShareReqKey = "MSREQKEY";
const char* MemShareGlobal::skMemShareRespKey= "MSRESKEY";

ThreadQueue<int> MemShareGlobal::s_send_resource_queue;
ThreadQueue<int> MemShareGlobal::s_recv_resource_queue;
ThreadQueue<int> MemShareGlobal::s_memshare_handle_queue;
ThreadQueue<int> MemShareGlobal::s_memshare_share_queue;

void MemShareGlobal::ReclaimSend(int index) {
	MemShareGlobal::s_send_resource_queue.push_back(index);
}

void MemShareGlobal::ReclaimRecv(int index) {
	MemShareGlobal::s_recv_resource_queue.push_back(index);
}

}

// vim: ts=4 sw=4 nu

