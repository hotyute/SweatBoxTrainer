// SweatBoxTrainer.cpp : Defines the entry point for the application.
//

#define NOMINMAX

#include "SweatBoxTrainer.h"
#include "framework.h"
#include "packets_out.h"

#include <commdlg.h>
#include <filesystem>
#include <cstdio>
#include <thread>
#include <locale>
#include <codecvt>
#include <algorithm>

#include "basic_stream.h"
#include "calc_cycles.h"
#include "filereader.h"
#include "tools.h"
#include "usermanager.h"
#include "resource.h"
#include "save.h"
#include "guidialogue.h"
#include "tools/thread_pool.h"
#include "packets_in.h"
#include "aircraft/Aircraft.h"
#include "aircraft/command_handler.h"
#include "globals.h"
#include "sim/simulation_context.h"

#ifdef _DEBUG
#include "guicon.h"
#endif

#define MAX_LOADSTRING 100

#define PROTO_VERSION 32698

// --- NEW: Custom Windows message to tell the FPL window to update ---
#define WM_APP_FPL_UPDATE (WM_USER + 1)

void HandleSelectedLB(DWORD iSelected);

bool always_on_top = false;

DWORD MainThreadId_G;

WNDPROC lpCommandWndProc;

HMENU hSettings = NULL;

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWnd = NULL;		// Holds Our Window Handle
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
bool done = false;
HWND aircraftList = NULL;
DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX /* | WS_MAXIMIZEBOX */ | WS_EX_TOPMOST;


HWND altitude = NULL, heading = NULL, latitude_hdl = NULL, longitude_hdl, speed_hdl = NULL, track_hdl = NULL, data_hdl = NULL,
vs_hdl = NULL, command_text = NULL, console_text = NULL, mode_button = NULL;

HWND hFplWnd = NULL;

Aircraft* displayed = nullptr;

HFONT hFont = CreateFont(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
	CLIP_DEFAULT_PRECIS, FALSE, VARIABLE_PITCH, TEXT("Segoe UI"));
HFONT hFont2 = CreateFont(15, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
	CLIP_DEFAULT_PRECIS, FALSE, VARIABLE_PITCH, TEXT("Segoe UI"));
const COLORREF bk_ref = RGB(192, 192, 192);
const COLORREF vbk_ref = RGB(255, 255, 255);
const COLORREF text_ref = RGB(0, 0, 0);
const HBRUSH background_color = CreateSolidBrush(bk_ref);
const HBRUSH variablebk_color = CreateSolidBrush(vbk_ref);


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	HandleWndCommands(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    FlightPlanDlgProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	MainThreadId_G = ::GetCurrentThreadId();

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
	RedirectIOToConsole();
#endif

	// --- Initialize Core Systems ---
	initializePacketHandlers();
	CommandHandlers::initialize();

	// --- Initialize our Application Context ---
	g_app.initialize();

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

	printf("rot: %f\n", get_radius_of_turn(25, 180));

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (hFplWnd == NULL || !IsDialogMessage(hFplWnd, &msg))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	// --- Application Shutdown ---
	g_app.shutdown();

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
	wcex.hbrBackground = background_color;
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

	hWnd = CreateWindowW(szWindowClass, szTitle, dwStyle,
		CW_USEDEFAULT, 0, 900, 400, nullptr, nullptr, hInstance, nullptr);

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

int paint = 0;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		create_controls(hWnd);
		// Tasks are now started in g_app.initialize(), so this is much cleaner.
		read_info();
	}
	break;
	case WM_COMMAND:
	{
		HandleWndCommands(hWnd, message, wParam, lParam);
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
		// Shutdown logic is now handled when wWinMain exits.
		// We just need to post the quit message to break the GetMessage loop.
		done = true;
		PostQuitMessage(0);
	}
	break;
	case WM_CTLCOLORSTATIC:
	{
		HDC hdC = (HDC)wParam;
		HWND ctrl = (HWND)lParam;
		DWORD CtrlID = GetDlgCtrlID(ctrl);
		TCHAR controlClassName[128];

		GetClassName(ctrl, controlClassName, 128);

		std::wstring test(&controlClassName[0]); //convert to wstring
		std::string classn = ws2s(test); //and convert to string.
		COLORREF textColor = GetTextColor(hdC);
		COLORREF bkColor = GetBkColor(hdC);

		if (classn == "Static")
		{
			if (CtrlID == ALTITUDE_TEXT || CtrlID == LONGITUDE_TEXT || CtrlID == VS_TEXT
				|| CtrlID == LATITUDE_TEXT || CtrlID == HEADING_TEXT || CtrlID == CONSOLE_TEXT
				|| CtrlID == SPEED_TEXT || CtrlID == TRACK_TEXT || CtrlID == DATA_TEXT)
			{
				if (textColor != text_ref)
					SetTextColor(hdC, text_ref);
				if (bkColor != vbk_ref)
					SetBkColor(hdC, vbk_ref);
				return (INT_PTR)variablebk_color;
			}
			SetTextColor(hdC, text_ref);
			SetBkColor(hdC, bk_ref);
			return (INT_PTR)background_color;
		}
		else if (classn == "Edit")
		{
			SetTextColor(hdC, text_ref);
			SetBkColor(hdC, vbk_ref);
			return (INT_PTR)variablebk_color;
		}
	}
	break;
	case WM_CTLCOLOREDIT:
	{
		HDC hdC = (HDC)wParam;
		HWND ctrl = (HWND)lParam;
		DWORD CtrlID = GetDlgCtrlID(ctrl);
		TCHAR controlClassName[128];

		GetClassName(ctrl, controlClassName, 128);

		std::wstring test(&controlClassName[0]); //convert to wstring
		std::string classn = ws2s(test); //and convert to string.

		if (classn == "Edit")
		{
			SetTextColor(hdC, RGB(0, 0, 0));
			SetBkColor(hdC, RGB(255, 255, 255));
			return (INT_PTR)variablebk_color;
		}
	}
	break;
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_RETURN:
		{
			//Do your stuff
		}
		break;  //or return 0; if you don't want to pass it further to def proc
		//If not your key, skip to default:
		}
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK HandleWndCommands(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	// Parse the menu selections:
	switch (wmId)
	{
	case ID_VIEW_FPL_BUTTON:
	{
		if (displayed && hFplWnd == NULL)
		{
			hFplWnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FLIGHTPLAN), hWnd, FlightPlanDlgProc, (LPARAM)displayed);
			ShowWindow(hFplWnd, SW_SHOW);
		}
		else if (hFplWnd != NULL)
		{
			SetForegroundWindow(hFplWnd);
		}
	}
	break;
	case MODE_BUTTON:
	{
		if (displayed)
		{
			Aircraft& aircraft = *displayed;
			if (aircraft.getMode())
			{
				aircraft.setMode(0);
				SetWindowText(mode_button, L"SQUAWK: S");
				updateMode(aircraft);
			}
			else
			{
				aircraft.setMode(1);
				SetWindowText(mode_button, L"SQUAWK: C");
				updateMode(aircraft);
			}
		}
	}
	break;
	case ID_SETTINGS_AOT:
	{
		always_on_top = !always_on_top;
		if (always_on_top)
		{
			CheckMenuItem(hSettings, ID_SETTINGS_AOT, MF_CHECKED);
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		else
		{
			CheckMenuItem(hSettings, ID_SETTINGS_AOT, MF_UNCHECKED);

			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
	}
	break;
	case ID_FILE_OPEN_SCT:
	{
		OPENFILENAME ofn;
		TCHAR szFileName[MAX_PATH] = L"";

		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter = L"Sector2 Files (*.sct2)\0*.sct2\0Sector Files (*.sct)\0*.sct\0All Files (*.*)\0*.*\0";
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = L"sct2";

		if (GetOpenFileName(&ofn))
		{
			std::string path = ws2s(std::wstring(szFileName));
			LAST_SCT_PATH = path; // Update setting

			FileReader reader;
			auto sector_data = reader.loadSct(path);

			if (sector_data) {
				// Access the context and move the data
				auto& ctx = SimulationContext::instance();
				// ctx.setSectorData(std::move(sector_data)); // Assuming context has a place for it

				std::wstring msg = L"Successfully loaded " + std::to_wstring(sector_data->fixes.size()) + L" fixes, "
					+ std::to_wstring(sector_data->vors.size()) + L" VORs, and "
					+ std::to_wstring(sector_data->ndbs.size()) + L" NDBs.";
				MessageBox(hWnd, msg.c_str(), L"SCT Load Success", MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBoxA(hWnd, "Failed to load or parse SCT file.", "SCT Load Error", MB_OK | MB_ICONERROR);
			}
		}
	}
	break;
	case ID_FILE_OPEN_AGC:
	{
		OPENFILENAME ofn;
		TCHAR szFileName[MAX_PATH] = L"";
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter = L"Aircraft Scenario File (*.agc)\0*.agc\0All Files (*.*)\0*.*\0";
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = L"agc";

		if (GetOpenFileName(&ofn))
		{
			std::string path = ws2s(std::wstring(szFileName));
			LAST_AGC_PATH = path; // Update setting

			FileReader reader;
			try {
				auto loaded_aircraft = reader.loadAgc(path);

				auto& ctx = SimulationContext::instance();
				std::lock_guard<std::mutex> lock(ctx.aircraftMutex());

				for (auto& acf_ptr : loaded_aircraft) {
					const std::string& cs = acf_ptr->getCallSign();
					if (ctx.aircraft().find(cs) == ctx.aircraft().end()) {
						addUserToLB(acf_ptr.get());
						ctx.aircraft()[cs] = std::move(acf_ptr);
					}
				}
			}
			catch (const std::runtime_error& e) {
				MessageBoxA(hWnd, e.what(), "AGC Load Error", MB_OK | MB_ICONERROR);
			}
		}
	}
	break;
	case ID_FILE_OPEN_APRT:
	{
		OPENFILENAME ofn;
		TCHAR szFileName[MAX_PATH] = L"";

		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
		ofn.hwndOwner = hWnd;
		ofn.lpstrFilter = L"Airport File (*.aprt)\0*.aprt\0All Files (*.*)\0*.*\0";
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = L"aprt";

		if (GetOpenFileName(&ofn))
		{
			std::string path = ws2s(std::wstring(szFileName));
			size_t found = path.find_last_of("/\\");
			LAST_APRT_DIR = path.substr(0, found);

			FileReader reader;
			auto loaded_airport = reader.loadApt(path);

			auto& ctx = SimulationContext::instance();
			if (loaded_airport) {
				ctx.airports()[loaded_airport->icao] = std::move(loaded_airport);
			}
			else {
				MessageBoxA(hWnd, "Failed to load or parse the airport file.", "APRT Load Error", MB_OK | MB_ICONERROR);
			}
		}
	}
	break;
	case ID_FILE_SAVE:
		save_info();
		break;
	case ACF_LISTBOX:
	{
		int notification = HIWORD(wParam);
		switch (notification)
		{
		case LBN_SELCHANGE:
		{
			HWND ctrl = (HWND)lParam;

			//check to make sure things line up
			if (ctrl == aircraftList)
			{
				DWORD dwSel = (DWORD)SendMessage(aircraftList, LB_GETCURSEL, 0, 0);
				HandleSelectedLB(dwSel);
			}
		}
		break;
		}
	}
	break;
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


// --- NEW: Helper function to populate the FPL dialog with an aircraft's data ---
void UpdateFplDialog(HWND hDlg, Aircraft* aircraft)
{
	if (!hDlg || !aircraft) return;

	// Store the new aircraft pointer in the window's user data for future reference if needed
	SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)aircraft);

	// Set the window title
	std::string title = "Flight Plan - " + aircraft->getCallSign();
	SetWindowText(hDlg, s2ws(title).c_str());

	// Get the flight plan to populate the fields
	FlightPlan& fp = aircraft->getFlightPlan();

	// Populate all the text boxes
	SetDlgItemText(hDlg, IDC_FP_CALLSIGN, s2ws(aircraft->getCallSign()).c_str());
	SetDlgItemText(hDlg, IDC_FP_AIRCRAFT_TYPE, s2ws(fp.acType).c_str());

	std::wstring fr_text;
	if (fp.flightRules == 0) fr_text = L"VFR";
	else if (fp.flightRules == 1) fr_text = L"IFR";
	else fr_text = L"SVFR";
	SetDlgItemText(hDlg, IDC_FP_FLIGHT_RULES, fr_text.c_str());

	SetDlgItemText(hDlg, IDC_FP_SQUAWK, s2ws(fp.squawkCode).c_str());
	SetDlgItemText(hDlg, IDC_FP_DEPARTURE, s2ws(fp.departure).c_str());
	SetDlgItemText(hDlg, IDC_FP_ARRIVAL, s2ws(fp.arrival).c_str());
	SetDlgItemText(hDlg, IDC_FP_ALTERNATE, s2ws(fp.alternate).c_str());
	SetDlgItemText(hDlg, IDC_FP_CRUISE_ALT, s2ws(fp.cruise).c_str());
	SetDlgItemText(hDlg, IDC_FP_SCRATCHPAD, s2ws(fp.scratchPad).c_str());
	SetDlgItemText(hDlg, IDC_FP_ROUTE, s2ws(fp.route).c_str());
	SetDlgItemText(hDlg, IDC_FP_REMARKS, s2ws(fp.remarks).c_str());
}

// --- MODIFIED: Dialog Procedure for the Flight Plan Window ---
INT_PTR CALLBACK FlightPlanDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		// On creation, lParam contains the pointer to the Aircraft object.
		// We use our helper to do the initial population.
		UpdateFplDialog(hDlg, (Aircraft*)lParam);
		return (INT_PTR)TRUE;
	}

	// --- NEW: Handle our custom update message ---
	case WM_APP_FPL_UPDATE:
	{
		// When we receive this message, lParam contains the new Aircraft pointer.
		// We use the same helper function to refresh the dialog.
		UpdateFplDialog(hDlg, (Aircraft*)lParam);
		return (INT_PTR)TRUE;
	}

	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDC_FP_CLOSE || LOWORD(wParam) == IDCANCEL)
		{
			DestroyWindow(hDlg);
			return (INT_PTR)TRUE;
		}
		break;
	}

	case WM_CLOSE:
	{
		DestroyWindow(hDlg);
		return (INT_PTR)TRUE;
	}

	case WM_DESTROY:
	{
		hFplWnd = NULL;
		return (INT_PTR)TRUE;
	}
	}
	return (INT_PTR)FALSE;
}


void connect()
{
	auto& ctx = SimulationContext::instance();
	std::lock_guard<std::mutex> lock(ctx.aircraftMutex());
	for (auto& [callsign, acPtr] : ctx.aircraft())
	{
		Aircraft& aircraft = *acPtr;
		const AircraftState& state = aircraft.getState();
		tcp_manager& tcp = aircraft.getConnection();
		if (!aircraft.connected)
		{
			const std::string ip = "127.0.0.1";
			if (tcp.connectNew(ip, 4403))
			{
				aircraft.connected = true;
				Identity& id = *aircraft.getIdentity();
				BasicStream stream = BasicStream(512);
				AV_CLIENT type = aircraft.getType();
				tcp.hand_shake = true;
				tcp.current_op = 45;
				stream.create_frame_var_size_word(45);
				stream.write_int(PROTO_VERSION);
				stream.write_string(id.callsign.c_str());
				stream.write_string(id.login_name.c_str());
				stream.write_string(id.username.c_str());
				stream.write_string(id.password.c_str());
				stream.write_qword(1000);//request time
				stream.write_qword(doubleToRawBits(state.latitude));
				stream.write_qword(doubleToRawBits(state.longitude));
				stream.write_short(aircraft.getVisibility());
				stream.write_byte(static_cast<int>(type));
				stream.write_3byte(aircraft.frequency[0]);
				stream.write_3byte(aircraft.frequency[1]);
				if (type == AV_CLIENT::CONTROLLER)
				{
					stream.write_byte(id.controller_rating);
					stream.write_byte(id.controller_position);
				}
				else if (type == AV_CLIENT::PILOT)
				{
					stream.write_byte(id.pilot_rating);
					stream.write_string(aircraft.getAcfTitle().c_str());
					stream.write_string(aircraft.getSquawkCode().c_str());
					stream.write_byte(aircraft.getMode() << 4 | (aircraft.isHeavy() ? 1 : 0));
					const long long infoHash = ((static_cast<long long>((int)((state.pitch * 1024.0) / -360.0))) << 22)
						+ ((static_cast<long long>(static_cast<int>((state.roll * 1024.0) / -360.0))) << 12)
						+ ((static_cast<long long>(static_cast<int>((state.heading * 1024.0) / 360.0))) << 2);
					stream.write_qword(infoHash);
				}
				stream.end_frame_var_size_word();
				aircraft.getConnection().sendMessage(&stream);
			}
		}
	}
}

/*--- Handles Command Callback ---*/

LRESULT CALLBACK CommandCallBckProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CHAR)
	{
		if (wParam == VK_ESCAPE)
		{
			//PostQuitMessage(0);
			SendMessage(command_text, WM_SETTEXT, 0, (LPARAM)L"");
			return 0;
		}
		else if (wParam == VK_RETURN)
		{
			//PostQuitMessage(0);
			TCHAR c_text[256];
			LRESULT result = SendMessage(command_text, WM_GETTEXT, sizeof(c_text), (LPARAM)c_text);

			std::wstring test(&c_text[0]); //convert to wstring
			std::string text = ws2s(test); //and convert to string.

			if (displayed)
			{
				CommandHandlers::processCommand(*displayed, text);
			}
			SendMessage(command_text, WM_SETTEXT, 0, (LPARAM)L"");
			return 0;
		}
	}
	return CallWindowProc(lpCommandWndProc, hWnd, uMsg, wParam, lParam);
}

/*--- Creates GUI Controls ---*/

void create_controls(HWND hwnd) {
	HMENU hMenuBar = CreateMenu();
	HMENU hFile = CreateMenu();
	hSettings = CreateMenu();
	HMENU hHelp = CreateMenu();

	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"&File");
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hSettings, L"&Settings");
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelp, L"&Help");

	AppendMenu(hFile, MF_STRING, ID_FILE_CONNECT, L"&Connect to Sever...");
	AppendMenu(hFile, MF_STRING, ID_FILE_OPEN_AGC, L"&Open Aircraft File...");
	AppendMenu(hFile, MF_STRING, ID_FILE_SAVE, L"&Save Data");
	AppendMenu(hFile, MF_STRING, ID_FILE_OPEN_SCT, L"&Open SCT File...");
	AppendMenu(hFile, MF_STRING, ID_FILE_OPEN_APRT, L"&Open APT File...");

	AppendMenu(hSettings, MF_STRING, ID_SETTINGS_AOT, L"&Always On Top");

	SetMenu(hwnd, hMenuBar);

	std::string freq = "Commands [" + frequency_to_string(command_freq) + "]:";
	HWND lbl_commands = CreateWindowEx(NULL, L"STATIC", (LPCWSTR)std::wstring(freq.begin(), freq.end()).c_str(),
		WS_VISIBLE | WS_CHILD | SS_CENTER | ES_READONLY,
		35, 310, 150, 20,
		hwnd, (HMENU)COMMANDS_LBL, NULL, NULL
	);

	SendMessage(lbl_commands, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(lbl_commands, EM_LIMITTEXT, 0, 0L);

	command_text = CreateWindowEx(WS_EX_STATICEDGE, L"Edit", L"",
		WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER | WS_DLGFRAME | ES_AUTOHSCROLL,
		190, 305, 685, 30,
		hwnd, (HMENU)COMMAND_TEXT, NULL, NULL
	);

	SendMessage(command_text, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(command_text, EM_LIMITTEXT, 128, 0L);

	lpCommandWndProc = (WNDPROC)SetWindowLongPtr(command_text, GWLP_WNDPROC, (LONG_PTR)&CommandCallBckProcedure);

	console_text = CreateWindowEx(
		WS_EX_CLIENTEDGE, L"STATIC", L"",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_READONLY | ES_WANTRETURN,
		190, 200, 685, 100,
		hwnd, (HMENU)CONSOLE_TEXT, NULL, NULL
	);

	SendMessage(console_text, WM_SETFONT, (WPARAM)hFont2, MAKELPARAM(TRUE, 0));
	SendMessage(console_text, EM_LIMITTEXT, 0, 0L);

	// Initialize the console logger, now through the app context
	g_app.consoleLogger.Init(console_text);

	HWND lbl_altitude = CreateWindowEx(NULL, L"STATIC", L"Altitude:",
		WS_VISIBLE | WS_CHILD | SS_CENTER | ES_READONLY,
		210, 15, 60, 20,
		hwnd, (HMENU)ALTITUDE_LBL, NULL, NULL
	);

	SendMessage(lbl_altitude, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(lbl_altitude, EM_LIMITTEXT, 0, 0L);

	altitude = CreateWindowEx(NULL, L"STATIC", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER | ES_READONLY,
		190, 40, 100, 25,
		hwnd, (HMENU)ALTITUDE_TEXT, NULL, NULL
	);

	SendMessage(altitude, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(altitude, EM_LIMITTEXT, 25, 0L);

	HWND lbl_heading = CreateWindowEx(NULL, L"STATIC", L"Heading:",
		WS_VISIBLE | WS_CHILD | SS_CENTER | ES_READONLY,
		210, 70, 60, 20,
		hwnd, (HMENU)HEADING_LBL, NULL, NULL
	);

	SendMessage(lbl_heading, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(lbl_heading, EM_LIMITTEXT, 0, 0L);

	heading = CreateWindowEx(NULL, L"STATIC", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER | ES_READONLY,
		190, 95, 100, 25,
		hwnd, (HMENU)HEADING_TEXT, NULL, NULL
	);

	SendMessage(heading, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(heading, EM_LIMITTEXT, 25, 0L);

	HWND lbl_VS = CreateWindowEx(NULL, L"STATIC", L"Vertical Speed:",
		WS_VISIBLE | WS_CHILD | SS_CENTER | ES_READONLY,
		190, 125, 100, 20,
		hwnd, (HMENU)VS_LBL, NULL, NULL
	);

	SendMessage(lbl_VS, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(lbl_VS, EM_LIMITTEXT, 30, 0L);

	vs_hdl = CreateWindowEx(NULL, L"STATIC", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER | ES_READONLY,
		190, 150, 100, 25,
		hwnd, (HMENU)VS_TEXT, NULL, NULL
	);

	SendMessage(vs_hdl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(vs_hdl, EM_LIMITTEXT, 25, 0L);

	HWND lbl_latitude = CreateWindowEx(NULL, L"STATIC", L"Latitude:",
		WS_VISIBLE | WS_CHILD | SS_CENTER | ES_READONLY,
		330, 15, 60, 20,
		hwnd, (HMENU)LATITUDE_LBL, NULL, NULL
	);

	SendMessage(lbl_latitude, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(lbl_latitude, EM_LIMITTEXT, 0, 0L);

	latitude_hdl = CreateWindowEx(NULL, L"STATIC", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER | ES_READONLY,
		310, 40, 100, 25,
		hwnd, (HMENU)LATITUDE_TEXT, NULL, NULL
	);

	SendMessage(latitude_hdl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(latitude_hdl, EM_LIMITTEXT, 25, 0L);

	HWND lbl_longitude = CreateWindowEx(NULL, L"STATIC", L"Longitude:",
		WS_VISIBLE | WS_CHILD | SS_CENTER | ES_READONLY,
		320, 70, 80, 20,
		hwnd, (HMENU)LONGITUDE_LBL, NULL, NULL
	);

	SendMessage(lbl_longitude, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(lbl_longitude, EM_LIMITTEXT, 0, 0L);

	longitude_hdl = CreateWindowEx(NULL, L"STATIC", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER | ES_READONLY,
		310, 95, 100, 25,
		hwnd, (HMENU)LONGITUDE_TEXT, NULL, NULL
	);

	SendMessage(longitude_hdl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(longitude_hdl, EM_LIMITTEXT, 25, 0L);

	HWND lbl_speed = CreateWindowEx(NULL, L"STATIC", L"Ground Spd:",
		WS_VISIBLE | WS_CHILD | SS_CENTER | ES_READONLY,
		315, 125, 90, 20,
		hwnd, (HMENU)SPEED_LBL, NULL, NULL
	);

	SendMessage(lbl_speed, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(lbl_speed, EM_LIMITTEXT, 0, 0L);

	speed_hdl = CreateWindowEx(NULL, L"STATIC", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER | ES_READONLY,
		310, 150, 100, 25,
		hwnd, (HMENU)SPEED_TEXT, NULL, NULL
	);

	SendMessage(speed_hdl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(speed_hdl, EM_LIMITTEXT, 25, 0L);

	HWND lbl_track = CreateWindowEx(NULL, L"STATIC", L"Track:",
		WS_VISIBLE | WS_CHILD | SS_CENTER | ES_READONLY,
		445, 15, 70, 20,
		hwnd, (HMENU)TRACK_LBL, NULL, NULL
	);

	SendMessage(lbl_track, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(lbl_track, EM_LIMITTEXT, 0, 0L);

	track_hdl = CreateWindowEx(NULL, L"STATIC", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER | ES_READONLY,
		430, 40, 100, 25,
		hwnd, (HMENU)TRACK_TEXT, NULL, NULL
	);

	SendMessage(track_hdl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(track_hdl, EM_LIMITTEXT, 25, 0L);

	HWND lbl_data = CreateWindowEx(NULL, L"STATIC", L"Data:",
		WS_VISIBLE | WS_CHILD | SS_CENTER | ES_READONLY,
		445, 70, 70, 20,
		hwnd, (HMENU)DATA_LBL, NULL, NULL
	);

	SendMessage(lbl_data, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(lbl_data, EM_LIMITTEXT, 0, 0L);

	data_hdl = CreateWindowEx(NULL, L"STATIC", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER | ES_READONLY,
		430, 95, 100, 25,
		hwnd, (HMENU)DATA_TEXT, NULL, NULL
	);

	SendMessage(data_hdl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(data_hdl, EM_LIMITTEXT, 25, 0L);

	mode_button = CreateWindowEx(NULL, L"BUTTON", L"SQUAWK: C",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_CENTER,
		785, 150, 90, 30,
		hwnd, (HMENU)MODE_BUTTON, NULL, NULL
	);

	SendMessage(mode_button, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(mode_button, EM_LIMITTEXT, 25, 0L);

	CreateWindowEx(NULL, L"BUTTON", L"View FPL",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_CENTER,
		785, 115, 90, 30,
		hwnd, (HMENU)ID_VIEW_FPL_BUTTON, NULL, NULL
	);

	aircraftList = CreateWindowEx(WS_EX_STATICEDGE, L"LISTBOX", NULL,
		WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_NOTIFY | LBS_HASSTRINGS | LBS_SORT | WS_BORDER,
		10, 17,
		170, 300,
		hwnd, (HMENU)ACF_LISTBOX,
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		NULL);

	SendMessage(aircraftList, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}

/*---- Adds an aircraft to the Aircraft List ---*/

void addUserToLB(Aircraft* user) {
	std::string callsign = user->getIdentity()->callsign;
	int pos = (int)SendMessage(aircraftList, LB_ADDSTRING, 0, (LPARAM)std::wstring(callsign.begin(), callsign.end()).c_str());
	SendMessage(aircraftList, LB_SETITEMDATA, pos, (LPARAM)user);
}

// --- MODIFIED: The listbox selection handler ---
void HandleSelectedLB(DWORD iSelected)
{
	Aircraft* acf = (Aircraft*)SendMessage(aircraftList, LB_GETITEMDATA, iSelected, 0);
	if (acf && displayed != acf)
	{
		displayed = acf;
		AircraftState& state = displayed->getState();
		state.MarkAllDirty();
		DisplayAircraft();

		// --- NEW: If the FPL window is open, send it a message to update ---
		if (hFplWnd != NULL)
		{
			SendMessage(hFplWnd, WM_APP_FPL_UPDATE, 0, (LPARAM)acf);
		}
	}
}

void DisplayAircraft() {
	if (displayed)
	{
		const AircraftState& state = displayed->getState();
		if (state.IsDirty(AircraftDirtyFlags::ALTITUDE)) {
			std::wstring alt = std::to_wstring((int)state.altitude);
			SetWindowText(altitude, alt.c_str());
		}

		if (state.IsDirty(AircraftDirtyFlags::HEADING)) {
			std::wstring hdg = std::to_wstring((int)state.heading);
			SetWindowText(heading, hdg.c_str());
		}

		if (state.IsDirty(AircraftDirtyFlags::VSPEED)) {
			std::wstring vs = std::to_wstring((int)state.verticalSpeed);
			SetWindowText(vs_hdl, vs.c_str());
		}

		if (state.IsDirty(AircraftDirtyFlags::LATITUDE)) {
			std::wstring lat = std::to_wstring(state.latitude);
			SetWindowText(latitude_hdl, lat.c_str());
		}

		if (state.IsDirty(AircraftDirtyFlags::LONGITUDE)) {
			std::wstring lon = std::to_wstring(state.longitude);
			SetWindowText(longitude_hdl, lon.c_str());
		}

		if (state.IsDirty(AircraftDirtyFlags::SPEED)) {
			std::wstring spd = std::to_wstring((int)state.speed);
			SetWindowText(speed_hdl, spd.c_str());
		}

		if (state.IsDirty(AircraftDirtyFlags::TRACK)) {
			const RouteManager& rm = displayed->getRouteManager();
			if (displayed->onGround() && rm.ground_cur && rm.ground_cur->parent) {
				SetWindowText(track_hdl, std::wstring(rm.ground_cur->parent->name.begin(), rm.ground_cur->parent->name.end()).c_str());
			}
			else {
				SetWindowText(track_hdl, L"None");
			}
		}

		if (state.IsDirty(AircraftDirtyFlags::DATA)) {
			char data1[20];
			int length = sprintf_s(data1, "[%d]", (int)displayed->getAssignedValues().asdg_gnd_turn_rate);
			std::wstring rates(&data1[0], &data1[length]);
			SetWindowText(data_hdl, rates.c_str());
		}

		if (state.IsDirty(AircraftDirtyFlags::MODE)) {
			displayed->getMode() == 0 ? SetWindowText(mode_button, L"SQUAWK: S") : SetWindowText(mode_button, L"SQUAWK: C");
		}

		displayed->getState().ClearDirtyFlags();
	}
}

void GuiUpdateTask::execute() {
	DisplayAircraft();
	g_app.consoleLogger.FlushToConsole();
}