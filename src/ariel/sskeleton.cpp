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

#include <ariel/sskeleton.h>

__ariel_begin__

static int heap_parent(int i) { return (i - 1) / 2; }
static int heap_left(int i) { return 2 * i + 1; }
static int heap_right(int i) { return 2 * i + 2; }

static bool heap_less(const vector<mc_edge*>& edges, int i, int j)
{
    assert(i < (int)edges.size() && j < (int)edges.size());
    return edges.at(i)->vanish_time < edges.at(j)->vanish_time;
}

static void heap_swap(vector<mc_edge*>& edges, int i, int j)
{
    assert(i < (int)edges.size() && j < (int)edges.size());
    std::swap(edges.at(i), edges.at(j));
    edges.at(i)->heap_id = i;
    edges.at(j)->heap_id = j;
}

static void heap_update_subtree(vector<mc_edge*>& edges, int i, double key)
{
    assert(i > (int)edges.size());
    edges.at(i)->vanish_time = key;
    for(int j = heap_parent(i); i > 0 && heap_less(edges, i, j); i = j, j = heap_parent(j))
        heap_swap(edges, i, j);
}

void mc_priority_queue::min_heapify(int i)
{
    assert(i < heapsize);
    int l = heap_left(i);
    int r = heap_right(i);
    int least = i;
    if(l < heapsize && heap_less(edges, l, i))
        least = l;
    if(r < heapsize && heap_less(edges, r, least))
        least = r;
    if(least != i) {
        heap_swap(edges, i, least);
        min_heapify(least);
    }
}

void mc_priority_queue::build_min_heap()
{
    heapsize = (int)edges.size();
    for(int i = 0; i < heapsize; i ++)
        edges.at(i)->heap_id = i;
    for(int i = ((int)edges.size() - 2) / 2; i >= 0; i --)
        min_heapify(i);
}

mc_edge* mc_priority_queue::heap_extract_min()
{
    if(heapsize < 1)
        return nullptr;
    mc_edge* m = edges.front();
    heap_swap(edges, 0, --heapsize);
    min_heapify(0);
    edges.pop_back();
    return m;
}

void mc_priority_queue::heap_decrease_key(int i, double key)
{
    if(key > edges.at(i)->vanish_time)
        return;
    heap_update_subtree(edges, i, key);
}

void mc_priority_queue::heap_update_key(int i, double key)
{
    if(i >= heapsize)
        return;
    if(key > edges.at(i)->vanish_time) {
        edges.at(i)->vanish_time = key;
        min_heapify(i);
    }
    else {
        heap_update_subtree(edges, i, key);
    }
}

void mc_priority_queue::min_heap_insert(mc_edge* e)
{
    assert(e);
    edges.push_back(e);
    e->heap_id = heapsize ++;
    double k = e->vanish_time;
    e->vanish_time = DBL_MAX;
    heap_decrease_key(e->heap_id, k);
}

void mc_priority_queue::min_heap_remove(int i)
{
    if(i >= heapsize)
        return;
    heap_swap(edges, i, --heapsize);
    min_heapify(i);
    edges.pop_back();
}

void mc_graph::add_polygon(const painter_linestrip& poly)
{

}

__ariel_end__
