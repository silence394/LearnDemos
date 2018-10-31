#include <windows.h>
#include <iostream>
using namespace std;

LRESULT CALLBACK windowprocessforwindow1(HWND handleforwindow1,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK windowprocessforwindow2(HWND handleforwindow2,UINT message,WPARAM wParam,LPARAM lParam);

bool window1closed = false;
bool window2closed = false;

HWND GNativeWindow;
int	GShowCmd;

WNDCLASSEX windowclassforwindow2;
void CreateWindow2( HINSTANCE hInst, int nShowCmd, HWND parent = NULL )
{
	int x = 640;
	int y = 480;
	int w = 320;
	int h = 240;
	if ( parent != NULL )
	{
		x = 0;
		y = 0;
	}

	int style = WS_OVERLAPPEDWINDOW;
	if ( parent )
		style = WS_CHILD | WS_CAPTION | WS_SYSMENU|WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

	HWND handleforwindow2=CreateWindowEx(NULL,
		windowclassforwindow2.lpszClassName,
		L"Child Window",
		style,
		x,
		y,
		w,
		h,
		parent,
		NULL,
		hInst,
		NULL);

	if(!handleforwindow2)
	{
		int nResult=GetLastError();

		MessageBox(NULL,
			L"Window creation failed",
			L"Window Creation Failed",
			MB_ICONERROR);
	}

	if ( parent )
		SetParent( handleforwindow2, parent );

	ShowWindow(handleforwindow2, nShowCmd);
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nShowCmd)
{
	bool endprogram=false;

	//create window 1
	WNDCLASSEX windowclassforwindow1;
	ZeroMemory(&windowclassforwindow1,sizeof(WNDCLASSEX));
	windowclassforwindow1.cbClsExtra=NULL;
	windowclassforwindow1.cbSize=sizeof(WNDCLASSEX);
	windowclassforwindow1.cbWndExtra=NULL;
	windowclassforwindow1.hbrBackground=(HBRUSH)COLOR_WINDOW;
	windowclassforwindow1.hCursor=LoadCursor(NULL,IDC_ARROW);
	windowclassforwindow1.hIcon=NULL;
	windowclassforwindow1.hIconSm=NULL;
	windowclassforwindow1.hInstance=hInst;
	windowclassforwindow1.lpfnWndProc=(WNDPROC)windowprocessforwindow1;
	windowclassforwindow1.lpszClassName=L"windowclass 1";
	windowclassforwindow1.lpszMenuName=NULL;
	windowclassforwindow1.style=CS_HREDRAW|CS_VREDRAW;

	if(!RegisterClassEx(&windowclassforwindow1))
	{
		int nResult=GetLastError();
		MessageBox(NULL,
			L"Window class creation failed",
			L"Window Class Failed",
			MB_ICONERROR);
	}

	HWND handleforwindow1=CreateWindowEx(NULL,
		windowclassforwindow1.lpszClassName,
			L"Parent Window",
			WS_OVERLAPPEDWINDOW,
			0,
			0,
			640,
			480,
			NULL,
			NULL,
			hInst,
			NULL                /* No Window Creation data */
);

	GNativeWindow = handleforwindow1;

	if(!handleforwindow1)
	{
		int nResult=GetLastError();

		MessageBox(NULL,
			L"Window creation failed",
			L"Window Creation Failed",
			MB_ICONERROR);
	}

	ShowWindow(handleforwindow1,nShowCmd);

	ZeroMemory(&windowclassforwindow2,sizeof(WNDCLASSEX));
	windowclassforwindow2.cbClsExtra=NULL;
	windowclassforwindow2.cbSize=sizeof(WNDCLASSEX);
	windowclassforwindow2.cbWndExtra=NULL;
	windowclassforwindow2.hbrBackground=(HBRUSH)COLOR_WINDOW;
	windowclassforwindow2.hCursor=LoadCursor(NULL,IDC_ARROW);
	windowclassforwindow2.hIcon=NULL;
	windowclassforwindow2.hIconSm=NULL;
	windowclassforwindow2.hInstance=hInst;
	windowclassforwindow2.lpfnWndProc=(WNDPROC)windowprocessforwindow2;
	windowclassforwindow2.lpszClassName=L"window class2";
	windowclassforwindow2.lpszMenuName=NULL;
	windowclassforwindow2.style=CS_HREDRAW|CS_VREDRAW;

	if(!RegisterClassEx(&windowclassforwindow2))
	{
		int nResult=GetLastError();
		MessageBox(NULL,
			L"Window class creation failed for window 2",
			L"Window Class Failed",
			MB_ICONERROR);
	}

	// create window 2
	CreateWindow2( hInst, nShowCmd );

	MSG msg;
	ZeroMemory(&msg,sizeof(MSG));
	while (endprogram==false) {
		if (GetMessage(&msg,NULL,0,0));
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (window1closed==true && window2closed==true) {
			endprogram=true;
		}
	}

	MessageBox(NULL,
	L"Both Windows are closed.  Program will now close.",
	L"",
	MB_ICONINFORMATION);
	return 0;
}

LRESULT CALLBACK windowprocessforwindow1(HWND handleforwindow,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_DESTROY: {
			MessageBox(NULL,
			L"Window 1 closed",
			L"Message",
			MB_ICONINFORMATION);

			window1closed=true;
			return 0;
		}
		break;
	}

	return DefWindowProc(handleforwindow,msg,wParam,lParam);
}

LRESULT CALLBACK windowprocessforwindow2(HWND handleforwindow,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
	case WM_DESTROY: {
			MessageBox(NULL,
			L"Window 2 closed",
			L"Message",
			MB_ICONINFORMATION);

			window2closed=true;
			return 0;
		case WM_KEYDOWN:
		{
			if ( wParam == VK_SPACE )
			{
				DestroyWindow( handleforwindow );
			}
			break;
		}
		case WM_MOVE:
			RECT rect;
			GetWindowRect( handleforwindow, &rect );

			RECT mainrect;
			GetWindowRect( GNativeWindow, &mainrect );
			 HWND hwnd =  GetParent( handleforwindow );
			if ( hwnd == nullptr )
			{
				int x, y, w, h;
				w = ( mainrect.right - mainrect.left ) / 2;
				h = ( mainrect.bottom - mainrect.top ) / 2;
				x = mainrect.left + w / 2;
				y = mainrect.top + h / 2;
				if ( ( rect.left < x + w && rect.left > x) &&
				( rect.top < y + h && rect.top > y ) )
				{
					DestroyWindow( handleforwindow );
					CreateWindow2( GetModuleHandle(NULL), 10, GNativeWindow );
				}
			}

			break;
		}
		break;
	}

	return DefWindowProc(handleforwindow,msg,wParam,lParam);
}