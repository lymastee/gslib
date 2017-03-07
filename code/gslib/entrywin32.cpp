/*
 * Copyright (c) 2016-2017 lymastee, All rights reserved.
 * Contact: lymastee@hotmail.com
 *
 * This file is part of the gslib project.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <assert.h>
#include <gslib/entrywin32.h>

using namespace gs;

int gs_app_loop()
{
    MSG msg;
    while(GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

static LRESULT __stdcall default_wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    switch(msg)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

static void setup_default_cfg(gs_app_config& cfg)
{
    memset(&cfg, 0, sizeof(cfg));
    cfg.window_proc = default_wndproc;
    /* more ... */
}

static void set_execute_path_as_directory()
{
    gchar dir[MAX_PATH];
    GetModuleFileName(0, dir, _countof(dir));
    int len = strtool::length(dir);
    gchar* slash = dir + len - 1;
    for( ; slash != dir; slash --) {
        if(slash[0] == _t('/') || slash[0] == _t('\\'))
            break;
    }
    slash[0] = 0;
    SetCurrentDirectory(dir);
}

#ifdef _UNICODE
int __stdcall wWinMain
#else
int __stdcall WinMain
#endif
    (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
{
    using namespace gs;
    gs_app_config cfg;
    set_execute_path_as_directory();
    setup_default_cfg(cfg);
    gs_app_setup(cfg);
    gs_app_initialized(hInstance, hPrevInstance, lpCmdLine, nShowCmd);

    WNDCLASSEX wcex;
    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize         = sizeof(wcex);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = cfg.window_proc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(0, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOWFRAME);
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = cfg.class_name;
    wcex.hIconSm        = LoadIcon(hInstance, IDI_WINLOGO);
    if(!RegisterClassEx(&wcex)) {
        assert(!"register class failed.");
        return 0;
    }

    RECT rc = { cfg.position.left, cfg.position.top, cfg.position.right, cfg.position.bottom };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hWnd = CreateWindow(cfg.class_name, cfg.window_name, WS_OVERLAPPEDWINDOW,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 
        0, 0, hInstance, 0
        );
    assert(hWnd);
    gs_app_windowed(hWnd);

    return gs_main();
}
