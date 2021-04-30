#include "clinc2.h"

#include <windows.h>
#include <winsock.h>
#include <iostream>
#include <tchar.h>

#include "events.h"
#include "config.h"
#include "usermanager.h"

static int packetSizes[256][2] = {
	{10, 8},
	{13, 0},
	{9, -1},
	{14, 36},
	{12, 2},
	{16, 3},
	{11, -2},
	{17, -2}
};

tcpinterface::tcpinterface() {
	this->in_stream = new Stream(5000);
	memset(tcpinterface::message, 0, 5000);
	timeout1.tv_sec = TimeoutSec1;
	timeout1.tv_usec = 0;
	sConnect = INVALID_SOCKET;
}

DWORD tcpinterface::run() {
	ZeroMemory(message, sizeof(message));
	fBreak = false;
	tcpinterface::packetType = -1;
	tcpinterface::packetSize = -3;
	int size1 = 0;

	FD_ZERO(&rfds);
	FD_SET(tcpinterface::sConnect, &rfds);

	//retval = select(tcpinterface::sConnect + 1, &rfds, 0, 0, &timeout1); // we comment this because we only use it for waiting for data

	while (!fBreak) {
		if (FD_ISSET(tcpinterface::sConnect, &rfds))
		{
			nBytesReceived = recv(tcpinterface::sConnect, message, 5000, 0);
			if (nBytesReceived < 0)
			{
				closed = true;
				printf("Connection was closed by remote person or timeout exceeded 60 seconds\n");
				break;
			}

			if (nBytesReceived == SOCKET_ERROR)
				break;

			if (nBytesReceived == 0)
				continue;

			if (tcpinterface::hand_shake)
			{
				if (tcpinterface::current_op == 45)
				{
					if (nBytesReceived >= 11)
					{
						Stream& in = Stream(11);
						in.currentOffset = 0;
						memcpy(in.buffer, message, 11);
						int loginCode = in.readUnsignedByte();
						int index = in.readUnsignedWord();
						long long updateTimeInMillis = in.readQWord();
						if (loginCode == 1)
						{
							//setIndex
							//setUpdateTimeinMillis
							//sendUpdates
							aircraft->setUserIndex(index);
							aircraft->setUpdateTime(updateTimeInMillis);
							Event& position_updates = PositionUpdates();
							position_updates.eAction.setTicks(0);
							event_manager1->addEvent(&position_updates);
							tcpinterface::hand_shake = false;
							fBreak = true;
						}
					}
				}
			}
			else
			{
				int available = nBytesReceived;
				int offset = 0;
				packetType = (unsigned char)(message[offset++]);
				available--;
				if (packetType != -1)
				{
					for (int j = 0; j < 256; j++)
					{
						if (packetSizes[j][0] == packetType)
						{
							packetSize = packetSizes[j][1];
							break;
						}
					}
					if (packetSize == -1)
					{
						if (available >= 1)
						{
							packetSize = (unsigned char)message[offset++];
							available--;
						}
					}
					else if (packetSize == -2)
					{
						if (available >= 2)
						{

							int firstByte = (((unsigned char)message[offset++]) << 8);
							packetSize = firstByte + (unsigned char)message[offset++];
							available -= 2;
						}
					}
					else if (packetSize == -3)
					{
						//packetSize = available; //Uncomment to auto buffer
					}
				#ifdef _DEBUG
					std::cout << "Packet_Id: " << (int)packetType << ", Packet_Size: " << packetSize << ", Bytes_Ava: " << available << std::endl;
				#endif
					if (available >= packetSize)
					{
						Stream& stream_in = Stream(tcpinterface::packetSize);
						stream_in.currentOffset = 0;
						memcpy(stream_in.buffer, message + offset, packetSize);
						//handle
						decodePackets(aircraft, tcpinterface::packetType, stream_in);
						fBreak = true;
					}
				}
			}
		}
		FD_ZERO(&rfds);
		FD_SET(tcpinterface::sConnect, &rfds);

		//retval = select(tcpinterface::sConnect + 1, &rfds, 0, 0, &timeout1);
	}
	if (retval == SOCKET_ERROR)
	{
		//do somethin
	}
	return 0;
}

void tcpinterface::sendMessage(Stream* stream) {
	if (stream->currentOffset == 0) {
		printf("Can't flush empty stream o.O\n");
		return;
	}

	DWORD what = send(tcpinterface::sConnect, stream->buffer, stream->currentOffset, NULL);
	stream->currentOffset = 0;
}

void tcpinterface::init_set()
{
	
}

int tcpinterface::connectNew(std::string saddr, unsigned short port) {
	long answer;
	WSADATA wsaData;
	WORD DLLVersion;
	DLLVersion = MAKEWORD(2, 1);
	answer = WSAStartup(DLLVersion, &wsaData);
	u_long iMode = 1;//0 for blocking, 1 for non-blocking

	SOCKADDR_IN addr;

	int addrlen = sizeof(addr);

	sConnect = socket(AF_INET, SOCK_STREAM, NULL);

	if (sConnect == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	addr.sin_addr.s_addr = inet_addr(saddr.c_str());

	addr.sin_port = htons(port);

	addr.sin_family = AF_INET;

	int iResult = connect(sConnect, (SOCKADDR*)&addr, sizeof(addr));

	if (iResult == SOCKET_ERROR) {
		int iError = WSAGetLastError();
		std::cout << iError << std::endl;
		if (iError == WSAEWOULDBLOCK)
		{
		#ifdef _DEBUG
			std::cout << "Attempting to connect.\n";
		#endif
			fd_set Write, Err;
			TIMEVAL Timeout;
			int TimeoutSec = 5; // timeout after 5 seconds

			FD_ZERO(&Write);
			FD_ZERO(&Err);
			FD_SET(sConnect, &Write);
			FD_SET(sConnect, &Err);

			Timeout.tv_sec = TimeoutSec;
			Timeout.tv_usec = 0;

			iResult = select(0,         //ignored
				NULL,      //read
				&Write,    //Write Check
				&Err,      //Error Check
				&Timeout);
			if (iResult == 0)
			{
				std::cout << "Connect Timeout (" << TimeoutSec << " Sec).\n";
				system("pause");
				return 1;
			}
			else
			{
				if (FD_ISSET(sConnect, &Write))
				{
					std::cout << "Connected!\n";
				}
				if (FD_ISSET(sConnect, &Err))
				{
					std::cout << "Select error.\n";
					//system("pause");
					return 0;
				}
			}
		}
		else
		{
			std::cout << "Failed to connect!" << std::endl;
			MessageBox(hWnd, L"Failed to connect to Server!", L"Notice",
				MB_OK | MB_ICONINFORMATION);
			WSACleanup();
			return 0;
		}
	}
	int r = ioctlsocket(sConnect, FIONBIO, &iMode);
	if (r != NO_ERROR)
		printf("ioctlsocket failed with error: %ld\n", r);
	std::cout << "Connected!\n";
	return 1;
}