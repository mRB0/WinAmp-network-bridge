// host.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "host.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <memory>

#include <winamp/in2.h>

#define MAX_LOADSTRING 100

typedef In_Module *(__stdcall *GetInModuleFn)(void);

// Global Variables:
static HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

struct WindowState {
	std::wstring why;
};

HMODULE pluginDll;
In_Module *pluginModule;

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
	LoadString(hInstance, IDC_HOST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HOST));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	FreeLibrary(pluginDll);

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
	//wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOST));
	wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_HOST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	//wcex.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);

	return RegisterClassEx(&wcex);
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

   auto windowState = new std::shared_ptr<WindowState>(new WindowState {
	std::wstring(L"oh gosh")
   });

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, reinterpret_cast<void*>(windowState));

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void logLastError(std::wstring const &text) {
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

	std::wostringstream dbg;
	dbg << L"Error: " << text << ": " << (LPTSTR)lpMsgBuf << std::endl;
	OutputDebugStringW(dbg.str().c_str());

	LocalFree(lpMsgBuf);
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

	switch (message)
	{
	case WM_CREATE:
	{
		std::shared_ptr<WindowState> *window_state_from_create_struct = reinterpret_cast<std::shared_ptr<WindowState> *>((reinterpret_cast<CREATESTRUCT*>(lParam))->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window_state_from_create_struct));

		std::shared_ptr<WindowState> ws(*window_state_from_create_struct);

		OutputDebugStringW(L"Why???: ");
		OutputDebugStringW(ws->why.c_str());
		OutputDebugStringW(L"\n");

		std::wcout << L"Thanks!" << std::endl;
		OutputDebugStringW(L"Gonna load that dll\n");

		SetDllDirectoryW(L"C:\\Users\\mrb\\Documents\\code\\winamp-network-bridge\\plugins");
		HMODULE pluginDll = LoadLibraryW(L"in_vgm.dll");

		if (pluginDll) {
			GetInModuleFn getInModuleFn = (GetInModuleFn)GetProcAddress(pluginDll, "winampGetInModule2");

			if (getInModuleFn) {
				pluginModule = getInModuleFn();

				pluginModule->hMainWindow = hWnd;
				pluginModule->hDllInstance = pluginDll;

				std::wostringstream dbg;
				dbg << L"Module description: " << pluginModule->description << std::endl;
				OutputDebugStringW(dbg.str().c_str());

				pluginModule->Init();

				CreateWindow(TEXT("button"),
					TEXT("About"),
					WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					0, 0,
					60, 40,
					hWnd,
					(HMENU)ID_ABOUTBUTTON,
					((LPCREATESTRUCT)lParam)->hInstance,
					NULL);

				CreateWindow(TEXT("button"),
					TEXT("Config"),
					WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					0, 40,
					60, 40,
					hWnd,
					(HMENU)ID_CONFIGBUTTON,
					((LPCREATESTRUCT)lParam)->hInstance,
					NULL);

			}
		}
		else {
			logLastError(L"Couldn't load dll");
		}
	}
		
	break;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_ABOUTBUTTON:
			if (pluginModule) {
				pluginModule->About(hWnd);
			}
			break;
		case ID_CONFIGBUTTON:
			if (pluginModule) {
				pluginModule->Config(hWnd);
			}
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
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
	{
		LONG_PTR long_ptr = SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
		std::shared_ptr<WindowState> *ptr = reinterpret_cast<std::shared_ptr<WindowState> *>(long_ptr);
		std::shared_ptr<WindowState> windowState(*ptr);
		delete ptr;
			
			
		OutputDebugString(L"DESTROY: ");
		OutputDebugString(windowState->why.c_str());
		OutputDebugString(L"\n");

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
