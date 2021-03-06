#ifndef _WINCREATION_H_
#define _WINCREATION_H_

#include <Windows.h>
#include <windowsx.h>
#include <stdlib.h>

extern int g_nCmdShow;

HWND CreateForm(HINSTANCE hInstance, LPWSTR lpFormName, LPWSTR lpFormTitle, WNDPROC wndProc, int iWidth, int iHeight, bool isFullscreen = false)
{
	DWORD windowStyle;

	if ( isFullscreen )
		windowStyle = WS_POPUP | WS_VISIBLE;
	else
		windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_OWNDC;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpszClassName = lpFormName;
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpszMenuName = NULL;
	wc.lpfnWndProc = wndProc;
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	
	RegisterClassEx(&wc);

	HWND hWnd = CreateWindowEx(NULL, wc.lpszClassName, lpFormTitle, windowStyle, 0, 0,
		iWidth, iHeight, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, g_nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

#endif