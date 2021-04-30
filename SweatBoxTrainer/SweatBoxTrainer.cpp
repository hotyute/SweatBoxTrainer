// SweatBoxTrainer.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "SweatBoxTrainer.h"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <thread>

#include "usermanager.h"
#include "events.h"
#include "tools.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWnd = NULL;		// Holds Our Window Handle
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
bool done = false;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_SWEATBOXTRAINER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SWEATBOXTRAINER));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SWEATBOXTRAINER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SWEATBOXTRAINER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			HMENU hMenuBar = CreateMenu();
			HMENU hFile = CreateMenu();
			HMENU hSettings = CreateMenu();
			HMENU hHelp = CreateMenu();

			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"&File");
			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hSettings, L"&Settings");
			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelp, L"&Help");

			AppendMenu(hFile, MF_STRING, ID_FILE_CONNECT, L"&Connect to Sever...");

			SetMenu(hWnd, hMenuBar);

			CreateThread(NULL, 0, EventThread1, hWnd, 0, NULL);
			CreateThread(NULL, 0, SocketThread1, hWnd, 0, NULL);

			userStorage1.resize(MAX_AIRCRAFT_SIZE);

			Aircraft* cur = new Aircraft();
			cur->lock();
			cur->setHeavy(true);
			cur->setCallsign("AAL2");
			cur->setLatitude(25.800704);
			cur->setLongitude(-80.300770);
			cur->setSpeed(0.0);
			cur->setHeading(85.0);
			cur->setCollision(true);
			cur->setMode(1);
			cur->unlock();

			cur->setSquawkCode(std::to_string(random(1000, 9999)));

			FlightPlan& fp = *cur->getFlightPlan();
			fp.departure = "KMIA";
			fp.route = "HEDLY1.HEDLY LAL";
			fp.remarks = "/v/";
			++fp.cycle;

			cur->getConnection()->init_set();

			AcfMap[cur->getCallsign()] = cur;

			userStorage1[0] = cur;
		}
		break;
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case ID_FILE_CONNECT:
				{
					connect();
				}
				break;
				case IDM_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hWnd, &ps);
		}
		break;
		case WM_DESTROY:
		{
			done = true;
			PostQuitMessage(0);
		}
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

void connect()
{
}

void disconnect()
{
}

DWORD WINAPI EventThread1(LPVOID lpParameter) {
	boost::posix_time::ptime start;
	boost::posix_time::ptime end;
	while (!done) {
		start = boost::posix_time::microsec_clock::local_time();
		event_manager1->update();
		end = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration time2 = end - start;
		long long time1 = 30L;
		long long time = time1 - time2.total_milliseconds();
		if (time < 1) {
			time = 1;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(time));
	}
	return 0;
}

DWORD WINAPI SocketThread1(LPVOID lpParameter) {
	while (!done)
	{
		for (auto it = AcfMap.begin(); it != AcfMap.end(); ++it)
		{
			Aircraft& aircraft = *(it->second);
			if (aircraft.connected) {
				aircraft.getConnection()->run();
			}
		}
	}
	return 0;
}
