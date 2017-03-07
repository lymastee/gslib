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

#ifndef cclist_fde1faaa_9083_443a_a2fa_704bb17ca03e_h
#define cclist_fde1faaa_9083_443a_a2fa_704bb17ca03e_h

#include <assert.h>
#include <gslib/pool.h>

__gslib_begin__

struct _cl_trait_copy {};
struct _cl_trait_detach {};

template<class _ty>
struct _cl_cpy_wrapper
{
    typedef _ty value;
    typedef _cl_cpy_wrapper<_ty> wrapper;
    typedef _cl_trait_copy tsf_behavior;

    value           _value;
    wrapper*        _prev;
    wrapper*        _next;

    wrapper(): _prev(0), _next(0) {}
    value* get_ptr() { return &_value; }
    const value* const_ptr() const { return &_value; }
    value& get_ref() { return _value; }
    const value& const_ref() const { return _value; }
    template<class _cst>
    void born() {}
    void born() {}
    void kill() {}
    void copy(const wrapper* a) { _value = a->const_ref(); }
    void attach(wrapper* a) { !!error!! }
    wrapper* prev() const { return _prev; }
    wrapper* next() const { return _next; }
    static void shake(wrapper* a, wrapper* b)
    {
        assert(a && b);
        a->_next = b, b->_prev = a;
    }
};

template<class _ty>
struct _cl_wrapper
{
    typedef _ty value;
    typedef _cl_wrapper<_ty> wrapper;
    typedef _cl_trait_detach tsf_behavior;

    value*          _value;
    wrapper*        _prev;
    wrapper*        _next;

    wrapper(): _value(0), _prev(0), _next(0) {}
    value* get_ptr() { return static_cast<value*>(this); }
    const value* const_ptr() const { return static_cast<const value*>(this); }
    value& get_ref() { return static_cast<value&>(*this); }
    const value& const_ref() const { return static_cast<const value&>(*this); }
    template<class _cst>
    void born() { _value = gs_new(_cst); }
    void born() { _value = gs_new(value); }
    void kill() { gs_del(value, _value); }
    void copy(const wrapper* a) { assert(!"prevent."); }
    void attach(wrapper* a)
    {
        assert(a && a->_value);
        kill();
        _value = a->_value;
        a->_value = 0;
    }
    wrapper* prev() const { return _prev; }
    wrapper* next() const { return _next; }
    static void shake(wrapper* a, wrapper* b)
    {
        assert(a && b);
        a->_next = b, b->_prev = a;
    }
};

template<class _wrapper>
struct _cl_allocator
{
    typedef _wrapper wrapper;
    static wrapper* born() { return gs_new(wrapper); }
    static void kill(wrapper* w) { gs_del(wrapper, w); }
};

template<class _val>
struct _cl_node_val
{
    typedef _val value;
    typedef const _val const_value;
    union
    {
        value*          _vptr;
        const_value*    _cvptr;
    };
    value* get_wrapper() const { return _vptr; }
    value* trace_front(int d) const
    {
        if(d < 0)
            trace_back(-d);
        value* p = _vptr;
        for( ; d > 0; d --) {
            assert(p);
            p = p->next();
        }
        return p;
    }
    value* trace_back(int d) const
    {
        if(d < 0)
            trace_front(-d);
        value* p = _vptr;
        for( ; d > 0; d --) {
            assert(p);
            p = p->prev();
        }
        return p;
    }
    value* trace_front() const
    {
        assert(_vptr);
        return _vptr->next();
    }
    value* trace_back() const
    {
        assert(_vptr);
        return _vptr->prev();
    }
    int distance_of(const_value* p) const
    {
        assert(p && _cvptr);
        int d = 0;
        bool end = for_a_loop([&d, &p, this](const_value* v) {
            if(v == p)
                return false;
            d ++;
            return true;
        });
        return end ? -1 : d;
    }

public:
    template<class _lambda>
    bool for_a_loop(_lambda lam)
    {
        value* v = _vptr;
        do
        {
            if(!lam(v))
                return false;
            v = v->next();
        }
        while(v != _vptr);
        return true;
    }
    template<class _lambda>
    bool for_a_loop(_lambda lam) const
    {
        const_value* v = _cvptr;
        do 
        {
            if(!lam(v))
                return false;
            v = v->next();
        }
        while(v != _cvptr);
        return true;
    }
};

template<class _ty, 
    class _wrapper = _cl_cpy_wrapper<_ty> >
class _cl_const_iterator:
    public _cl_node_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _cl_const_iterator<_ty, _wrapper> iterator;

public:
    iterator(const wrapper* w = 0) { _cvptr = w; }
    bool is_valid() const { return _cvptr != 0; }
    const value* get_ptr() const { return _cvptr->const_ptr(); }
    const value* operator->() const { return _cvptr->const_ptr(); }
    const value& operator*() const { return _cvptr->const_ref(); }
    bool operator==(const iterator& that) const { return _cvptr == that._cvptr; }
    bool operator!=(const iterator& that) const { return _cvptr != that._cvptr; }
    iterator next() const { return _cvptr ? iterator(_cvptr->next()) : iterator(); }
    iterator prev() const { return _cvptr ? iterator(_cvptr->prev()) : iterator(); }
    iterator& operator++()
    {
        assert(_cvptr);
        _cvptr = _cvptr->next();
        return *this;
    }
    iterator operator++(int)
    {
        assert(_cvptr);
        const wrapper* r = _cvptr;
        _cvptr = _cvptr->next();
        return iterator(r);
    }
    iterator& operator--()
    {
        assert(_cvptr);
        _cvptr = _cvptr->prev();
        return *this;
    }
    iterator operator--(int)
    {
        assert(_cvptr);
        const wrapper* r = _cvptr;
        _cvptr = _cvptr->prev();
        return iterator(r);
    }
    void operator+=(int d) { _cvptr = trace_front(d); }
    void operator-=(int d) { _cvptr = trace_back(d); }
    iterator operator+(int d) const { return iterator(trace_front(d)); }
    iterator operator-(int d) const { return iterator(trace_back(d)); }
    int operator-(const iterator& that) const { return distance_of(that->get_wrapper()); }
};

template<class _ty, 
    class _wrapper = _cl_cpy_wrapper<_ty> >
class _cl_iterator:
    public _cl_const_iterator<_ty, _wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _cl_const_iterator<_ty, _wrapper> superref;
    typedef _cl_const_iterator<_ty, _wrapper> const_iterator;
    typedef _cl_iterator<_ty, _wrapper> iterator;

public:
    iterator(wrapper* w = 0) { _vptr = w; }
    value* get_ptr() const { return _vptr->get_ptr(); }
    value* operator->() const { return _vptr->get_ptr(); }
    value& operator*() const { return _vptr->get_ref(); }
    bool operator==(const iterator& that) const { return _vptr == that._vptr; }
    bool operator!=(const iterator& that) const { return _vptr != that._vptr; }
    bool operator==(const const_iterator& that) const { return _vptr == that._vptr; }
    bool operator!=(const const_iterator& that) const { return _vptr != that._vptr; }
    iterator& operator++()
    {
        assert(_vptr);
        _vptr = _vptr->next();
        return *this;
    }
    iterator operator++(int)
    {
        assert(_vptr);
        wrapper* r = _vptr;
        _vptr = _vptr->next();
        return iterator(r);
    }
    iterator& operator--()
    {
        assert(_vptr);
        _vptr = _vptr->prev();
        return *this;
    }
    iterator operator--(int)
    {
        assert(_vptr);
        wrapper* r = _vptr;
        _vptr = _vptr->prev();
        return iterator(r);
    }
    void operator+=(int d) { _vptr = trace_front(d); }
    void operator-=(int d) { _vptr = trace_back(d); }
    iterator operator+(int d) const { return iterator(trace_front(d)); }
    iterator operator-(int d) const { return iterator(trace_back(d)); }
    int operator-(const const_iterator& that) const { return distance_of(that->get_wrapper()); }
    operator const_iterator() { return const_iterator(_cvptr); }
};

template<class _ty, 
    class _wrapper = _cl_cpy_wrapper<_ty>,
    class _alloc = _cl_allocator<_wrapper> >
class circular_list:
    public _cl_node_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _alloc alloc;
    typedef circular_list<_ty, _wrapper, _alloc> myref;
    typedef _cl_const_iterator<_ty, _wrapper> const_iterator;
    typedef _cl_iterator<_ty, _wrapper> iterator;

protected:
    int             _size;

public:
    circular_list() { _vptr = 0, _size = 0; }
    ~circular_list() { clear(); }
    void set_pin(const const_iterator& i) { _cvptr = i.get_wrapper(); }
    void set_pin(const iterator& i) { _cvptr = i.get_wrapper(); }
    iterator get_pin() { return iterator(_vptr); }
    const_iterator get_pin() const { return const_iterator(_cvptr); }
    int size() const { return _size; }
    bool empty() const { return !_size; }
    iterator insert(iterator p) { return insert<value>(p); }
    iterator insert_after(iterator p) { return insert_after<value>(p); }
    iterator insert(iterator p, const value& v)
    {
        iterator i = insert(p);
        *i = v;
        return i;
    }
    iterator insert_after(iterator p, const value& v)
    {
        iterator i = insert_after(p);
        *i = v;
        return i;
    }
    iterator erase(iterator p)
    {
        switch(_size)
        {
        case 1:
            return _destroy1();
        case 2:
            return _destroy2(p);
        }
        assert(p.is_valid());
        wrapper* w = p.get_wrapper();
        wrapper* l = w->prev();
        wrapper* n = w->next();
        wrapper::shake(l, n);
        w->kill();
        alloc::kill(w);
        _size --;
        if(w == _vptr)
            _vptr = n;
        return iterator(l);
    }
    void clear()
    {
        if(empty())
            return;
        assert(_vptr);
        for(wrapper* w = _vptr->next(); w != _vptr; ) {
            wrapper* n = w->next();
            w->kill();
            alloc::kill(w);
            w = n;
        }
        _vptr->kill();
        alloc::kill(_vptr);
        _vptr = 0;
        _size = 0;
    }
    value& front() { return _vptr->get_ref(); }
    value& back() { return _vptr->prev()->get_ref(); }
    const value& front() const { return _vptr->const_ref(); }
    const value& back() const { return _vptr->prev()->const_ref(); }
    iterator begin() const { return iterator(_vptr); }
    iterator end() const { return iterator(_vptr->prev()); }
    void push_front(const value& v) { *insert(get_pin()) = v; }
    void push_back(const value& v) { *insert_after(get_pin()) = v; }
    void splice();
    iterator find(const value& v)
    {
        wrapper* f = 0;
        for_a_loop([&](wrapper* w)->bool {
            if(w->const_ref() == v) {
                f = w;
                return false;
            }
            return true;
        });
        return iterator(f);
    }
    const_iterator find_const(const value& v) const
    {
        const wrapper* f = 0;
        for_a_loop([&](const wrapper* w)->bool {
            if(w->const_ref() == v) {
                f = w;
                return false;
            }
            return true;
        });
        return const_iterator(f);
    }

public:
    template<class _cst>
    iterator insert(iterator p)
    {
        switch(_size)
        {
        case 0:
            return _init1<_cst>();
        case 1:
            return _init2<_cst>();
        }
        wrapper* w = alloc::born();
        w->born<_cst>();
        wrapper* c = p.get_wrapper();
        wrapper* l = c->prev();
        assert(c && l && l->next() == c);
        wrapper::shake(l, w);
        wrapper::shake(w, c);
        _size ++;
        return iterator(w);
    }
    template<class _cst>
    iterator insert_after(iterator p)
    {
        return _size >= 2 ? insert<_cst>(++p) :
            insert<_cst>(p);
    }
    template<class _lambda>
    void for_each(_lambda lam)
    {
        assert(_vptr);  // todo: skip determine
        for_a_loop([&lam](wrapper* w)->bool {
            lam(w->get_ref());
            return true;
        });
    }
    template<class _lambda>
    void for_each(_lambda lam) const
    {
        assert(_cvptr); // todo: skip determine
        for_a_loop([&lam](const wrapper* w)->bool {
            lam(w->const_ref());
            return true;
        });
    }

protected:
    template<class _cst>
    iterator _init1()
    {
        assert(!_size && !_vptr);
        _vptr = alloc::born();
        _vptr->born<_cst>();
        wrapper::shake(_vptr, _vptr);
        _size = 1;
        return iterator(_vptr);
    }
    template<class _cst>
    iterator _init2()
    {
        assert(_size == 1 && _vptr);
        wrapper* w = alloc::born();
        w->born<_cst>();
        wrapper::shake(_vptr, w);
        wrapper::shake(w, _vptr);
        _size = 2;
        return iterator(w);
    }
    iterator _destroy1()
    {
        assert(_size == 1 && _vptr);
        _vptr->kill();
        alloc::kill(_vptr);
        _vptr = 0;
        _size = 0;
        return iterator(0);
    }
    iterator _destroy2(iterator p)
    {
        assert(_size == 2 && p.is_valid());
        wrapper* w = p.get_wrapper();
        wrapper* rm = w->next();
        assert(rm);
        w->kill();
        alloc::kill(w);
        wrapper::shake(rm, rm);
        if(_vptr == w)
            _vptr = rm;
        _size = 1;
        return iterator(rm);
    }
};

__gslib_end__

#endif
