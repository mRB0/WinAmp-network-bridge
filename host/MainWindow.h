#pragma once

#include <Windows.h>
#include <memory>

class MainWindow
{
public:
	MainWindow(HINSTANCE hInstance, int nCmdShow);
	~MainWindow();

	void show();
	void update();

private:

	HINSTANCE hInstance;
	HWND hWnd;
	int nCmdShow;

	LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static void registerWindowClass(HINSTANCE hInstance);
	static std::shared_ptr<MainWindow> getWindowClass(HWND hWnd);
	static LRESULT CALLBACK staticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

