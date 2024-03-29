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

#include <windows.h>
#include <gslib/std.h>
#include <gslib/string.h>
#include <gslib/error.h>
#include <gslib/pool.h>

__gslib_begin__

void* error_dump_callstack(void*)
{
    typedef gs::vector<dword> callstack_addr;
    callstack_addr csa;
    dword* debp;
    __asm mov debp, ebp;
    while(*debp) {
        csa.push_back(*(debp+1));
        debp = (dword*)(*debp);
    }
    int size = (int)csa.size();
    size *= sizeof(dword);
    size += sizeof(dword) * 2;
    vessel vsl;
    vsl.flex(size);
    vsl.front<dword>() = (size -= sizeof(dword));
    vsl.occupy(sizeof(dword));
    vsl.store(&csa.at(0), size);
    vsl.current<dword>() = 0;
    void* ret = vsl.get_ptr();
    vsl.detach();
    return ret;
}

class error_stack
{
public:
    static error_stack* get_singleton_ptr()
    {
        static error_stack inst;
        return &inst;
    }
    ~error_stack()
    {
        reset();
    }
    void reset()
    {
        errstack::iterator end = _stack.end();
        for(errstack::iterator i = _stack.begin(); i != end; ++ i)
            erase(&(*i));
        _stack.clear();
    }
    void erase(errinfo* e)
    {
        if(gchar* c = e->desc) { delete [] c; e->desc = nullptr; }
        if(gchar* c = e->file) { delete [] c; e->file = nullptr; }
        if(dword* d = (dword*)e->user) { delete d; e->user = nullptr; }
    }
    errinfo* get_back()
    {
        if(_stack.size())
            return &_stack.back();
        return 0;
    }
    errinfo* push()
    {
        _stack.push_back(errinfo());
        return &_stack.back();
    }
    void pop()
    {
        assert(_stack.size());
        _stack.pop_back();
    }
    int get_size() const
    {
        return (int)_stack.size();
    }

private:
    error_stack() {}

protected:
    typedef vector<errinfo> errstack;
    errstack        _stack;
};

static inline void allocpy(gchar*& p, const gchar* s)
{
    assert(s);
    int len = strtool::length(s);
    if(len == 0) { p = 0; return; }
    p = new gchar[len+1];
    assert(p);
    memcpy_s(p, len, s, len);
    p[len] = 0;
}

void _set_error(const gchar* desc, error_dump dump, void* user, const gchar* file, int line)
{
    errinfo* ptr = error_stack::get_singleton_ptr()->push();
    assert(ptr);
    allocpy(ptr->desc, desc);
    allocpy(ptr->file, file);
    ptr->line = line;
    if(dump != 0)
        user = dump(user);
    ptr->user = user;
}

void _set_last_error(const gchar* desc, error_dump dump, void* user, const gchar* file, int line)
{
    error_stack* errst = error_stack::get_singleton_ptr();
    errinfo* ptr = errst->get_back();
    if(ptr == 0)
        ptr = errst->push();
    assert(ptr);
    allocpy(ptr->desc, desc);
    allocpy(ptr->file, file);
    ptr->line = line;
    if(dump != 0)
        user = dump(user);
    ptr->user = user;
}

void pop_error()
{
    error_stack* errst = error_stack::get_singleton_ptr();
    assert(errst);
    errst->pop();
}

errinfo* get_last_error()
{
    error_stack* errst = error_stack::get_singleton_ptr();
    assert(errst);
    return errst->get_back();
}

void reset_error()
{
    error_stack* errst = error_stack::get_singleton_ptr();
    assert(errst);
    errst->reset();
}

/* CAUTION: multi-thread was not supported. */
static string __holding_trace_string;

void trace_hold(const gchar* fmt, ...)
{
    assert(fmt);
    va_list ptr;
    va_start(ptr, fmt);
    static string str;
    str.formatv(fmt, ptr);
    __holding_trace_string += str;
}

void _trace_to_clipboard()
{
#if defined(_WIN32)
    if(__holding_trace_string.empty())
        return;
    /* prepare memory */
    auto size = sizeof(gchar) * __holding_trace_string.length();
    HGLOBAL hmem =  GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size + sizeof(gchar));
    void* mem = GlobalLock(hmem);
    memcpy_s(mem, size + sizeof(gchar), __holding_trace_string.c_str(), size);
    memset(((byte*)mem) + size, 0, sizeof(gchar));      /* eos */
    GlobalUnlock(hmem);
    /* op clipboard */
    OpenClipboard(nullptr);
    EmptyClipboard();
    UINT fmt = (sizeof(gchar) == 1) ? CF_TEXT : CF_UNICODETEXT;
    SetClipboardData(fmt, hmem);
    CloseClipboard();
    /* report */
    printf("trace to clipboard done.\n");
#endif
}

#ifdef _GS_TRACE_TO_CLIPBOARD

#elif defined (DEBUG) || defined (_DEBUG)

void trace(const gchar* fmt, ...)
{
    va_list ptr;
    va_start(ptr, fmt);
    string str;
    str.formatv(fmt, ptr);
    static const int max_len = 32766;
    int len = str.length();
    gchar* pstr = const_cast<gchar*>(str.c_str());
    while(len > max_len) {
        gchar trap = pstr[max_len];
        pstr[max_len] = 0;
        OutputDebugString(pstr);
        pstr[max_len] = trap;
        pstr += max_len;
        len -= max_len;
    }
    OutputDebugString(pstr);
}

void trace_all(const gchar* str)
{
    assert(str);
    OutputDebugString(str);
}

#endif

#if defined(WIN32) || defined(_WINDOWS)
void sound_alarm()
{
    Beep(442, 80);
}
#endif

__gslib_end__
