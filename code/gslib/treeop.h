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

#ifndef treeop_00fc8cde_c6b6_464d_b51d_71eaad31b9ef_h
#define treeop_00fc8cde_c6b6_464d_b51d_71eaad31b9ef_h

#include <gslib/tree.h>

__gslib_begin__

/*
 * The original tree was created from top to bottom, to implement the new operation it might be needed
 * to create the tree from bottom to top.
 * These classes were designed to achieve that.
 * Caution:
 * You'd better NOT include me in headers.
 * You'd better use def_tr$ macros in a function scope.
 * with argumented def_tr$ versions, IE. like def_tr$_v1, def_tr$_v2, and so on...
 * the tree should NOT be a copy wrapper one.
 */

template<class _tree>
class treeop_helper
{
public:
    typedef _tree tree;
    typedef typename _tree::value value;
    typedef typename _tree::wrapper wrapper;
    typedef typename _tree::alloc alloc;
    typedef typename _tree::const_iterator const_iterator;
    typedef typename _tree::iterator iterator;
    typedef typename wrapper::children children;
};

template<class _tree>
class treeop_set
{
public:
    typedef treeop_helper<_tree> helper;
    typedef typename helper::tree tree;
    typedef typename helper::value value;
    typedef typename helper::wrapper wrapper;
    typedef typename helper::alloc alloc;
    typedef typename helper::const_iterator const_iterator;
    typedef typename helper::iterator iterator;
    typedef typename helper::children children;
    typedef children opset;

public:
    opset           _opset;

public:
    treeop_set() {}
    treeop_set(wrapper* w) { _opset.init(w); }
    treeop_set(treeop_set& rhs) { _opset.swap(rhs.dump()); }
    bool empty() const { return _opset.empty(); }
    bool unary() const { return _opset.size() == 1; }
    opset& dump() { return _opset; }
    treeop_set& operator+ (treeop_set& rhs) { return connect(rhs); }
    void acquire(treeop_set& ops)
    {
        assert(unary());
        wrapper* m = _opset.front();
        assert(m);
        m->acquire_children(ops.dump());
    }
    treeop_set& sub(treeop_set& ops)
    {
        acquire(ops);
        return *this;
    }
    treeop_set& connect(treeop_set& ops)
    {
        _opset.connect(ops.dump());
        return *this;
    }
    wrapper* detach()
    {
        assert(unary());
        wrapper* w = _opset.front();
        _opset.reset();
        return w;
    }
};

template<class _tree>
class treeop
{
public:
    typedef treeop_helper<_tree> helper;
    typedef typename helper::tree tree;
    typedef typename helper::value value;
    typedef typename helper::wrapper wrapper;
    typedef typename helper::alloc alloc;
    typedef typename helper::const_iterator const_iterator;
    typedef typename helper::iterator iterator;
    typedef typename helper::children children;
    typedef treeop_set<_tree> tropset;
    typedef children opset;

public:
    treeop(_tree& tr) { _tr = &tr; }
    void create(tropset& ops) { _tr->set_root(ops.detach()); }

protected:
    _tree*          _tr;
};

/*
 * Usage:
 * For example,
 * 1.if we wanted to create a tree in the following structure,
 *   { a = 1, b = "hello"; }
 *        { a = 2, b = "bingo"; }
 *            { a = 3, b = "bravo"; }
 *        { a = 6, b = "amigo"; }
 *   and the node was for: struct node { int a; string b; };
 *   We could write it like,
 *   typedef tree<node, _treenode_wrapper<node> > mytree;
 *   def_tr$(node1, mytree, ([&](){ a = 1, b = "hello"; }));
 *   def_tr$(node2, mytree, ([&](){ a = 2, b = "bingo"; }));
 *   def_tr$(node3, mytree, ([&](){ a = 3, b = "bravo"; }));
 *   def_tr$(node4, mytree, ([&](){ a = 6, b = "amigo"; }));
 *   tr$(node1).sub(
 *      tr$(node2).sub(
 *          tr$(node3)
 *          )
 *      + tr$(node4)
 *      );
 * 2.if your compiler doesn't support c++0x style of lambda,
 *   you may modify the def_tr$ micro a little bit, like assignments;
 *   then you could temporally write it like:
 *   typedef tree<node, _treenode_wrapper<node> > mytree;
 *   def_tr$(node1, mytree, (a = 1, b = "hello";));
 *   it works, just maybe unsafe.
 */

#define def_tr$(name, treeop, prototype, assignments) \
struct tag##name \
{ \
    typedef treeop::wrapper wrapper; \
    typedef treeop::value value; \
    typedef treeop::alloc alloc; \
    typedef treeop::tropset tropset; \
    struct constructor: \
        public prototype \
    { \
        constructor() { (assignments)(); } \
    }; \
    tropset create() \
    { \
        wrapper* w = alloc::born(); \
        w->born<constructor>(); \
        return tropset(w); \
    } \
} name;

#define def_tr$_v1(name, treeop, prototype, arg1, assignments) \
struct tag##name \
{ \
    typedef treeop::wrapper wrapper; \
    typedef treeop::value value; \
    typedef treeop::alloc alloc; \
    typedef treeop::tropset tropset; \
    typedef decltype(arg1) type1; \
    type1& ref_arg1; \
    tag##name(type1& arg1): ref_arg1(arg1) {} \
    struct constructor: \
        public prototype \
    { \
        constructor(type1& arg1) { (assignments)(arg1); } \
    }; \
    tropset create() \
    { \
        wrapper* w = alloc::born(); \
        w->born<constructor>(ref_arg1); \
        return tropset(w); \
    } \
} name(arg1);

#define def_tr$_v2(name, treeop, prototype, arg1, arg2, assignments) \
struct tag##name \
{ \
    typedef treeop::wrapper wrapper; \
    typedef treeop::value value; \
    typedef treeop::alloc alloc; \
    typedef treeop::tropset tropset; \
    typedef decltype(arg1) type1; \
    typedef decltype(arg2) type2; \
    type1& ref_arg1; \
    type2& ref_arg2; \
    tag##name(type1& arg1, type2& arg2): ref_arg1(arg1), ref_arg2(arg2) {} \
    struct constructor: \
        public prototype \
    { \
        constructor(type1& arg1, type2& arg2) { (assignments)(arg1, arg2); } \
    }; \
    tropset create() \
    { \
        wrapper* w = alloc::born(); \
        w->born<constructor>(ref_arg1, ref_arg2); \
        return tropset(w); \
    } \
} name(arg1, arg2);

#define def_tr$_v3(name, treeop, prototype, arg1, arg2, arg3, assignments) \
struct tag##name \
{ \
    typedef treeop::wrapper wrapper; \
    typedef treeop::value value; \
    typedef treeop::alloc alloc; \
    typedef treeop::tropset tropset; \
    typedef decltype(arg1) type1; \
    typedef decltype(arg2) type2; \
    typedef decltype(arg3) type3; \
    type1& ref_arg1; \
    type2& ref_arg2; \
    type3& ref_arg3; \
    tag##name(type1& arg1, type2& arg2, type3& arg3): ref_arg1(arg1), ref_arg2(arg2), ref_arg3(arg3) {} \
    struct constructor: \
        public prototype \
    { \
        constructor(type1& arg1, type2& arg2, type3& arg3) { (assignments)(arg1, arg2, arg3); } \
    }; \
    tropset create() \
    { \
        wrapper* w = alloc::born(); \
        w->born<constructor>(ref_arg1, ref_arg2, ref_arg3); \
        return tropset(w); \
    } \
} name(arg1, arg2, arg3);

#define tr$(name) (name.create())

__gslib_end__

#endif
