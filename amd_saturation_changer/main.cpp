// amd_saturation_changer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "main.h"
#include "src/amd_saturation_changer.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
NOTIFYICONDATA	niData;							// notify icon data
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

std::unique_ptr<class amd_saturation_changer> amd_saturation_changer;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_AMD_SATURATION_CHANGER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AMD_SATURATION_CHANGER));

	amd_saturation_changer = std::unique_ptr<class amd_saturation_changer>(new class amd_saturation_changer());
	amd_saturation_changer->start();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AMD_SATURATION_CHANGER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_AMD_SATURATION_CHANGER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

void ShowContextMenu(HWND hWnd)
{
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		InsertMenu(hMenu, -1, MF_BYPOSITION, IDM_SHOW_SETTINGS, _T("Show settings"));
		InsertMenu(hMenu, -1, MF_BYPOSITION, IDM_RELOAD_SETTINGS, _T("Reload settings"));
		InsertMenu(hMenu, -1, MF_BYPOSITION, IDM_EXIT, _T("Exit"));

		// note:	must set window to the foreground or the
		//			menu won't disappear when it should
		SetForegroundWindow(hWnd);

		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN,
			pt.x, pt.y, 0, hWnd, NULL);
		DestroyMenu(hMenu);
	}
}

void ShowCurrentSettings()
{
	std::string process_name("Process name = " + amd_saturation_changer->settings->process_name + "\n");
	std::string normal_saturation("Normal saturation = " + std::to_string(amd_saturation_changer->settings->normal_saturation) + "\n");
	std::string process_saturation("Process running saturation = " + std::to_string(amd_saturation_changer->settings->process_saturation) + "\n");
	std::string normal_brightness("Normal brightess = " + std::to_string(amd_saturation_changer->settings->normal_brightness) + "\n");
	std::string process_brightness("Process running brightess = " + std::to_string(amd_saturation_changer->settings->process_brightness) + "\n");
	std::string normal_contrast("Normal contrast = " + std::to_string(amd_saturation_changer->settings->normal_contrast) + "\n");
	std::string process_contrast("Process running contrast  = " + std::to_string(amd_saturation_changer->settings->process_contrast) + "\n");
	std::string display_id("Display id = " + std::to_string(amd_saturation_changer->settings->logical_display_id) + "\n");

	std::string message(process_name + normal_saturation + process_saturation + normal_brightness + process_brightness + normal_contrast + process_contrast + display_id);
	MessageBox(NULL,
		s2ws(message).c_str(),
		L"Current settings",
		MB_OK);
}

ULONGLONG GetDllVersion(LPCTSTR lpszDllName)
{
	ULONGLONG ullVersion = 0;
	HINSTANCE hinstDll;
	hinstDll = LoadLibrary(lpszDllName);
	if (hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");
		if (pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;
			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);
			hr = (*pDllGetVersion)(&dvi);
			if (SUCCEEDED(hr))
				ullVersion = MAKEDLLVERULL(dvi.dwMajorVersion, dvi.dwMinorVersion, 0, 0);
		}
		FreeLibrary(hinstDll);
	}
	return ullVersion;
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
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   //ShowWindow(hWnd, nCmdShow);
   //UpdateWindow(hWnd);

   // Fill the NOTIFYICONDATA structure and call Shell_NotifyIcon

   // zero the structure - note:	Some Windows funtions require this but
   //								I can't be bothered which ones do and
   //								which ones don't.
   ZeroMemory(&niData, sizeof(NOTIFYICONDATA));

   // get Shell32 version number and set the size of the structure
   //		note:	the MSDN documentation about this is a little
   //				dubious and I'm not at all sure if the method
   //				bellow is correct
   ULONGLONG ullVersion = GetDllVersion(_T("Shell32.dll"));
   if (ullVersion >= MAKEDLLVERULL(5, 0, 0, 0))
	   niData.cbSize = sizeof(NOTIFYICONDATA);
   else niData.cbSize = NOTIFYICONDATA_V2_SIZE;

   // the ID number can be anything you choose
   niData.uID = 1;

   // state which structure members are valid
   niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

   // load the icon
   niData.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_AMD_SATURATION_CHANGER),
	   IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
	   LR_DEFAULTCOLOR);

   // the window to send messages to and the message to send
   //		note:	the message value should be in the
   //				range of WM_APP through 0xBFFF
   niData.hWnd = hWnd;
   niData.uCallbackMessage = WM_APP;

   // tooltip message
   lstrcpyn(niData.szTip, _T("Amd saturation changer"), sizeof(niData.szTip) / sizeof(TCHAR));

   Shell_NotifyIcon(NIM_ADD, &niData);

   // free icon handle
   if (niData.hIcon && DestroyIcon(niData.hIcon))
	   niData.hIcon = NULL;

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_APP:
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
			//ShowWindow(hWnd, SW_RESTORE);
			ShowContextMenu(hWnd);
			break;
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			ShowContextMenu(hWnd);
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_SHOW_SETTINGS:
			ShowCurrentSettings();
			break;
		case IDM_RELOAD_SETTINGS:
			amd_saturation_changer->load_settings();
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
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		amd_saturation_changer->stop();
		niData.uFlags = 0;
		Shell_NotifyIcon(NIM_DELETE, &niData);
		PostQuitMessage(0);
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
