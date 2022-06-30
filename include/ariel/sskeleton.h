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

#ifndef sskeleton_6a0ee44f_788e_4fdd_8716_2f25f8d161f0_h
#define sskeleton_6a0ee44f_788e_4fdd_8716_2f25f8d161f0_h

#include <gslib/math.h>
#include <gslib/std.h>
#include <ariel/config.h>
#include <ariel/painterpath.h>

#ifdef max
#undef max
#endif

__ariel_begin__

struct mc_edge;

enum mc_vertex_type
{
    mc_reflex_vertex,
    mc_convex_vertex,
    mc_moving_steiner_vertex,
    mc_multi_steiner_vertex,
    mc_resting_steiner_vertex,
    mc_other_vertex,
};

enum mc_edge_type
{
    mc_polygon_edge,
    mc_motorcycle_edge,
};

struct mc_vertex
{
    vec2                start_pos;
    vec2                speed;
    double              start_time = 0.0;
    mc_edge*            firstin = nullptr;
    mc_edge*            firstout = nullptr;
    mc_vertex_type      type = mc_other_vertex;
};

struct mc_edge
{
    mc_vertex*          tail_vt = nullptr;
    mc_vertex*          head_vt = nullptr;
    mc_edge*            tail_edge = nullptr;
    mc_edge*            head_edge = nullptr;
    double              vanish_time = DBL_MAX;
    mc_edge_type        type = mc_polygon_edge;
    int                 heap_id = -1;
    bool                mark = false;
};

class mc_priority_queue
{
public:
    vector<mc_edge*>    edges;
    int                 heapsize = 0;

public:
    mc_priority_queue() {}
    mc_priority_queue(const vector<mc_edge*>& es): edges(es) {}
    void min_heapify(int i);
    void build_min_heap();
    mc_edge* heap_minimum() const { return edges.front(); }
    mc_edge* heap_extract_min();
    void heap_decrease_key(int i, double key);
    void min_heap_insert(mc_edge* e);
    void heap_update_key(int i, double key);
    bool empty() const { return edges.empty(); }
    void min_heap_remove(int i);
};

class mc_graph
{
public:
    vector<mc_edge*>    edge_list;

public:
    mc_graph() {}
    ~mc_graph() {}
    void add_polygon(const painter_linestrip& poly);

};

class straight_skeleton
{

};

__ariel_end__

#endif
