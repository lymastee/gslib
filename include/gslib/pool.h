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

#ifndef pool_81b70dbf_d70f_42d7_8770_678651c585c4_h
#define pool_81b70dbf_d70f_42d7_8770_678651c585c4_h

#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <gslib/type.h>

__gslib_begin__

template<class _construct>
class factory
{
public:
    typedef _construct mycon;
    typedef factory<mycon> myref;

public:
    factory(void* ptr = nullptr) { _generated = (mycon*)ptr; }
    const mycon* get_ptr() const { return _generated; }
    mycon* get_ptr() { return _generated; }
    mycon* emerge()
    {
        if(!_generated)
            _generated = new mycon;
        assert(_generated);
        return _generated;
    }
    void destroy()
    {
        if(!_generated)
            return;
        delete _generated;
        _generated = nullptr;
    }

protected:
    mycon*              _generated;
};

class vessel
{
protected:
    void*               _buf;
    int                 _cap;
    int                 _cur;

public:
    vessel()
    {
        _buf = nullptr;
        _cap = 0;
        _cur = 0;
    }
    ~vessel()
    {
        destroy();
    }
    void destroy()
    {
        if(_buf) {
            free(_buf);
            _buf = nullptr;
        }
        _cap = _cur = 0;
    }
    int get_cap() const
    {
        return _cap;
    }
    int get_cur() const
    {
        return _cur;
    }
    void* get_ptr() const
    {
        return _buf;
    }
    int get_rest() const
    {
        return _cap - _cur;
    }
    void flex(int size)
    {
        _buf = realloc(_buf, size);
        _cap = size;
    }
    void expand(int size)
    {
        flex(get_cap() + size);
    }
    void fit()
    {
        _buf = realloc(_buf, _cur);
        _cap = _cur;
    }
    void occupy(int size)
    {
        _cur += size;
        assert(_cur <= get_cap());
    }
    void set_cur(int size)
    {
        _cur = size;
        assert(_cur <= get_cap());
    }
    int curaddr(int ofs = 0) const
    {
        return (int)_buf + _cur + ofs;
    }
    void store(const void* buf, int size)
    {
        if(_cur + size > get_cap())
            expand(get_cap() > size ? get_cap() : size);
        assert(get_cap() >= _cur + size);
        memcpy_s((void*)curaddr(), get_rest(), buf, size);
        occupy(size);
    }
    void attach(void* buf, int size)
    {
        flex(0);
        _buf = buf;
        _cap = size;
        _cur = size;
    }
    void attach(vessel* vsl)
    {
        assert(vsl);
        attach(vsl->get_ptr(), vsl->get_cap());
        _cur = vsl->get_cur();
        vsl->detach();
    }
    void detach()
    {
        _buf = nullptr;
        _cap = 0;
        _cur = 0;
    }
    template<class tpl>
    tpl& current(int ofs = 0)
    {
        return *((tpl*)((int)_buf + _cur + ofs));
    }
    template<class tpl>
    tpl& front(int ofs = 0)
    {
        return *((tpl*)((int)_buf + ofs));
    }
};

__gslib_end__

#endif