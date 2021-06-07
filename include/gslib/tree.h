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

#ifndef tree_5172eda7_51f9_405a_9f3b_bcaa5e4243c1_h
#define tree_5172eda7_51f9_405a_9f3b_bcaa5e4243c1_h

#include <assert.h>
#include <gslib/std.h>
#include <gslib/string.h>

__gslib_begin__

struct _tree_trait_copy {};
struct _tree_trait_detach {};

template<class _node>
struct _treenode_list
{
public:
    typedef _node mynode;
    typedef _treenode_list<_node> mylist;

protected:
    mynode*         _first;
    mynode*         _last;
    int             _size;

public:
    _treenode_list() { reset(); }
    _treenode_list(mynode* node) { setup(node, node, 1); }
    _treenode_list(mynode* first, mynode* last, int size) { setup(first, last, size); }
    _treenode_list(mynode* first, mynode* last) { setup(first, last, count_nodes(first, last)); }
    bool empty() const { return !_size; }
    int size() const { return _size; }
    mynode* front() const { return _first; }
    mynode* back() const { return _last; }
    template<class _lambda>
    void for_each(_lambda lam) { for_each(_first, _last, lam); }

public:
    void setup(mynode* first, mynode* last, int size)
    {
        _first = first;
        _last = last;
        _size = size;
    }
    void reset()
    {
        _first = _last = nullptr;
        _size = 0;
    }
    void swap(mylist& that)
    {
        gs_swap(_first, that._first);
        gs_swap(_last, that._last);
        gs_swap(_size, that._size);
    }
    void erase(mynode* node)
    {
        assert(node);
        mynode* prev = node->prev();
        mynode* next = node->next();
        join(prev, next);
        if(node == _first)
            _first = next;
        if(node == _last)
            _last = prev;
        node->_prev = node->_next = nullptr;
        _size --;
    }
    void init(mynode* node)
    {
        assert(node);
        assert(!_first && !_last && !_size);
        _first = _last = node;
        _size = 1;
    }
    void insert_before(mynode* pos, mynode* node)
    {
        assert(node);
        if(pos == nullptr)
            return init(node);
        join(pos->prev(), node);
        join(node, pos);
        if(pos == _first)
            _first = node;
        _size ++;
    }
    void insert_after(mynode* pos, mynode* node)
    {
        assert(node);
        if(pos == nullptr)
            return init(node);
        join(node, pos->next());
        join(pos, node);
        if(pos == _last)
            _last = node;
        _size ++;
    }
    void connect(mylist& that)
    {
        if(empty())
            return swap(that);
        else if(that.empty())
            return;
        assert(!empty() && !that.empty());
        join(back(), that.front());
        _size += that.size();
        _last = that.back();
        that.reset();
    }

public:
    static int count_nodes(mynode* first, mynode* last)
    {
        assert(first && last);
        int c = 0;
        mynode* n = first;
        for(;;) {
            c ++;
            if(n == last)
                return c;
            n = n->next();
        }
        return c;
    }
    static void join(mynode* node1, mynode* node2)
    {
        if(node1 != nullptr)
            node1->_next = node2;
        if(node2 != nullptr)
            node2->_prev = node1;
    }
    template<class _lambda>
    static void for_each(mynode* first, mynode* last, _lambda lam)
    {
        if(!first)
            return;
        for(mynode* node = first; node != last; ) {
            mynode* next = node->next();
            lam(node);
            node = next;
        }
        lam(last);
    }
};

template<class _wrapper>
struct _treenode_default_wrapper
{
    typedef _wrapper wrapper;
    typedef _treenode_default_wrapper<wrapper> default_wrapper;
    typedef _treenode_list<wrapper> children;
    template<class, class, class> friend class tree;
    template<class> friend struct _treenode_list;

protected:
    wrapper*        _parent;
    wrapper*        _prev;
    wrapper*        _next;
    children        _children;

public:
    default_wrapper() { _parent = _prev = _next = nullptr; }
    int childs() const { return _children.size(); }
    wrapper* parent() const { return _parent; }
    wrapper* prev() const { return _prev; }
    wrapper* next() const { return _next; }
    wrapper* child() const { return _children.empty() ? nullptr : _children.front(); }
    wrapper* last_child() const { return _children.empty() ? nullptr : _children.back(); }
    void birth_before(wrapper* pos, wrapper* node) { return _children.insert_before(pos, node); }
    void birth_after(wrapper* pos, wrapper* node) { return _children.insert_after(pos, node); }
    void erase(wrapper* node) { return _children.erase(node); }
    void acquire_children(children& chs)
    {
        chs.for_each([this](wrapper* w) { w->_parent = static_cast<wrapper*>(this); });
        _children.swap(chs);
    }
    void swap_children(wrapper& w)
    {
        children t;
        t.swap(_children);
        w.acquire_children(t);
        acquire_children(t);
    }
    void reset_children() { _children.reset(); }
    template<class _lambda>
    void for_each(_lambda lam) { _children.for_each(lam); }
};

template<class _ty>
struct _treenode_cpy_wrapper:
    public _treenode_default_wrapper<_treenode_cpy_wrapper<_ty> >
{
    typedef _ty value;
    typedef _treenode_cpy_wrapper<_ty> wrapper;
    typedef _treenode_list<wrapper> children;
    typedef _tree_trait_copy tsf_behavior;
    template<class, class, class> friend class tree;

protected:
    value           _value;

public:
    value* get_ptr() { return &_value; }
    const value* get_ptr() const { return &_value; }
    value& get_ref() { return _value; }
    const value& get_ref() const { return _value; }
    template<class _ctor>
    void born() {}
    void born() {}
    void kill() {}
    void copy(const wrapper* a) { _value = a->_value; }
    void attach(wrapper* a) { !!error!! }
};

template<class _ty>
struct _treenode_wrapper:
    public _treenode_default_wrapper<_treenode_wrapper<_ty> >
{
    typedef _ty value;
    typedef _treenode_wrapper<_ty> wrapper;
    typedef _treenode_list<wrapper> children;
    typedef _tree_trait_detach tsf_behavior;
    template<class, class, class> friend class tree;

protected:
    value*          _value;

public:
    wrapper() { _value = nullptr; }
    value* get_ptr() const { return _value; }
    value& get_ref() const { return *_value; }
    template<class _ctor>
    void born() { _value = new _ctor; }
    template<class _ctor, class _a1>
    void born(_a1& a) { _value = new _ctor(a); }
    template<class _ctor, class _a1, class _a2>
    void born(_a1& a, _a2& b) { _value = new _ctor(a, b); }
    template<class _ctor, class _a1, class _a2, class _a3>
    void born(_a1& a, _a2& b, _a3& c) { _value = new _ctor(a, b, c); }
    void born() { _value = new value; }
    void kill() { delete _value; }
    void copy(const wrapper* a) { assert(!"prevent."); }
    void attach(wrapper* a)
    {
        assert(a && a->_value);
        kill();
        _value = a->_value;
        a->_value = nullptr;
    }
};

template<class _wrapper>
struct _tree_allocator
{
    typedef _wrapper wrapper;
    static wrapper* born() { return new wrapper; }
    static void kill(wrapper* w) { delete w; }
};

template<class _val>
struct _treenode_val
{
    typedef _val value;
    typedef const _val const_value;
    template<class, class, class> friend class tree;
    union
    {
        value*          _vptr;
        const_value*    _cvptr;
    };
    value* get_wrapper() const { return _vptr; }
    operator bool() const { return _vptr != nullptr; }

protected:
    value* vparent() const { return _vptr ? _vptr->_parent : nullptr; }
    value* vchild() const { return _vptr ? _vptr->front() : nullptr; }
    value* vchild_last() const { return _vptr ? _vptr->back() : nullptr; }
    value* vsibling() const {  return _vptr ? _vptr->next() : nullptr; }
    value* vsibling_last() const { return _vptr ? _vptr->prev() : nullptr; }

protected:
    template<class _lambda, class _value>
    static void preorder_traversal(_lambda lam, _value* v)
    {
        assert(v);
        lam(v);
        for(_value* w = v->child(); w; w = w->next())
            preorder_traversal(lam, w);
    }
    template<class _lambda, class _value>
    static void postorder_traversal(_lambda lam, _value* v)
    {
        assert(v);
        for(_value* w = v->child(); w; w = w->next())
            postorder_traversal(lam, w);
        lam(v);
    }

public:
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
    class _wrapper = _treenode_cpy_wrapper<_ty> >
class _tree_const_iterator:
    public _treenode_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _tree_const_iterator<value, wrapper> iterator;
    template<class, class, class> friend class tree;
    
public:
    iterator(const wrapper* w = nullptr) { _cvptr = w; }
    bool is_valid() const { return _cvptr != nullptr; }
    const value* get_ptr() const { return _cvptr->get_ptr(); }
    const value* operator->() const { return _cvptr->get_ptr(); }
    const value& operator*() const { return _cvptr->get_ref(); }
    int childs() const { return _cvptr->childs(); }
    int siblings() const { return _cvptr->parent() ? _cvptr->parent()->childs() : 1; }
    bool is_root() const { return _cvptr ? !_cvptr->parent() : false; }
    bool is_leaf() const { return _cvptr ? !_cvptr->childs() : false; }
    bool operator==(const iterator& that) const { return _cvptr == that._cvptr; }
    bool operator!=(const iterator& that) const { return _cvptr != that._cvptr; }
    iterator child() const { return _cvptr ? iterator(_cvptr->child()) : iterator(); }
    iterator last_child() const { return _cvptr ? iterator(_cvptr->last_child()) : iterator(); }
    iterator parent() const { return _cvptr ? iterator(_cvptr->parent()) : iterator(); }
    iterator next() const { return _cvptr ? iterator(_cvptr->next()) : iterator(); }
    iterator prev() const { return _cvptr ? iterator(_cvptr->prev()) : iterator(); }
    iterator root() const
    {
        if(const wrapper* w = _cvptr)
        {
            for( ; w->parent(); w = w->parent());
            return iterator(w);
        }
        return iterator();
    }
    iterator eldest() const
    {
        if(const wrapper* w = _cvptr ? _cvptr->parent() : nullptr)
            return iterator(w->child());
        return iterator();
    }
    iterator youngest() const
    {
        if(const wrapper* w = _cvptr ? _cvptr->parent() : nullptr)
            return iterator(w->last_child());
        return iterator();
    }
};

template<class _ty,
    class _wrapper = _treenode_cpy_wrapper<_ty> >
class _tree_iterator:
    public _tree_const_iterator<_ty, _wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _tree_const_iterator<value, wrapper> superref;
    typedef _tree_const_iterator<value, wrapper> const_iterator;
    typedef _tree_iterator<value, wrapper> iterator;
    template<class, class, class> friend class tree;

public:
    iterator(wrapper* w = nullptr): superref(w) {}
    value* get_ptr() const { return _vptr->get_ptr(); }
    value* operator->() const { return _vptr->get_ptr(); }
    value& operator*() const { return _vptr->get_ref(); }
    bool operator==(const iterator& that) const { return _vptr == that._vptr; }
    bool operator!=(const iterator& that) const { return _vptr != that._vptr; }
    bool operator==(const const_iterator& that) const { return _vptr == that._vptr; }
    bool operator!=(const const_iterator& that) const { return _vptr != that._vptr; }
    operator const_iterator() { return const_iterator(_cvptr); }
    void to_root() { while(wrapper* w = _vptr->parent()) _vptr = w; }
    void to_child() { _vptr = _vptr->child(); }
    void to_last_child() { _vptr = _vptr->last_child(); }
    void to_parent() { _vptr = _vptr->parent(); }
    void to_next() { _vptr = _vptr->next(); }
    void to_prev() { _vptr = _vptr->prev(); }
    void to_eldest() { if(wrapper* w = _vptr->parent()) _vptr = w->child(); }
    void to_youngest() { if(wrapper* w = _vptr->parent()) _vptr = w->last_child(); }
    iterator root() const
    {
        iterator i(_vptr);
        i.to_root();
        return i;
    }
    iterator child() const { return _vptr ? iterator(_vptr->child()) : iterator(); }
    iterator last_child() const { return _vptr ? iterator(_vptr->last_child()) : iterator(); }
    iterator parent() const { return _vptr ? iterator(_vptr->parent()) : iterator(); }
    iterator next() const { return _vptr ? iterator(_vptr->next()) : iterator(); }
    iterator prev() const { return _vptr ? iterator(_vptr->prev()) : iterator(); }
    iterator eldest() const
    {
        iterator i(_vptr);
        i.to_eldest();
        return i;
    }
    iterator youngest() const
    {
        iterator i(_vptr);
        i.to_youngest();
        return i;
    }
};

template<class _ty, 
    class _wrapper = _treenode_cpy_wrapper<_ty>,
    class _alloc = _tree_allocator<_wrapper> >
class tree:
    public _treenode_val<_wrapper>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _alloc alloc;
    typedef tree<value, wrapper, alloc> myref;
    typedef _tree_const_iterator<value, wrapper> const_iterator;
    typedef _tree_iterator<value, wrapper> iterator;
    friend class const_iterator;
    friend class iterator;

public:
    tree() { _vptr = nullptr; }
    ~tree() { destroy(); }
    void destroy() { erase(get_root()); }
    void clear() { destroy(); }
    void swap(myref& that) { gs_swap(_vptr, that._vptr); }
    iterator get_root() { return iterator(_vptr); }
    const_iterator get_root() const { return const_iterator(_cvptr); }
    bool is_valid() const { return _cvptr != nullptr; }
    iterator insert(iterator i) { return insert<value>(i); }
    iterator insert_after(iterator i) { return insert_after<value>(i); }
    iterator birth(iterator i) { return birth<value>(i); }
    iterator birth_tail(iterator i) { return birth_tail<value>(i); }

public:
    bool is_root(const_iterator i) const
    {
        if(!i.is_valid())
            return false;
        return _cvptr == i._cvptr;
    }
    bool is_mine(const_iterator i) const
    {
        if(!i.is_valid())
            return false;
        return is_root(i.root());
    }
    void set_root(wrapper* w)
    {
        assert(!_vptr && "use attach method.");
        _vptr = w;
    }
    void erase(iterator i)
    {
        if(!i.is_valid())
            return;
        if(is_root(i)) {
            _erase(i);
            _vptr = nullptr;
        }
        else {
            wrapper* w = i._vptr;
            wrapper* p = w->parent();
            p->erase(w);
            _erase(i);
        }
    }
    template<class _ctor>
    iterator insert(iterator i)
    {
        if(!i.is_valid())
            return _cvptr ? get_root() : _init<_ctor>();
        wrapper* w = i._vptr;
        wrapper* p = w->_parent;
        wrapper* n = alloc::born();
        n->born<_ctor>();
        p->birth_before(w, n);
        n->_parent = p;
        return iterator(n);
    }
    template<class _ctor>
    iterator insert_after(iterator i)
    {
        if(!i.is_valid())
            return _cvptr ? get_root() : _init<_ctor>();
        wrapper* w = i._vptr;
        wrapper* p = w->_parent;
        wrapper* n = alloc::born();
        n->born<_ctor>();
        p->birth_after(w, n);
        n->_parent = p;
        return iterator(n);
    }
    template<class _ctor>
    iterator birth(iterator i)
    {
        if(!i.is_valid())
            return _cvptr ? get_root() : _init<_ctor>();
        wrapper* w = i._vptr;
        wrapper* n = alloc::born();
        n->born<_ctor>();
        w->birth_before(w->child(), n);
        n->_parent = w;
        return iterator(n);
    }
    template<class _ctor>
    iterator birth_tail(iterator i)
    {
        if(!i.is_valid())
            return _cvptr ? get_root() : _init<_ctor>();
        wrapper* w = i._vptr;
        wrapper* n = alloc::born();
        n->born<_ctor>();
        w->birth_after(w->last_child(), n);
        n->_parent = w;
        return iterator(n);
    }
    myref& detach(myref& subtree, iterator i)
    {
        if(!i.is_valid() || !is_mine(i))
            return subtree;
        if(is_root(i))
            _vptr = nullptr;
        else {
            wrapper* m = i._vptr;
            wrapper* p = m->_parent;
            p->erase(m);
            m->_parent = nullptr;
        }
        subtree.~tree();
        new (&subtree) tree(i);
        return subtree;
    }
    template<class _tsftrait>
    void transfer(wrapper* src, wrapper* des)
    {
    }
    template<>
    void transfer<_tree_trait_copy>(wrapper* src, wrapper* des)
    {
        assert(src && des);
        des->copy(src);
        des->swap_children(*src);
        src->for_each([&](wrapper* w) { _erase(w); });
        src->reset_children();
    }
    template<>
    void transfer<_tree_trait_detach>(wrapper* src, wrapper* des)
    {
        assert(src && des);
        des->attach(src);
        des->swap_children(*src);
        src->for_each([&](wrapper* w) { _erase(w); });
        src->reset_children();
    }
    void attach(myref& subtree, iterator i)
    {
        assert(i.is_valid() && is_mine(i));
        wrapper* w = subtree.get_root()._vptr;
        assert(w);
        transfer<wrapper::tsf_behavior>(w, i._vptr);
        w = i._vptr;
        for(wrapper* c = w->child(); c; c = c->next())
            c->_parent = w;
    }
    template<class _ctor>
    iterator swap(iterator p, myref& that)
    {
        assert(this != &that);
        if(p == get_root()) {
            swap(that);
            return get_root();
        }
        iterator prevsib, grand;
        prevsib = p.prev();
        grand = p.parent();
        assert(prevsib || grand);
        myref t;
        detach(t, p);
        auto pos = prevsib ? insert_after<_ctor>(prevsib) : birth<_ctor>(grand);
        assert(pos);
        attach(that, pos);
        that.swap(t);
        return pos;
    }

public:
    template<class lambda>
    void for_each(lambda lam) { for_each(get_root(), lam); }
    template<class lambda>
    void for_each(iterator i, lambda lam)
    {
        if(i.is_valid()) {
            for(iterator j = i.child(); j.is_valid(); j.to_next())
                for_each(j, lam);
            lam(i.get_ptr());
        }
    }
    template<class lambda>
    void const_for_each(lambda lam) const { const_for_each(get_root(), lam); }
    template<class lambda>
    void const_for_each(const_iterator i, lambda lam) const
    {
        if(i.is_valid()) {
            for(const_iterator j = i.child(); j.is_valid(); j = j.next())
                for_each(j, lam);
            lam(i.get_ptr());
        }
    }

protected:
    tree(iterator i) { _vptr = i._vptr; }
    void _erase(iterator i) { _erase(i.get_wrapper()); }
    void _erase(wrapper* w)
    {
        assert(w);
        for(auto* c = w->child(); c;) {
            auto* n = c->next();
            _erase(c);
            c = n;
        }
        w->kill();
        alloc::kill(w);
    }
    template<class _ctor>
    iterator _init()
    {
        _vptr = alloc::born();
        _vptr->born<_ctor>();
        _vptr->_parent = nullptr;
        return iterator(_vptr);
    }

public:
    bool debug_check(iterator i)
    {
        iterator f = i.prev(), b = i.next();
        if(f.is_valid() && f.next() != i) { assert(0); return false; }
        if(b.is_valid() && b.prev() != i) { assert(0); return false; }
        for(iterator j = i.child(); j.is_valid(); j.to_next())
            if(j.parent() != i) { assert(0); return false; }
        return true;
    }
};

template<class _tree>
class _print_tree
{
public:
    typedef _tree tree;
    typedef typename tree::value value;
    typedef typename tree::iterator iterator;
    typedef typename tree::const_iterator const_iterator;

protected:
    tree&       _inst;
    string      _src;

public:
    _print_tree(tree& t):
        _inst(t)
    {
    }
    const gchar* print()
    {
        _src.clear();
        append(_inst.get_root(), 0);
        return _src.c_str();
    }

protected:
    string decorate(const_iterator i, int level)
    {
        if(!i.is_valid())
            return string();
        string r, a;
        for(int j = 0; j < level; j ++)
            r.append(_t("  "));
        i.is_leaf() ? r.append(_t("+ ")) : r.append(_t("- "));
        i.is_root() ? r.append(_t("root(")) : a.format(_t("level%d("), level);
        r.append(a);
        r.append(i->to_string());
        r.append(_t(")\n"));
        return r;
    }
    void append(const_iterator i, int level)
    {
        if(!i.is_valid())
            return;
        _src.append(decorate(i, level).c_str());
        level ++;
        for(i.to_child(); i.is_valid(); i.to_next())
            append(i, level);
    }
};

__gslib_end__

#endif
