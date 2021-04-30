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
	tcpinterface();
	DWORD run();
	void sendMessage(Stream*);
	void init_set();
	int connectNew(std::string, unsigned short);

	char message[5000];
	int packetSize;
	unsigned char packetType;
	Stream* in_stream;
	bool hand_shake;
	int current_op;

	int nBytesReceived = 0;
	bool fBreak = false;
	TIMEVAL timeout1;
	int TimeoutSec1 = 30; //
	fd_set rfds;
	int retval;
	bool closed = false;
};

#endif