// SweatBoxTrainer.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "SweatBoxTrainer.h"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <thread>

#include "usermanager.h"
#include "events.h"
#include "tools.h"
#include "guicon.h"
#include "calc_cycles.h"

#define MAX_LOADSTRING 100

#define PROTO_VERSION 32698

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWnd = NULL;		// Holds Our Window Handle
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
bool done = false;

HFONT hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
	CLIP_DEFAULT_PRECIS, FALSE, VARIABLE_PITCH, TEXT("Segoe UI"));

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

#ifdef _DEBUG
	RedirectIOToConsole();
#endif

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

			HWND callsign_text = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", L"",
				WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
				150, 400, 700, 30,
				hWnd, (HMENU)COMMAND_TEXT, NULL, NULL
			);

			SendMessage(callsign_text, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
			SendMessage(callsign_text, EM_LIMITTEXT, 0, 0L);

			CreateThread(NULL, 0, EventThread1, hWnd, 0, NULL);
			CreateThread(NULL, 0, SocketThread1, hWnd, 0, NULL);
			CreateThread(NULL, 0, CalcThread1, hWnd, 0, NULL);

			userStorage1.resize(MAX_AIRCRAFT_SIZE);

			Aircraft* cur = new Aircraft();
			cur->lock();
			cur->getIdentity()->type = AV_CLIENT::PILOT;
			cur->setHeavy(false);
			cur->getIdentity()->callsign = "DAL220";
			cur->getIdentity()->login_name = "Samuel Mason";
			cur->getIdentity()->password = "password";
			cur->getIdentity()->pilot_rating = 1;
			cur->setLatitude(25.798429);
			cur->setLongitude(-80.278852);
			cur->setSpeed(5.0);
			cur->setHeading(120.0);
			cur->setCollision(false);
			cur->setMode(1);
			cur->unlock();

			cur->setSquawkCode(std::to_string(random(1000, 9999)));

			FlightPlan& fp = *cur->getFlightPlan();
			fp.departure = "KMIA";
			fp.route = "HEDLY1.HEDLY LAL";
			fp.remarks = "/v/";
			++fp.cycle;

			cur->getConnection()->init_set();

			AcfMap[cur->getIdentity()->callsign] = cur;

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
	for (auto it = AcfMap.begin(); it != AcfMap.end(); ++it)
	{
		Aircraft& aircraft = *(it->second);
		if (!aircraft.connected) {
			if (aircraft.getConnection()->connectNew("127.0.0.1", 4403))
			{
				aircraft.connected = true;
				Identity& id = *aircraft.getIdentity();
				Stream stream = Stream(200);
				int type = aircraft.getIdentity()->type;
				tcpinterface& intter = *aircraft.getConnection();
				intter.hand_shake = true;
				intter.current_op = 45;
				stream.createFrameVarSizeWord(45);
				stream.writeDWord(PROTO_VERSION);
				stream.writeString((char*)id.callsign.c_str());
				stream.writeString((char*)id.login_name.c_str());
				stream.writeString((char*)id.username.c_str());
				stream.writeString((char*)id.password.c_str());
				stream.writeByte(id.controller_rating);
				stream.writeByte(id.pilot_rating);
				stream.writeQWord(1000);//request time
				stream.writeQWord(doubleToRawBits(aircraft.getLatitude()));
				stream.writeQWord(doubleToRawBits(aircraft.getLongitude()));
				stream.writeWord(aircraft.getVisibility());
				stream.writeByte(type);
				if (type == AV_CLIENT::PILOT) {
					stream.writeString((char*)aircraft.getAcfTitle().c_str());
					stream.writeString((char*)aircraft.getSquawkCode().c_str());
					stream.writeByte(aircraft.getMode());
					long long infoHash = ((static_cast<long long>((int)((aircraft.getPitch() * 1024.0) / -360.0))) << 22)
						+ ((static_cast<long long>((int)((aircraft.getRoll() * 1024.0) / -360.0))) << 12)
						+ ((static_cast<long long>((int)((aircraft.getHeading() * 1024.0) / 360.0))) << 2);
					stream.writeQWord(infoHash);
				}
				stream.endFrameVarSizeWord();
				intter.sendMessage(&stream);
			}
		}
	}
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
	boost::posix_time::ptime start;
	boost::posix_time::ptime end;
	while (!done)
	{
		start = boost::posix_time::microsec_clock::local_time();

		for (auto it = AcfMap.begin(); it != AcfMap.end(); ++it)//TODO possible make this thread awake when there is any data in any aircraft socket?
		{
			Aircraft& aircraft = *(it->second);
			if (aircraft.connected) {
				aircraft.getConnection()->run();
			}
		}

		end = boost::posix_time::microsec_clock::local_time();

		boost::posix_time::time_duration time2 = end - start;
		long long time1 = 3L;
		long long time = time1 - time2.total_milliseconds();
		if (time < 1) {
			time = 1;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(time));
	}
	return 0;
}

DWORD WINAPI CalcThread1(LPVOID)
{
	boost::posix_time::ptime start;
	boost::posix_time::ptime end;
	boost::posix_time::time_duration time;

	while (true)
	{
		start = boost::posix_time::microsec_clock::local_time();

		//code here
		update();
		CalculateMovements();

		end = boost::posix_time::microsec_clock::local_time();

		time = (end - start);
		long long time1 = 30L;
		long long time2 = time1 - time.total_milliseconds();
		if (time2 < 1) {
			time2 = 1;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(time2));

	}
	return 0;
}
