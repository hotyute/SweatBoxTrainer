#ifndef CLINC2_H
#define CLINC2_H

#include <winsock.h>
#include <string>

class Aircraft;

#include "SweatBoxTrainer.h"
#include "Stream.h"

class tcpinterface {
public:
	Aircraft* aircraft;
	SOCKET sConnect;
	tcpinterface(Aircraft* aircraft);
	DWORD run();
	void sendMessage(Stream*);
	void init_set();
	int connectNew(std::string, unsigned short);

	char message[5000];
	Stream* in_stream;
	bool hand_shake;
	int current_op = -1;

	int nBytesReceived = 0;
	bool fBreak = false;
	TIMEVAL timeout1;
	int TimeoutSec1 = 30; //
	fd_set rfds;
	int retval;
	bool closed = false;
};

#endif

void decodePackets(Aircraft* aircraft, Stream& in);

bool SetSocketBlocking(SOCKET sock, bool blocking);
