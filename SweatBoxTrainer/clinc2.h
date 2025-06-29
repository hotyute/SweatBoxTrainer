#pragma once

#define _WINSOCK2API_
#include <mutex>
#include <string>
#include <vector>
#include <windows.h>
#include <WinSock2.h>
#include <memory> // For std::unique_ptr

#include "basic_stream.h"
#include "tools/timed_task.h"
class Aircraft;

class tcp_manager {
public:
	std::mutex writeMutex;
	Aircraft* aircraft; // Non-owning pointer back to parent
	SOCKET sConnect;
	tcp_manager(Aircraft* aircraft);
	DWORD poll_socket();
	void sendMessage(BasicStream* stream);
	static void send_data(SOCKET clientSocket, const std::vector<char>& buffer);
	void init_set();
	int disconnect_socket();
	int connectNew(std::string, unsigned short);

	std::unique_ptr<BasicStream> in_stream;
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

class SocketPollingTask : public TimedTask {
public:
	// Run very frequently to be responsive
	SocketPollingTask(ThreadPool& pool)
		: TimedTask(pool, std::chrono::milliseconds(5), true) {}

protected:
	void execute() override;
};

void decodePackets(Aircraft* aircraft, BasicStream& in, int nBytesReceived);

bool SetSocketBlocking(SOCKET sock, bool blocking);

bool send_initial_packets(Aircraft& aircraft);