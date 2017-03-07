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
#include <atlbase.h>
#include <atlwin.h>
#include <imm.h>
#include <pink/widget.h>
#include <pink/dataxchg.h>
#include <pink/wsysdrvwin32.h>
#include <gslib/entrywin32.h>

#pragma comment(lib, "imm32.lib")

__pink_begin__

static wsysdrvwin32* __win32drv_ptr = 0;

wsysdrvwin32::wsysdrvwin32()
{
    _realwnd = 0;
    _realinst = 0;
    _realdc = 0;
    _memdc = 0;
    _membmp = 0;
    _imc = 0;
    _notify = 0;
    assert(!__win32drv_ptr);
    __win32drv_ptr = this;
    memset(&_immpos, 0, sizeof(_immpos));
    memset(&_immfont, 0, sizeof(_immfont));
}

wsysdrvwin32::~wsysdrvwin32()
{
    if(_memdc) {
        assert(_membmp);
        SelectObject(_memdc, _oldbmp);
        DeleteObject(_membmp);
        DeleteDC(_memdc);
        _memdc = 0;
        _membmp = 0;
    }
}

void wsysdrvwin32::initialize(const system_context& ctx)
{
    if(ctx.mask & system_context::sct_notify)
        _notify = ctx.notify;
    if(ctx.mask & system_context::sct_painter)
        _painter = reinterpret_cast<painter*>(ctx.painter);
    if(ctx.mask & system_context::sct_rectangle) {
        assert(_realwnd);
        rect rc = ctx.rectangle;
        AdjustWindowRect((LPRECT)&rc, WS_OVERLAPPEDWINDOW, FALSE);
        MoveWindow(_realwnd, rc.left, rc.top, rc.width(), rc.height(), FALSE);
    }
}

void wsysdrvwin32::setup()
{
    rect rc;
    GetClientRect(_realwnd, (LPRECT)&rc);
    SetWindowLong(_realwnd, GWL_WNDPROC, (LONG)wndproc);
    assert(_notify);
    _notify->on_create(this, rc);

    /* create memory dc */
    assert(!_memdc && !_membmp);
    HDC hdc = GetDC(_realwnd);
    _memdc = CreateCompatibleDC(hdc);
    _membmp = CreateCompatibleBitmap(hdc, rc.width(), rc.height());
    _oldbmp = (HBITMAP)SelectObject(_memdc, _membmp);

    /* initilize */
    ShowWindow(_realwnd, SW_SHOW);
    UpdateWindow(_realwnd);
}

// void wsysdrvwin32::initialize(wsys_notify* ptr, void* paint, const rect& rc)
// {
//     rect rc1 = rc;
//     AdjustWindowRect((LPRECT)&rc1, WS_OVERLAPPEDWINDOW, FALSE);
//     MoveWindow(_realwnd, rc1.left, rc1.top, rc1.width(), rc1.height(), FALSE);
//     _painter = reinterpret_cast<painter*>(paint);
//     _notify = ptr;
//     GetClientRect(_realwnd, (LPRECT)&rc1);
//     SetWindowLong(_realwnd, GWL_WNDPROC, (LONG)wndproc);
//     _notify->on_create(this, rc1);
// 
//     /* create memory dc */
//     assert(!_memdc && !_membmp);
//     HDC hdc = GetDC(_realwnd);
//     _memdc = CreateCompatibleDC(hdc);
//     _membmp = CreateCompatibleBitmap(hdc, rc1.width(), rc1.height());
//     _oldbmp = (HBITMAP)SelectObject(_memdc, _membmp);
// 
//     /* initilize */
//     ShowWindow(_realwnd, SW_SHOW);
//     UpdateWindow(_realwnd);
// }

void wsysdrvwin32::close()
{
    /* DONOT use SendMessage! */
    if(_realwnd) {
        PostMessage(_realwnd, WM_DESTROY, 0, 0);
        _realwnd = 0;
    }
}

void wsysdrvwin32::set_timer(uint tid, int t)
{
    assert(_realwnd);
    SetTimer(_realwnd, tid, t, 0);
}

void wsysdrvwin32::kill_timer(uint tid)
{
    assert(_realwnd);
    KillTimer(_realwnd, tid);
}

void wsysdrvwin32::update()
{
    const painter& paint = *_painter;
    const dirty_list* plist = paint.get_dirty();
    assert(plist);
    if(!plist->is_whole() && !plist->size())
        return;

    HDC hdc = _realdc;
    if(!hdc) {
        if(!(hdc = GetDC(_realwnd)))
            return;
    }

    image* img = paint.get_image();
    assert(img);
    if(plist->is_whole()) {
        rect rc(0, 0, img->get_width(), img->get_height());
        int rvsy = paint.get_height() - rc.bottom;
        SetDIBitsToDevice(_memdc, rc.left, rc.top, rc.width(), rc.height(), 
            rc.left, rvsy, rvsy, rc.height(), img->get_color(rvsy, false), (BITMAPINFO*)img->get_header(), DIB_RGB_COLORS
            );
    }
    else {
        BITMAPINFO* pinfo = (BITMAPINFO*)img->get_header();
        for(dirty_list::const_iterator i = plist->begin(); i != plist->end(); ++ i) {
            const rect& rc = *i;
            int rvsy = paint.get_height() - rc.bottom;
            assert(img->get_color(rvsy, false));
            SetDIBitsToDevice(_memdc, rc.left, rc.top, rc.width(), rc.height(), 
                rc.left, rvsy, rvsy, rc.height(), img->get_color(rvsy, false), pinfo, DIB_RGB_COLORS
                );
        }
    }
    BitBlt(hdc, 0, 0, img->get_width(), img->get_height(), _memdc, 0, 0, SRCCOPY);

    if(!_realdc)
        ReleaseDC(_realwnd, hdc);
}

void wsysdrvwin32::emit(int msgid, void* msg, int size)
{
    assert(_realwnd);
    PostMessage(_realwnd, msgid, (WPARAM)msg, (LPARAM)size);
}

void wsysdrvwin32::set_ime(point pt, const font& ft)
{
    memset(&_immpos, 0, sizeof(_immpos));
    memset(&_immfont, 0, sizeof(_immfont));
    _immpos.dwStyle = CFS_POINT;
    _immpos.ptCurrentPos.x = pt.x;
    _immpos.ptCurrentPos.y = pt.y;
    _immfont.lfCharSet = DEFAULT_CHARSET;
    _immfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    _immfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    _immfont.lfQuality = DEFAULT_QUALITY;
    _immfont.lfPitchAndFamily = DEFAULT_PITCH;
    _immfont.lfHeight = ft.height;
    _immfont.lfWidth = ft.width;
    _immfont.lfEscapement = ft.escape;
    _immfont.lfOrientation = ft.orient;
    _immfont.lfWeight = ft.weight<10 ? ft.weight : ft.weight%10*100;
    if(_imc) {
        ImmSetCompositionWindow(_imc, &_immpos);
        ImmSetCompositionFont(_imc, &_immfont);
    }
}

// void wsysdrvwin32::set_clipboard(const gchar* fmt, const void* ptr, int size)
// {
//     BOOL b = OpenClipboard(_realwnd);
//     assert(b);
//     if(lstrcmp(fmt, clipfmt_text) == 0) {
//         b = EmptyClipboard();
//         assert(b);
//         HGLOBAL hcpy = GlobalAlloc(GMEM_MOVEABLE, size);
//         assert(hcpy);
//         void* des = GlobalLock(hcpy);
//         memcpy_s(des, size, ptr, size);
//         ((gchar*)des)[size-1] = 0;
//         GlobalUnlock(hcpy);
//         SetClipboardData(CF_TEXT, hcpy);
//     }
//     CloseClipboard();
// }
// 
// int wsysdrvwin32::get_clipboard(const gchar* fmt, const void*& ptr)
// {
//     BOOL b = OpenClipboard(_realwnd);
//     assert(b);
//     if(lstrcmp(fmt, clipfmt_text) == 0) {
//         HANDLE h = GetClipboardData(CF_TEXT);
//         ptr = GlobalLock(h);
//         GlobalUnlock(h);
//         CloseClipboard();
//         return ptr ? lstrlen((gchar*)ptr) : 0;
//     }
//     CloseClipboard();
//     return 0;
// }
// 
// int wsysdrvwin32::get_clipboard(clipboard_list& cl, int c)
// {
//     assert(_realwnd);
//     return _get_clipdata_by_hwnd(cl, c, _realwnd);
// }

__pink_end__

using namespace gs;
using namespace pink;

void gs_app_setup(gs_app_config& cfg)
{
    strtool::copy(cfg.class_name, _countof(cfg.class_name), _t("wsysdrvwin32"));
    strtool::copy(cfg.window_name, _countof(cfg.window_name), _t(""));
    cfg.position = rect(100, 100, 5, 5);
}

void gs_app_initialized(HINSTANCE hinst, HINSTANCE hprev, LPCTSTR cmd, int show)
{
    wsysdrvwin32::get_singleton_ptr()->_realinst= hinst;
}

void gs_app_windowed(HWND hwnd)
{
    wsysdrvwin32::get_singleton_ptr()->_realwnd = hwnd;
}

static void wsysdrvwin32_getpos(LPARAM lParam, point& pt)
{
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
}

static void wsysdrvwin32_getsize(LPARAM lParam, rect& rc)
{
    rc.left = 0;
    rc.top = 0;
    rc.right = LOWORD(lParam);
    rc.bottom = HIWORD(lParam);
}

static void wsysdrvwin32_gettagpos(LPARAM lParam, rect& rc)
{
    LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;
    rc.left = 0;
    rc.top = 0;
    rc.right = pwp->cx;
    rc.bottom = pwp->cy;
}

static void wsysdrvwin32_getmskkey(WPARAM wParam, uint& um)
{
    um = 0;
    if(wParam & MK_LBUTTON)
        um |= um_lmouse;
    if(wParam & MK_RBUTTON)
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

static bool wsysdrvwin32_getchar(WPARAM wParam, uint& ch)
{
    if(wParam >= 0x20 && wParam < 0x7F) {
        ch = wParam;
        return true;
    }
    else if(wParam == 0x09) {
        ch = wParam;
        return true;
    }
    else if(wParam == 0x0A || wParam == 0x0D) {
        /* change cr to lf */
        ch = 0x0A;
        return true;
    }
    return false;
}

static bool wsysdrvwin32_getkey(WPARAM wParam, unikey& uk)
{
    if(wParam >= 0x30 && wParam < 0x7F) {
        uk = (unikey)wParam;
        return true;
    }
    switch(wParam)
    {
    case VK_BACK:
    case VK_TAB:
    case VK_RETURN:
    case VK_ESCAPE:
    case VK_SPACE:
        uk = (unikey)wParam;
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
        uk = (unikey)(vk_f1 + (wParam - VK_F1));
        return true;
    }
    return false;
}

LRESULT __stdcall wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT ret = 0;

    switch(msg)
    {
    case WM_TIMER:
        {
            __win32drv_ptr->_notify->on_timer((uint)wParam);
            break;
        }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            __win32drv_ptr->_realdc = hdc;
            __win32drv_ptr->_notify->on_paint((const rect&)(ps.rcPaint));
            EndPaint(hWnd, &ps);
            __win32drv_ptr->_realdc = 0;
            break;
        }
    case WM_MOUSEMOVE:
        {
            point pt;
            uint um;
            wsysdrvwin32_getpos(lParam, pt);
            wsysdrvwin32_getmskkey(wParam, um);
            __win32drv_ptr->_notify->on_move(um, pt);
            ret = DefWindowProc(hWnd, msg, wParam, lParam);
            break;
        }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        {
            point pt;
            uint um;
            wsysdrvwin32_getpos(lParam, pt);
            wsysdrvwin32_getmskkey(wParam, um);
            __win32drv_ptr->_notify->on_mouse_down(um, msg==WM_LBUTTONDOWN?mk_left:mk_right, pt);
            ret = DefWindowProc(hWnd, msg, wParam, lParam);
            SetCapture(hWnd);
            break;
        }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        {
            point pt;
            uint um;
            wsysdrvwin32_getpos(lParam, pt);
            wsysdrvwin32_getmskkey(wParam, um);
            __win32drv_ptr->_notify->on_mouse_up(um, msg==WM_LBUTTONUP?mk_left:mk_right, pt);
            ret = DefWindowProc(hWnd, msg, wParam, lParam);
            ReleaseCapture();
            break;
        }
    case WM_ERASEBKGND:
        break;
    case WM_SHOWWINDOW:
        {
            ret = DefWindowProc(hWnd, msg, wParam, lParam);
            if(!wParam)
                ReleaseCapture();
            __win32drv_ptr->_notify->on_show(wParam?true:false);
            break;
        }
    case WM_DESTROY:
        {
            __win32drv_ptr->_notify->on_close();
            ImmDisableIME(0);
            PostQuitMessage(0);
            break;
        }
    case WM_SIZE:
        {
            ret = DefWindowProc(hWnd, msg, wParam, lParam);
            rect rc;
            wsysdrvwin32_getsize(lParam, rc);
            __win32drv_ptr->_notify->on_size(rc);
            break;
        }
    case WM_KEYDOWN:
        {
            uint um;
            unikey uk;
            wsysdrvwin32_getmskkey(0, um);
            if(wsysdrvwin32_getkey(wParam, uk))
                __win32drv_ptr->_notify->on_key_down(um, uk);
            break;
        }
    case WM_KEYUP:
        {
            uint um;
            unikey uk;
            wsysdrvwin32_getmskkey(0, um);
            if(wsysdrvwin32_getkey(wParam, uk))
                __win32drv_ptr->_notify->on_key_up(um, uk);
            break;
        }
    case WM_CHAR:
        {
            uint um, ch;
            if(wsysdrvwin32_getchar(wParam, ch)) {
                wsysdrvwin32_getmskkey(0, um);
                __win32drv_ptr->_notify->on_char(um, ch);
            }
            break;
        }
    case WM_IME_CHAR:
        {
            __win32drv_ptr->_notify->on_char(0, (uint)wParam);
            break;
        }
    case WM_IME_STARTCOMPOSITION:
        {
            __win32drv_ptr->_imc = ImmGetContext(__win32drv_ptr->_realwnd);
            if(__win32drv_ptr->_imc) {
                ImmSetCompositionWindow(__win32drv_ptr->_imc, &__win32drv_ptr->_immpos);
                ImmSetCompositionFont(__win32drv_ptr->_imc, &__win32drv_ptr->_immfont);
            }
            /*
             * Importantly, should resend this message to the default composition window here, 
             * I've seen a lot of applications ignore this message that the composition window 
             * would disappear.
             */
            ret = DefWindowProc(hWnd, msg, wParam, lParam);
            break;
        }
    case WM_IME_NOTIFY:
        {
            if(wParam == IMN_OPENCANDIDATE) {
                __win32drv_ptr->_imc = ImmGetContext(__win32drv_ptr->_realwnd);
                if(__win32drv_ptr->_imc) {
                    ImmSetCompositionWindow(__win32drv_ptr->_imc, &__win32drv_ptr->_immpos);
                    ImmSetCompositionFont(__win32drv_ptr->_imc, &__win32drv_ptr->_immfont);
                }
            }
            else if(wParam == IMN_CLOSECANDIDATE)
                __win32drv_ptr->_imc = 0;
            ret = DefWindowProc(hWnd, msg, wParam, lParam);
            break;
        }
    default:
        {
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }
    }

    return ret;
}
