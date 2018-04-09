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

#ifndef rectpack_89b58399_0f97_4cb9_9160_0226504762b1_h
#define rectpack_89b58399_0f97_4cb9_9160_0226504762b1_h

#include <ariel/config.h>
#include <gslib/bintree.h>
#include <gslib/std.h>
#include <gslib/math.h>
#include <pink/type.h>

__ariel_begin__

struct rp_rect
{
    float               x, y;                           /* left, top */
    float               width, height;

public:
    rp_rect() { x = y = width = height = 0.f; }
    rp_rect(float l, float t, float w, float h) { set_rect(l, t, w, h); }
    void set_rect(float l, float t, float w, float h) { x = l, y = t, width = w, height = h; }
    float left() const { return x; }
    float top() const { return y; }
    float right() const { return x + width; }
    float bottom() const { return y + height; }
    void move_to(float destx, float desty) { x = destx, y = desty; }
    void offset(float cx, float cy) { x += cx, y += cy; }
    float area() const { return width * height; }
};

struct rp_node:
    public rp_rect
{
    bool                transposed;
    void*               bind_ptr;

public:
    rp_node()
    {
        transposed = false;
        bind_ptr = nullptr;
    }
    virtual ~rp_node() {}
    virtual bool is_dynamic() const { return false; }
    virtual void set_transposed(bool b) { transposed = b; }
    virtual string to_string() const { return string(); }   /* for debug only */
    void set_bind_ptr(void* p) { bind_ptr = p; }
    bool is_empty() const { return !bind_ptr; }
};

struct rp_dynamic_node:
    public rp_node
{
    rp_rect             blank;
    bool                has_blank;                      /* tag of blank.                | only valid on a non-leaf node. */
    bool                is_vert_arrange;                /* vertical arrangement?        | only valid on a non-leaf node. */

public:
    rp_dynamic_node() { has_blank = false; }
    virtual bool is_dynamic() const override { return true; }
    virtual void set_transposed(bool b) override;
    virtual string to_string() const override;
    float get_area() const;
    void set_no_blank();
    void set_blank(const rp_rect& rc);
    void set_from_local(rp_dynamic_node& p);
    void set_dimensions(float w, float h);
    void move_to_pos(float x, float y);
    void flip_leaf();
    void flip_non_leaf();
};

typedef _bintreenode_wrapper<rp_node> rp_wrapper;
typedef _bintree_allocator<rp_wrapper> rp_allocator;
typedef bintree<rp_node, rp_wrapper, rp_allocator> rp_tree;
typedef rp_tree::iterator rp_iterator;
typedef rp_tree::const_iterator rp_const_iterator;

struct rp_input
{
    float               width, height;
    void*               binding;
};

typedef list<rp_input> rp_input_list;

extern void rp_global_location(rp_const_iterator i, rp_rect& rc);
extern bool rp_is_node_transposed(rp_const_iterator i);

class rect_packer
{
public:
    enum packing_strategy
    {
        ps_unknown,
        ps_compactly,
        ps_dynamically,
    };

public:
    rect_packer();
    ~rect_packer();
    const rp_node& get_root_node() const { return *_tree.const_root(); }
    bool is_empty() const { return !_tree.is_valid(); }
    float get_width() const { return _tree.const_root()->width; }
    float get_height() const { return _tree.const_root()->height; }
    int get_pack_times() const { return _pack_times; }
    void pack_automatically(rp_input_list& inputs);
    void tracing() const;
    void trace_total() const;
    void trace_blank() const;
    void trace_tree() const;
    bool checkups() const;

public:
    template<class _lamb>
    void for_each_compactly(rp_iterator i, _lamb fn)
    {
        assert(i);
        _tree.preorder_traversal([&fn](rp_wrapper* w) {
            assert(w);
            rp_iterator i(w);
            if(i.is_leaf() && !i->is_empty()) {
                rp_rect rc;
                rp_global_location(i, rc);
                fn(i->bind_ptr, rc, i->transposed);
            }
        });
    }
    template<class _lamb>
    void for_each_dynamically(rp_iterator i, _lamb fn)
    {
        assert(i);
        _tree.preorder_traversal([&fn](rp_wrapper* w) {
            assert(w);
            rp_iterator i(w);
            if(i.is_leaf() && !i->is_empty()) {
                rp_rect rc;
                rp_global_location(i, rc);
                fn(i->bind_ptr, rc, rp_is_node_transposed(i));
            }
        });
    }
    template<class _lamb>
    void for_each(_lamb fn)
    {
        if(!_tree.is_valid())
            return;
        switch(_strategy)
        {
        case ps_compactly:
            return for_each_compactly(_tree.get_root(), fn);
        case ps_dynamically:
            return for_each_dynamically(_tree.get_root(), fn);
        default:
            assert(!"unexpected.");
        }
    }

protected:
    rp_tree             _tree;
    packing_strategy    _strategy;
    int                 _pack_times;

protected:
    void initialize_compactly(float w, float h);
    bool pack_compactly(const rp_input_list& inputs);
    bool add_rect_compactly(float w, float h, void* binding);
    void trace_compactly() const;
    void initialize_dynamically();
    void add_rect_dynamically(float w, float h, void* binding);
    void add_rect_dynamically(rp_tree& rc);
    bool add_rect_dynamically(rp_tree& rc, rp_tree& xchg);
    void proc_fill_up(rp_iterator p, rp_tree& t);
    void proc_combine(rp_iterator p, rp_tree& t);
    void proc_combine_root(rp_iterator p, rp_tree& t);
    void proc_combine_non_root(rp_iterator p, rp_iterator i, rp_tree& t);
    void proc_replace(rp_iterator p, rp_tree& t, rp_tree& xchg);
    void trace_dynamically() const;
};

__ariel_end__

#endif
