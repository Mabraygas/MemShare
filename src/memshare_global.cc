#include "memshare_global.h"

namespace MEMSHARE
{

const char* MemShareGlobal::skMemShareReqKey = "MSREQKEY";
const char* MemShareGlobal::skMemShareRespKey= "MSRESKEY";

void MemShareGlobal::ReclaimSend(int index) {
	s_send_resource_queue.push_back(index);
}

void MemShareGlobal::ReclaimRecv(int index) {
	s_recv_resource_queue.push_back(index);
}

}

// vim: ts=4 sw=4 nu

