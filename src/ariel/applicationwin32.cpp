/*
 * Copyright (c) 2016-2020 lymastee, All rights reserved.
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

#include <ariel/applicationwin32.h>
#include <ariel/framesys.h>
#include <ariel/scene.h>

__ariel_begin__

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

void set_execute_path_as_directory()
{
    gchar dir[MAX_PATH];
    GetModuleFileName(0, dir, _countof(dir));
    int len = strtool::length(dir);
    gchar* slash = dir + len - 1;
    for(; slash != dir; slash --) {
        if(slash[0] == _t('/') || slash[0] == _t('\\'))
            break;
    }
    slash[0] = 0;
    SetCurrentDirectory(dir);
}

void init_application_environment(app_env& env, HINSTANCE hinst, HINSTANCE hprevinst, const gchar* argv[], int argc)
{
    env.hinst = hinst;
    env.hprevinst = hprevinst;
    env.arglist.clear();
    for(int i = 0; i < argc; i ++)
        env.arglist.push_back(string(argv[i]));
}

bool app_data::install(const app_config& cfg, const app_env& env)
{
    this->wndproc = (fnwndproc)cfg.window_proc;
    if(!this->wndproc)
        this->wndproc = default_wndproc;

    WNDCLASSEX wcex;
    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize         = sizeof(wcex);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)this->wndproc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = env.hinst;
    wcex.hIcon          = LoadIcon(env.hinst, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(0, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOWFRAME);
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = cfg.class_name;
    wcex.hIconSm        = LoadIcon(env.hinst, IDI_WINLOGO);
    if(!RegisterClassEx(&wcex)) {
        assert(!"register class failed.");
        return false;
    }

    RECT rc = { cfg.position.left, cfg.position.top, cfg.position.right, cfg.position.bottom };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindow(cfg.class_name, cfg.window_name, WS_OVERLAPPEDWINDOW,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        0, 0, env.hinst, 0
    );
    if(!hwnd) {
        assert(!"create window failed.");
        return false;
    }
    this->hwnd = hwnd;
    this->arglist = env.arglist;
    /* set to framesys */
    framesys* sys = framesys::get_framesys();
    assert(sys);
    system_context ctx;
    ctx.hwnd = (uint)hwnd;
    ctx.hinst = (uint)env.hinst;
    ctx.mask = system_context::sct_hwnd | system_context::sct_hinst;
    sys->initialize(ctx);
    scene* scn = scene::get_singleton_ptr();
    assert(scn);
    rect client_rc;
    GetClientRect(hwnd, (LPRECT)&client_rc);
    sys->initialize(client_rc);
    scn->set_rendersys(sys->get_rendersys());
    scn->set_rose(sys->get_rose()); // todo: remove
    scn->setup();
    ShowWindow(hwnd, SW_SHOW);
    return true;
}

int app_data::run()
{
    framesys* sys = framesys::get_framesys();
    assert(sys);
    return sys->run();
}

bool application::setup(const app_config& cfg, const app_env& env)
{
    if(_data)
        return false;
    _data = new app_data;
    assert(_data);
    if(!_data->install(cfg, env))
        return false;
    set_execute_path_as_directory();
    return true;
}

void application::destroy()
{
    if(_data) {
        delete _data;
        _data = nullptr;
    }
}

int application::run()
{
    if(_data)
        return _data->run();
    return default_loop();
}

int application::default_loop()
{
    MSG msg;
    while(GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

__ariel_end__
