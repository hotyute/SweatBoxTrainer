#pragma once

#define _WINSOCK2API_
#include <mutex>
#include <string>
#include <vector>
#include <windows.h>
#include <WinSock2.h>

#include "basic_stream.h"
class Aircraft;

class tcp_manager {
public:
	std::mutex writeMutex;
	Aircraft* aircraft;
	SOCKET sConnect;
	tcp_manager(Aircraft* aircraft);
	DWORD run();
	void sendMessage(BasicStream* stream);
	static void send_data(SOCKET clientSocket, const std::vector<char>& buffer);
	void init_set();
	int disconnect_socket();
	int connectNew(std::string, unsigned short);

	BasicStream* in_stream;
	bool hand_shake{};
	int current_op = -1;

	int nBytesReceived = 0;
	bool fBreak = false;
	TIMEVAL timeout1{};
	int TimeoutSec1 = 30; //
	fd_set rfds{};
	int retval{};
	bool closed = true;
};

void decodePackets(Aircraft* aircraft, BasicStream& in, int nBytesReceived);

bool SetSocketBlocking(SOCKET sock, bool blocking);

bool send_initial_packets(Aircraft& aircraft);
