// OpenGLTest1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <GL/gl.h>			/* OpenGL header file */
#include <GL/glu.h>			/* OpenGL utilities header file */

#include "gravity.h"

#include <atomic>

#include "RuntimeConfig.h"
#include "WorldObjects.h"
#include "World.h"
#include "WorldView.h"
#include "MainController.h"

#include "Props.h"

#include "PngLogger.h"

#define MAX_LOADSTRING 100

using TMainController = gravity::MainController;

std::unique_ptr<TMainController> controller;

void Init()
{
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);
}

void Reshape(int width, int height)
{
	if (controller)
		controller->onViewportResize(width, height);
    glViewport(0, 0, width, height);
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //gluPerspective(60.0, (float)width / height, 0.001, 100.0);
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //glTranslatef(0.0f, 0.0f, -3.0f);
}

void Display(bool force)
{
    if (!controller)
        return;

    if (!force && !controller->IsUINeedsUpdate())
        return;

    //	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glClear(GL_COLOR_BUFFER_BIT);

    controller->DrawWorld();
    glFlush();

    SwapBuffers(controller->GetHDC());
}

void HandleKeyboard(WPARAM wParam)
{
    if (!controller)
        return;

	controller->OnKeyboard(wParam);
}

LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!controller)
        return LRESULT();

    static PAINTSTRUCT ps;

    switch (uMsg)
    {
    case WM_PAINT:
        Display(true);
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;

    case WM_SIZE:
        //glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
        //PostMessage(hWnd, WM_PAINT, 0, 0);
		Reshape(LOWORD(lParam), HIWORD(lParam));
        PostMessage(hWnd, WM_PAINT, 0, 0);
        return 0;

    case WM_CHAR:
        HandleKeyboard(wParam);
        return 0;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        /* if we don't set the capture we won't get mouse move
               messages when the mouse moves outside the window. */
        SetCapture(hWnd);
        //mx = LOWORD(lParam);
        //my = HIWORD(lParam);
        //if (uMsg == WM_LBUTTONDOWN)
        //	state |= PAN;
        //if (uMsg == WM_RBUTTONDOWN)
        //	state |= ROTATE;
        return 0;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        /* remember to release the capture when we are finished. */
        ReleaseCapture();
        //state = 0;
        return 0;

    case WM_MOUSEMOVE:
        //if (state) {
        //	omx = mx;
        //	omy = my;
        //	mx = LOWORD(lParam);
        //	my = HIWORD(lParam);
        //	/* Win32 is pretty braindead about the x, y position that
        //	   it returns when the mouse is off the left or top edge
        //	   of the window (due to them being unsigned). therefore,
        //	   roll the Win32's 0..2^16 pointer co-ord range to the
        //	   more amenable (and useful) 0..+/-2^15. */
        //	if (mx & 1 << 15) mx -= (1 << 16);
        //	if (my & 1 << 15) my -= (1 << 16);
        //	update(state, omx, mx, omy, my);
        //	PostMessage(hWnd, WM_PAINT, 0, 0);
        //}
        return 0;

    case WM_ACTIVATE:
        //pauseApp = IsIconic(hWnd);
        return 0;

    case WM_PALETTECHANGED:
        if (hWnd == (HWND)wParam)
            break;
        /* fall through to WM_QUERYNEWPALETTE */

    case WM_QUERYNEWPALETTE:
    {
        auto& hPalette = controller->GetHPalette();
        auto &hDC = controller->GetHDC();
        if (hPalette)
        {
            UnrealizeObject(hPalette);
            SelectPalette(hDC, hPalette, FALSE);
            RealizePalette(hDC);
            return TRUE;
        }
        return FALSE;
    }
    case WM_CLOSE:
        controller->Stop();
        PostQuitMessage(0);
        return 0;

    case WM_USER:
        Display(false);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND  CreateOpenGLWindow(const TCHAR* title, int x, int y, int width, int height, BYTE type, DWORD flags)
{
    static HINSTANCE hInstance = nullptr;

    if (!controller)
        return nullptr;

    /* only register the window class once - use hInstance as a flag. */
    WNDCLASS wc;

    if (!hInstance)
    {
        hInstance = GetModuleHandle(NULL);
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = (WNDPROC)WindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = _T("NNative");

        if (!RegisterClass(&wc))
        {
            MessageBox(NULL, _T("RegisterClass() failed:  "
                "Cannot register window class."), _T("Error"), MB_OK);
            return nullptr;
        }
    }

    HWND hWnd = CreateWindow(_T("NNative"), title, WS_OVERLAPPEDWINDOW |
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        x, y, width, height, NULL, NULL, hInstance, NULL);

    if (hWnd == NULL)
    {
        MessageBox(NULL, _T("CreateWindow() failed:  Cannot create a window."),
            _T("Error"), MB_OK);
        return NULL;
    }

    controller->SetHDC(GetDC(hWnd));

    auto& hDC = controller->GetHDC();

    /* there is no guarantee that the contents of the stack that become
       the pfd are zeroed, therefore _make sure_ to clear these bits. */
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
    pfd.iPixelType = type;
    pfd.cColorBits = 32;

    int pf = ChoosePixelFormat(hDC, &pfd);
    if (pf == 0)
    {
        MessageBox(NULL, _T("ChoosePixelFormat() failed:  "
            "Cannot find a suitable pixel format."), _T("Error"), MB_OK);
        return 0;
    }

    if (SetPixelFormat(hDC, pf, &pfd) == FALSE)
    {
        MessageBox(NULL, _T("SetPixelFormat() failed:  "
            "Cannot set format specified."), _T("Error"), MB_OK);
        return 0;
    }

    DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    if (pfd.dwFlags & PFD_NEED_PALETTE || pfd.iPixelType == PFD_TYPE_COLORINDEX)
    {
        int n = 1 << pfd.cColorBits;
        if (n > 256) n = 256;
        
        std::vector<unsigned char> lpPalMem(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * n, 0);

        LOGPALETTE* lpPal = reinterpret_cast<LOGPALETTE*>(&lpPalMem[0]);
        lpPal->palVersion = 0x300;
        lpPal->palNumEntries = n;

        GetSystemPaletteEntries(hDC, 0, n, &lpPal->palPalEntry[0]);

        /* if the pixel type is RGBA, then we want to make an RGB ramp,
           otherwise (color index) set individual colors. */
        if (pfd.iPixelType == PFD_TYPE_RGBA)
        {
            int redMask = (1 << pfd.cRedBits) - 1;
            int greenMask = (1 << pfd.cGreenBits) - 1;
            int blueMask = (1 << pfd.cBlueBits) - 1;

            /* fill in the entries with an RGB color ramp. */
            for (int i = 0; i < n; ++i)
            {
                lpPal->palPalEntry[i].peRed =
                    (((i >> pfd.cRedShift)   & redMask) * 255) / redMask;
                lpPal->palPalEntry[i].peGreen =
                    (((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
                lpPal->palPalEntry[i].peBlue =
                    (((i >> pfd.cBlueShift)  & blueMask) * 255) / blueMask;
                lpPal->palPalEntry[i].peFlags = 0;
            }
        }
        else
        {
            lpPal->palPalEntry[0].peRed = 0;
            lpPal->palPalEntry[0].peGreen = 0;
            lpPal->palPalEntry[0].peBlue = 0;
            lpPal->palPalEntry[0].peFlags = PC_NOCOLLAPSE;
            lpPal->palPalEntry[1].peRed = 255;
            lpPal->palPalEntry[1].peGreen = 0;
            lpPal->palPalEntry[1].peBlue = 0;
            lpPal->palPalEntry[1].peFlags = PC_NOCOLLAPSE;
            lpPal->palPalEntry[2].peRed = 0;
            lpPal->palPalEntry[2].peGreen = 255;
            lpPal->palPalEntry[2].peBlue = 0;
            lpPal->palPalEntry[2].peFlags = PC_NOCOLLAPSE;
            lpPal->palPalEntry[3].peRed = 0;
            lpPal->palPalEntry[3].peGreen = 0;
            lpPal->palPalEntry[3].peBlue = 255;
            lpPal->palPalEntry[3].peFlags = PC_NOCOLLAPSE;
        }

        controller->SetHPalette(CreatePalette(lpPal));

        auto& hPalette = controller->GetHPalette();
        if (hPalette)
        {
            SelectPalette(hDC, hPalette, FALSE);
            RealizePalette(hDC);
        }
    }

    ReleaseDC(hWnd, hDC);

    return hWnd;
}


int APIENTRY wWinMain(_In_ HINSTANCE hCurrentInst, _In_opt_ HINSTANCE hPreviousInst, _In_ LPWSTR lpszCmdLine, _In_ int nCmdShow)
{
    if (wcsstr(lpszCmdLine, L"-burn") == nullptr)
    {
        ::SetPriorityClass(::GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
    }

    gravity::RuntimeConfig config;

    controller = std::make_unique<TMainController>(config);

    controller->SetHWND(
        CreateOpenGLWindow(_T("Gravity Sandbox"), 0, 0,
			gravity::props::ViewPortWidth, gravity::props::ViewPortHeight,
            PFD_TYPE_RGBA, PFD_DOUBLEBUFFER));

    if (controller->GetHWND() == nullptr)
        return 1;

    auto& hDC = controller->GetHDC();
    auto& hPalette = controller->GetHPalette();

    HGLRC hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    ShowWindow(controller->GetHWND(), SW_SHOW);
    UpdateWindow(controller->GetHWND());

    Init();

    controller->Start();

    MSG msg;
    while (!controller->IsTerminating() && GetMessage(&msg, controller->GetHWND(), 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    controller->Stop();

    wglMakeCurrent(NULL, NULL);
    ReleaseDC(controller->GetHWND(), hDC);
    wglDeleteContext(hRC);
    DestroyWindow(controller->GetHWND());

    if (hPalette)
        DeleteObject(hPalette);

    return 0;
}

