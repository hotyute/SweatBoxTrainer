#include "clinc2.h"

#include <windows.h>
#include <winsock.h>
#include <iostream>
#include <tchar.h>

#include "usermanager.h"
#include "packets_out.h"
#include "SweatBoxTrainer.h"
#include "packets_in.h"
#include "globals.h"

#pragma comment(lib, "ws2_32.lib")


/* Use Official Packet Output App to update this. */
constexpr int packet_sizes[256] =
{
-3, -3, -3, -3, -3, -3, -3, -1, -1, -2, 8, -2, 2, 0, 36,
-2, 3, -2, 19, 4, -1, 7, -2, 4, -3, -3, -3, -3, -3, -3,
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

tcp_manager::tcp_manager(Aircraft* aircraft) {
	this->in_stream = new BasicStream();
	this->aircraft = aircraft;
	timeout1.tv_sec = TimeoutSec1;
	timeout1.tv_usec = 0;
	sConnect = INVALID_SOCKET;
}

DWORD tcp_manager::poll_socket() {

	FD_ZERO(&rfds);
	FD_SET(sConnect, &rfds);

	//int sel = select(tcp_manager::sConnect + 1, &rfds, 0, 0, &timeout1);

	if (FD_ISSET(sConnect, &rfds))
	{
		nBytesReceived = in_stream->add_data(sConnect);

		if (nBytesReceived == SOCKET_ERROR)
		{
			int error = WSAGetLastError();

			//because we are non blocking, this error tells us when bytes are fragmented
			if (error == WSAEWOULDBLOCK)
				return 0;

			if (error == WSAECONNABORTED || error == WSAENOTSOCK
				|| error == WSAECONNRESET || error == WSAETIMEDOUT)
			{
				aircraft->disconnect(false);
				in_stream->clear();
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

		BasicStream& in = *in_stream;

		if (hand_shake)
		{
			if (current_op == 45)
			{
				in.mark_position();
				if (nBytesReceived >= 1)
				{
					const int loginCode = in.read_unsigned_byte();
					if (loginCode == 1)
					{
						if (in.available() >= 10)
						{
							const int index = in.read_unsigned_short();
							const long long updateTimeInMillis = in.readQWord();

							printf("update_time: %lld\n", updateTimeInMillis);
							aircraft->setUserIndex(index);
							userStorage1[index] = aircraft;
							aircraft->setUpdateTime(updateTimeInMillis);

							if (g_threadPool) { // Safety check
								aircraft->startPositionUpdates(*g_threadPool);
							}

							hand_shake = false;
							current_op = -1;
							in.delete_marked_block();

							send_initial_packets(*aircraft);
						}
						else
						{
							in.reset();
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
							in.delete_marked_block();
							break;
						}
						default:
						{
							in.delete_marked_block();
							break;
						}
						}
					}
				}
				else
				{
					in.reset();
				}
			}
		}

		if (!hand_shake)
		{
			if (in.available() > 0)
			{
				decodePackets(aircraft, in, nBytesReceived);
			}
		}

		FD_ZERO(&rfds);
		FD_SET(tcp_manager::sConnect, &rfds);

		//retval = select(tcp_manager::sConnect + 1, &rfds, 0, 0, &timeout1);
	}
	if (retval == SOCKET_ERROR)
	{
		//do somethin
	}
	return 0;
}

void decodePackets(Aircraft* aircraft, BasicStream& in, int nBytesReceived) {
	while (in.available() > 0)
	{
		in.mark_position();
		const int opCode = in.read_unsigned_byte();
		int length = -3;
		if (opCode != -1)
		{
			length = packet_sizes[opCode];
			if (length == -1)
			{
				if (in.available() >= 1)
				{
					length = in.read_unsigned_byte();
				}
				else
				{
					in.reset();
					break;
				}
			}
			else if (length == -2)
			{
				if (in.available() >= 2)
				{
					length = in.read_unsigned_short();
				}
				else
				{
					in.reset();
					break;
				}
			}
			else if (length == -3)
			{
#ifdef _DEBUG
				printf("%s [UNHANDLED] Packet_Id!! : %d, Packet_Size: %d, Bytes_Ava: %d nBytes: %d\n",
					aircraft->getIdentity()->callsign.c_str(), opCode, length, (int)in.available(), nBytesReceived);
#endif
				length = static_cast<int>(in.available());
			}
#ifdef _DEBUG
			//printf("%s Packet_Id: %d, Packet_Size: %d, Bytes_Ava: %d nBytes: %d\n", 
			//	aircraft->getIdentity()->callsign.c_str(), opCode, length, in.available(), nBytesReceived);
#endif
			if (in.available() >= length)
			{
				//handle
				processIncomingPackets(aircraft, opCode, in);
				in.delete_marked_block();
			}
			else
			{
				in.reset();
				break;
			}
		}
	}
}

void tcp_manager::sendMessage(BasicStream* stream) {
	std::lock_guard<std::mutex> lock(tcp_manager::writeMutex);
	if (this->aircraft && !this->aircraft->connected)
		return;

	if (stream->get_index() == 0) {
		MessageBox(hWnd, L"Can't flush empty stream o.O", L"Notice", MB_OK | MB_ICONINFORMATION);
		return;
	}
	send_data(tcp_manager::sConnect, std::vector<char>(stream->data, stream->data + stream->index));
	stream->clear();
}

void tcp_manager::send_data(SOCKET clientSocket, const std::vector<char>& buffer) {
	size_t totalSent = 0;
	size_t remaining = buffer.size();

	while (totalSent < buffer.size()) {
		const int sent = send(clientSocket, buffer.data() + totalSent, static_cast<int>(remaining), 0);
		if (sent == SOCKET_ERROR) {
			std::cerr << "Error sending broadcast message: " << WSAGetLastError() << std::endl;
			break;
		}

		//printf("sent: %d\n", sent);

		totalSent += sent;
		remaining -= sent;
	}
}

void tcp_manager::init_set()
{

}

int tcp_manager::disconnect_socket() {
	int iResult = shutdown(tcp_manager::sConnect, 0x01);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(tcp_manager::sConnect);
		WSACleanup();
		return 1;
	}
	return 0;
}

int tcp_manager::connectNew(std::string saddr, unsigned short port) {
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
				char buff[256];
				sprintf_s(buff, "Connect Timeout (%d Sec).", TimeoutSec);
				MessageBox(hWnd, (LPCWSTR)buff, L"Notice", MB_OK | MB_ICONINFORMATION);
				//system("pause");
				return 0;
			}
			else
			{
				if (FD_ISSET(sConnect, &Write))
				{
#ifdef _DEBUG
					std::cout << "Connected!\n";
#endif
				}
				if (FD_ISSET(sConnect, &Err))
				{
					MessageBox(hWnd, L"Select Error.", L"Notice", MB_OK | MB_ICONINFORMATION);
					//system("pause");
					return 0;
				}
			}
		}
		else
		{
#ifdef _DEBUG
			std::cout << "Failed to connect!" << std::endl;
#endif
			MessageBox(hWnd, L"Failed to connect to Server!", L"Notice",
				MB_OK | MB_ICONINFORMATION);
			WSACleanup();
			return 0;
		}
	}
	closed = false;
	SetSocketBlocking(sConnect, false);
#ifdef _DEBUG
	std::cout << "Connected!\n";
#endif
	return 1;
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

void SocketPollingTask::execute() {
	// This replaces the SocketThread1 loop body.
	// We need to iterate over all connected aircraft and poll their sockets.

	// PROTECT THIS LOOP WITH A MUTEX!
	std::lock_guard<std::mutex> lock(g_acfMapMutex); // You'll need to create a global mutex for AcfMap
	for (auto const& [key, val] : AcfMap) {
		Aircraft& aircraft = *val;
		// The tcp_manager::poll_socket() logic needs to be refactored slightly.
		// It should perform a single, non-blocking poll, not an infinite loop.
		// Your current implementation already uses select with a timeout, which is good.
		if (aircraft.connected && !aircraft.getConnection().closed) {
			aircraft.getConnection().poll_socket(); // Create a new method for one-shot polling
		}
	}
}
