/*
 * Copyright (c) 2016 lymastee, All rights reserved.
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

#ifndef gfxobj_ae667559_1772_4e7b_a808_6f15e70fae88_h
#define gfxobj_ae667559_1772_4e7b_a808_6f15e70fae88_h

#include <ariel/loopblinn.h>

__ariel_begin__

class graphics_obj_entity:
    public loop_blinn_processor
{
public:
    graphics_obj_entity(float w, float h): loop_blinn_processor(w, h) {}
    void proceed_fill(const painter_path& path) { __super::proceed(path); }
    void proceed_stroke(const painter_path& path);

protected:
    int create_from_path(const painter_path& path, int start);

    struct path_seg
    {
        lb_line*        first;
        lb_line*        last;
        path_seg() { first = last = 0; }
    };
    void add_line_seg(path_seg& seg, const painter_path::line_to_node* node);
    void add_quad_seg(path_seg& seg, const painter_node* node1, const painter_path::quad_to_node* node2);
    void add_cubic_seg(path_seg& seg, const painter_node* node1, const painter_path::cubic_to_node* node2);
};

class graphics_obj
{
public:
    graphics_obj(float w, float h);
    graphics_obj(const graphics_obj& that);
    ~graphics_obj();
    graphics_obj_entity* operator->() { return _ptr; }
    const graphics_obj_entity* operator->() const { return _ptr; }

protected:
    graphics_obj_entity* _ptr;
};

__ariel_end__

#endif
