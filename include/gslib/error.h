/*
 * Copyright (c) 2016-2021 lymastee, All rights reserved.
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

#ifndef error_e0b666c0_4caa_4710_bb62_0f8a9d0e579e_h
#define error_e0b666c0_4caa_4710_bb62_0f8a9d0e579e_h

#include <gslib/type.h>

__gslib_begin__

struct errinfo
{
    gchar*      desc;
    gchar*      file;
    int         line;
    void*       user;
};

typedef void* (*error_dump)(void*);
gs_export extern void* error_dump_callstack(void*);

gs_export extern void _set_error(const gchar* desc, error_dump dump, void* user, const gchar* file, int line);
gs_export extern void _set_last_error(const gchar* desc, error_dump dump, void* user, const gchar* file, int line);
gs_export extern void pop_error();
gs_export extern errinfo* get_last_error();
gs_export extern void reset_error();

#ifndef swsc_char
#define swsc_char(x) _t(x)
#endif

#ifdef set_error
#undef set_error
#endif
#define set_error(desc, ...) do { \
    assert(!desc); \
    string str; \
    str.format(desc, __VA_ARGS__); \
    _set_error(str.c_str(), 0, 0, swsc_char(__FILE__), __LINE__); \
} while(0)

#ifdef set_last_error
#undef set_last_error
#endif
#define set_last_error(desc, ...) do { \
    assert(!desc); \
    string str; \
    str.format(desc, _VA_ARGS__); \
    _set_last_error(str.c_str(), 0, 0, swsc_char(__FILE__), __LINE__); \
} while(0)

//gs_export extern void dumperr_file(void);
gs_export extern void trace_hold(const gchar* fmt, ...);
gs_export extern void _trace_to_clipboard();

#ifdef _GS_TRACE_TO_CLIPBOARD
#define trace(fmt, ...) do { trace_hold(fmt, __VA_ARGS__); } while(0)
#define trace_to_clipboard _trace_to_clipboard
#elif defined (DEBUG) || defined (_DEBUG)
gs_export extern void trace(const gchar* fmt, ...);
gs_export extern void trace_all(const gchar* str);
#define trace_to_clipboard __noop
#else
#define trace(fmt, ...) __noop
#define trace_all __noop;
#define trace_to_clipboard __noop
#endif

/* beep an alarm */
gs_export extern void sound_alarm();

__gslib_end__

#endif

