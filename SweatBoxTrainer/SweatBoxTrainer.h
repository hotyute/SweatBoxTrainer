#pragma once

#include <Windows.h>
#include <string>

static const int CLIENT_PORT = 6809;
extern bool done;
extern HWND hWnd;		// Holds Our Window Handle
extern HWND console_text;

void connect();
DWORD WINAPI EventThread1(LPVOID lpParameter);

DWORD __stdcall SocketThread1(LPVOID lpParameter);

DWORD WINAPI CalcThread1(LPVOID);

void create_controls(HWND hWnd);
