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

#ifndef graph_46d3170b_a19a_4663_bbf9_c7372932671b_h
#define graph_46d3170b_a19a_4663_bbf9_c7372932671b_h

#include <assert.h>
#include <gslib/std.h>

__gslib_begin__

template<class _meta>
struct _mono_rel
{
    _meta*          _next;

    _mono_rel() { _next = 0; }
    _meta* next() const { return _next; }
    void set_next(_meta* ptr) { _next = ptr; }
    _meta* operator+(int d) const
    {
        assert(d >= 0);
        _meta* p = static_cast<_meta*>(this);
        for( ; d > 0; d --)
            p = p->_next;
        return p;
    }
    int operator-(const _meta& that) const
    {
        int c = 0;
        _meta* p = static_cast<_meta*>(this);
        for( ; p->_next; p = p->_next, c ++) {
            if(p == &that)
                return c;
        }
        assert(!"unexpected.");
        return -1;
    }
    int count_to_tail() const
    {
        int c = 0;
        for(_meta* p = static_cast<_meta*>(this); p->_next; p = p->_next, c ++);
        return c;
    }
    static void shake(_meta* p1, _meta* p2) { p1->_next = p2; }
};

template<class _meta>
struct _dual_rel
{
    _meta           *_prev, *_next;

    _dual_rel() { _prev = _next = 0; }
    _meta* prev() const { return _prev; }
    _meta* next() const { return _next; }
    void set_prev(_meta* ptr) { _prev = ptr; }
    void set_next(_meta* ptr) { _next = ptr; }
    _meta* operator+(int d) const
    {
        if(d < 0)
            return operator-(-d);
        _meta* p = static_cast<_meta*>(this);
        for( ; d > 0; d --)
            p = p->_next;
        return p;
    }
    _meta* operator-(int d) const
    {
        if(d < 0)
            return operator+(-d);
        _meta* p = static_cast<_meta*>(this);
        for( ; d > 0; d --)
            p = p->_prev;
        return p;
    }
    int operator-(const _meta& that) const
    {
        int c = 0;
        _meta* p = static_cast<_meta*>(this);
        for( ; p->_next; p = p->_next, c ++) {
            if(p == &that)
                return c;
        }
        assert(!"unexpected.");
        return -1;
    }
    int count_to_head() const
    {
        int c = 0;
        for(_meta* p = static_cast<_meta*>(this); p->_next; p = p->_prev, c ++);
        return c;
    }
    int count_to_tail() const
    {
        int c = 0;
        for(_meta* p = static_cast<_meta*>(this); p->_next; p = p->_next, c ++);
        return c;
    }
    static void shake(_meta* p1, _meta* p2) { p1->_next = p2, p2->_prev = p1; }
};

template<class _meta>
struct _pack_rel
{
    _meta       *_from, *_to;
    int         _size;

    _pack_rel() { _from = _to = 0, _size = 0; }
    _pack_rel(_meta* from, _meta* to) { _from = from, _to = to, _size = 0; }
    _meta* from() const { return _from; }
    _meta* to() const { return _to; }
    bool empty() const { return !_from; }
    int size() const { return _size; }
    void set_from(_meta* ptr) { _from = ptr; }
    void set_to(_meta* ptr) { _to = ptr; }
    void increase_size() { _size ++; }
    void decrease_size() { _size --; }
    void set_size(int s) { _size = s; }
    bool check_mono() const
    {
        if(!_from)
            return false;
        _meta* p = static_cast<_meta*>(_from);
        for( ; p->_next; p = p->_next);
        return p == _to;
    }
    bool check_dual() const
    {
        if(!_from || _from->_prev)
            return false;
        _meta* p = static_cast<_meta*>(_from);
        for( ; p->_next; p = p->_next) {
            if(p->_prev && p != p->_prev->_next)
                return false;
        }
        return p == _to;
    }
    bool is_belong_to(const _meta* ptr) const
    {
        _meta* p = static_cast<_meta*>(_from);
        for( ; p; p = p->_next) {
            if(ptr == p)
                return true;
        }
        return false;
    }
};

class _graph_ortho_rel:
    public _dual_rel<_graph_ortho_rel>
{
public:
    typedef _pack_rel<_graph_ortho_rel> hierarchic_rel;
    typedef _dual_rel<_graph_ortho_rel> peer_rel;
    typedef hierarchic_rel upper_rel;
    typedef hierarchic_rel lower_rel;

protected:
    hierarchic_rel  _parents, _children;

public:
    hierarchic_rel& parents() { return _parents; }
    hierarchic_rel& children() { return _children; }
    peer_rel& siblings() { return static_cast<peer_rel&>(*this); }
    const hierarchic_rel& const_parents() const { return _parents; }
    const hierarchic_rel& const_children() const { return _children; }
    const peer_rel& const_siblings() const { return static_cast<const peer_rel&>(*this); }
    int parents_count() const { return _parents.size(); }
    int children_count() const { return _children.size(); }
};

struct _graph_trait_copy {};
struct _graph_trait_detach {};

template<class _ty, 
    class _rel = _graph_ortho_rel
    >
struct _graph_ortho_cpy_wrapper:
    public _ty,
    public _rel
{
    typedef _ty value;
    typedef _rel relation;
    typedef _graph_ortho_cpy_wrapper<_ty, _rel> wrapper;
    typedef _graph_trait_copy tsf_behavior;
    
    value* get_ptr() { return static_cast<value*>(this); }
    const value* const_ptr() const { return static_cast<const value*>(this); }
    value& get_ref() { return static_cast<value&>(*this); }
    const value& const_ref() const { return static_cast<const value&>(*this); }
    template<class _cst>
    void born() {}  /* maybe a problem. */
    void born() {}
    void kill() {}
    void copy(const wrapper* a) { assign<value>(a); }
    void attach(wrapper* a) { !!error!! }
    int get_parents_count() const { return parents_count(); }
    int get_children_count() const { return children_count(); }
    template<class c>
    void assign(const wrapper* a) { static_cast<c&>(*this) = static_cast<c&>(*a); }
};

template<class _ty, 
    class _rel = _graph_ortho_rel
    >
struct _graph_ortho_wrapper:
    public _rel
{
    typedef _ty value;
    typedef _rel relation;
    typedef _graph_ortho_wrapper<_ty, _rel> wrapper;
    typedef _graph_trait_detach tsf_behavior;

    value*          _value;

    wrapper() { _value = 0; }
    value* get_ptr() { return _value; }
    const value* const_ptr() const { return _value; }
    value& get_ref() { return *_value; }
    const value& const_ref() const { return *_value; }
    relation& get_relation() { return static_cast<relation&>(*this); }
    const relation& const_relation() const { return static_cast<const relation&>(*this); }
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
    int get_parents_count() const { return parents_count(); }
    int get_children_count() const { return children_count(); }
};

template<class _wrapper>
struct _graph_allocator
{
    typedef _wrapper wrapper;
    static wrapper* born() { return gs_new(wrapper); }
    static void kill(wrapper* w) { gs_del(wrapper, w); }
};

template<class _val>
struct _graph_ortho_val
{
    typedef _val value;
    typedef const _val const_value;
    union
    {
        value*          _vptr;
        const_value*    _cvptr;
    };
    value* get_wrapper() const { return _vptr; }
    value* trace_front(int d) const { return _vptr->siblings() + d; }
    value* trace_back(int d) const { return _vptr->siblings() - d; }
    value* trace_up(int d) const
    {
        if(d < 0)
            return trace_down(-d);
        value* p = _vptr;
        for( ; d > 0; d --) {
            assert(!p->parents().empty());
            p = p->parents().from();
        }
        return p;
    }
    value* trace_down(int d) const
    {
        if(d < 0)
            return trace_up(-d);
        value* p = _vptr;
        for( ; d > 0; d --) {
            assert(!p->children().empty());
            p = p->children().from();
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
};

template<class _ty,
    class _wrapper = _graph_ortho_cpy_wrapper<_ty>
    >
class _graph_ortho_const_iterator:
    public _graph_ortho_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _graph_ortho_const_iterator<_ty, _wrapper> iterator;

public:
    iterator(const wrapper* w = 0) { _cvptr = w; }
    bool is_valid() const { return _cvptr != 0; }
    const value* get_ptr() const { return _cvptr->const_ptr(); }
    const value* operator->() const { return _cvptr->const_ptr(); }
    const value& operator*() const { return _cvptr->const_ref(); }
    iterator operator+(int d) const { return iterator(trace_front(d)); }
    iterator operator-(int d) const { return iterator(trace_back(d)); }
    iterator operator<<(int d) const { return iterator(trace_up(d)); }
    iterator operator>>(int d) const { return iterator(trace_down(d)); }
    int operator-(iterator i) const { return _cvptr->peer_rel::operator-(*(i._cvptr)); }
};

template<class _ty,
    class _wrapper = _graph_ortho_cpy_wrapper<_ty>
    >
class _graph_ortho_iterator:
    public _graph_ortho_const_iterator<_ty, _wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _graph_ortho_const_iterator<_ty, _wrapper> superref;
    typedef _graph_ortho_const_iterator<_ty, _wrapper> const_iterator;
    typedef _graph_ortho_iterator<_ty, _wrapper> iterator;

public:
    iterator(wrapper* w = 0) { _vptr = w; }
    value* get_ptr() const { return _vptr->get_ptr(); }
    value* operator->() const { return _vptr->get_ptr(); }
    value& operator*() const { return _vptr->get_ref(); }
    bool operator==(const iterator& that) const { return _vptr == that._vptr; }
    bool operator!=(const iterator& that) const { return _vptr != that._vptr; }
    bool operator==(const const_iterator& that) const { return _vptr == that._vptr; }
    bool operator!=(const const_iterator& that) const { return _vptr != that._vptr; }
    void operator++() { _vptr = trace_front(); }
    void operator--() { _vptr = trace_back(); }
    void operator+=(int d) { _vptr = trace_front(d); }
    void operator-=(int d) { _vptr = trace_back(d); }
    void operator<<=(int d) { _vptr = trace_up(d); }
    void operator>>=(int d) { _vptr = trace_down(d); }
    operator const_iterator() { return const_iterator(_cvptr); }
};

template<class _ty,
    class _wrapper = _graph_ortho_wrapper<_ty>,
    class _alloc = _graph_allocator<_wrapper>
    >
class ortho_graph:
    public _graph_ortho_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _alloc alloc;
    typedef ortho_graph<_ty, _wrapper, _alloc> myref;
    typedef _graph_ortho_const_iterator const_iterator;
    typedef _graph_ortho_iterator iterator;

public:
    ortho_graph() { _vptr = 0; }
    ~ortho_graph() { destroy(); }
    void destroy()
    {
        for(wrapper* i = _vptr; i; ) {
            wrapper* n = i->next();
            _erase(i);
            i = n;
        }
    }
    void clear() { destroy(); }
    iterator get_root() { return iterator(_vptr); }
    const_iterator const_root() const { return const_iterator(_cvptr); }
    bool is_valid() const { return !_cvptr; }
    iterator insert_before(iterator i) { return insert_before<value>(i); }
    iterator insert_after(iterator i) { return insert_after<value>(i); }
    iterator create_child(iterator from, iterator to = from) { return create_child<value>(from, to); }
    iterator erase(iterator i)
    {
        iterator r(i->next());
        _erase(i.get_wrapper());
        return r;
    }
    
public:
    template<class _cst>
    iterator insert_before(iterator i)
    {
        if(!i.is_valid())
            return _cvptr ? get_root() : _init<_cst>();
        wrapper* n = alloc::born();
        n->born<_cst>();
        _fix_peer_rel(i->prev(), n, *i);
        _fix_upper_rel(n, *i);
        _fix_lower_rel(n, *i);
        /* fix root, keep the root always be the top left corner. */
        if(i.get_wrapper() == _vptr)
            _vptr = n;
        return iterator(n);
    }
    template<class _cst>
    iterator insert_after(iterator i)
    {
        if(!i.is_valid())
            return _cvptr ? get_root() : _init<_cst>();
        wrapper* n = alloc::born();
        n->born<_cst>();
        _fix_peer_rel(*i, n, i->next());
        _fix_upper_rel(n, *i);
        _fix_lower_rel(n, *i);
        return iterator(n);
    }
    template<class _cst>
    iterator create_child(iterator from, iterator to = from)
    {
        assert(from.is_valid() && from->children().empty());
        wrapper* n = alloc::born();
        n->born<_cst>();
        _fix_upper_rel(n, *from, *to);
        return iterator(n);
    }
    template<class _lambda>
    void _for_range(iterator from, iterator to, _lambda lam)
    {
        for(iterator i = from; ; i ++) {
            lam(i->get_wrapper());
            if(i == to)
                break;
        }
    }
    template<class _lambda>
    void for_range(iterator from, iterator to, _lambda lam)
    {
        for(iterator i = from; ; i ++) {
            lam(*i);
            if(i == to)
                break;
        }
    }

protected:
    template<class _cst>
    iterator _init()
    {
        _vptr = alloc::born();
        _vptr->born<_cst>();
        return iterator(_vptr);
    }
    void _erase(wrapper* w)
    {
        assert(w);
        _fix_upper_rel(w);
        _fix_lower_rel(w);
        peer_rel::shake(w->prev(), w->next());
        lower_rel& rel = w->children();
        if(!rel.empty())
            _for_range(rel.from(), rel.to(), [](wrapper* w) { _erase(w); });
        if(w == _vptr)
            _vptr = w->next();
        w->kill();
        alloc::kill(w);
    }
    void _set_range_upper(wrapper* from, wrapper* to, wrapper* r1, wrapper* r2, int size)
    {
        _for_range(iterator(from), iterator(to), [&r1, &r2, &size](wrapper* w) {
            upper_rel& rel = w->parents();
            rel.set_from(r1);
            rel.set_to(r2);
            rel.set_size(size);
        });
    }
    void _set_range_lower(wrapper* from, wrapper* to, wrapper* r1, wrapper* r2, int size)
    {
        _for_range(iterator(from), iterator(to), [&r1, &r2, &size](wrapper* w) {
            lower_rel& rel = w->children();
            rel.set_from(r1);
            rel.set_to(r2);
            rel.set_size(size);
        });
    }
    void _fix_peer_rel(wrapper* w1, wrapper* w2, wrapper* w3)
    {
        assert(w2 && (w1 || w3));
        if(w1 != 0)
            peer_rel::shake(w1, w2);
        if(w3 != 0)
            peer_rel::shake(w2, w3);
    }
    void _fix_upper_rel(wrapper* n, wrapper* w)
    {
        assert(n && w);
        upper_rel& urel = w->parents();
        wrapper* u1 = urel.from();
        if(u1 == 0)
            return;
        wrapper* u2 = urel.to();
        lower_rel& lrel = u1->children();
        wrapper* l1 = lrel.from();
        wrapper* l2 = lrel.to();
        assert(l1 && l2);
        wrapper *from = l1, *to = l2;
        if(l1 == w && n == w->prev())
            from = n;
        else if(l2 == w && n == w->next())
            to = n;
        _set_range_lower(u1, u2, from, to, lrel.size() + 1);
        upper_rel& u = n->parents();
        u.set_from(u1);
        u.set_to(u2);
    }
    void _fix_lower_rel(wrapper* n, wrapper* w)
    {
        assert(n && w);
        lower_rel& lrel = w->children();
        wrapper* l1 = lrel.from();
        if(l1 == 0)
            return;
        wrapper* l2 = lrel.to();
        upper_rel& urel = l1->parents();
        wrapper* u1 = urel.from();
        wrapper* u2 = urel.to();
        assert(u1 && u2);
        wrapper *from = u1, *to = u2;
        if(u1 == w && n == w->prev())
            from = n;
        else if(u2 == w && n == w->next())
            to = n;
        _set_range_upper(l1, l2, from, to, urel.size() + 1);
        lower_rel& l = n->children();
        l.set_from(l1);
        l.set_to(l2);
    }
    void _fix_upper_rel(wrapper* w)
    {
        assert(w);
        upper_rel& urel = w->parents();
        wrapper* u1 = urel.from();
        if(u1 == 0)
            return;
        wrapper* u2 = urel.to();
        assert(u2);
        lower_rel& lrel = u1->children();
        wrapper* l1 = lrel.from();
        wrapper* l2 = lrel.to();
        assert(l1 && l2);
        wrapper *from = l1, *to = l2;
        if(l1 == w && l2 == w)
            from = to = 0;
        else if(l1 == w)
            from = w->next();
        else if(l2 == w)
            to = w->prev();
        _set_range_lower(u1, u2, from, to, lrel.size() - 1);
        urel.set_from(0);
        urel.set_to(0);
        urel.set_size(0);
    }
    void _fix_lower_rel(wrapper* w)
    {
        assert(w);
        lower_rel& lrel = w->children();
        wrapper* l1 = lrel.from();
        if(l1 == 0)
            return;
        wrapper* l2 = lrel.to();
        assert(l2);
        upper_rel& urel = l1->parents();
        wrapper* u1 = urel.from();
        wrapper* u2 = urel.to();
        assert(u1 && u2);
        wrapper *from = u1, *to = u2;
        if(u1 == w && u2 == w);
        else if(u1 == w || u2 == w) {
            u1 == w ? from = w->next() : to = w->prev();
            lrel.set_from(0);
            lrel.set_to(0);
            lrel.set_size(0);
        }
        _set_range_upper(l1, l2, from, to, urel.size() - 1);
    }
    void _fix_upper_rel(wrapper* w, wrapper* u1, wrapper* u2)
    {
        assert(w && u1 && u2);
        upper_rel& urel = w->parents();
        urel.set_from(u1);
        urel.set_to(u2);
        urel.set_size(u1->count_to_tail(u2) + 1);
        _set_range_lower(u1, u2, w, w, 1);
    }
};

__gslib_end__

#endif
