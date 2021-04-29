#ifndef SWEATBOXTRAINER_H
#define SWEATBOXTRAINER_H

#include "resource.h"

static const int CLIENT_PORT = 6809;
extern bool done;
extern HWND hWnd;		// Holds Our Window Handle

void connect();
void disconnect();
DWORD WINAPI EventThread1(LPVOID lpParameter);

DWORD __stdcall SocketThread1(LPVOID lpParameter);

#endif
