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

#include <ariel/quadpack.h>

__ariel_begin__

// static bool is_node_transposed(qp_const_iterator i)
// {
//     assert(i);
//     auto p = i.parent();
//     return p ? (i->is_transposed != is_node_transposed(p)) : i->is_transposed;
// }

float qp_node::get_area() const
{
    return quad.area();
}

void qp_node::set_no_blank()
{
    blank = rectf(0.f, 0.f, 0.f, 0.f);
    has_blank = false;
}

void qp_node::set_blank(const rectf& rc)
{
    blank = rc;
    has_blank = true;
}

void qp_node::inherit_as_local(qp_node& org)
{
    quad = org.quad;
    quad.move_to(0.f, 0.f);
    is_transposed = org.is_transposed;
    org.is_transposed = false;
}

void qp_node::set_from_local(qp_node& p)
{
    quad.move_to(p.quad.left, p.quad.top);
    p.quad.move_to(0.f, 0.f);
}

void qp_node::set_dimensions(float w, float h)
{
    quad.set_rect(0.f, 0.f, w, h);
}

void qp_node::move_to_pos(float x, float y)
{
    quad.move_to(x, y);
}

void qp_node::set_transposed(bool transposed)
{
    /* must be leaf */
    is_transposed = transposed;
    if(transposed) {
        gs_swap(quad.left, quad.top);
        gs_swap(quad.right, quad.bottom);
    }
}

void qp_node::flip_leaf()
{
    is_transposed = !is_transposed;
    gs_swap(quad.left, quad.top);
    gs_swap(quad.right, quad.bottom);
}

void qp_node::flip_non_leaf()
{
    is_transposed = !is_transposed;
    is_vert_arrange = !is_vert_arrange;
    gs_swap(quad.left, quad.top);
    gs_swap(quad.right, quad.bottom);
    if(has_blank) {
        gs_swap(blank.left, blank.top);
        gs_swap(blank.right, blank.bottom);
    }
}

quad_packer::quad_packer()
{
}

quad_packer::~quad_packer()
{
}

void quad_packer::initialize()
{
    _tree.destroy();
}

void quad_packer::add_quad(float w, float h)
{
    auto r = _tree.get_root();
    if(!r) {
        auto i = _tree.insertll(r);
        assert(i);
        i->quad = rectf(0.f, 0.f, w, h);
        i->set_no_blank();
    }
    else if(r.is_leaf()) {
        auto i = _tree.insertll(r);
        auto j = _tree.insertrl(r);
        assert(i && j);
        i->inherit_as_local(*r);
        j->set_dimensions(w, h);
        make_pack(r, i, j);
    }
    else {
        ipos_info info;
        if(!find_insert_position(r, info, w, h)) {
            /* treated as combine, enlarge here was useless. */
            info.tag = ipos_combine;
            info.iter = r;
        }
        proc_add_quad(info, w, h);
    }
}

void quad_packer::make_pack(qp_iterator parent, qp_const_iterator left, qp_iterator right)
{
    float w1 = left->quad.width();
    float h1 = left->quad.height();
    float w = right->quad.width();
    float h = right->quad.height();
    (w1 > h1) ? make_vertical_pack(parent, left, right, w < h) :
        make_horizontal_pack(parent, left, right, w > h);
}

static void transpose_node(qp_iterator p)
{
    assert(p);
    if(p.is_leaf())
        p->flip_leaf();
    else {
        assert(p.left() && p.right());
        p->flip_non_leaf();
        transpose_node(p.left());
        transpose_node(p.right());
    }
}

static void set_node_transposed(qp_iterator p, bool transposed)
{
    assert(p);
    if(p.is_leaf())
        p->set_transposed(transposed);
    else if(transposed)
        transpose_node(p);
}

void quad_packer::make_vertical_pack(qp_iterator parent, qp_const_iterator left, qp_iterator right, bool transposed)
{
    float w1 = left->quad.width();
    float h1 = left->quad.height();
    float w = right->quad.width();
    float h = right->quad.height();
    /* setup right child */
    set_node_transposed(right, transposed);
    right->move_to_pos(0.f, h1);
    right->set_no_blank();
    /* update parent */
    parent->is_vert_arrange = true;
    if(abs(w1 - w) < 1e-6f) {
        parent->quad.set_rect(parent->quad.left, parent->quad.top, gs_max(w, w1), h + h1);
        parent->set_no_blank();
    }
    else if(w1 < w) {
        parent->quad.set_rect(parent->quad.left, parent->quad.top, w, h + h1);
        parent->set_blank(rectf(w1, 0.f, w - w1, h1));
    }
    else {
        parent->quad.set_rect(parent->quad.left, parent->quad.top, w1, h + h1);
        parent->set_blank(rectf(w, h1, w1 - w, h));
    }
}

void quad_packer::make_horizontal_pack(qp_iterator parent, qp_const_iterator left, qp_iterator right, bool transposed)
{
    float w1 = left->quad.width();
    float h1 = left->quad.height();
    float w = right->quad.width();
    float h = right->quad.height();
    /* setup right child */
    set_node_transposed(right, transposed);
    right->move_to_pos(w1, 0.f);
    right->set_no_blank();
    /* update parent */
    parent->is_vert_arrange = false;
    if(abs(h1 - h) < 1e-6f) {
        parent->quad.set_rect(parent->quad.left, parent->quad.top, w + w1, gs_max(h, h1));
        parent->set_no_blank();
    }
    else if(h1 < h) {
        parent->quad.set_rect(parent->quad.left, parent->quad.top, w + w1, h);
        parent->set_blank(rectf(0.f, h1, w1, h - h1));
    }
    else {
        parent->quad.set_rect(parent->quad.left, parent->quad.top, w + w1, h1);
        parent->set_blank(rectf(w1, h, w, h1 - h));
    }
}

void quad_packer::make_vertical_pack(qp_iterator parent, qp_const_iterator left, qp_iterator right)
{
    float wl = left->quad.width();
    float w = right->quad.width();
    float h = right->quad.height();
    if((w < wl) && (h < wl))
        make_vertical_pack(parent, left, right, w < h);
    else if(w < wl)
        make_vertical_pack(parent, left, right, false);
    else if(h < wl)
        make_vertical_pack(parent, left, right, true);
    else {
        assert(!"unexpected. This function only used for fill up logic.");
    }
}

void quad_packer::make_horizontal_pack(qp_iterator parent, qp_const_iterator left, qp_iterator right)
{
    float hl = left->quad.height();
    float w = right->quad.width();
    float h = right->quad.height();
    if((w < hl) && (h < hl))
        make_horizontal_pack(parent, left, right, h < w);
    else if(w < hl)
        make_horizontal_pack(parent, left, right, true);
    else if(h < hl)
        make_horizontal_pack(parent, left, right, false);
    else {
        assert(!"unexpected. This function only used for fill up logic.");
    }
}

static float get_enlarge_area(const rectf& rc1, float w, float h)
{
    float w1 = rc1.width();
    float h1 = rc1.height();
    bool vert = w < h, vert1 = w1 < h1;
    if(vert && vert1)
        return (h1 > h) ? (h1 - h) * w : (h - h1) * w1;
    else if(!vert && !vert1)
        return (w1 > w) ? (w1 - w) * h : (w - w1) * h1;
    else if(vert && !vert1)
        return (w1 > h) ? (w1 - h) * w : (h - w1) * h1;
    else
        return (h1 > w) ? (h1 - w) * h : (w - h1) * w1;
}

bool quad_packer::find_insert_position(qp_iterator p, ipos_info& info, float w, float h) const
{
    assert(p && !p.is_leaf());
    /* 1.test area, if the rect was big enough to match the current node, simply combine them.
     * this step was to reduce the efficiency
     */
    float a1 = w * h;
    float a2 = p->get_area();
    if(a1 > a2 * 0.8f) {
        info.tag = ipos_combine;
        info.iter = p;
        info.enlarge = get_enlarge_area(p->quad, w, h);
        return true;
    }
    /* 2.test if the rect was better matched with the existed rects.
     * this step must be done before the 3rd step to improve the pack quality
     */
    auto left = p.left();
    auto right = p.right();
    assert(left && right);
    if(p->has_blank) {
        float enlarge0 = p->blank.area();
        float enlarge1 = get_enlarge_area(left->quad, w, h);
        float enlarge2 = get_enlarge_area(right->quad, w, h);
        if(enlarge1 < enlarge2) {
            if(enlarge1 < enlarge0) {
                info.tag = ipos_replace;
                info.iter = left;
                info.enlarge = enlarge1;
                return true;
            }
        }
        else {
            if(enlarge2 < enlarge0) {
                info.tag = ipos_replace;
                info.iter = right;
                info.enlarge = enlarge2;
                return true;
            }
        }

        /* 3.test if the rect could be contained in the blank.
         * if the rect was good enough to fill in the blank, stop the following detection.
         * we do this step before the 4th step to reduce the efficiency
         */
        auto& blank = p->blank;
        float wb = blank.width(), hb = blank.height();
        if((gs_max(wb, hb) >= gs_max(w, h)) && (gs_min(wb, hb) >= gs_min(w, h))) {
            info.tag = ipos_blank;
            info.iter = p;
            info.enlarge = enlarge0 - a1;
            assert(info.enlarge >= 0.f);
            return true;
        }
    }
    /* 4.find a suitable position from the descendant */
    bool f1 = false, f2 = false;
    ipos_info info1, info2;
    if(!left.is_leaf())
        f1 = find_insert_position(left, info1, w, h);
    if(!right.is_leaf())
        f2 = find_insert_position(right, info2, w, h);
    if(f1) {
        if(f2) {
            info = (info1.enlarge < info2.enlarge) ? info1 : info2;
            return true;
        }
        info = info1;
        return true;
    }
    else if(f2) {
        info = info2;
        return true;
    }
    info.tag = ipos_nowhere;
    return false;
}

void quad_packer::proc_add_quad(const ipos_info& info, float w, float h)
{
    assert(info.tag != ipos_nowhere);
    switch(info.tag)
    {
    case ipos_blank:
        return proc_fill_up(info.iter, w, h);
    case ipos_combine:
        return proc_combine(info.iter, w, h);
    case ipos_replace:
        return proc_replace(info.iter, w, h);
    default:
        assert(!"unexpected.");
    }
}

void quad_packer::proc_fill_up(qp_iterator p, float w, float h)
{
    assert(p && p->has_blank);
    auto left = p.left();
    auto right = p.right();
    assert(left && right);
    if(p->is_vert_arrange) {
        auto i = (left->quad.width() > right->quad.width()) ? _tree.insertrl(p) : _tree.insertll(p);
        assert(i && !i.right());
        auto j = _tree.insertrl(i);
        assert(j);
        i->set_from_local(*j);
        j->set_dimensions(w, h);
        make_horizontal_pack(i, i.left(), j);
        update_pack_rect(p);
    }
    else {
        auto i = (left->quad.height() > right->quad.height()) ? _tree.insertrl(p) : _tree.insertll(p);
        assert(i && !i.right());
        auto j = _tree.insertrl(i);
        assert(j);
        i->set_from_local(*j);
        j->set_dimensions(w, h);
        make_vertical_pack(i, i.left(), j);
        update_pack_rect(p);
    }
}

void quad_packer::proc_combine(qp_iterator p, float w, float h)
{
    assert(p);
    auto g = p.parent();
    g ? proc_combine_non_root(g, p, w, h) :
        proc_combine_root(p, w, h);
}

void quad_packer::proc_combine_root(qp_iterator p, float w, float h)
{
    assert(p);
    qp_tree t;
    auto r = t.insertll(qp_iterator(0));    /* add a root */
    auto left = t.insertll(r);
    auto right = t.insertrl(r);
    assert(r && left && right);
    t.attach(_tree, left);
    right->set_dimensions(w, h);
    right->set_no_blank();
    right->is_transposed = false;
    make_pack(r, left, right);
    update_pack_rect(r);
    _tree.swap(t);
}

void quad_packer::proc_combine_non_root(qp_iterator p, qp_iterator i, float w, float h)
{
    assert(p && i && (i.parent() == p));
    auto ins = i.is_left() ? _tree.insertll(p) : _tree.insertrl(p);
    assert(ins);
    auto left = ins.left();
    assert(left && !ins.right());
    auto right = _tree.insertrl(ins);
    assert(right);
    right->set_dimensions(w, h);
    right->set_no_blank();
    right->is_transposed = false;
    make_pack(ins, left, right);
    update_pack_rect(ins);
}

void quad_packer::proc_replace(qp_iterator p, float w, float h)
{
    assert(p);
    auto g = p.parent();
    assert(g && "must have parent.");
    float ow = p->quad.width(), oh = p->quad.height();
    if(p->is_transposed)
        gs_swap(ow, oh);
    p->set_dimensions(w, h);
    p->set_no_blank();
    p->is_transposed = false;
    assert(p.is_right() && "check it.");
    make_pack(g, p.sibling(), p);
    update_pack_rect(g);
    /* replaced ow, oh need to be reinserted */
    add_quad(ow, oh);
}

void quad_packer::update_pack_rect(qp_iterator i)
{
    assert(i);
    auto left = i.left();
    auto right = i.right();
    assert(left && right);
    float ow = i->quad.width();
    float oh = i->quad.height();
    /* pack it again */
    make_pack(i, left, right);
    /* to see has dimensions changed? */
    if((i->quad.width() != ow) || (i->quad.height() != oh)) {
        if(i.parent())
            update_pack_rect(i.parent());
    }
}

__ariel_end__
