/*
 * Copyright (c) 2016-2018 lymastee, All rights reserved.
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

#ifndef avl_6c9ab2d0_f8bc_4fb1_bc21_7fc0c6eb4bec_h
#define avl_6c9ab2d0_f8bc_4fb1_bc21_7fc0c6eb4bec_h

#include <assert.h>
#include <gslib/pool.h>
#include <gslib/std.h>

__gslib_begin__

struct _avltree_trait_copy {};
struct _avltree_trait_detach {};

template<class _ty>
struct _avltreenode_cpy_wrapper
{
    typedef _ty value;
    typedef _avltreenode_cpy_wrapper<_ty> myref;
    typedef _avltree_trait_copy tsf_behavior;

    value               _value;
    myref*              _left;
    myref*              _right;
    myref*              _parent;
    int                 _balance;

    myref()
    {
        _left = _right = _parent = 0;
        _balance = 0;
    }
    value* get_ptr() { return &_value; }
    const value* const_ptr() const { return &_value; }
    value& get_ref() { return _value; }
    const value& const_ref() const { return _value; }
    void born() {}
    void kill() {}
    template<class _ctor>
    void born() {}
    template<class _ctor>
    void kill() {}
    void copy(const myref* a) { get_ref() = a->const_ref(); }
    void attach(myref* a) { assert(0); }
    void swap_data(myref* a) { std::swap(_value, a->_value); }
};

template<class _ty>
struct _avltreenode_wrapper
{
    typedef _ty value;
    typedef _avltreenode_wrapper<_ty> myref;
    typedef _avltree_trait_detach tsf_behavior;

    value*              _value;
    myref*              _left;
    myref*              _right;
    myref*              _parent;
    int                 _balance;

    myref()
    {
        _left = _right = _parent = 0;
        _value = 0;
        _balance = 0;
    }
    value* get_ptr() { return _value; }
    const value* const_ptr() const { return _value; }
    value& get_ref() { return *_value; }
    const value& const_ref() const { return *_value; }
    void copy(const myref* a) { get_ref() = a->const_ref(); }
    void born() { !! }
    template<class _ctor>
    void born() { _value = gs_new(_ctor); }
    void kill() { if(_value) { gs_del(value, _value); _value = 0; } }
    template<class _ctor>
    void kill() { if(_value) { gs_del(_ctor, _value); _value = 0; } }
    void attach(myref* a)
    {
        assert(a && a->_value);
        kill();
        _value = a->_value;
        a->_value = 0;
    }
    void swap_data(myref* a) { gs_swap(_value, a->_value); }
};

template<class _wrapper>
struct _avltree_allocator
{
    typedef _wrapper wrapper;
    static wrapper* born() { return gs_new(wrapper); }
    static void kill(wrapper* w) { gs_del(wrapper, w); }
};

template<class _val>
struct _avltreenode_val
{
    typedef _val value;
    typedef const _val const_value;
    typedef _avltreenode_val<_val> myref;

    union
    {
        value*          _vptr;
        const_value*    _cvptr;
    };

    myref() { _vptr = 0; }
    value* get_wrapper() const { return _vptr; }
    operator bool() const { return _vptr != 0; }
    bool is_left() const { return (_cvptr && _cvptr->_parent) ? _cvptr->_parent->_left == _cvptr : false; }
    bool is_right() const { return (_cvptr && _cvptr->_parent) ? _cvptr->_parent->_right == _cvptr : false; }
    bool is_root() const { return _cvptr ? (!_cvptr->_parent) : false; }
    bool is_leaf() const { return _cvptr ? (!_cvptr->_left && !_cvptr->_right) : false; }
    int up_depth() const
    {
        int depth = 0;
        for(value* p = _vptr; p; p = p->_parent, depth ++);
        return depth;
    }
    int down_depth() const { return _down_depth(_vptr, 0); }
    bool operator==(const value* v) const { return _vptr == v; }
    bool operator!=(const value* v) const { return _vptr != v; }
    int get_balance() const { return _vptr->_balance; }
    void set_balance(int b) { _vptr->_balance = b; }
    void swap_data(myref& a) { _vptr->swap_data(a._vptr); }

public:
    static void connect_left_child(value* p, value* l)
    {
        assert(p);
        p->_left = l;
        if(l)
            l->_parent = p;
    }
    static void connect_right_child(value* p, value* r)
    {
        assert(p);
        p->_right = r;
        if(r)
            r->_parent = p;
    }
    static bool disconnect_parent_child(value* p, value* c)
    {
        assert(p && c && (c->_parent == p));
        if(p->_left == c) {
            p->_left = c->_parent = 0;
            return true;
        }
        assert(p->_right == c);
        p->_right = c->_parent = 0;
        return false;
    }

private:
    static int _down_depth(value* v, int ctr)
    {
        if(v == 0)
            return ctr;
        ctr ++;
        return gs_max(_down_depth(v->_left, ctr), 
            _down_depth(v->_right, ctr)
            );
    }

protected:
    value* vleft() const { return _vptr ? _vptr->_left : 0; }
    value* vright() const { return _vptr ? _vptr->_right : 0; }
    value* vparent() const { return _vptr ? _vptr->_parent : 0; }
    value* vsibling() const
    {
        if(!_vptr || !_vptr->_parent)
            return 0;
        if(is_left())
            return _vptr->_right;
        else if(is_right())
            return _vptr->_left;
        assert(!"unexpected.");
        return 0;
    }
    value* vroot() const
    {
        if(!_vptr)
            return 0;
        value* p = _vptr;
        for( ; p->_parent; p = p->_parent);
        return p;
    }

protected:
    template<class _lambda, class _value>
    static void preorder_traversal(_lambda lam, _value* v)
    {
        assert(v);
        lam(v);
        if(v->_left)
            preorder_traversal(lam, v->_left);
        if(v->_right)
            preorder_traversal(lam, v->_right);
    }
    template<class _lambda, class _value>
    static void inorder_traversal(_lambda lam, _value* v)
    {
        assert(v);
        if(v->_left)
            inorder_traversal(lam, v->_left);
        lam(v);
        if(v->_right)
            inorder_traversal(lam, v->_right);
    }
    template<class _lambda, class _value>
    static void postorder_traversal(_lambda lam, _value* v)
    {
        assert(v);
        if(v->_left)
            postorder_traversal(lam, v->_left);
        if(v->_right)
            postorder_traversal(lam, v->-right);
        lam(v);
    }

public:
    template<class _lambda>
    void inorder_traversal(_lambda lam) { if(_vptr) inorder_traversal(lam, _vptr); }
    template<class _lambda>
    void inorder_traversal(_lambda lam) const { if(_cvptr) inorder_traversal(lam, _cvptr); }
    template<class _lambda>
    void preorder_traversal(_lambda lam) { if(_vptr) preorder_traversal(lam, _vptr); }
    template<class _lambda>
    void preorder_traversal(_lambda lam) const { if(_cvptr) preorder_traversal(lam, _cvptr); }
    template<class _lambda>
    void postorder_traversal(_lambda lam) { if(_vptr) postorder_traversal(lam, _vptr); }
    template<class _lambda>
    void postorder_traversal(_lambda lam) const { if(_cvptr) postorder_traversal(lam, _cvptr); }
};

template<class _ty,
    class _wrapper = _avltreenode_cpy_wrapper<_ty> >
class _avltree_const_iterator:
    public _avltreenode_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _avltree_const_iterator<_ty, _wrapper> iterator;

public:
    iterator(const wrapper* w = 0) { _cvptr = w; }
    bool is_valid() const { return _cvptr != 0; }
    const value* get_ptr() const { return _cvptr->const_ptr(); }
    const value* operator->() const { return _cvptr->const_ptr(); }
    const value& operator*() const { return _cvptr->const_ref(); }
    iterator left() const { return iterator(vleft()); }
    iterator right() const { return iterator(vright()); }
    iterator parent() const { return iterator(vparent()); }
    iterator sibling() const { return iterator(vsibling()); }
    iterator root() const { return iterator(vroot()); }
    bool operator==(const iterator& that) const { return _cvptr == that._cvptr; }
    bool operator!=(const iterator& that) const { return _cvptr != that._cvptr; }
};

template<class _ty,
    class _wrapper = _avltreenode_cpy_wrapper<_ty> >
class _avltree_iterator:
    public _avltree_const_iterator<_ty, _wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _avltree_const_iterator<_ty, _wrapper> const_iterator;
    typedef _avltree_const_iterator<_ty, _wrapper> superref;
    typedef _avltree_iterator<_ty, _wrapper> iterator;

public:
    iterator(wrapper* w): superref(w) {}
    value* get_ptr() const { return _vptr->get_ptr(); }
    value* operator->() const { return _vptr->get_ptr(); }
    value& operator*() const { return _vptr->get_ref(); }
    bool operator==(const iterator& that) const { return _vptr == that._vptr; }
    bool operator!=(const iterator& that) const { return _vptr != that._vptr; }
    bool operator==(const const_iterator& that) const { return _vptr == that._vptr; }
    bool operator!=(const const_iterator& that) const { return _vptr != that._vptr; }
    operator const_iterator() { return const_iterator(_cvptr); }
    void to_root() { _vptr = vroot(); }
    void to_left() { _vptr = vleft(); }
    void to_right() { _vptr = vright(); }
    void to_sibling() { _vptr = vsibling(); }
    void to_parent() { _vptr = vparent(); }
    iterator left() const { return iterator(vleft()); }
    iterator right() const { return iterator(vright()); }
    iterator parent() const { return iterator(vparent()); }
    iterator sibling() const { return iterator(vsibling()); }
    iterator root() const { return iterator(vroot()); }
};

template<class _ty,
    class _wrapper = _avltreenode_cpy_wrapper<_ty>,
    class _alloc = _avltree_allocator<_wrapper> >
class avltree:
    public _avltreenode_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _alloc alloc;
    typedef avltree<value, wrapper, alloc> myref;
    typedef _avltree_const_iterator<_ty, _wrapper> const_iterator;
    typedef _avltree_iterator<_ty, _wrapper> iterator;

public:
    avltree() { _vptr = 0; }
    ~avltree() { clear(); }
    void clear() { destroy(get_root()); }
    void destroy(iterator i)
    {
        if(!i.is_valid())
            return;
        if(iterator p = i.parent())
            disconnect_parent_child(p.get_wrapper(), i.get_wrapper());
        else {
            assert(is_root(i));
            _vptr = 0;
        }
        _destroy(i);
    }
    void adopt(wrapper* w)
    {
        assert(!_vptr && "use attach method.");
        _vptr = w;
    }
    iterator get_root() const { return iterator(_vptr); }
    const_iterator const_root() const { return const_iterator(_cvptr); }
    bool is_root(iterator i) const { return i.is_valid() ? (_cvptr == i.get_wrapper()) : false; }
    bool is_valid() const { return _cvptr != 0; }
    bool is_mine(iterator i) const
    {
        if(!i.is_valid())
            return false;
        i.to_root();
        return i.get_wrapper() == _vptr;
    }
    int depth() const { return _cvptr->down_depth(); }
    void swap(myref& that) { gs_swap(_vptr, that._vptr); }
    iterator find(const value& v) const { return find(get_root(), v); }
    iterator find(iterator i, const value& v) const
    {
        if(!i) {
            if(!is_valid())
                return i;
            i = get_root();
        }
        assert(i);
        if(v == *i)
            return i;
        iterator n = (v < *i) ? i.left() : i.right();
        return n ? find(n, v) : i;
    }
    template<class _ctor = value>
    iterator insert(const value& v)
    {
        iterator i = get_root();
        return !i ? _init<_ctor>(v) : _insert<_ctor>(i, v);
    }
    void erase(iterator i)
    {
        assert(i && is_mine(i));
        if(i.is_leaf()) {
            iterator p = i.parent();
            if(p) {
                disconnect_parent_child(p.get_wrapper(), i.get_wrapper());
                _destroy(i);
                _balance_erase(p);
                return;
            }
            else {
                assert(is_root(i));
                _destroy(i);
                _vptr = 0;
                return;
            }
        }
        if(i.left()) {
            if(i.right()) {     /* left & right */
                iterator l = i.left();
                iterator li = l;
                for(; li.right(); li.to_right());
                iterator lil = li.left();
                i.swap_data(li);
                if(lil) {
                    li.swap_data(lil);
                    destroy(lil);
                    _balance_erase(li);
                    return;
                }
                else {
                    iterator q = li.parent();
                    assert(q);
                    destroy(li);
                    _balance_erase(q);
                    return;
                }
            }
            else {      /* only left */
                if(i.parent()) {
                    myref t;
                    iterator j = attach(detach(t, i.left()), i);
                    j.set_balance(i.get_balance());
                    _balance_erase(j);
                    return;
                }
                else {
                    myref t;
                    swap(detach(t, i.left()));
                    return;
                }
            }
        }
        else {
            assert(i.right());      /* only right */
            if(i.parent()) {
                myref t;
                iterator j = attach(detach(t, i.right()), i);
                j.set_balance(i.get_balance());
                _balance_erase(j);
                return;
            }
            else {
                myref t;
                swap(detach(t, i.right()));
                return;
            }
        }
    }
    void erase(const value& v)
    {
        if(iterator i = find(v))
            erase(i);
    }

    /* The detach and attach methods, provide subtree operations */
    myref& detach(myref& subtree, iterator i)
    {
        assert(i && is_mine(i));
        if(subtree.is_valid())
            subtree.clear();
        detach<myref>(subtree, i);
        return subtree;
    }
    template<class _cont>
    void detach(_cont& cont)
    {
        cont.adopt(_vptr);
        _vptr = 0;
    }
    template<class _cont>
    void detach(_cont& cont, iterator i)
    {
        assert(i && is_mine(i));
        if(i == get_root())
            return detach(cont);
        iterator p = i.parent();
        assert(p);
        disconnect_parent_child(p.get_wrapper(), i.get_wrapper());
        cont.adopt(i.get_wrapper());
    }
    iterator attach(myref& subtree, iterator i)
    {
        assert(i && is_mine(i) && i.is_leaf());
        if(i.is_root()) {
            swap(subtree);
            return get_root();
        }
        iterator p = i.parent();
        assert(p);
        bool leftp = disconnect_parent_child(p.get_wrapper(), i.get_wrapper());
        gs_swap(subtree._vptr, i._vptr);
        leftp ? connect_left_child(p.get_wrapper(), i.get_wrapper()) :
            connect_right_child(p.get_wrapper(), i.get_wrapper());
        subtree.clear();
        return i;
    }

public:
    template<class _lambda>
    void preorder_for_each(_lambda lam) { preorder_traversal([](wrapper* w) { lam(w->get_ptr()); }); }
    template<class _lambda>
    void preorder_const_for_each(_lambda lam) const { preorder_traversal([](const wrapper* w) { lam(w->const_ptr()); }); }
    template<class _lambda>
    void inorder_for_each(_lambda lam) { inorder_traversal([](wrapper* w) { lam(w->get_ptr()); }); }
    template<class _lambda>
    void inorder_const_for_each(_lambda lam) const { inorder_traversal([](const wrapper* w) { lam(w->const_ptr()); }); }
    template<class _lambda>
    void postorder_for_each(_lambda lam) { postorder_traversal([](wrapper* w) { lam(w->get_ptr()); }); }
    template<class _lambda>
    void postorder_const_for_each(_lambda lam) const { postorder_traversal([](const wrapper* w) { lam(w->const_ptr()); }); }

protected:
    void _destroy(iterator i)
    {
        if(!i.is_valid())
            return;
        _destroy(i.left());
        _destroy(i.right());
        wrapper* w = i.get_wrapper();
        w->kill();
        alloc::kill(w);
    }
    template<class _ctor>
    iterator _insert(iterator i, const value& v)
    {
        assert(i);
        if(v == *i)
            return iterator(0);     /* failed */
        if(v < *i) {
            if(i.left())
                return _insert<_ctor>(i.left(), v);
            iterator j = _add_left<_ctor>(i, v);
            _balance_insert(j);
            return j;
        }
        else {
            if(i.right())
                return _insert<_ctor>(i.right(), v);
            iterator j = _add_right<_ctor>(i, v);
            _balance_insert(j);
            return j;
        }
    }
    void _balance_insert(iterator i)
    {
        assert(i);
        iterator p = i.parent();
        if(!p)
            return;
        if(i.is_left()) {
            switch(p.get_balance())
            {
            case -1:
                (i.get_balance() == 1) ?
                    _left_right_rotate(p) :
                    _right_rotate(p);
                break;
            case 0:
                p.set_balance(-1);
                return _balance_insert(p);
            case 1:
                p.set_balance(0);
                break;
            default:
                assert(!"unexpected.");
                break;
            }
        }
        else {
            switch(p.get_balance())
            {
            case -1:
                p.set_balance(0);
                break;
            case 0:
                p.set_balance(1);
                return _balance_insert(p);
            case 1:
                (i.get_balance() == -1) ?
                    _right_left_rotate(p) :
                    _left_rotate(p);
                break;
            default:
                assert(!"unexpected.");
                break;
            }
        }
    }
    void _balance_erase(iterator i)
    {
        assert(i);
        int b = i.get_balance();
        if(!b) {
            i.set_balance(i.left() ? -1 : 1);
            return;
        }
        if(!i.left()) {
            if(b == -1)
                i.set_balance(0);
            else if(b == 1) {
                iterator r = i.right();
                if(!r)
                    i.set_balance(0);
                else {
                    (r.get_balance() == -1) ?
                        _right_left_rotate(i) :
                        _left_rotate(i);
                    i.to_parent();
                    if(i.get_balance() == -1)
                        return;
                }
            }
            else {
                assert(!"unexpected.");
            }
        }
        else if(!i.right()) {
            if(b == 1)
                i.set_balance(0);
            else if(b == -1) {
                iterator l = i.left();
                if(!l)
                    i.set_balance(0);
                else {
                    (l.get_balance() == 1) ?
                        _left_right_rotate(i) :
                        _right_rotate(i);
                    i.to_parent();
                    if(i.get_balance() == 1)
                        return;
                }
            }
            else {
                assert(!"unexpected.");
            }
        }
        _balance_erase_(i);
    }
    void _balance_erase_(iterator i)
    {
        assert(i);
        iterator p = i.parent();
        if(!p)
            return;
        if(i.is_left()) {
            switch(p.get_balance())
            {
            case -1:
                p.set_balance(0);
                return _balance_erase_(p);
            case 0:
                p.set_balance(1);
                break;
            case 1:
                (p.right().get_balance() == -1) ?
                    _right_left_rotate(p) :
                    _left_rotate(p);
                if(p.parent().get_balance() != -1)
                    return _balance_erase_(p.parent());
                break;
            default:
                assert(!"unexpected.");
                break;
            }
        }
        else {
            switch(p.get_balance())
            {
            case -1:
                (p.left().get_balance() == 1) ?
                    _left_right_rotate(p) :
                    _right_rotate(p);
                if(p.parent().get_balance() != 1)
                    return _balance_erase_(p.parent());
                break;
            case 0:
                p.set_balance(-1);
                break;
            case 1:
                p.set_balance(0);
                return _balance_erase_(p);
            default:
                assert(!"unexpected.");
                break;
            }
        }
    }
    iterator _left_rotate(iterator i)
    {
        assert(i && i.right());
        iterator p = i.parent();
        iterator r = i.right();
        iterator rl = r.left();
        if(p) {
            i.is_left() ? connect_left_child(p.get_wrapper(), r.get_wrapper()) :
                connect_right_child(p.get_wrapper(), r.get_wrapper());
        }
        else {
            _vptr = r.get_wrapper();
            _vptr->_parent = 0;
        }
        connect_left_child(r.get_wrapper(), i.get_wrapper());
        connect_right_child(i.get_wrapper(), rl.get_wrapper());
        if(r.get_balance() == 0) {
            i.set_balance(1);
            r.set_balance(-1);
        }
        else {
            i.set_balance(0);
            r.set_balance(0);
        }
        return r;
    }
    iterator _right_rotate(iterator i)
    {
        assert(i && i.left());
        iterator p = i.parent();
        iterator l = i.left();
        iterator lr = l.right();
        if(p) {
            i.is_left() ? connect_left_child(p.get_wrapper(), l.get_wrapper()) :
                connect_right_child(p.get_wrapper(), l.get_wrapper());
        }
        else {
            _vptr = l.get_wrapper();
            _vptr->_parent = 0;
        }
        connect_right_child(l.get_wrapper(), i.get_wrapper());
        connect_left_child(i.get_wrapper(), lr.get_wrapper());
        if(l.get_balance() == 0) {
            i.set_balance(-1);
            l.set_balance(1);
        }
        else {
            i.set_balance(0);
            l.set_balance(0);
        }
        return l;
    }
    iterator _left_right_rotate(iterator i)
    {
        assert(i && i.left());
        iterator l = i.left();
        iterator lr = l.right();
        assert(lr);
        iterator lrl = lr.left();
        iterator lrr = lr.right();
        iterator p = i.parent();
        if(p) {
            i.is_left() ? connect_left_child(p.get_wrapper(), lr.get_wrapper()) :
                connect_right_child(p.get_wrapper(), lr.get_wrapper());
        }
        else {
            _vptr = lr.get_wrapper();
            _vptr->_parent = 0;
        }
        connect_left_child(lr.get_wrapper(), l.get_wrapper());
        connect_right_child(lr.get_wrapper(), i.get_wrapper());
        connect_right_child(l.get_wrapper(), lrl.get_wrapper());
        connect_left_child(i.get_wrapper(), lrr.get_wrapper());
        switch(lr.get_balance())
        {
        case -1:
            i.set_balance(1);
            l.set_balance(0);
            break;
        case 0:
            i.set_balance(0);
            l.set_balance(0);
            break;
        case 1:
            i.set_balance(0);
            l.set_balance(-1);
            break;
        default:
            assert(!"unexpected.");
            break;
        }
        lr.set_balance(0);
        return lr;
    }
    iterator _right_left_rotate(iterator i)
    {
        assert(i && i.right());
        iterator r = i.right();
        iterator rl = r.left();
        assert(rl);
        iterator rll = rl.left();
        iterator rlr = rl.right();
        iterator p = i.parent();
        if(p) {
            i.is_left() ? connect_left_child(p.get_wrapper(), rl.get_wrapper()) :
                connect_right_child(p.get_wrapper(), rl.get_wrapper());
        }
        else {
            _vptr = rl.get_wrapper();
            _vptr->_parent = 0;
        }
        connect_right_child(rl.get_wrapper(), r.get_wrapper());
        connect_left_child(rl.get_wrapper(), i.get_wrapper());
        connect_right_child(i.get_wrapper(), rll.get_wrapper());
        connect_left_child(r.get_wrapper(), rlr.get_wrapper());
        switch(rl.get_balance())
        {
        case -1:
            i.set_balance(0);
            r.set_balance(1);
            break;
        case 0:
            i.set_balance(0);
            r.set_balance(0);
            break;
        case 1:
            i.set_balance(-1);
            r.set_balance(0);
            break;
        default:
            assert(!"unexpected.");
            break;
        }
        rl.set_balance(0);
        return rl;
    }

    template<class _ctor>
    iterator _init()
    {
        assert(!_vptr);
        _vptr = alloc::born();
        _vptr->born<_ctor>();
        return iterator(_vptr);
    }
    template<class _ctor>
    iterator _init(const value& v)
    {
        assert(!_vptr);
        _vptr = initval<_ctor, wrapper::tsf_behavior>::run(alloc::born(), v);
        assert(_vptr);
        return iterator(_vptr);
    }
    template<class _ctor>
    iterator _add_left(iterator i, const value& v)
    {
        assert(i && !i.left());
        wrapper* n = initval<_ctor, wrapper::tsf_behavior>::run(alloc::born(), v);
        connect_left_child(i.get_wrapper(), n);
        return iterator(n);
    }
    template<class _ctor>
    iterator _add_right(iterator i, const value& v)
    {
        assert(i && !i.right());
        wrapper* n = initval<_ctor, wrapper::tsf_behavior>::run(alloc::born(), v);
        connect_right_child(i.get_wrapper(), n);
        return iterator(n);
    }
    void _modified()
    {
#if defined (DEBUG) || defined (_DEBUG)
        debug_check(get_root());
#endif
    }

protected:
    friend struct initval;
    template<class _ctor, class _tsftrait>
    struct initval;
    template<class _ctor>
    struct initval<_ctor, _avltree_trait_copy>
    {
        static wrapper* run(wrapper* w, const value& v)
        {
            assert(w);
            w->get_ref() = v;
            return w;
        }
    };
    template<class _ctor>
    struct initval<_ctor, _avltree_trait_detach>
    {
        static wrapper* run(wrapper* w, const value& v)
        {
            assert(w);
            w->_value = &v;     /* or duplicate? */
            return w;
        }
    };

public:
    bool debug_check(iterator i)
    {
        if(!i)
            return true;
        check_root(i);
        check_linkage(i);
        check_order(i);
        check_balance(i);
        iterator l = i.left(), r = i.right();
        if(!(l && debug_check(l)))
            return false;
        if(!(r && debug_check(r)))
            return false;
        return true;
    }
    bool check_root(iterator i)
    {
        assert(i);
        if(is_root(i)) {
            assert(!i.parent() && "root has no parent.");
            return true;
        }
        assert(i.parent() && "non-root must have parent.");
        return false;
    }
    void check_linkage(iterator i)
    {
        assert(i);
        iterator l = i.left(), r = i.right();
        if(l) { assert(l.parent() == i && "left link wrong."); }
        if(r) { assert(r.parent() == i && "right link wrong."); }
    }
    void check_order(iterator i)
    {
        assert(i);
        iterator l = i.left(), r = i.right();
        if(l) {
            iterator lm = l;
            for(; lm.right(); lm = lm.right());
            assert(*lm < *i && "left order wrong.");
        }
        if(r) {
            iterator rm = r;
            for(; rm.left(); rm = rm.left());
            assert(*i < *rm && "right order wrong.");
        }
    }
    void check_balance(iterator i)
    {
        assert(i);
        iterator l = i.left(), r = i.right();
        int h1 = 0, h2 = 0;
        if(l) h1 = l.down_depth();
        if(r) h2 = r.down_depth();
        int b = i.get_balance();
        assert(b == h2 - h1 && "balance wrong.");
    }
};

__gslib_end__

#endif
