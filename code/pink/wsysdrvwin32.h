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

#ifndef wsysdrvwin32_da682ed0_7df4_4f15_8cb5_2dc272369de3_h
#define wsysdrvwin32_da682ed0_7df4_4f15_8cb5_2dc272369de3_h

#include <windows.h>
#include <pink/widget.h>

__pink_begin__

class wsysdrvwin32:
    public wsys_driver
{
public:
    virtual ~wsysdrvwin32();
    virtual void initialize(const system_context& ctx) override;
    virtual void setup() override;
    virtual void close() override;
    virtual void set_timer(uint tid, int t) override;
    virtual void kill_timer(uint tid) override;
    virtual void update(/*const painter& cvs*/) override;
    virtual void emit(int msgid, void* msg, int size) override;
    virtual void set_ime(point pt, const font& ft) override;
    //void set_clipboard(const gchar* fmt, const void* ptr, int size) override;
    //int get_clipboard(const gchar* fmt, const void*& ptr) override;
    //int get_clipboard(clipboard_list& cl, int c) override;

private:
    wsysdrvwin32();

public:
    static wsysdrvwin32* get_singleton_ptr()
    {
        static wsysdrvwin32 inst;
        return &inst;
    }

public:
    HWND            _realwnd;
    HINSTANCE       _realinst;
    HDC             _realdc;
    HIMC            _imc;
    point           _imepos;
    wsys_notify*    _notify;
    painter*        _painter;
    HDC             _memdc;
    HBITMAP         _membmp;
    HBITMAP         _oldbmp;

public:
    COMPOSITIONFORM _immpos;
    LOGFONT         _immfont;
};

__pink_end__

#endif
