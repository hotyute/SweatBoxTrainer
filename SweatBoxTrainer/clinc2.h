#ifndef CLINC2_H
#define CLINC2_H

#include <windows.h>

#include "Stream.h"
#include "aircraft.h"

#ifndef clinc2_tcpinterface_h
#define clinc2_tcpinterface_h
class tcpinterface;
class tcpinterface {
public:
	SOCKET sConnect;
	tcpinterface();
	static DWORD WINAPI staticStart(void*);
	DWORD run();
	void sendMessage(Stream*);
	void startT(HWND);
	void init_set();
	int connectNew(HWND, std::string, unsigned short);

	char message[5000];
	int packetSize;
	unsigned char packetType;
	Stream* out_stream;
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

#endif