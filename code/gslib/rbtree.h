/* 
 * Copyright (c) 2016 lymastee, All rights reserved.
 * Contact: lymastee@hotmail.com
 *
 * This file is part of the GSLIB project.
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

#ifndef rbtree_ebcaa429_edae_4ac6_b9a5_2fe774778822_h
#define rbtree_ebcaa429_edae_4ac6_b9a5_2fe774778822_h

#include <assert.h>
#include <gslib/pool.h>
#include <gslib/std.h>

__gslib_begin__

struct _rbtree_trait_copy {};
struct _rbtree_trait_detach {};

enum rbtree_color
{
    rb_red,
    rb_black,
};

template<class _ty>
struct _rbtreenode_cpy_wrapper
{
    typedef _ty value;
    typedef _rbtreenode_cpy_wrapper<_ty> myref;
    typedef _rbtree_trait_copy tsf_behavior;
    typedef rbtree_color color;

    value               _value;
    myref*              _left;
    myref*              _right;
    myref*              _parent;
    color               _color;

    myref()
    {
        _left = _right = _parent = 0;
        _color = rb_black;
    }
    value* get_ptr() { return &_value; }
    const value* const_ptr() const { return &_value; }
    value& get_ref() { return _value; }
    const value& const_ref() const { return _value; }
    void born() {}
    void kill() {}
    template<class _cst>
    void born() {}
    template<class _cst>
    void kill() {}
    void copy(const myref* a) { get_ref() = a->const_ref(); }
    void attach(myref* a) { assert(0); }
    void swap_data(myref* a) { std::swap(_value, a->_value); }
    void set_color(color c) { _color = c; }
    color get_color() const { return _color; }
    bool is_red() const { return _color == rb_red; }
};

template<class _ty>
struct _rbtreenode_wrapper
{
    typedef _ty value;
    typedef _rbtreenode_wrapper<_ty> myref;
    typedef _rbtree_trait_detach tsf_behavior;
    typedef rbtree_color color;

    value*              _value;
    myref*              _left;
    myref*              _right;
    myref*              _parent;
    color               _color;

    myref()
    {
        _left = _right = _parent = 0;
        _value = 0;
        _color = rb_black;
    }
    value* get_ptr() { return _value; }
    const value* const_ptr() const { return _value; }
    value& get_ref() { return *_value; }
    const value& const_ref() const { return *_value; }
    void copy(const myref* a) { get_ref() = a->const_ref(); }
    void born() { !! }
    template<class _cst>
    void born() { _value = gs_new(_cst); }
    void kill() { if(_value) { gs_del(value, _value); _value = 0; } }
    template<class _cst>
    void kill() { if(_value) { gs_del(_cst, _value); _value = 0; } }
    void attach(myref* a)
    {
        assert(a && a->_value);
        kill();
        _value = a->_value;
        a->_value = 0;
    }
    void swap_data(myref* a) { gs_swap(_value, a->_value); }
    void set_color(color c) { _color = c; }
    color get_color() const { return _color; }
    bool is_red() const { return _color == rb_red; }
};

template<class _wrapper>
struct _rbtree_allocator
{
    typedef _wrapper wrapper;
    static wrapper* born() { return gs_new(wrapper); }
    static void kill(wrapper* w) { gs_del(wrapper, w); }
};

template<class _val>
struct _rbtreenode_val
{
    typedef _val value;
    typedef const _val const_value;
    typedef _rbtreenode_val<_val> myref;
    typedef rbtree_color color;

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
    color get_color() const { return _vptr->_color; }
    void set_color(color c) { _vptr->_color = c; }
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
    class _wrapper = _rbtreenode_cpy_wrapper<_ty> >
class _rbtree_const_iterator:
    public _rbtreenode_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _rbtree_const_iterator<_ty, _wrapper> iterator;

public:
    iterator(const wrapper* w = 0) { _cvptr = w; }
    bool is_valid() const { return _cvptr != 0; }
    const value* get_ptr() const { return _cvptr->const_ptr(); }
    const value* operator->() const { return _cvptr->const_ptr(); }
    const value& operator*() const { return _cvptr->const_ref(); }
    iterator left() const { return iterator(vleft()); }
    iterator right() const { return iterator(vright()) }
    iterator parent() const { return iterator(vparent()); }
    iterator sibling() const { return iterator(vsibling()); }
    iterator root() const { return iterator(vroot()); }
    bool operator==(const iterator& that) const { return _cvptr == that._cvptr; }
    bool operator!=(const iterator& that) const { return _cvptr != that._cvptr; }
};

template<class _ty,
    class _wrapper = _rbtreenode_cpy_wrapper<_ty> >
class _rbtree_iterator:
    public _rbtree_const_iterator<_ty, _wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _rbtree_const_iterator<_ty, _wrapper> const_iterator;
    typedef _rbtree_const_iterator<_ty, _wrapper> superref;
    typedef _rbtree_iterator<_ty, _wrapper> iterator;

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
    class _wrapper = _rbtreenode_cpy_wrapper<_ty>,
    class _alloc = _rbtree_allocator<_wrapper> >
class rbtree:
    public _rbtreenode_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _alloc alloc;
    typedef rbtree<value, wrapper, alloc> myref;
    typedef _rbtree_const_iterator<_ty, _wrapper> const_iterator;
    typedef _rbtree_iterator<_ty, _wrapper> iterator;
    typedef rbtree_color color;

public:
    rbtree() { _vptr = 0; }
    ~rbtree() { clear(); }
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
    bool is_root(iterator i) const { return i ? (_vptr == i.get_wrapper()) : false; }
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
        wrapper* f = _find(i.get_wrapper(), v);
        assert(f);
        if(!(f->const_ref() == v))
            f = 0;
        return iterator(f);
    }
    template<class _cst = value>
    iterator insert(const value& v)
    {
        wrapper* f = 0;
        if(_vptr) {
            f = _find(_vptr, v);
            assert(f);
            if(f->const_ref() == v)
                return iterator(f);
        }
        wrapper* n = initval<_cst, wrapper::tsf_behavior>::run(alloc::born(), v);
        n->set_color(rb_red);
        if(!f)
            _vptr = n;
        else {
            (v < f->const_ref()) ? connect_left_child(f, n) :
                connect_right_child(f, n);
        }
        _fix_insert(n);
        return iterator(n);
    }
    void erase(iterator i)
    {
        assert(i);
        wrapper* n = i.get_wrapper();
        wrapper *c, *p;
        rbtree_color cr;
        if(!n->_left)
            c = n->_right;
        else if(!n->_right)
            c = n->_left;
        else {
            wrapper* b = n, *l;
            n = n->_right;
            for(; l = n->_left; n = l);
            if(!b->_parent)
                _vptr = n;
            else {
                (b->_parent->_left == b) ? b->_parent->_left = n :
                    b->_parent->_right = n;
            }
            c = n->_right;
            p = n->_parent;
            cr = n->_color;
            if(p == b)
                p = n;
            else {
                if(c)
                    c->_parent = p;
                p->_left = c;
                n->_right = b->_right;
                b->_right->_parent = n;
            }
            n->_parent = b->_parent;
            n->_color = b->_color;
            n->_left = b->_left;
            b->_left->_parent = n;
            if(cr == rb_black)
                _fix_erase(c, p);
            return;
        }
        p = n->_parent;
        cr = n->_color;
        if(c)
            c->_parent = p;
        if(!p)
            _vptr = c;
        else {
            (p->_left == n) ? p->_left = c :
                p->_right = c;
        }
        if(cr == rb_black)
            _fix_erase(c, p);
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
    wrapper* _find(wrapper* w, const value& v) const
    {
        assert(w);
        wrapper* n = w;
        for(;;) {
            wrapper* m = 0;
            const value& r = n->const_ref();
            if(v < r)
                m = n->_left;
            else if(r < v)
                m = n->_right;
            if(!m)
                return n;
            n = m;
        }
        return 0;
    }
    void _left_rotate(wrapper* n)
    {
        assert(n);
        wrapper* r = n->_right;
        if(n->_right = r->_left)
            r->_left->_parent = n;
        r->_left = n;
        if(!(r->_parent = n->_parent))
            _vptr = r;
        else {
            (n == n->_parent->_right) ? n->_parent->_right = r :
                n->_parent->_left = r;
        }
        n->_parent = r;
    }
    void _right_rotate(wrapper* n)
    {
        assert(n);
        wrapper* l = n->_left;
        if(n->_left = l->_right)
            l->_right->_parent = n;
        l->_right = n;
        if(!(l->_parent = n->_parent))
            _vptr = l;
        else {
            (n == n->_parent->_right) ? n->_parent->_right = l :
                n->_parent->_left = l;
        }
        n->_parent = l;
    }
    void _fix_insert(wrapper* n)
    {
        assert(n);
        wrapper *p, *g, *u;
        while((p = n->_parent) && p->is_red()) {
            g = p->_parent;
            if(p == g->_left) {
                u = g->_right;
                if(u && u->is_red()) {
                    u->set_color(rb_black);
                    p->set_color(rb_black);
                    g->set_color(rb_red);
                    n = g;
                }
                else {
                    if(p->_right == n) {
                        _left_rotate(p);
                        gs_swap(p, n);
                    }
                    p->set_color(rb_black);
                    g->set_color(rb_red);
                    _right_rotate(g);
                }
            }
            else {
                u = g->_left;
                if(u && u->is_red()) {
                    u->set_color(rb_black);
                    p->set_color(rb_black);
                    g->set_color(rb_red);
                    n = g;
                }
                else {
                    if(p->_left == n) {
                        _right_rotate(p);
                        gs_swap(p, n);
                    }
                    p->set_color(rb_black);
                    g->set_color(rb_red);
                    _left_rotate(g);
                }
            }
        }
        _vptr->set_color(rb_black);
    }
    void _fix_erase(wrapper* n, wrapper* p)
    {
        wrapper *s, *sl, *sr;
        while((!n || !n->is_red()) && n != _vptr) {
            if(p->_left == n) {
                s = p->_right;
                if(s->is_red()) {
                    s->set_color(rb_black);
                    p->set_color(rb_red);
                    _left_rotate(p);
                    s = p->_right;
                }
                if((!s->_left || !s->_left->is_red()) &&
                    (!s->_right || !s->_right->is_red())
                    ) {
                    s->set_color(rb_red);
                    n = p;
                    p = n->_parent;
                }
                else {
                    if(!s->_right || !s->_right->is_red()) {
                        if(sl = s->_left)
                            sl->set_color(rb_black);
                        s->set_color(rb_red);
                        _right_rotate(s);
                        s = p->_right;
                    }
                    s->set_color(p->get_color());
                    p->set_color(rb_black);
                    if(sr = s->_right)
                        sr->set_color(rb_black);
                    _left_rotate(p);
                    n = _vptr;
                    break;
                }
            }
            else {
                s = p->_left;
                if(s->is_red()) {
                    s->set_color(rb_black);
                    p->set_color(rb_red);
                    _right_rotate(p);
                    s = p->_left;
                }
                if((!s->_left || !s->_left->is_red()) &&
                    (!s->_right || !s->_right->is_red())
                    ) {
                    s->set_color(rb_red);
                    n = p;
                    p = n->_parent;
                }
                else {
                    if(!s->_left || !s->_left->is_red()) {
                        if(sr = s->_right)
                            sr->set_color(rb_black);
                        s->set_color(rb_red);
                        _left_rotate(s);
                        s = p->_left;
                    }
                    s->set_color(p->get_color());
                    p->set_color(rb_black);
                    if(sl = s->_left)
                        sl->set_color(rb_black);
                    _right_rotate(p);
                    n = _vptr;
                    break;
                }
            }
        }
        if(n)
            n->set_color(rb_black);
    }

protected:
    friend struct initval;
    template<class _cst, class _tsftrait>
    struct initval;
    template<class _cst>
    struct initval<_cst, _rbtree_trait_copy>
    {
        static wrapper* run(wrapper* w, const value& v)
        {
            assert(w);
            w->get_ref() = v;
            return w;
        }
    };
    template<class _cst>
    struct initval<_cst, _rbtree_trait_detach>
    {
        static wrapper* run(wrapper* w, const value& v)
        {
            assert(w);
            w->_value = &v;     /* or duplicate? */
            return w;
        }
    };
};

__gslib_end__

#endif
