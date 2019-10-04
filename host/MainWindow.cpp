#include "stdafx.h"
#include "MainWindow.h"

#include <Windows.h>
#include <memory>
#include <iostream>
#include "host.h"


static TCHAR *szWindowClassName = _T("Host_MainWindow");

void MainWindow::registerWindowClass(HINSTANCE hInstance) 
{
	WNDCLASSEX wcex;

	if (GetClassInfoEx(hInstance, szWindowClassName, &wcex) != 0) {
		return;
	}

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &MainWindow::staticWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	//wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOST));
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_HOST);
	wcex.lpszClassName = szWindowClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	//wcex.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wcex);
}

std::shared_ptr<MainWindow> MainWindow::getWindowClass(HWND hWnd)
{
	LONG_PTR long_ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	std::shared_ptr<MainWindow> *ptr = reinterpret_cast<std::shared_ptr<MainWindow> *>(long_ptr);
	return *ptr;
}

LRESULT CALLBACK MainWindow::staticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CREATE) {
		std::shared_ptr<MainWindow> *window_class_from_create_struct = reinterpret_cast<std::shared_ptr<MainWindow> *>((reinterpret_cast<CREATESTRUCT*>(lParam))->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window_class_from_create_struct));

		std::shared_ptr<MainWindow> ws(*window_class_from_create_struct);
	}

	LRESULT result = getWindowClass(hWnd)->wndProc(hWnd, message, wParam, lParam);

	if (message == WM_DESTROY) {
		LONG_PTR long_ptr = SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
		std::shared_ptr<MainWindow> *ptr = reinterpret_cast<std::shared_ptr<MainWindow> *>(long_ptr);
		std::shared_ptr<MainWindow> windowState(*ptr);
		delete ptr;
	}

	return result;
}

MainWindow::MainWindow(HINSTANCE hInstance)
	: hInstance(hInstance)
{
}

LRESULT CALLBACK MainWindow::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		break;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_ABOUTBUTTON:
			break;
		case ID_CONFIGBUTTON:
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
		PostQuitMessage(0);
	}
	break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK MainWindow::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

MainWindow::~MainWindow()
{
}
