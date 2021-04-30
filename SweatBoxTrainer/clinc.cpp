#include <vector>

#include "clinc.h"
#include "tools.h"
#include "config.h"

tcpinterface1::tcpinterface1() {

}

DWORD WINAPI tcpinterface1::staticStart(void* param) {
	tcpinterface1* intter = (tcpinterface1*) param;
	return intter->run();
}
int nBytesReceived1 = 0;
bool fBreak1 = false;

DWORD tcpinterface1::run() {
	char message[1024];
	std::string strmessage;
	while (!quit) {
		strmessage = "";
		fBreak1 = false;
		while (!fBreak1) {
			ZeroMemory(message, sizeof(message));
			nBytesReceived1 = recv(tcpinterface1::sConnect, message, sizeof(message), NULL);
			if (nBytesReceived1 == SOCKET_ERROR)
				break;
			strmessage += message;
			int length = strmessage.length();
			if (length >= 2) 
			{
				if(strmessage[length - 2] == '\r' && strmessage[length - 1] == '\n') 
				{
					std::cout << strmessage.c_str() << std::endl;
					fBreak1 = true;
					if (strmessage.length() != 0) 
					{
						if (strmessage.substr(strmessage.length() - 1) == "\0")
						{
							strmessage = strmessage.substr(0, strmessage.length() - 1);
						}
						std::vector<std::string> array2 = split(strmessage, "\r\n");
						for (size_t i = 0; i < array2.size(); i++)
						{
							std::string text = array2[i];
							if (text.length() != 0)
							{
								std::vector<std::string> array3 = split(strmessage, ":");
								char c = array3[0][0];
								char c2 = c;
								switch (c2)
								{
								case '#':
								case '$':
									{
										if (array3[0].length() < 3)
										{
											//printf("%s%s", "Invalid PDU type.", text);
											throw;
										}
										break;
									}
								default:
									{
										if (c2 != '@')
										{
											//goto IL_1058;
											break;
										}
										array3[0] = array3[0].substr(1);
										/*if (this.Field_5 != null)
										{
										this.Field_5(this, new Class_177_EventArgs<Class_233_Class_176_Object>(Class_233_Class_176_Object.Function_Class_233_Class_176_Object_26(array3), this.Field_53));
										}*/
										char header = array3[0][0];
										if (header == 'S' || header == 'N' || header == 'Y') {
											
										}
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

DWORD tcpinterface1::sendMessage(std::string message) {
	message += "\r\n";
	return send(tcpinterface1::sConnect, message.c_str(), message.length(), NULL);  
}

void tcpinterface1::startT(HWND hWnd) {
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) staticStart, (void*) this, 0, NULL);
}

int tcpinterface1::connectNew(HWND hWnd, std::string saddr, unsigned short port) {
	long answer;
	WSADATA wsaData;
	WORD DLLVersion;
	DLLVersion = MAKEWORD(2, 1);
	answer = WSAStartup(DLLVersion, &wsaData);

	SOCKADDR_IN addr;

	int addrlen = sizeof(addr);

	sConnect = socket(AF_INET, SOCK_STREAM, NULL);

	addr.sin_addr.s_addr = inet_addr(saddr.c_str());

	addr.sin_port = htons(port);

	addr.sin_family = AF_INET;

	if (connect(tcpinterface1::sConnect, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cout << "Failed to connect!" << std::endl;
		MessageBox(hWnd, L"Failed to connect to Server!", L"Notice",
							MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	return 1;
}