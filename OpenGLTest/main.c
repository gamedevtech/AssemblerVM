
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include "opengl.h"
#include "test.h"


HWND		hWndGlobal;
HINSTANCE	hInstGlobal;
HDC			hdc;
HGLRC		hrc;
BYTE		keytab[256];


long WINAPI MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int	key;

	switch (message)
	{
		case (WM_CREATE):
			return (0L);

		case (WM_KEYDOWN):
			key = wParam;
			keytab[key] = 1;
			return (0L);

		case (WM_KEYUP):
			key = wParam;
			keytab[key] = 0;
			return (0L);

		case (WM_CLOSE):
			SendMessage(hWnd, WM_DESTROY, 0, 0);
			return (0L);

		case (WM_DESTROY):
			ReleaseDC(hWnd, hdc);
			PostQuitMessage(0);
			return (0L);
	}

	return (DefWindowProc(hWnd, message, wParam, lParam));
}


int PASCAL	WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS	wc;
	MSG			message;
	int			position_x = 0, position_y = 0;
	int			rotation = 0;

	wc.cbClsExtra					= 0;
	wc.cbWndExtra					= 0;
	wc.hbrBackground				= (HBRUSH)(COLOR_WINDOW);
	wc.hCursor						= LoadCursor(NULL, IDC_ARROW);
	wc.hIcon						= NULL;
	wc.hInstance					= hInstance;
	wc.lpfnWndProc					= MainWndProc;
	wc.lpszClassName				= "OpenGL Test";
	wc.lpszMenuName					= NULL;
	wc.style						= CS_OWNDC;

	if (!RegisterClass(&wc))
		return (0);

	hWndGlobal = CreateWindowEx(0, "OpenGL Test", "Test", WS_OVERLAPPEDWINDOW |
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 100, 100, 400, 400, NULL, NULL, hInstance, NULL);

	if (hWndGlobal == NULL)
		return (0);

	UpdateWindow(hWndGlobal);
	ShowWindow(hWndGlobal, nCmdShow);

	StartFunction();

	while (1)
	{
		if (PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&message, NULL, 0, 0)) break;;

			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		else
		{
			MainLoop();
		}
	}

	EndFunction();

	return (1);
}