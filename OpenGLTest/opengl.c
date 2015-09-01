
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>


int GLPixelIndex;


void EnableOpenGL(HWND hWnd, HDC *hdc, HGLRC * hrc)
{
	PIXELFORMATDESCRIPTOR	pfd;
	int						iFormat;

	// get the device context (DC)
	*hdc = GetDC(hWnd);

	// set the pixel format for the DC
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	iFormat = ChoosePixelFormat(*hdc, &pfd);
	SetPixelFormat( *hdc, iFormat, &pfd);

	// create and enable the render context (RC)
	*hrc = wglCreateContext(*hdc);
	wglMakeCurrent(*hdc, *hrc);
}


void DisableOpenGL(HWND hWnd, HDC hdc, HGLRC hrc)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hrc);
	ReleaseDC(hWnd, hdc);
}
