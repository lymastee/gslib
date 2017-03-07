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

#ifndef entrywin32_c99dd31d_a655_463f_95d6_082845c49699_h
#define entrywin32_c99dd31d_a655_463f_95d6_082845c49699_h

#include <windows.h>
#include <pink/type.h>

typedef LRESULT (__stdcall *fnwndproc)(HWND, UINT, WPARAM, LPARAM);
extern LRESULT __stdcall wndproc(HWND, UINT, WPARAM, LPARAM);

struct gs_app_config
{
    gs::gchar       class_name[128];
    gs::gchar       window_name[128];
    gs::pink::rect  position;
    fnwndproc       window_proc;
    /* more .. */
};

extern void gs_app_setup(gs_app_config& cfg);
extern void gs_app_initialized(HINSTANCE, HINSTANCE, LPCTSTR, int);
extern void gs_app_windowed(HWND);
extern int gs_app_loop();
extern int gs_main();

#endif
