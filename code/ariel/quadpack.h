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

#ifndef quadpack_89b58399_0f97_4cb9_9160_0226504762b1_h
#define quadpack_89b58399_0f97_4cb9_9160_0226504762b1_h

#include <ariel/config.h>
#include <gslib/bintree.h>
#include <pink/type.h>

__ariel_begin__

using pink::rectf;

struct qp_node
{
    rectf               quad;
    rectf               blank;
    bool                has_blank;                      /* tag of blank.                | only valid on a non-leaf node. */
    bool                is_transposed;                  /* rotate by a right angle?     | valid on all kinds of node */
    bool                is_vert_arrange;                /* vertical arrangement?        | only valid on a non-leaf node. */

public:
    qp_node()
    {
        has_blank = false;
        is_transposed = false;
    }
    float get_area() const;
    void set_no_blank();
    void set_blank(const rectf& rc);
    void inherit_as_local(qp_node& org);
    void set_from_local(qp_node& p);
    void set_dimensions(float w, float h);
    void move_to_pos(float x, float y);
    void set_transposed(bool transposed);
    void flip_leaf();
    void flip_non_leaf();
};

typedef _bintreenode_wrapper<qp_node> qp_wrapper;
typedef _bintree_allocator<qp_wrapper> qp_allocator;
typedef bintree<qp_node, qp_wrapper, qp_allocator> qp_tree;
typedef qp_tree::iterator qp_iterator;
typedef qp_tree::const_iterator qp_const_iterator;

class quad_packer
{
public:
    enum ipos
    {
        ipos_nowhere,
        ipos_combine,
        ipos_blank,
        ipos_replace,
    };

    struct ipos_info
    {
        ipos            tag;
        qp_iterator     iter;
        float           enlarge;                        /* increased area */

        ipos_info(): iter(0) {}
    };

public:
    quad_packer();
    ~quad_packer();
    void initialize();
    void add_quad(float w, float h);

protected:
    qp_tree             _tree;

protected:
    void make_pack(qp_iterator parent, qp_const_iterator left, qp_iterator right);
    void make_vertical_pack(qp_iterator parent, qp_const_iterator left, qp_iterator right, bool transposed);
    void make_horizontal_pack(qp_iterator parent, qp_const_iterator left, qp_iterator right, bool transposed);
    void make_vertical_pack(qp_iterator parent, qp_const_iterator left, qp_iterator right);
    void make_horizontal_pack(qp_iterator parent, qp_const_iterator left, qp_iterator right);
    bool find_insert_position(qp_iterator p, ipos_info& info, float w, float h) const;
    void proc_add_quad(const ipos_info& info, float w, float h);
    void proc_fill_up(qp_iterator p, float w, float h);
    void proc_combine(qp_iterator p, float w, float h);
    void proc_combine_root(qp_iterator p, float w, float h);
    void proc_combine_non_root(qp_iterator p, qp_iterator i, float w, float h);
    void proc_replace(qp_iterator p, float w, float h);
    void update_pack_rect(qp_iterator i);
};

__ariel_end__

#endif
