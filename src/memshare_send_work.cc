#include "memshare_send_work.h"

namespace MEMSHARE
{

EpollServerPtr MemShareSendWork::s_server;

void MemShareSendWork::SendLastBuf(char *buf, int index, int status) {

	//构造回复包
	int packet_len = ConstructResp(buf, index, status);

	//发送
	s_server->send(MemShareGlobal::s_recv_data[index].dest_id, buf, packet_len);

}

int MemShareSendWork::ConstructResp(char *buf, int index, int status) {

	int offset = 0;

	*(uint64_t *)buf = *(uint64_t *)MemShareGlobal::skMemShareRespKey;
	offset += 8;

	*(uint8_t *)(buf + offset) = MemShareGlobal::skMemShareRespVer;
	offset += 1;

	*(int *)(buf + offset) = status;
	offset += 4;

	return offset;
}

}

// vim: ts=4 sw=4 nu

