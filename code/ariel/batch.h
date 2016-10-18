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

#ifndef batch_8317ae94_87f4_4cc2_8dab_1eef2919a461_h
#define batch_8317ae94_87f4_4cc2_8dab_1eef2919a461_h

#include <ariel/config.h>
#include <gslib/rtree.h>
#include <ariel/loopblinn.h>

__ariel_begin__


struct bat_triangle;
struct bat_batch;

typedef vector<bat_triangle*> bat_triangles;
typedef rtree_entity<bat_triangle*> bat_rtree_entity;
typedef rtree_node<bat_rtree_entity> bat_rtree_node;
typedef _tree_allocator<bat_rtree_node> bat_rtree_alloc;
typedef tree<bat_rtree_entity, bat_rtree_node, bat_rtree_alloc> bat_tree;
typedef rtree<bat_rtree_entity, quadratic_split_alg<5, 2, bat_tree>, bat_rtree_node, bat_rtree_alloc> bat_rtree;
typedef vector<bat_batch*> bat_batches;

enum bat_type
{
    bf_cr,              /* pt, cr */
    bf_klm_cr,          /* pt, klm, cr */
    bf_tex,             /* pt, tex */
    bf_klm_tex,         /* pt, klm, tex */
};

struct bat_triangle
{
    lb_joint*           _joints[3];
    vec2                _reduced[3];        /* get a reduced triangle so that no overlapping caused by error would be detected. */
    bool                _is_reduced;

public:
    bat_triangle();
    bat_triangle(lb_joint* i, lb_joint* j, lb_joint* k);
    ~bat_triangle() {}
    void make_rect(rectf& rc);
    void set_joint(int i, lb_joint* p) { _joints[i] = p; }
    lb_joint* get_joint(int i) const { return _joints[i]; }
    const vec2& get_point(int i) const { return _joints[i]->get_point(); }
    void* get_lb_binding(int i) const { return _joints[i]->get_binding(); }
    bat_type decide() const;
    const vec2& get_reduced_point(int i) const { return _reduced[i]; }
    bool is_overlapped(const bat_triangle& other) const;
    void ensure_make_reduced();
    void tracing() const;
    void trace_reduced_points() const;
};

struct bat_batch
{
    bat_type            _type;
    bat_rtree           _rtree;

public:
    bat_batch(bat_type t): _type(t) {}
    ~bat_batch() {}
    bat_type get_type() const { return _type; }
    bat_rtree& get_rtree() { return _rtree; }
    const bat_rtree& const_rtree() const { return _rtree; }
};

class batch_processor
{
public:
    typedef bat_batches::iterator bat_iter;
    typedef bat_batches::const_iterator bat_const_iter;

public:
    batch_processor();
    ~batch_processor();
    void add_polygon(lb_polygon* poly);
    bat_batches& get_batches() { return _batches; }
    void clear_batches();

protected:
    bat_triangles       _triangles;
    bat_batches         _batches;

protected:
    bat_batch* create_batch(bat_type t);
    bat_triangle* create_triangle(lb_joint* i, lb_joint* j, lb_joint* k);
    void add_triangle(lb_joint* i, lb_joint* j, lb_joint* k);
};

__ariel_end__

#endif
