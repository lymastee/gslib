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

#include <windows.h>
#include <gslib/std.h>
#include <gslib/string.h>
#include <gslib/error.h>

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
        if(gchar* c = e->desc) { delete [] c; e->desc = 0; }
        if(gchar* c = e->file) { delete [] c; e->file = 0; }
        if(dword* d = (dword*)e->user) { gs_del(dword, d); e->user = 0; }
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

#if defined (DEBUG) || defined (_DEBUG)

void trace(const gchar* fmt, ...)
{
    va_list ptr;
    va_start(ptr, fmt);
    static string str;
    //str.formatv(4196, fmt, ptr);
    str.formatv(fmt, ptr);
    OutputDebugString(str.c_str());
}

void trace_all(const gchar* str)
{
    assert(str);
    OutputDebugString(str);
}

#endif

__gslib_end__
