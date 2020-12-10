/*
 * Copyright (c) 2016-2020 lymastee, All rights reserved.
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

#include <gslib/error.h>
#include <ariel/rectpack.h>

__ariel_begin__

static void rp_recursive_locate(rp_const_iterator i, rp_rect& rc)
{
    assert(i);
    rc.offset(i->left(), i->top());
    auto p = i.parent();
    if(p)
        rp_recursive_locate(p, rc);
}

void rp_global_location(rp_const_iterator i, rp_rect& rc)
{
    assert(i);
    rc = *i;
    auto p = i.parent();
    if(p)
        rp_recursive_locate(p, rc);
}

bool rp_is_node_transposed(rp_const_iterator i)
{
    assert(i);
    assert(i->is_dynamic() && "only dynamic node needs a recursively check.");
    bool transposed = i->transposed;
    if(i.is_root())
        return transposed;
    return transposed ? !rp_is_node_transposed(i.parent()) : rp_is_node_transposed(i.parent());
}

rect_packer::rect_packer()
{
    _strategy = ps_unknown;
    _pack_times = 0;
}

rect_packer::~rect_packer()
{
    _tree.destroy();
    _strategy = ps_unknown;
}

void rect_packer::pack_automatically(rp_input_list& inputs)
{
    /* 1.few inputs, use dynamic strategy. */
    if(inputs.size() <= 10) {
        _pack_times = 1;
        initialize_dynamically();
        for(auto& i : inputs)
            add_rect_dynamically(i.width, i.height, i.binding);
        return;
    }
    /* 2.otherwise use compact strategy, first sort them by max side */
    inputs.sort([](const rp_input& a, const rp_input& b)-> bool {
        return gs_max(a.width, a.height) > gs_max(b.width, b.height);
    });
    /* 3.calculate sum of the areas */
    float sum_area = 0.f;
    for(const auto& rc : inputs) {
        float a = rc.width * rc.height;
        sum_area += a;
    }
    /* 4.expand the area initially */
    float c = (float)inputs.size();
    float expand_ratio = 9.21327548f / (c + 19.65655514f) + 1.048729155f;       /* by curve fit */
    float totals = sum_area * expand_ratio;
    /* 5.pack it until done, if fails, expand and try again */
    float step = gs_max(0.1047f - c * 0.0005f, 0.01f);
    _pack_times = 0;
    for(;;) {
        _pack_times ++;
        float side = sqrtf(totals);
        initialize_compactly(side, side);
        if(pack_compactly(inputs))
            return;
        expand_ratio += step;
        totals = sum_area * expand_ratio;
    }
}

void rect_packer::tracing() const
{
    switch(_strategy)
    {
    case ps_compactly:
        trace_compactly();
        break;
    case ps_dynamically:
        trace_dynamically();
        break;
    default:
        assert(!"unknown strategy.");
    }
}

void rect_packer::initialize_compactly(float w, float h)
{
    _strategy = ps_compactly;
    _tree.destroy();
    auto i = _tree.insertll(rp_iterator(0));
    assert(i);
    i->set_rect(0.f, 0.f, w, h);
}

bool rect_packer::pack_compactly(const rp_input_list& inputs)
{
    for(const auto& rc : inputs) {
        if(!add_rect_compactly(rc.width, rc.height, rc.binding))
            return false;
    }
    return true;
}

void rect_packer::trace_compactly() const
{
    assert(_strategy == ps_compactly);
    trace(_t("#start tracing compactly.\n"));
    _tree.preorder_traversal([](const rp_wrapper* w) {
        assert(w);
        rp_const_iterator i(w);
        if(i.is_leaf() && !i->is_empty()) {
            trace(_t("@!\n"));
            /* rand color */
            int r = rand() % 256;
            int g = rand() % 256;
            int b = rand() % 256;
            string cr;
            cr.format(_t("rgb(%d,%d,%d)"), r, g, b);
            trace(_t("@&strokeColor=%s;\n"), cr.c_str());
            trace(_t("@&withArrow=false;\n"));
            rp_rect rc;
            rp_global_location(i, rc);
            trace(_t("@rect %f, %f, %f, %f;\n"), rc.left(), rc.top(), rc.right(), rc.bottom());
            if(i->transposed)
                trace(_t("#this rect was transposed.\n"));
            trace(_t("@@\n"));
        }
    });
    /* add a boundary. */
    trace(_t("@!\n"));
    trace(_t("@&strokeColor=rgb(0,0,0);\n"));
    auto r = _tree.const_root();
    trace(_t("@rect %f, %f, %f, %f;\n"), r->left(), r->top(), r->right(), r->bottom());
    trace(_t("@@\n"));
    trace(_t("#end tracing.\n"));
}

static rp_iterator rp_split_horizontal(rp_tree& t, rp_iterator i, float w)
{
    assert(t.is_valid() && i && i.is_leaf());
    auto left = t.insertll(i);
    auto right = t.insertrl(i);
    assert(left && right);
    assert(w < i->width);
    left->set_rect(0.f, 0.f, w, i->height);
    right->set_rect(w, 0.f, i->width - w, i->height);
    return left;
}

static rp_iterator rp_split_vertical(rp_tree& t, rp_iterator i, float h)
{
    assert(t.is_valid() && i && i.is_leaf());
    auto left = t.insertll(i);
    auto right = t.insertrl(i);
    assert(left && right);
    assert(h < i->height);
    left->set_rect(0.f, 0.f, i->width, h);
    right->set_rect(0.f, h, i->width, i->height - h);
    return left;
}

static void rp_insert_rect(rp_tree& t, rp_iterator i, float w, float h, void* binding, bool transposed)
{
    assert(t.is_valid() && i);
    assert(i.is_leaf());
    float dw = i->width - w;
    float dh = i->height - h;
    assert(dw >= 0.f && dh >= 0.f);
    if(dw > dh) {
        auto left = rp_split_horizontal(t, i, w);
        assert(left);
        if(dh <= 0.f) {
            left->set_bind_ptr(binding);
            left->set_transposed(transposed);
        }
        else {
            auto leftleft = rp_split_vertical(t, left, h);
            assert(leftleft);
            leftleft->set_bind_ptr(binding);
            leftleft->set_transposed(transposed);
        }
    }
    else {
        auto left = rp_split_vertical(t, i, h);
        assert(left);
        if(dw <= 0.f) {
            left->set_bind_ptr(binding);
            left->set_transposed(transposed);
        }
        else {
            auto leftleft = rp_split_horizontal(t, left, w);
            assert(leftleft);
            leftleft->set_bind_ptr(binding);
            leftleft->set_transposed(transposed);
        }
    }
}

static bool rp_add_rect(rp_tree& t, rp_iterator i, float w, float h, void* binding)
{
    assert(t.is_valid() && i);
    if(i.is_leaf()) {
        if(!i->is_empty())
            return false;
        /* get side infos of the blank and the filling */
        float min_blank, max_blank, min_filling, max_filling;
        if(i->width < i->height)
            min_blank = i->width, max_blank = i->height;
        else
            min_blank = i->height, max_blank = i->width;
        if(w < h)
            min_filling = w, max_filling = h;
        else
            min_filling = h, max_filling = w;
        /* no enough space? */
        if((min_blank < min_filling) || (max_blank < max_filling))
            return false;
        /* just to accept */
        if((min_blank == min_filling) && (max_blank == max_filling)) {
            if(i->width == w) {
                assert(i->height == h);
                i->set_bind_ptr(binding);
                i->set_transposed(false);
            }
            else {
                assert((i->width == h) && (i->height == w));
                i->set_bind_ptr(binding);
                i->set_transposed(true);
            }
            return true;
        }
        /* decide which way */
        bool transposed;
        float dw1 = i->width - w;
        float dh1 = i->height - h;
        float dw2 = i->width - h;
        float dh2 = i->height - w;
        bool untransposed_available = (dw1 >= 0.f && dh1 >= 0.f);
        bool transposed_available = (dw2 >= 0.f && dh2 >= 0.f);
        if(untransposed_available && transposed_available)
            transposed = (dw1 * dh1 > dw2 * dh2);
        else if(untransposed_available)
            transposed = false;
        else {
            assert(transposed_available);
            transposed = true;
        }
        /* do split */
        transposed ? rp_insert_rect(t, i, h, w, binding, true) :
            rp_insert_rect(t, i, w, h, binding, false);
        return true;
    }
    else {
        assert(i.left() && i.right());
        return rp_add_rect(t, i.left(), w, h, binding) || rp_add_rect(t, i.right(), w, h, binding);
    }
}

bool rect_packer::add_rect_compactly(float w, float h, void* binding)
{
    assert(_strategy == ps_compactly);
    return rp_add_rect(_tree, _tree.get_root(), w, h, binding);
}

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
    rp_iterator     iter;
    float           enlarge;                        /* increased area */

    ipos_info(): iter(0) {}
};

static void trace_rp_node(const rp_wrapper* w)
{
    assert(w);
    auto* node = w->const_ptr();
    assert(node);
    if(!rp_const_iterator(w).is_leaf())
        return;
    rp_rect rc;
    rp_global_location(rp_const_iterator(w), rc);
    assert(!w->const_ptr()->is_empty());
    int r = rand() % 256;
    int g = rand() % 256;
    int b = rand() % 256;
    string cr;
    cr.format(_t("rgb(%d,%d,%d)"), r, g, b);
    trace(_t("@!\n"));
    trace(_t("@&strokeColor=%s;\n"), cr.c_str());
    trace(_t("@rect %f, %f, %f, %f;\n"), rc.left(), rc.top(), rc.right(), rc.bottom());
    trace(_t("@@\n"));
    if(rp_is_node_transposed(rp_const_iterator(w))) {
        trace(_t("#this rect was transposed.\n"));
    }
}

static void trace_rp_node_total(const rp_wrapper* w)
{
    assert(w);
    auto* node = w->const_ptr();
    assert(node);
    rp_rect rc;
    rp_global_location(rp_const_iterator(w), rc);
    int r = rand() % 256;
    int g = rand() % 256;
    int b = rand() % 256;
    string cr;
    cr.format(_t("rgb(%d,%d,%d)"), r, g, b);
    trace(_t("@!\n"));
    trace(_t("@&strokeColor=%s;\n"), cr.c_str());
    trace(_t("@rect %f, %f, %f, %f;\n"), rc.left(), rc.top(), rc.right(), rc.bottom());
    trace(_t("@@\n"));
    if(node->transposed) {
        trace(_t("#this rect was transposed.\n"));
    }
}

static void trace_rp_blank(const rp_wrapper* w)
{
    assert(w);
    auto* xnode = w->const_ptr();
    assert(xnode);
    assert(xnode->is_dynamic());
    auto* node = static_cast<const rp_dynamic_node*>(xnode);
    if(node->has_blank) {
        rp_rect rc = node->blank;
        if(w->_parent)
            rp_recursive_locate(rp_const_iterator(w->_parent), rc);
        trace(_t("#trace blank.\n"));
        trace(_t("@rect %f, %f, %f, %f;\n"), rc.left(), rc.top(), rc.right(), rc.bottom());
    }
}

static void trace_rp_tree(const rp_tree& t)
{
    trace(gs::_print_bintree<const rp_tree>(t).print());
}

static bool checkup_rp_tree(rp_const_iterator p)
{
    assert(p);
    if(p.is_leaf())
        return true;
    auto left = p.left();
    auto right = p.right();
    assert(left && right && "unexpected situation.");
    /* the contain box must be compact to the boundary box of the two children */
    float w, h;
    bool has_blank = false;
    assert(p->is_dynamic());
    auto* node = static_cast<const rp_dynamic_node*>(p.get_ptr());
    if(node->is_vert_arrange) {
        w = gs_max(left->width, right->width);
        h = left->height + right->height;
        has_blank = (left->width != right->width);
    }
    else {
        w = left->width + right->width;
        h = gs_max(left->height, right->height);
        has_blank = (left->height != right->height);
    }
    if(w != node->width || h != node->height) {
        trace(_t("rect mismatch: %f, %f;\n"), node->width, node->height);
        assert(!"bad pack.");
        return false;
    }
    /* check the blank */
    if(has_blank) {
        if(!node->has_blank) {
            trace(_t("rect blank missed: %f, %f;\n"), node->width, node->height);
            assert(!"no blank here?");
            return false;
        }
        if(node->is_vert_arrange) {
            float sw = (left->width < right->width) ?
                left->width + node->blank.width :
                right->width + node->blank.width;
            if(abs(w - sw) > 2e-4f) {
                trace(_t("wrong blank size: %f, %f;\n"), node->width, node->height);
                assert(!"wrong blank size");
                return false;
            }
        }
        else {
            float sh = (left->height < right->height) ?
                left->height + node->blank.height :
                right->height + node->blank.height;
            if(abs(h - sh) > 2e-4f) {
                trace(_t("wrong blank size: %f, %f;\n"), node->width, node->height);
                assert(!"wrong blank size");
                return false;
            }
        }
    }
    return checkup_rp_tree(left) && checkup_rp_tree(right);
}

static void transpose_node(rp_iterator p)
{
    assert(p && p->is_dynamic());
    if(p.is_leaf())
        static_cast<rp_dynamic_node*>(p.get_ptr())->flip_leaf();
    else {
        assert(p.left() && p.right());
        static_cast<rp_dynamic_node*>(p.get_ptr())->flip_non_leaf();
        transpose_node(p.left());
        transpose_node(p.right());
    }
}

static void make_vertical_pack(rp_iterator parent, rp_iterator left, rp_iterator right, bool transposed)
{
    if(transposed)
        transpose_node(right);
    /* retrieve dimensions */
    float w1 = left->width;
    float h1 = left->height;
    float w = right->width;
    float h = right->height;
    /* retrieve node */
    assert(left->is_dynamic() && right->is_dynamic() && parent->is_dynamic());
    auto* leftnode = static_cast<rp_dynamic_node*>(left.get_ptr());
    auto* rightnode = static_cast<rp_dynamic_node*>(right.get_ptr());
    auto* parentnode = static_cast<rp_dynamic_node*>(parent.get_ptr());
    /* setup children */
    leftnode->move_to_pos(0.f, 0.f);
    rightnode->move_to_pos(0.f, h1);
    /* update parent */
    parentnode->is_vert_arrange = true;
    if(abs(w1 - w) < 1e-6f) {
        parentnode->set_rect(parentnode->left(), parentnode->top(), gs_max(w, w1), h + h1);
        parentnode->set_no_blank();
    }
    else if(w1 < w) {
        parentnode->set_rect(parentnode->left(), parentnode->top(), w, h + h1);
        parentnode->set_blank(rp_rect(w1, 0.f, w - w1, h1));
    }
    else {
        parentnode->set_rect(parentnode->left(), parentnode->top(), w1, h + h1);
        parentnode->set_blank(rp_rect(w, h1, w1 - w, h));
    }
}

static void make_horizontal_pack(rp_iterator parent, rp_iterator left, rp_iterator right, bool transposed)
{
    if(transposed)
        transpose_node(right);
    /* retrieve dimensions */
    float w1 = left->width;
    float h1 = left->height;
    float w = right->width;
    float h = right->height;
    /* retrieve node */
    assert(left->is_dynamic() && right->is_dynamic() && parent->is_dynamic());
    auto* leftnode = static_cast<rp_dynamic_node*>(left.get_ptr());
    auto* rightnode = static_cast<rp_dynamic_node*>(right.get_ptr());
    auto* parentnode = static_cast<rp_dynamic_node*>(parent.get_ptr());
    /* setup children */
    leftnode->move_to_pos(0.f, 0.f);
    rightnode->move_to_pos(w1, 0.f);
    /* update parent */
    parentnode->is_vert_arrange = false;
    if(abs(h1 - h) < 1e-6f) {
        parentnode->set_rect(parentnode->left(), parentnode->top(), w + w1, gs_max(h, h1));
        parentnode->set_no_blank();
    }
    else if(h1 < h) {
        parentnode->set_rect(parentnode->left(), parentnode->top(), w + w1, h);
        parentnode->set_blank(rp_rect(0.f, h1, w1, h - h1));
    }
    else {
        parentnode->set_rect(parentnode->left(), parentnode->top(), w + w1, h1);
        parentnode->set_blank(rp_rect(w1, h, w, h1 - h));
    }
}

static void make_vertical_pack(rp_iterator parent, rp_iterator left, rp_iterator right)
{
    float wl = left->width;
    float w = right->width;
    float h = right->height;
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

static void make_horizontal_pack(rp_iterator parent, rp_iterator left, rp_iterator right)
{
    float hl = left->height;
    float w = right->width;
    float h = right->height;
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

static void make_pack(rp_iterator parent, rp_iterator left, rp_iterator right)
{
    assert(left && right);
    /* reset the left position first */
    left->move_to(0.f, 0.f);
    /* then make pack */
    float w1 = left->width;
    float h1 = left->height;
    float w = right->width;
    float h = right->height;
    (w1 > h1) ? make_vertical_pack(parent, left, right, w < h) :
        make_horizontal_pack(parent, left, right, w > h);
}

static float get_enlarge_area(const rp_rect& rc1, float w, float h)
{
    float w1 = rc1.width;
    float h1 = rc1.height;
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

static bool find_insert_position(rp_iterator p, ipos_info& info, float w, float h)
{
    assert(p && !p.is_leaf());
    /* 1.test if the rect was better matched with the existed rects.
     * this step must be done before the 3rd step to improve the pack quality
     */
    float a1 = w * h;
    auto left = p.left();
    auto right = p.right();
    assert(left && right);
    assert(p->is_dynamic());
    auto* node = static_cast<rp_dynamic_node*>(p.get_ptr());
    if(node->has_blank) {
        float enlarge0 = node->blank.area();
        float enlarge1 = get_enlarge_area(*left, w, h);
        float enlarge2 = get_enlarge_area(*right, w, h);
        if(enlarge1 < enlarge2) {
            if(enlarge1 < enlarge0) {
                info.tag = ipos_replace;
                info.iter = right;
                info.enlarge = enlarge1;
                return true;
            }
        }
        else {
            if(enlarge2 < enlarge0) {
                info.tag = ipos_replace;
                info.iter = left;
                info.enlarge = enlarge2;
                return true;
            }
        }
        /* 2.test if the rect could be contained in the blank.
         * if the rect was good enough to fill in the blank, stop the following detection.
         * we do this step before the 4th step to reduce the efficiency
         */
        auto& blank = node->blank;
        float wb = blank.width, hb = blank.height;
        if((gs_max(wb, hb) >= gs_max(w, h)) && (gs_min(wb, hb) >= gs_min(w, h))) {
            info.tag = ipos_blank;
            info.iter = p;
            info.enlarge = enlarge0 - a1;
            assert(info.enlarge >= 0.f);
            return true;
        }
    }
    /* 3.find a suitable position from the descendant */
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

static void update_pack_rect(rp_iterator i)
{
    assert(i);
    auto left = i.left();
    auto right = i.right();
    assert(left && right);
    float ow = i->width;
    float oh = i->height;
    /* pack it again */
    make_pack(i, left, right);
    /* to see has dimensions changed? */
    if((i->width != ow) || (i->height != oh)) {
        if(i.parent())
            update_pack_rect(i.parent());
    }
}

float rp_dynamic_node::get_area() const
{
    return __super::area();
}

void rp_dynamic_node::set_no_blank()
{
    blank = rp_rect(0.f, 0.f, 0.f, 0.f);
    has_blank = false;
}

void rp_dynamic_node::set_blank(const rp_rect& rc)
{
    blank = rc;
    has_blank = true;
}

void rp_dynamic_node::set_from_local(rp_dynamic_node& p)
{
    move_to(p.left(), p.top());
    p.move_to(0.f, 0.f);
}

void rp_dynamic_node::set_dimensions(float w, float h)
{
    set_rect(0.f, 0.f, w, h);
}

void rp_dynamic_node::move_to_pos(float x, float y)
{
    move_to(x, y);
}

void rp_dynamic_node::set_transposed(bool b)
{
    /* must be leaf */
    transposed = b;
    if(b) {
        gs_swap(x, y);
        gs_swap(width, height);
    }
}

void rp_dynamic_node::flip_leaf()
{
    transposed = !transposed;
    gs_swap(x, y);
    gs_swap(width, height);
}

void rp_dynamic_node::flip_non_leaf()
{
    transposed = !transposed;
    is_vert_arrange = !is_vert_arrange;
    gs_swap(x, y);
    gs_swap(width, height);
    if(has_blank) {
        gs_swap(blank.x, blank.y);
        gs_swap(blank.width, blank.height);
    }
}

string rp_dynamic_node::to_string() const
{
    string str;
    str.format(_t("w,h:{%f,%f}"), width, height);
    return str;
}

void rect_packer::initialize_dynamically()
{
    _strategy = ps_dynamically;
    _tree.destroy();
}

void rect_packer::add_rect_dynamically(float w, float h, void* binding)
{
    assert(_strategy == ps_dynamically);
    rp_tree rc;
    auto r = rc.insertll<rp_dynamic_node>(rp_iterator(0));
    assert(r->is_dynamic());
    r->set_bind_ptr(binding);
    static_cast<rp_dynamic_node*>(r.get_ptr())->set_dimensions(w, h);
    add_rect_dynamically(rc);
}

void rect_packer::add_rect_dynamically(rp_tree& rc)
{
    assert(_strategy == ps_dynamically);
    assert(rc.is_valid());
    rp_tree xchg;
    while(!add_rect_dynamically(rc, xchg)) {
        rc.swap(xchg);
        xchg.clear();
    }
}

bool rect_packer::add_rect_dynamically(rp_tree& rc, rp_tree& xchg)
{
    assert(_strategy == ps_dynamically);
    assert(rc.is_valid());
    auto r = _tree.get_root();
    if(!r) {
        assert(!_tree.is_valid());
        _tree.swap(rc);
        return true;
    }
    else if(r.is_leaf()) {
        rp_tree root;
        auto nr = root.insertll<rp_dynamic_node>(rp_iterator(0));
        assert(nr);
        auto i = root.insertll<rp_dynamic_node>(nr);
        auto j = root.insertrl<rp_dynamic_node>(nr);
        i = root.attach(_tree, i);
        j = root.attach(rc, j);
        make_pack(nr, i, j);
        _tree.swap(root);
        return true;
    }
    else {
        ipos_info info;
        auto& rcrc = *rc.get_root();
        if(!find_insert_position(r, info, rcrc.width, rcrc.height)) {
            /* treated as combine, enlarge here was useless. */
            info.tag = ipos_combine;
            info.iter = r;
        }
        assert(info.tag != ipos_nowhere);
        switch(info.tag)
        {
        case ipos_blank:
            proc_fill_up(info.iter, rc);
            return true;
        case ipos_combine:
            proc_combine(info.iter, rc);
            return true;
        case ipos_replace:
            proc_replace(info.iter, rc, xchg);
            return false;
        default:
            assert(!"unexpected.");
            return true;
        }
    }
}

void rect_packer::proc_fill_up(rp_iterator p, rp_tree& t)
{
    assert(_strategy == ps_dynamically);
    assert(p && p->is_dynamic());
    auto* node = static_cast<rp_dynamic_node*>(p.get_ptr());
    assert(node->has_blank);
    auto left = p.left();
    auto right = p.right();
    assert(left && right);
    if(node->is_vert_arrange) {
        auto i = (left->width > right->width) ? _tree.insertrl<rp_dynamic_node>(p) : _tree.insertll<rp_dynamic_node>(p);
        assert(i && !i.right());
        auto j = _tree.insertrl<rp_dynamic_node>(i);
        assert(j);
        assert(i->is_dynamic() && j->is_dynamic());
        static_cast<rp_dynamic_node*>(i.get_ptr())->set_from_local(static_cast<rp_dynamic_node&>(*j));
        j = _tree.attach(t, j);
        make_horizontal_pack(i, i.left(), j);
        update_pack_rect(p);
    }
    else {
        auto i = (left->height > right->height) ? _tree.insertrl<rp_dynamic_node>(p) : _tree.insertll<rp_dynamic_node>(p);
        assert(i && !i.right());
        auto j = _tree.insertrl<rp_dynamic_node>(i);
        assert(j);
        assert(i->is_dynamic() && j->is_dynamic());
        static_cast<rp_dynamic_node*>(i.get_ptr())->set_from_local(static_cast<rp_dynamic_node&>(*j));
        j = _tree.attach(t, j);
        make_vertical_pack(i, i.left(), j);
        update_pack_rect(p);
    }
}

void rect_packer::proc_combine(rp_iterator p, rp_tree& t)
{
    assert(_strategy == ps_dynamically);
    assert(p);
    auto g = p.parent();
    g ? proc_combine_non_root(g, p, t) :
        proc_combine_root(p, t);
}

void rect_packer::proc_combine_root(rp_iterator p, rp_tree& t)
{
    assert(_strategy == ps_dynamically);
    assert(p);
    rp_tree t2;
    auto r = t2.insertll<rp_dynamic_node>(rp_iterator(0));    /* add a root */
    auto left = t2.insertll<rp_dynamic_node>(r);
    auto right = t2.insertrl<rp_dynamic_node>(r);
    assert(r && left && right);
    left = t2.attach(_tree, left);
    right = t2.attach(t, right);
    update_pack_rect(r);
    _tree.swap(t2);
}

void rect_packer::proc_combine_non_root(rp_iterator p, rp_iterator i, rp_tree& t)
{
    assert(_strategy == ps_dynamically);
    assert(p && i && (i.parent() == p));
    auto ins = i.is_left() ? _tree.insertll<rp_dynamic_node>(p) : _tree.insertrl<rp_dynamic_node>(p);
    assert(ins);
    auto left = ins.left();
    assert(left && !ins.right());
    auto right = _tree.insertrl<rp_dynamic_node>(ins);
    assert(right);
    right = _tree.attach(t, right);
    update_pack_rect(ins);
}

void rect_packer::proc_replace(rp_iterator p, rp_tree& t, rp_tree& xchg)
{
    assert(_strategy == ps_dynamically);
    assert(p && t.is_valid());
    auto g = p.parent();
    assert(g && "must have parent.");
    auto sib = p.sibling();
    bool isleft = p.is_left();
    assert(!xchg.is_valid());
    _tree.detach(xchg, p);
    auto np = isleft ? _tree.insertll<rp_dynamic_node>(g) : _tree.insertrl<rp_dynamic_node>(g);
    np = _tree.attach(t, np);
    update_pack_rect(g);
}

void rect_packer::trace_total() const
{
    assert(_strategy == ps_dynamically);
    _tree.preorder_traversal([](const rp_wrapper* w) {
        assert(w);
        trace_rp_node_total(w);
    });
}

void rect_packer::trace_blank() const
{
    assert(_strategy == ps_dynamically);
    _tree.preorder_traversal([](const rp_wrapper* w) {
        assert(w);
        trace_rp_blank(w);
    });
}

void rect_packer::trace_tree() const
{
    assert(_strategy == ps_dynamically);
    trace_rp_tree(_tree);
}

bool rect_packer::checkups() const
{
    assert(_strategy == ps_dynamically);
    return checkup_rp_tree(_tree.const_root());
}

void rect_packer::trace_dynamically() const
{
    trace(_t("#start tracing dynamically.\n"));
    _tree.preorder_traversal([](const rp_wrapper* w) {
        assert(w);
        trace_rp_node(w);
    });
    /* add a boundary. */
    trace(_t("@!\n"));
    trace(_t("@&strokeColor=rgb(0,0,0);\n"));
    auto r = _tree.const_root();
    trace(_t("@rect %f, %f, %f, %f;\n"), r->left(), r->top(), r->right(), r->bottom());
    trace(_t("@@\n"));
    trace(_t("#end tracing.\n"));
}

__ariel_end__
