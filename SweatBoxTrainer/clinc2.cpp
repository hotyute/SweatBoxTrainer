#include "clinc2.h"

#include <windows.h>
#include <winsock.h>
#include <iostream>
#include <tchar.h>
#include <errno.h>

#include "events.h"
#include "config.h"
#include "usermanager.h"
#include "packets_out.h"

/* Use Official Packet Output App to update this. */
const int packetSizes[256] =
{
-3, -3, -3, -3, -3, -3, -3, -3, -3, -2, 8, -2, 2, 0, 36,
-2, 3, -2, 19, 4, -1, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3,
};

tcpinterface::tcpinterface(Aircraft* aircraft) {
	this->in_stream = new Stream(5000);
	this->in_stream->clearBuf();
	this->aircraft = aircraft;
	memset(tcpinterface::message, 0, 5000);
	timeout1.tv_sec = TimeoutSec1;
	timeout1.tv_usec = 0;
	sConnect = INVALID_SOCKET;
}

DWORD tcpinterface::run() {
	ZeroMemory(message, sizeof(message));

	FD_ZERO(&rfds);
	FD_SET(sConnect, &rfds);

	//int sel = select(tcpinterface::sConnect + 1, &rfds, 0, 0, &timeout1);

	if (FD_ISSET(sConnect, &rfds))
	{
		nBytesReceived = recv(sConnect, message, 5000, 0);

		if (nBytesReceived == SOCKET_ERROR)
		{
			int error = WSAGetLastError();

			//because we are non blocking, this error tells us when bytes are fragmented
			if (error == WSAEWOULDBLOCK)
				return 0;

			if (error == WSAECONNABORTED || error == WSAENOTSOCK
				|| error == WSAECONNRESET || error == WSAETIMEDOUT)
			{
				disconnect(aircraft, false);
				in_stream->clearBuf();
				memset(tcpinterface::message, 0, 5000);
				closed = true;
				if (error == WSAETIMEDOUT)
					printf("Connection timeout exceeded 11 seconds\n");
				else
					printf("Connection was closed by remote person or timeout exceeded 60 seconds\n");
			}
			else
			{
				printf("Unhandled_Error: %d\n", error);
			}
			return 0;
		}

		if (nBytesReceived == 0)
			return 0;

		Stream& in = *in_stream;
		memcpy(in.buffer + in.length, message, nBytesReceived);
		in.length += nBytesReceived;

		if (tcpinterface::hand_shake)
		{
			if (tcpinterface::current_op == 45)
			{
				in.markReaderIndex();
				if (nBytesReceived >= 1)
				{
					int loginCode = in.readUnsignedByte();
					if (loginCode == 1)
					{
						if (in.remaining() >= 10)
						{
							int index = in.readUnsignedWord();
							long long updateTimeInMillis = in.readQWord();
							aircraft->setUserIndex(index);
							userStorage1[index] = aircraft;
							aircraft->setUpdateTime(updateTimeInMillis);
							if (aircraft->position_updates->eAction.paused)
							{
								aircraft->position_updates->eAction.setTicks(0);
								aircraft->position_updates->toggle_pause();
							}
							else
							{
								aircraft->position_updates->eAction.setTicks(0);
								event_manager1->addEvent(aircraft->position_updates);
							}
							hand_shake = false;
							current_op = -1;
							in.deleteReaderBlock();

							send_initial_packets(*aircraft);
						}
						else
						{
							in.resetReaderIndex();
						}
					}
					else
					{
						switch (loginCode)
						{
						case 2:
						{
							//sendErrorMessage("Invalid protocol Version.");
							//if (connected)
							//	disconnect(true);
							in.deleteReaderBlock();
							break;
						}
						default:
						{
							in.deleteReaderBlock();
							break;
						}
						}
					}
				}
				else
				{
					in.resetReaderIndex();
				}
			}
		}

		if (!hand_shake)
		{
			if (in.remaining() > 0)
			{
				decodePackets(aircraft, in, nBytesReceived);
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

void decodePackets(Aircraft* aircraft, Stream& in, int nBytesRecieved) {
	while (in.remaining() > 0)
	{
		in.markReaderIndex();
		int opCode = in.readUnsignedByte(), length = -3;
		if (opCode != -1)
		{
			length = packetSizes[opCode];
			if (length == -1)
			{
				if (in.remaining() >= 1)
				{
					length = in.readUnsignedByte();
				}
				else
				{
					in.resetReaderIndex();
					break;
				}
			}
			else if (length == -2)
			{
				if (in.remaining() >= 2)
				{
					length = in.readUnsignedWord();
				}
				else
				{
					in.resetReaderIndex();
					break;
				}
			}
			else if (length == -3)
			{
#ifdef _DEBUG
				printf("%s [UNHANDLED] Packet_Id!! : %d, Packet_Size: %d, Bytes_Ava: %d nBytes: %d\n",
					aircraft->getIdentity()->callsign.c_str(), opCode, length, in.remaining(), nBytesRecieved);
#endif
				length = in.remaining();
			}
#ifdef _DEBUG
			//printf("%s Packet_Id: %d, Packet_Size: %d, Bytes_Ava: %d nBytes: %d\n", 
			//	aircraft->getIdentity()->callsign.c_str(), opCode, length, in.remaining(), nBytesRecieved);
#endif
			if (in.remaining() >= length)
			{
				//handle
				processIncomingPackets(aircraft, opCode, in);
				in.deleteReaderBlock();
			}
			else
			{
				in.resetReaderIndex();
				break;
			}
		}
	}
}

void tcpinterface::sendMessage(Stream* stream) {
	if (!this->aircraft->connected)
		return;

	if (stream->currentOffset == 0) {
		printf("Can't flush empty stream o.O\n");
		return;
	}

	w_lock();
	DWORD response = send(tcpinterface::sConnect, stream->buffer, stream->currentOffset, NULL);
	stream->currentOffset = 0;
	w_unlock();
}

void tcpinterface::init_set()
{

}

int tcpinterface::disconnect_socket() {
	int iResult = shutdown(tcpinterface::sConnect, 0x01);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(tcpinterface::sConnect);
		WSACleanup();
		return 1;
	}
	return 0;
}

int tcpinterface::connectNew(std::string saddr, unsigned short port) {
	int err;
	WSADATA wsaData;
	WORD DLLVersion;
	DLLVersion = MAKEWORD(2, 1);
	err = WSAStartup(DLLVersion, &wsaData);

	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return 0;
	}

	SOCKADDR_IN addr;

	int addrlen = sizeof(addr);

	sConnect = socket(AF_INET, SOCK_STREAM, NULL);

	if (sConnect == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	struct hostent* to;
	if ((to = gethostbyname(saddr.c_str())) == NULL)
	{
		fprintf(stderr, "gethostbyname() error...\n");
		MessageBox(hWnd, L"Host Name Error!", L"Notice",
			MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	//addr.sin_addr.s_addr = inet_addr(saddr.c_str());
	memcpy(&addr.sin_addr, to->h_addr_list[0], to->h_length);

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
	closed = false;
	SetSocketBlocking(sConnect, false);
	std::cout << "Connected!\n";
	return 1;
}

void tcpinterface::w_lock()
{
	WaitForSingleObject(writeMutex, INFINITE);
}

void tcpinterface::w_unlock()
{
	ReleaseMutex(writeMutex);
}

bool SetSocketBlocking(SOCKET sock, bool blocking)
{
	unsigned long nonblocking_long = blocking ? 0 : 1;
	if (ioctlsocket(sock, FIONBIO, &nonblocking_long) == SOCKET_ERROR)
		return false;
	return true;
}

bool send_initial_packets(Aircraft& aircraft)
{
	sendFlightPlan(aircraft);
	return true;
}
