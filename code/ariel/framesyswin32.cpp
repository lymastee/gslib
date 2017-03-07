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
#include <windows.h>
#include <windowsx.h>
#include <imm.h>
#include <gslib/entrywin32.h>
#include <gslib/string.h>
#include <ariel/framesys.h>
#include <ariel/scene.h>
#include <ariel/rendersysd3d11.h>

#pragma comment(lib, "imm32.lib")

__ariel_begin__

static HWND                 __frame_hwnd = 0;
static HINSTANCE            __frame_hinst = 0;
static HIMC                 __frame_imc = 0;
static COMPOSITIONFORM      __frame_immpos;
static LOGFONT              __frame_immfont;

int framesys::run()
{
    MSG msg = { 0 };
    while(msg.message != WM_QUIT) {
        if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            framesys* frmsys = framesys::get_framesys();
            assert(frmsys);
            frmsys->idle();
        }
    }
    return (int)msg.wParam;
}

void framesys::initialize(const system_context& ctx)
{
    _configs.handles = (uint)__frame_hwnd;
    if(ctx.mask & system_context::sct_notify)
        _notify = ctx.notify;
    if(ctx.mask & system_context::sct_rectangle) {
        _configs.width = ctx.rectangle.width();
        _configs.height = ctx.rectangle.height();
    }
    if(ctx.mask & system_context::sct_hwnd)
        _configs.handles = ctx.hwnd;
}

void framesys::setup()
{
    assert(__frame_hwnd);
    RECT rc;
    GetWindowRect(__frame_hwnd, &rc);
    rc.right = rc.left + _configs.width;
    rc.bottom = rc.top + _configs.height;
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    GetClientRect(__frame_hwnd, &rc);
    if(_notify)
        _notify->on_create(this, (const rect&)rc);
    ShowWindow(__frame_hwnd, SW_SHOW);
    UpdateWindow(__frame_hwnd);
}

void framesys::close()
{
    if(__frame_hwnd) {
        PostMessage(__frame_hwnd, WM_DESTROY, 0, 0);
        __frame_hwnd = 0;
    }
}

void framesys::set_timer(uint tid, int t)
{
    assert(__frame_hwnd);
    SetTimer(__frame_hwnd, tid, t, 0);
}

void framesys::kill_timer(uint tid)
{
    assert(__frame_hwnd);
    KillTimer(__frame_hwnd, tid);
}

void framesys::update()
{
}

void framesys::emit(int msgid, void* msg, int size)
{
    assert(__frame_hwnd);
    PostMessage(__frame_hwnd, msgid, (WPARAM)msg, (LPARAM)size);
}

void framesys::set_ime(point pt, const font& ft)
{
    memset(&__frame_immpos, 0, sizeof(__frame_immpos));
    memset(&__frame_immfont, 0, sizeof(__frame_immfont));
    __frame_immpos.dwStyle = CFS_POINT;
    __frame_immpos.ptCurrentPos.x = pt.x;
    __frame_immpos.ptCurrentPos.y = pt.y;
    __frame_immfont.lfCharSet = DEFAULT_CHARSET;
    __frame_immfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    __frame_immfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    __frame_immfont.lfQuality = DEFAULT_QUALITY;
    __frame_immfont.lfPitchAndFamily = DEFAULT_PITCH;
    __frame_immfont.lfHeight = ft.height;
    __frame_immfont.lfWidth = ft.width;
    __frame_immfont.lfEscapement = ft.escape;
    __frame_immfont.lfOrientation = ft.orient;
    __frame_immfont.lfWeight = ft.weight<10 ? ft.weight : ft.weight%10*100;
    if(__frame_imc) {
        ImmSetCompositionWindow(__frame_imc, &__frame_immpos);
        ImmSetCompositionFont(__frame_imc, &__frame_immfont);
    }
}

static void framesys_getpos(LPARAM lparam, point& pt)
{
    pt.x = GET_X_LPARAM(lparam);
    pt.y = GET_Y_LPARAM(lparam);
}

static void framesys_getsize(LPARAM lparam, rect& rc)
{
    rc.left = 0;
    rc.top = 0;
    rc.right = LOWORD(lparam);
    rc.bottom = HIWORD(lparam);
}

static void framesys_getmskkey(WPARAM wparam, uint& um)
{
    um = 0;
    if(wparam & MK_LBUTTON)
        um |= um_lmouse;
    if(wparam & MK_RBUTTON)
        um |= um_rmouse;
    if(GetKeyState(VK_LSHIFT) < 0)
        um |= um_shift;
    if(GetKeyState(VK_RSHIFT) < 0)
        um |= um_sshift;
    if(GetKeyState(VK_LCONTROL) < 0)
        um |= um_control;
    if(GetKeyState(VK_RCONTROL) < 0)
        um |= um_scontrol;
    if(GetKeyState(VK_LMENU) < 0)
        um |= um_alter;
    if(GetKeyState(VK_RMENU) < 0)
        um |= um_salter;
}

static bool framesys_getchar(WPARAM wparam, uint& ch)
{
    if(wparam >= 0x20 && wparam < 0x7F) {
        ch = wparam;
        return true;
    }
    else if(wparam == 0x09) {
        ch = wparam;
        return true;
    }
    else if(wparam == 0x0A || wparam == 0x0D) {
        /* change cr to lf */
        ch = 0x0A;
        return true;
    }
    return false;
}

static bool framesys_getkey(WPARAM wparam, unikey& uk)
{
    if(wparam >= 0x30 && wparam < 0x7F) {
        uk = (unikey)wparam;
        return true;
    }
    switch(wparam)
    {
    case VK_BACK:
    case VK_TAB:
    case VK_RETURN:
    case VK_ESCAPE:
    case VK_SPACE:
        uk = (unikey)wparam;
        return true;
        /* need translate */
    case VK_SHIFT:
        uk = vk_shift;
        return true;
    case VK_CONTROL:
        uk = vk_control;
        return true;
    case VK_MENU:
        uk = vk_alter;
        return true;
    case VK_CAPITAL:
        uk = vk_caps;
        return true;
    case VK_PRIOR:
        uk = vk_pageup;
        return true;
    case VK_NEXT:
        uk = vk_pagedown;
        return true;
    case VK_END:
        uk = vk_end;
        return true;
    case VK_HOME:
        uk = vk_home;
        return true;
    case VK_LEFT:
        uk = vk_left;
        return true;
    case VK_UP:
        uk = vk_up;
        return true;
    case VK_RIGHT:
        uk = vk_right;
        return true;
    case VK_DOWN:
        uk = vk_down;
        return true;
    case VK_INSERT:
        uk = vk_insert;
        return true;
    case VK_DELETE:
        uk = uk_del;
        return true;
        /* translate 2 */
    case VK_F1:
    case VK_F2:
    case VK_F3:
    case VK_F4:
    case VK_F5:
    case VK_F6:
    case VK_F7:
    case VK_F8:
    case VK_F9:
    case VK_F10:
    case VK_F11:
    case VK_F12:
        uk = (unikey)(vk_f1 + (wparam - VK_F1));
        return true;
    }
    return false;
}

static system_notify* get_frame_notify()
{
    framesys* frmsys = framesys::get_framesys();
    assert(frmsys);
    return frmsys->get_notify();
}

static LRESULT __stdcall frame_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT ret = 0;
    switch(msg)
    {
    case WM_TIMER:
        {
            get_frame_notify()->on_timer((uint)wparam);
            break;
        }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            get_frame_notify()->on_paint((const rect&)ps.rcPaint);
            EndPaint(hwnd, &ps);
            break;
        }
    case WM_MOUSEMOVE:
        {
            point pt;
            uint um;
            framesys_getpos(lparam, pt);
            framesys_getmskkey(wparam, um);
            get_frame_notify()->on_move(um, pt);
            ret = DefWindowProc(hwnd, msg, wparam, lparam);
            break;
        }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        {
            point pt;
            uint um;
            framesys_getpos(lparam, pt);
            framesys_getmskkey(wparam, um);
            get_frame_notify()->on_mouse_down(um, msg==WM_LBUTTONDOWN?mk_left:mk_right, pt);
            ret = DefWindowProc(hwnd, msg, wparam, lparam);
            SetCapture(hwnd);
            break;
        }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        {
            point pt;
            uint um;
            framesys_getpos(lparam, pt);
            framesys_getmskkey(wparam, um);
            get_frame_notify()->on_mouse_up(um, msg==WM_LBUTTONUP?mk_left:mk_right, pt);
            ret = DefWindowProc(hwnd, msg, wparam, lparam);
            ReleaseCapture();
            break;
        }
    case WM_ERASEBKGND:
        break;
    case WM_SHOWWINDOW:
        {
            ret = DefWindowProc(hwnd, msg, wparam, lparam);
            if(!wparam)
                ReleaseCapture();
            get_frame_notify()->on_show(wparam?true:false);
            break;
        }
    case WM_DESTROY:
        {
            get_frame_notify()->on_close();
            ImmDisableIME(0);
            PostQuitMessage(0);
            break;
        }
    case WM_SIZE:
        {
            ret = DefWindowProc(hwnd, msg, wparam, lparam);
            rect rc;
            framesys_getsize(lparam, rc);
            get_frame_notify()->on_size(rc);
            break;
        }
    case WM_KEYDOWN:
        {
            uint um;
            unikey uk;
            framesys_getmskkey(0, um);
            if(framesys_getkey(wparam, uk))
                get_frame_notify()->on_key_down(um, uk);
            break;
        }
    case WM_KEYUP:
        {
            uint um;
            unikey uk;
            framesys_getmskkey(0, um);
            if(framesys_getkey(wparam, uk))
                get_frame_notify()->on_key_up(um, uk);
            break;
        }
    case WM_CHAR:
        {
            uint um, ch;
            if(framesys_getchar(wparam, ch)) {
                framesys_getmskkey(0, um);
                get_frame_notify()->on_char(um, ch);
            }
            break;
        }
    case WM_IME_CHAR:
        {
            get_frame_notify()->on_char(0, (uint)wparam);
            break;
        }
    case WM_IME_STARTCOMPOSITION:
        {
            __frame_imc = ImmGetContext(__frame_hwnd);
            if(__frame_imc) {
                ImmSetCompositionWindow(__frame_imc, &__frame_immpos);
                ImmSetCompositionFont(__frame_imc, &__frame_immfont);
            }
            /*
             * Importantly, should resend this message to the default composition window here, 
             * I've seen a lot of applications ignore this message that the composition window 
             * would disappear.
             */
            ret = DefWindowProc(hwnd, msg, wparam, lparam);
            break;
        }
    case WM_IME_NOTIFY:
        {
            if(wparam == IMN_OPENCANDIDATE) {
                __frame_imc = ImmGetContext(__frame_hwnd);
                if(__frame_imc) {
                    ImmSetCompositionWindow(__frame_imc, &__frame_immpos);
                    ImmSetCompositionFont(__frame_imc, &__frame_immfont);
                }
            }
            else if(wparam == IMN_CLOSECANDIDATE)
                __frame_imc = 0;
            ret = DefWindowProc(hwnd, msg, wparam, lparam);
            break;
        }
    default:
        {
            ret = DefWindowProc(hwnd, msg, wparam, lparam);
            break;
        }
    }
    return ret;
}

__ariel_end__

using namespace gs;
using namespace ariel;

void gs_app_setup(gs_app_config& cfg)
{
    strtool::copy(cfg.class_name, _countof(cfg.class_name), _t("rendersysd11"));
    strtool::copy(cfg.window_name, _countof(cfg.window_name), _t(""));
    cfg.window_proc = frame_window_proc;
    cfg.position = rect(50, 50, 640, 480);
}

void gs_app_initialized(HINSTANCE hinst, HINSTANCE, LPCTSTR, int)
{
    /* record hinstance */
    __frame_hinst = hinst;
}

void gs_app_windowed(HWND hwnd)
{
    /* record hwnd */
    __frame_hwnd = hwnd;
    framesys* sys = framesys::get_framesys();
    scene* scn = scene::get_singleton_ptr();
    rect rc;
    GetClientRect(hwnd, (LPRECT)&rc);
    sys->initialize(rc);
    scn->set_rendersys(sys->get_rendersys());
    scn->set_rose(sys->get_rose());
    scn->setup();
    ShowWindow(hwnd, SW_SHOW);
}
