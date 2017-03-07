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

#include <pink/utility.h>
#include <ariel/gobj.h>

__ariel_begin__

static void lb_connect(lb_joint* joint, lb_line* line)
{
    assert(joint && line);
    joint->set_next_line(line);
    line->set_prev_joint(joint);
}

static void lb_connect(lb_line* line, lb_joint* joint)
{
    assert(joint && line);
    line->set_next_joint(joint);
    joint->set_prev_line(line);
}

static void lb_connect(lb_joint* joint1, lb_line* line, lb_joint* joint2)
{
    assert(joint1 && line && joint2);
    lb_connect(joint1, line);
    lb_connect(line, joint2);
}

void graphics_obj_entity::proceed_stroke(const painter_path& path)
{
    int size = path.size();
    for(int i = 0; i < size; )
        i = create_from_path(path, i);
}

int graphics_obj_entity::create_from_path(const painter_path& path, int start)
{
    int size = path.size();
    assert(start < size);
    auto* first = path.get_node(start);
    assert(first && first->get_tag() == painter_path::pt_moveto);
    int i = start + 1;
    if((i == size)) {
        assert(!"bad path.");
        return i;
    }
    auto* node = path.get_node(i);
    assert(node);
    if(node->get_tag() == painter_path::pt_moveto) {
        assert(!"bad path.");
        return i;
    }
    path_seg firstseg;
    switch(node->get_tag())
    {
    case painter_path::pt_lineto:
        add_line_seg(firstseg, static_cast<const painter_path::line_to_node*>(node));
        break;
    case painter_path::pt_quadto:
        add_quad_seg(firstseg, first, static_cast<const painter_path::quad_to_node*>(node));
        break;
    case painter_path::pt_cubicto:
        add_cubic_seg(firstseg, first, static_cast<const painter_path::cubic_to_node*>(node));
        break;
    default:
        assert(!"bad path.");
        return size;
    }
    assert(firstseg.first && firstseg.last);
    auto* lastline = firstseg.last;
    for(i ++; i < size; i ++) {
        auto* node = path.get_node(i);
        assert(node);
        path_seg seg;
        auto t = node->get_tag();
        if(t == painter_path::pt_moveto)
            break;
        switch(t)
        {
        case painter_path::pt_lineto:
            add_line_seg(seg, static_cast<const painter_path::line_to_node*>(node));
            break;
        case painter_path::pt_quadto:
            add_quad_seg(seg, path.get_node(i - 1), static_cast<const painter_path::quad_to_node*>(node));
            break;
        case painter_path::pt_cubicto:
            add_cubic_seg(seg, path.get_node(i - 1), static_cast<const painter_path::cubic_to_node*>(node));
            break;
        default:
            assert(!"bad path.");
            break;
        }
        lb_connect(lastline->get_next_joint(), seg.first);
        lastline = seg.last;
    }
    assert(lastline);
    if(first->get_point() == lastline->get_next_point())
        lb_connect(lastline->get_next_joint(), firstseg.first);
    else {
        auto* firstj = create_joint<lb_end_joint>(first->get_point());
        assert(firstj);
        lb_connect(firstj, firstseg.first);
    }
    return i;
}

void graphics_obj_entity::add_line_seg(path_seg& seg, const painter_path::line_to_node* node)
{
    assert(node);
    auto* joint = create_joint<lb_end_joint>(node->get_point());
    auto* line = create_line();
    lb_connect(line, joint);
    seg.first = seg.last = line;
}

void graphics_obj_entity::add_quad_seg(path_seg& seg, const painter_node* node1, const painter_path::quad_to_node* node2)
{
    assert(node1 && node2);
    vec3 para[2];
    pink::get_quad_parameter_equation(para, node1->get_point(), node2->get_control(), node2->get_point());
    int step = pink::get_interpolate_step(node1->get_point(), node2->get_control(), node2->get_point());
    float t, chord;
    t = chord = 1.f / (step - 1);
    auto* lastline = seg.first = create_line();
    for(int i = 1; i < step - 1; i ++, t += chord) {
        vec2 p;
        pink::eval_quad(p, para, t);
        auto* line = create_line();
        auto* joint = create_joint<lb_end_joint>(p);
        assert(line && joint);
        lb_connect(lastline, joint);
        lb_connect(joint, line);
        lastline = line;
    }
    auto* joint = create_joint<lb_end_joint>(node2->get_point());
    assert(joint);
    lb_connect(lastline, joint);
    seg.last = lastline;
}

void graphics_obj_entity::add_cubic_seg(path_seg& seg, const painter_node* node1, const painter_path::cubic_to_node* node2)
{
    assert(node1 && node2);
    vec4 para[2];
    pink::get_cubic_parameter_equation(para, node1->get_point(), node2->get_control1(), node2->get_control2(), node2->get_point());
    int step = pink::get_interpolate_step(node1->get_point(), node2->get_control1(), node2->get_control2(), node2->get_point());
    float t, chord;
    t = chord = 1.f / (step - 1);
    auto* lastline = seg.first = create_line();
    for(int i = 1; i < step - 1; i ++, t += chord) {
        vec2 p;
        pink::eval_cubic(p, para, t);
        auto* line = create_line();
        auto* joint = create_joint<lb_end_joint>(p);
        assert(line && joint);
        lb_connect(lastline, joint);
        lb_connect(joint, line);
        lastline = line;
    }
    auto* joint = create_joint<lb_end_joint>(node2->get_point());
    assert(joint);
    lb_connect(lastline, joint);
    seg.last = lastline;
}

graphics_obj::graphics_obj(float w, float h)
{
    __super::reset(gs_new(graphics_obj_entity, w, h), [](graphics_obj_entity* p) {
        assert(p);
        gs_del(graphics_obj_entity, p);
    });
}

__ariel_end__
