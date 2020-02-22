#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool InitWindow(HINSTANCE hInstance, int nCmdShow);

void MainLoop();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{

    // create the window
    if (!InitWindow(hInstance, nCmdShow))
    {
        MessageBox(0, L"Window Initialization - Failed",
            L"Error", MB_OK);
        return 0;
    }

    // start the main loop
    MainLoop();

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            if (MessageBox(0, L"Are you sure you want to exit?",
                L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
                DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd,
        msg,
        wParam,
        lParam);
}

bool InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Handle to the window
    HWND hwnd = NULL;

    // name of the window (not the title)
    LPCTSTR WindowName = L"DX12";

    // title of the window
    LPCTSTR WindowTitle = L"Learn DX12";

    int Width = 800;
    int Height = 600;

    // is window full screen?
    bool FullScreen = false;

    if (FullScreen)
    {
        HMONITOR hmon = MonitorFromWindow(hwnd,
            MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(hmon, &mi);

        Width = mi.rcMonitor.right - mi.rcMonitor.left;
        Height = mi.rcMonitor.bottom - mi.rcMonitor.top;
    }

    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WindowName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Error registering class",
            L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    hwnd = CreateWindowEx(NULL,
        WindowName,
        WindowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        Width, Height,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hwnd)
    {
        MessageBox(NULL, L"Error creating window",
            L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    if (FullScreen)
    {
        SetWindowLong(hwnd, GWL_STYLE, 0);
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return true;
}

void MainLoop()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // run game code
        }
    }
}