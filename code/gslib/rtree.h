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

#ifndef rtree_77e9ff20_6040_46f3_b87d_ab64a4ac506a_h
#define rtree_77e9ff20_6040_46f3_b87d_ab64a4ac506a_h

#include <gslib/error.h>
#include <gslib/tree.h>
#include <gslib/std.h>
#include <pink/type.h>

__gslib_begin__

using pink::pointf;
using pink::rectf;

template<class _bind, int _bsize = sizeof(_bind)>
class rtree_entity
{
public:
    typedef rtree_entity<_bind, _bsize> myref;
    typedef _bind bind_type;
    typedef const _bind& bind_arg;

protected:
    rectf           _rect;
    bind_type       _bind_arg;

public:
    rtree_entity() {}
    rtree_entity(bind_arg ba, const rectf& rc): _rect(rc), _bind_arg(ba) {}
    const rectf& const_rect() const { return _rect; }
    rectf& get_rect() { return _rect; }
    bind_arg get_bind_arg() const { return _bind_arg; }
    void set_rect(const rectf& rc) { _rect = rc; }
    void set_bind_arg(bind_arg ba) { _bind_arg = ba; }
};

template<class _bind>
class rtree_entity<_bind, sizeof(void*)>
{
public:
    typedef rtree_entity<_bind, sizeof(void*)> myref;
    typedef _bind bind_type;
    typedef _bind bind_arg;

protected:
    rectf           _rect;
    bind_type       _bind_arg;

public:
    rtree_entity() { _bind_arg = 0; }
    rtree_entity(bind_arg ba, const rectf& rc): _rect(rc), _bind_arg(ba) {}
    const rectf& const_rect() const { return _rect; }
    rectf& get_rect() { return _rect; }
    bind_arg get_bind_arg() const { return _bind_arg; }
    void set_rect(const rectf& rc) { _rect = rc; }
    void set_bind_arg(bind_arg ba) { _bind_arg = ba; }
};

template<class _entity>
using rtree_node = _treenode_cpy_wrapper<_entity>;

template<class _rect>
static float calc_enlarge_area(const _rect& rc1, const _rect& rc2, float s)
{
    assert(s == rc1.area());
    float s2 = rc2.area();
    _rect u, d;
    union_rect(u, rc1, rc2);
    if(intersect_rect(d, rc1, rc2))
        return u.area() - s - s2 + d.area();
    return u.area() - s - s2;
}

template<class _rect>
static float calc_enlarge_area(const _rect& rc1, const _rect& rc2)
{
    return calc_enlarge_area(rc1, rc2, rc1.area());
}

inline float calc_orientation(const float coef[3], const pointf& p)
{
    assert(coef);
    return coef[0] * p.x + coef[1] * p.y + coef[2];
}

inline void calc_perpendicular_bisector(float coef[3], const pointf& p1, const pointf& p2)
{
    assert(coef);
    pointf c;
    c.x = (p1.x + p2.x) * 0.5f;
    c.y = (p1.y + p2.y) * 0.5f;
    float d[2];
    d[0] = p1.y - p2.y;
    d[1] = p1.x - p2.x;
    if(abs(d[0]) < 1e-5f) {
        coef[0] = 1.f;
        coef[1] = -c.x;
        coef[2] = 0.f;
    }
    else if(abs(d[1]) < 1e-5f) {
        coef[0] = -c.y;
        coef[1] = 1.f;
        coef[2] = 0.f;
    }
    else {
        coef[0] = d[1] / d[0];
        coef[1] = 1.f;
        coef[2] = -c.x * coef[0] - c.y;
    }
    /* test orientation */
    float t = calc_orientation(coef, p1);
    if(t > 0.f) {
        coef[0] = -coef[0];
        coef[1] = -coef[1];
        coef[2] = -coef[2];
    }
}

template<class _tree>
class rtree_finder
{
public:
    typedef _tree mytree;
    typedef typename _tree::value value;
    typedef typename _tree::iterator iterator;
    typedef typename _tree::const_iterator const_iterator;
    typedef typename value::bind_arg bind_arg;

protected:
    mytree&         _mytree;

public:
    rtree_finder(mytree& mt): _mytree(mt) {}
    iterator find(bind_arg ba, const rectf& rc) const { return find(_mytree.get_root(), ba, rc); }

protected:
    iterator find(iterator p, bind_arg ba, const rectf& rc) const
    {
        assert(p.is_valid());
        if(!is_rect_intersected(rc, p->const_rect()))
            return iterator(0);
        if(p.is_leaf())
            return (p->get_bind_arg() == ba) ? p : iterator(0);
        for(auto i = p.child(); i.is_valid(); i.to_next()) {
            auto f = find(i, ba, rc);
            if(f.is_valid())
                return f;
        }
        return iterator(0);
    }
};

/*
 * Here we implement an algorithm for the bulk loading. For a set of input inserting into
 * the rtree, original insert was a waste.
 * The algorithm was very similar to the STR insert as the following paper stated:
 * http://www.dtic.mil/dtic/tr/fulltext/u2/a324493.pdf
 */
template<int _max_record, int _min_record, class _tree>
class rtree_str_load
{
public:
    typedef _tree mytree;
    typedef typename _tree::value value;
    typedef typename _tree::wrapper wrapper;
    typedef typename _tree::iterator iterator;
    typedef typename _tree::const_iterator const_iterator;
    typedef typename _tree::value value;
    typedef typename value::bind_arg bind_arg;
    typedef rtree_finder<mytree> finder;
    typedef list<value> value_list;
    static const int max_record = _max_record;
    static const int min_record = _min_record;
    struct bulk_node;
    typedef list<bulk_node*> bulk_list;
    typedef list<wrapper*> wrapper_list;

    struct bulk_node:
        public value
    {
        pointf      center;
        wrapper*    assoc_wrapper;

    public:
        bulk_node(const value& v):
            value(v)
        {
            auto& rc = v.const_rect();
            center.x = 0.5f * (rc.left + rc.right);
            center.y = 0.5f * (rc.top + rc.bottom);
            assoc_wrapper = 0;
        }
        bulk_node(wrapper* w):
            value(w->get_ref())
        {
            auto& v = w->get_ref();
            auto& rc = v.const_rect();
            center.x = 0.5f * (rc.left + rc.right);
            center.y = 0.5f * (rc.top + rc.bottom);
            assoc_wrapper = w;
        }
    };

public:
    rtree_str_load(mytree& tr): _mytree(tr) {}
    template<class _vsl>
    void load(const _vsl& input)
    {
        wrapper_list in, out;
        for(auto& p : input) {
            auto* w = gs_new(wrapper);
            w->born();
            auto& v = w->get_ref();
            v.set_bind_arg(p.get_bind_arg());
            v.set_rect(p.const_rect());
            in.push_back(w);
        }
        for(;;) {
            int s = packing(out, in);
            if(s == 1) {
                _mytree.set_root(out.front());
                return;
            }
            out.swap(in);
            out.clear();
        }
    }

protected:
    mytree&         _mytree;

protected:
    void set_as_children(wrapper* p, wrapper_list& ch)
    {
        assert(p);
        assert(!ch.empty());
        int size = (int)ch.size();
        if(size == 1) {
            p->acquire_children(wrapper::children(ch.front()));
            return;
        }
        auto i = ch.begin(), j = std::next(i), end = ch.end();
        for(; j != end; i = j ++)
            wrapper::children::join(*i, *j);
        p->acquire_children(wrapper::children(ch.front(), ch.back(), (int)ch.size()));
    }
    void set_bound_rect(wrapper* p)
    {
        assert(p);
        float l = FLT_MAX, t = FLT_MAX, r = -FLT_MAX, b = -FLT_MAX;
        p->for_each([&](wrapper* w) {
            assert(w);
            auto& v = w->get_ref();
            auto& rc = v.get_rect();
            l = gs_min(l, rc.left);
            t = gs_min(t, rc.top);
            r = gs_max(r, rc.right);
            b = gs_max(b, rc.bottom);
        });
        p->born();
        auto& v = p->get_ref();
        rectf rc;
        rc.set_ltrb(l, t, r, b);
        v.set_rect(rc);
    }
    int packing(wrapper_list& output, wrapper_list& input)
    {
        assert(output.empty());
        assert(!input.empty());
        int size = (int)input.size();
        if(size <= max_record) {
            auto* p = gs_new(wrapper);
            set_as_children(p, input);
            set_bound_rect(p);
            output.push_back(p);
            return 1;
        }
        list<bulk_node> bulks;
        for(auto* w : input)
            bulks.push_back(bulk_node(w));
        bulks.sort([](const bulk_node& n1, const bulk_node& n2)-> bool {
            return n1.center.x < n2.center.x;
        });
        float rough_slice_count = sqrtf((float)size / max_record);
        float rough_slice_size = (float)size / rough_slice_count;
        int slice_size = (int)(rough_slice_size + 0.5f);
        int full_slice_count = size / slice_size;
        int slice_count = (size % slice_size) ? full_slice_count + 1 : full_slice_count;
        auto* slices = new list<bulk_node*>[slice_count];
        assert(slices);
        auto i = bulks.begin(), end = bulks.end();
        bool ascend = true;
        auto asc_pr = [](const bulk_node* n1, const bulk_node* n2)-> bool { return n1->center.y < n2->center.y; };
        auto desc_pr = [](const bulk_node* n1, const bulk_node* n2)-> bool { return n1->center.y > n2->center.y; };
        for(int j = 0, k = 0; i != end; ++ i) {
            assert(k < slice_count);
            slices[k].push_back(&(*i));
            if(++ j >= slice_size) {
                assert((int)slices[k].size() == slice_size);
                ascend ? slices[k].sort(asc_pr) : slices[k].sort(desc_pr);
                ascend = !ascend;
                j = 0, k ++;
            }
        }
        if(slice_count != full_slice_count) {
            assert(full_slice_count + 1 == slice_count);
            ascend ? slices[full_slice_count].sort(asc_pr) : slices[full_slice_count].sort(desc_pr);
        }
        /* join the list */
        vector<bulk_node*> sorted;
        for(int j = 0; j < slice_count; j ++)
            sorted.insert(sorted.end(), slices[j].begin(), slices[j].end());
        divide(output, sorted);
        delete [] slices;
        return (int)output.size();
    }
    void divide(wrapper_list& output, vector<bulk_node*>& sorted)
    {
        int size = (int)sorted.size();
        for(int i = 0; i < size; i += max_record) {
            int left = size - i;
            if(left < max_record + min_record) {
                if(left <= max_record) {
                    wrapper_list ch;
                    for(int j = i; j < size; j ++)
                        ch.push_back(sorted.at(j)->assoc_wrapper);
                    auto* w = gs_new(wrapper);
                    set_as_children(w, ch);
                    set_bound_rect(w);
                    output.push_back(w);
                }
                else {
                    int size1 = left / 2;
                    int size2 = left - size1;
                    wrapper_list ch1, ch2;
                    for(int j = 0; j < size1; j ++)
                        ch1.push_back(sorted.at(j + i)->assoc_wrapper);
                    for(int j = i + size1; j < size; j ++)
                        ch2.push_back(sorted.at(j)->assoc_wrapper);
                    auto* w1 = gs_new(wrapper);
                    auto* w2 = gs_new(wrapper);
                    set_as_children(w1, ch1);
                    set_bound_rect(w1);
                    set_as_children(w2, ch2);
                    set_bound_rect(w2);
                    output.push_back(w1);
                    output.push_back(w2);
                }
                return;
            }
            wrapper_list ch;
            for(int j = 0; j < max_record; j ++)
                ch.push_back(sorted.at(i + j)->assoc_wrapper);
            auto* w = gs_new(wrapper);
            set_as_children(w, ch);
            set_bound_rect(w);
            output.push_back(w);
        }
    }
};

template<int _max_record, int _min_record, class _tree>
class rtree_alg_framework
{
public:
    typedef _tree mytree;
    typedef typename _tree::value value;
    typedef typename _tree::wrapper wrapper;
    typedef typename _tree::iterator iterator;
    typedef typename _tree::const_iterator const_iterator;
    typedef typename _tree::value value;
    typedef typename value::bind_arg bind_arg;
    typedef rtree_finder<mytree> finder;
    typedef list<value> value_list;
    static const int max_record = _max_record;
    static const int min_record = _min_record;

public:
    rtree_alg_framework(mytree& tr): _mytree(tr) {}

protected:
    mytree&         _mytree;

protected:
    static void self_transfer(mytree& mt, iterator i, iterator dp)
    {
        assert(i.is_valid() && dp.is_valid());
        mytree t;
        mt.detach(t, i);
        mt.attach(t, mt.birth_tail(dp));
    }
    static void update_subtree_rect(iterator p)
    {
        assert(p.is_valid() && !p.is_leaf());
        auto i = p.child();
        rectf rc = i->const_rect();
        for(i.to_next(); i.is_valid(); i.to_next())
            union_rect(rc, rc, i->const_rect());
        p->set_rect(rc);
    }
    static void update_rect_recursively(iterator p)
    {
        assert(p.is_valid() && !p.is_leaf());
        auto i = p.child();
        rectf rc = i->const_rect();
        for(i.to_next(); i.is_valid(); i.to_next())
            union_rect(rc, rc, i->const_rect());
        if(p->const_rect() != rc) {
            p->set_rect(rc);
            auto g = p.parent();
            if(g.is_valid())
                update_rect_recursively(g);
        }
    }
    static void update_rect_recursively(iterator p, iterator i)
    {
        assert(p.is_valid() && i.is_valid() && (i.parent() == p));
        rectf rc = p->const_rect();
        union_rect(rc, rc, i->const_rect());
        if(p->const_rect() != rc) {
            p->set_rect(rc);
            auto g = p.parent();
            if(g.is_valid())
                update_rect_recursively(g, p);
        }
    }
    static void collect_leaves(value_list& candidates, iterator i)
    {
        assert(i.is_valid());
        if(i.is_leaf()) {
            candidates.push_back(*i);
            return;
        }
        for(auto j = i.child(); j.is_valid(); j.to_next())
            collect_leaves(candidates, j);
    }

protected:
    void split_binary(iterator p, iterator i, iterator j)
    {
        assert(p.is_valid() && (p.childs() == max_record + 1));
        assert(i.is_valid() && j.is_valid() && (i.parent() == p) && (j.parent() == p));
        auto g = p.parent();
        iterator q;
        if(g.is_valid())
            q = _mytree.insert_after(p);
        else {
            mytree mt;
            auto r = mt.insert(iterator(0));
            r->set_rect(p->const_rect());
            auto p1 = mt.birth(r);
            auto p2 = mt.birth_tail(r);
            mt.attach(_mytree, p1);
            mt.swap(_mytree);
            p = p1;
            q = p2;
        }
        int max_split = max_record + 1 - min_record;
        /* retrieve the split line */
        auto c1 = i->const_rect().center();
        auto c2 = j->const_rect().center();
        float coef[3];
        calc_perpendicular_bisector(coef, c1, c2);
        /* sort them by signed distance */
        typedef multimap<float, iterator> sort_by_sd;
        sort_by_sd sbsd;
        float muler = 1.f / sqrtf(coef[0] * coef[0] + coef[1] * coef[1]);
        for(auto m = p.child(); m.is_valid(); m.to_next()) {
            if(m != i && m != j) {
                float d = muler * calc_orientation(coef, m->const_rect().center());
                sbsd.insert(std::make_pair(d, m));
            }
        }
        /* dump to a random access container */
        typedef vector<iterator> sd_queue;
        assert(sbsd.size() == max_record - 1);
        int divpos = -1, idx = 0;
        sd_queue sdq;
        sdq.resize(sbsd.size());
        for(auto n = sbsd.begin(); n != sbsd.end(); ++ n, ++ idx) {
            sdq.at(idx) = n->second;
            if((divpos < 0) && (n->first >= 0.f))
                divpos = idx;
        }
        if(divpos < 0)
            divpos = 0;
        /* clamp */
        if(divpos < min_record)
            divpos = min_record;
        else if(divpos > max_split)
            divpos = max_split;
        /* transfer from p to q */
        self_transfer(_mytree, j, q);
        for(auto pos = divpos; pos != (int)sdq.size(); ++ pos)
            self_transfer(_mytree, sdq.at(pos), q);
        /* update p, q's rect */
        update_subtree_rect(p);
        update_subtree_rect(q);
    }
};

/*
 * It's a bit weird that the Guttman's paper said that:
 * "PickNext simply chooses any of the remaining entries"
 * Does this algorithm actually useful, or maybe I implemented it wrong.
 * Use quadratic split in preference.
 * plus TODO:
 * R* algorithm to be implemented, but undone.
 */
template<int _max_record, int _min_record, class _tree>
class linear_split_alg:
    public rtree_alg_framework<_max_record, _min_record, _tree>
{
public:
    linear_split_alg(mytree& tr):
        rtree_alg_framework<_max_record, _min_record, _tree>(tr)
    {
        /* better be static_assert! */
        assert((min_record * 2 <= max_record) &&
            "min_record should be less than half max_record."
            );
    }
    void insert(bind_arg ba, const rectf& rc)
    {
        if(!_mytree.is_valid()) {
            auto i = _mytree.insert(iterator(0));
            *i = value(ba, rc);
            return;
        }
        auto i = find_insert_position(_mytree.get_root());
        if(i.is_valid()) {
            assert(i.childs() < max_record);
            auto j = _mytree.birth_tail(i);
            *j = value(ba, rc);
            update_rect_recursively(i, j);
            return;
        }
        i = find_first_leaf(_mytree.get_root());
        assert(i.is_valid() && i.is_leaf());
        auto p = i.parent();
        if(!p.is_valid()) {
            assert(i == _mytree.get_root());
            auto j = _mytree.birth(i);
            auto k = _mytree.birth_tail(i);
            *j = *i;
            *k = value(ba, rc);
            rectf new_rc;
            union_rect(new_rc, i->const_rect(), rc);
            i->set_bind_arg(0);
            i->set_rect(new_rc);
            return;
        }
        assert(p.childs() == max_record);
        /* temporarily insert into p, then split */
        auto j = _mytree.birth_tail(p);
        *j = value(ba, rc);
        update_rect_recursively(p, j);
        do { p = adjust_tree(p); }
        while(p.is_valid());
    }
    void remove(bind_arg ba, const rectf& rc)
    {
        auto f = finder(_mytree).find(ba, rc);
        if(!f.is_valid())
            return;
        assert(f.is_leaf());
        if(f.is_root()) {
            _mytree.clear();
            return;
        }
        auto p = f.parent();
        assert(p.is_valid());
        _mytree.erase(f);
        if(p.childs() >= min_record) {
            update_rect_recursively(p);
            return;
        }
        value_list candidates;
        do { p = condense_tree(candidates, p); }
        while(p.is_valid());
        reinsert(candidates);
    }

protected:
    iterator find_insert_position(iterator i)
    {
        assert(i.is_valid());
        if(i.is_leaf())
            return iterator(0);
        auto j = i.child();
        if(j.is_leaf())
            return (i.childs() < max_record) ? i : iterator(0);
        for(; j.is_valid(); j.to_next()) {
            auto r = find_insert_position(j);
            if(r.is_valid())
                return r;
        }
        return iterator(0);
    }
    iterator find_first_leaf(iterator i)
    {
        assert(i.is_valid());
        return i.is_leaf() ? i : find_first_leaf(i.child());
    }
    iterator find_extreme_u(iterator p)
    {
        assert(p.is_valid() && !p.is_leaf());
        auto i = p.child();
        assert(i.is_valid());
        float m = i->const_rect().width();
        auto r = i;
        for(i.to_next(); i.is_valid(); i.to_next()) {
            float u = i->const_rect().width();
            if(u > m)
                r = i, m = u;
        }
        return r;
    }
    iterator find_extreme_v_but_not(iterator p, iterator c)
    {
        assert(p.is_valid() && !p.is_leaf());
        assert(c.is_valid() && (c.parent() == p));
        iterator r;
        float m = -FLT_MAX;
        for(auto i = p.child(); i.is_valid(); i.to_next()) {
            if(i == c)
                continue;
            float v = i->const_rect().height();
            if(v > m)
                r = i, m = v;
        }
        assert(r.is_valid());
        return r;
    }
    void split(iterator p)
    {
        assert(p.is_valid() && (p.childs() == max_record + 1));
        auto i = find_extreme_u(p);
        auto j = find_extreme_v_but_not(p, i);
        split_binary(p, i, j);
    }
    iterator adjust_tree(iterator i)
    {
        assert(i.is_valid() && !i.is_leaf());
        assert(i.childs() == max_record + 1);
        auto p = i.parent();
        split(i);
        if(!p.is_valid())
            return iterator(0);
        return (p.childs() <= max_record) ? iterator(0) : p;
    }
    iterator condense_tree(value_list& candidates, iterator i)
    {
        assert(i.is_valid() && !i.is_leaf());
        assert(i.childs() < min_record);
        if(i.is_root()) {
            if(i.childs() == 1) {
                /* height decrease */
                mytree t;
                _mytree.detach(t, i.child());
                _mytree.swap(t);
            }
            return iterator(0);
        }
        collect_leaves(candidates, i);
        auto p = i.parent();
        assert(p.is_valid());
        _mytree.erase(i);
        if(p.childs() >= min_record) {
            update_subtree_rect(p);
            return iterator(0);
        }
        return p;
    }
    void reinsert(value_list& candidates)
    {
        if(candidates.empty())
            return;
        std::for_each(candidates.begin(), candidates.end(), [this](const value& val) {
            this->insert(val.get_bind_arg(), val.const_rect());
        });
    }
};

template<int _max_record, int _min_record, class _tree>
class quadratic_split_alg:
    public rtree_alg_framework<_max_record, _min_record, _tree>
{
public:
    quadratic_split_alg(mytree& tr):
        rtree_alg_framework<_max_record, _min_record, _tree>(tr)
    {
        /* better be static_assert! */
        assert((min_record * 2 <= max_record) &&
            "min_record should be less than half max_record."
            );
    }
    void insert(bind_arg ba, const rectf& rc)
    {
        if(!_mytree.is_valid()) {
            auto i = _mytree.insert(iterator(0));
            *i = value(ba, rc);
            return;
        }
        auto i = find_insert_position(_mytree.get_root(), rc);
        assert(i.is_valid());
        auto p = i.parent();
        if(!p.is_valid()) {
            assert(i == _mytree.get_root());
            auto j = _mytree.birth(i);
            auto k = _mytree.birth_tail(i);
            *j = *i;
            *k = value(ba, rc);
            rectf new_rc;
            union_rect(new_rc, i->const_rect(), rc);
            i->set_bind_arg(0);
            i->set_rect(new_rc);
            return;
        }
        if(p.childs() < max_record) {
            auto j = _mytree.birth_tail(p);
            *j = value(ba, rc);
            update_rect_recursively(p, j);
            return;
        }
        /* temporarily insert into p, then split */
        auto j = _mytree.birth_tail(p);
        *j = value(ba, rc);
        update_rect_recursively(p, j);
        do { p = adjust_tree(p); }
        while(p.is_valid());
    }
    void remove(bind_arg ba, const rectf& rc)
    {
        auto f = finder(_mytree).find(ba, rc);
        if(!f.is_valid())
            return;
        assert(f.is_leaf());
        if(f.is_root()) {
            _mytree.clear();
            return;
        }
        auto p = f.parent();
        assert(p.is_valid());
        _mytree.erase(f);
        if(p.childs() >= min_record) {
            update_rect_recursively(p);
            return;
        }
        value_list candidates;
        do { p = condense_tree(candidates, p); }
        while(p.is_valid());
        reinsert(candidates);
    }

protected:
    iterator find_least_enlargement(iterator i, const rectf& rc)
    {
        assert(i.is_valid());
        auto c = i.child();
        auto ret = c;
        float enlargement = FLT_MAX;
        for(; c.is_valid(); c = c.next()) {
            const rectf& oldrc = c->const_rect();
            float oldarea = oldrc.area();
            rectf fr;
            union_rect(fr, oldrc, rc);
            float area = fr.area();
            float enlarge = area - oldarea;
            if(enlarge < enlargement) {
                enlargement = enlarge;
                ret = c;
            }
        }
        return ret;
    }
    float find_biggest_enlargement(iterator& i, iterator& j, iterator p)
    {
        assert(p.is_valid() && !p.is_leaf());
        assert(p.childs() == max_record + 1);
        iterator f = p.child(), g;
        assert(f.next().is_valid());
        float m = find_biggest_enlargement(g, f, f.next(), p.last_child());
        i = f, j = g;
        for(f.to_next(); f != p.last_child(); f.to_next()) {
            float n = find_biggest_enlargement(g, f, f.next(), p.last_child());
            if(n > m) {
                m = n;
                i = f, j = g;
            }
        }
        return m;
    }
    float find_biggest_enlargement(iterator& j, iterator i, iterator from, iterator to)
    {
        assert(i.is_valid() && from.is_valid() && to.is_valid());
        iterator t = from;
        float s = i->const_rect().area();
        float m = calc_enlarge_area(i->const_rect(), t->const_rect(), s);
        j = t;
        if(t == to)
            return m;
        for(t.to_next();; t.to_next()) {
            float n = calc_enlarge_area(i->const_rect(), t->const_rect(), s);
            if(n > m) { m = n, j = t; }
            if(t == to)
                break;
        }
        return m;
    }
    iterator find_insert_position(iterator i, const rectf& rc)
    {
        if(i.is_leaf())
            return i;
        auto trygetch = i.child();
        if(trygetch.is_leaf())
            return trygetch;
        auto j = find_least_enlargement(i, rc);
        assert(j.is_valid());
        return find_insert_position(j, rc);
    }
    void split(iterator p)
    {
        assert(p.is_valid() && (p.childs() == max_record + 1));
        iterator i, j;
        find_biggest_enlargement(i, j, p);
        split_binary(p, i, j);
    }
    iterator adjust_tree(iterator i)
    {
        assert(i.is_valid() && !i.is_leaf());
        assert(i.childs() == max_record + 1);
        auto p = i.parent();
        split(i);
        if(!p.is_valid())
            return iterator(0);
        return (p.childs() <= max_record) ? iterator(0) : p;
    }
    iterator condense_tree(value_list& candidates, iterator i)
    {
        assert(i.is_valid() && !i.is_leaf());
        assert(i.childs() < min_record);
        if(i.is_root()) {
            if(i.childs() == 1) {
                /* height decrease */
                mytree t;
                _mytree.detach(t, i.child());
                _mytree.swap(t);
            }
            return iterator(0);
        }
        collect_leaves(candidates, i);
        auto p = i.parent();
        assert(p.is_valid());
        _mytree.erase(i);
        if(p.childs() >= min_record) {
            update_subtree_rect(p);
            return iterator(0);
        }
        return p;
    }
    void reinsert(value_list& candidates)
    {
        if(candidates.empty())
            return;
        std::for_each(candidates.begin(), candidates.end(), [this](const value& val) {
            this->insert(val.get_bind_arg(), val.const_rect());
        });
    }
};

template<class _ty,
    class _alg,
    class _wrapper = rtree_node<_ty>,
    class _alloc = _tree_allocator<_wrapper>
    >
class rtree:
    public tree<_ty, _wrapper, _alloc>
{
public:
    typedef _ty value;
    typedef _wrapper wrapper;
    typedef _alloc alloc;
    typedef _alg alg;
    typedef rtree<_ty, _alg, _wrapper, _alloc> myref;
    typedef tree<_ty, _wrapper, _alloc> superref;
    typedef superref::iterator iterator;
    typedef superref::const_iterator const_iterator;
    typedef typename value::bind_arg bind_arg;
    typedef unordered_set<bind_arg> lookup_table;

public:
    template<class _querier, class _cont>
    int query(const _querier& q, _cont& out) const
    {
        if(!is_valid())
            return 0;
        lookup_table lookups;
        auto i = get_root();
        return query_overlapped(i, q, lookups, out);
    }
    void insert(bind_arg ba, const rectf& rc) { alg(*this).insert(ba, rc); }
    void remove(bind_arg ba, const rectf& rc) { alg(*this).remove(ba, rc); }
    void tracing() const
    {
        if(!is_valid())
            return;
        auto r = get_root();
        trace_subtree(r);
        trace_layer(r);
    }
    bool empty() const { return !is_valid(); }
    template<class _vsl>
    void insert(const _vsl& input)
    {
        if(input.size() == 1) {
            auto& p = input.front();
            insert(p.get_bind_arg(), p.const_rect());
            return;
        }
        rtree_str_load<alg::max_record, alg::min_record, alg::mytree>(*this).load(input);
    }

protected:
    template<class _cont>
    int query_overlapped(const_iterator i, const rectf& q, lookup_table& lookups, _cont& out) const
    {
        assert(i.is_valid());
        const auto& rc = i->const_rect();
        if(!is_rect_intersected(rc, q))
            return 0;
        if(i.is_leaf()) {
            if(lookups.find(i->get_bind_arg()) != lookups.end())
                return 0;
            lookups.insert(i->get_bind_arg());
            out.push_back(i->get_bind_arg());
            return 1;
        }
        int c = 0;
        for(auto j = i.child(); j.is_valid(); j = j.next())
            c += query_overlapped(j, q, lookups, out);
        return c;
    }
    template<class _cont>
    int query_overlapped(const_iterator i, const pointf& q, lookup_table& lookups, _cont& out) const
    {
        assert(i.is_valid());
        const auto& rc = i->const_rect();
        if(!rc.in_rect(q))
            return 0;
        if(i.is_leaf()) {
            if(lookups.find(i->get_bind_arg()) != lookups.end())
                return 0;
            lookups.insert(i->get_bind_arg());
            out.push_back(i->get_bind_arg());
            return 1;
        }
        int c = 0;
        for(auto j = i.child(); j.is_valid(); j = j.next())
            c += query_overlapped(j, q, lookups, out);
        return c;
    }
    void trace_layer(const_iterator i) const
    {
        assert(i.is_valid());
        int r = rand() % 256;
        int g = rand() % 256;
        int b = rand() % 256;
        string cr;
        cr.format(_t("rgb(%d,%d,%d)"), r, g, b);
        trace(_t("@!\n"));
        trace(_t("@&strokeColor=%s;\n"), cr.c_str());
        trace(_t("@&withArrow=false;\n"));
        for(auto j = i; j.is_valid(); j = j.next()) {
            const auto& rc = j->const_rect();
            trace(_t("@rect %f, %f, %f, %f;\n"), rc.left, rc.top, rc.right, rc.bottom);
        }
        trace(_t("@@\n"));
    }
    void trace_subtree(const_iterator i) const
    {
        assert(i.is_valid());
        if(i.is_leaf())
            return;
        for(auto j = i.child(); j.is_valid(); j = j.next())
            trace_subtree(j);
        trace_layer(i.child());
    }
};

__gslib_end__

#endif
