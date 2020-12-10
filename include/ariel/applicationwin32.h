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

#pragma once

#ifndef applicationwin32_3971e4cb_1be6_4066_b7ea_da679d40c987_h
#define applicationwin32_3971e4cb_1be6_4066_b7ea_da679d40c987_h

#include <gslib/string.h>
#include <gslib/std.h>
#include <ariel/application.h>
#include <windows.h>

#undef min
#undef max

__ariel_begin__

typedef LRESULT(__stdcall *fnwndproc)(HWND, UINT, WPARAM, LPARAM);
typedef list<string> arg_list;

extern void set_execute_path_as_directory();
extern void init_application_environment(app_env& env, HINSTANCE hinst, HINSTANCE hprevinst, const gchar* argv[], int argc);

struct app_data
{
    HINSTANCE           hinst = nullptr;
    HWND                hwnd = nullptr;
    fnwndproc           wndproc = nullptr;
    arg_list            arglist;

public:
    bool install(const app_config& cfg, const app_env& env);
    int run();
};

struct app_env
{
    HINSTANCE           hinst = nullptr;
    HINSTANCE           hprevinst = nullptr;
    arg_list            arglist;

public:
    void init(HINSTANCE hinst, HINSTANCE hprevinst, const gchar* argv[], int argc) { init_application_environment(*this, hinst, hprevinst, argv, argc); }
};

__ariel_end__

#endif
