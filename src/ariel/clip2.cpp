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

#include <gslib/error.h>
#include <ariel/clip2.h>

__ariel_begin__

static clip_src_edge_line* create_src_edge_line(clip_src_edges& edges, const vec2& p0, const vec2& p1)
{
    auto* e = new clip_src_edge_line(p0, p1);
    assert(e);
    edges.push_back(e);
    return e;
}

static clip_src_edge_quad* create_src_edge_quad(clip_src_edges& edges, const vec2& p0, const vec2& p1, const vec2& p2)
{
    auto* e = new clip_src_edge_quad(p0, p1, p2);
    assert(e);
    edges.push_back(e);
    return e;
}

static clip_src_edge_cubic* create_src_edge_cubic(clip_src_edges& edges, const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3)
{
    auto* e = new clip_src_edge_cubic(p0, p1, p2, p3);
    assert(e);
    edges.push_back(e);
    return e;
}

static void connect_edge(clip_edge* last_edge, clip_edge* edge)
{
    assert(last_edge && edge);
    auto* last_symm = last_edge->get_symmetric();
    auto* symm = edge->get_symmetric();
    assert(last_symm && symm);
    assert(fuzz_cmp(last_edge->get_dest(), edge->get_org()) < 1e-3f);
    last_edge->set_next(edge);
    edge->set_prev(last_edge);
    symm->set_next(last_symm);
    last_symm->set_prev(symm);
}

static void disconnect_edge(clip_edge* edge0, clip_edge* edge1)
{
    assert(edge0 && edge1);
    assert(fuzz_cmp(edge0->get_dest(), edge1->get_org()) < 1e-3f);
    auto* symm0 = edge0->get_symmetric();
    auto* symm1 = edge1->get_symmetric();
    assert(symm0 && symm1);
    edge0->set_next(nullptr);
    edge1->set_prev(nullptr);
    symm1->set_next(nullptr);
    symm0->set_prev(nullptr);
}

static inline float tolerance_correct(float f)
{
    if(f < 0.f && f > -0.001f)
        return 0.f;
    else if(f > 1.f && f < 1.001f)
        return 1.f;
    return f;
}

static inline bool is_fuzzy_neighbour(float t, float s)
{
    return (fuzzy_zero(t) && fuzzy_zero(s - 1.f)) ||
        (fuzzy_zero(t - 1.f) && fuzzy_zero(s));
}

static inline bool is_fuzzy_neighbour(const vec2& p1, const vec2& p2)
{
    return fuzz_cmp(p1, p2) < 0.5f;
}

static inline bool clip_edge_ahead_of(const clip_edge& e1, const clip_edge& e2)
{
    assert(&e1 != &e2);
    const auto& ids1 = e1.get_ids();
    const auto& ids2 = e2.get_ids();
    if(ids1.empty() || ids2.empty()) {
        assert(!"unexpected.");
        return false;
    }
    int c1 = (int)ids1.size(), c2 = (int)ids2.size();
    int c = gs_min(c1, c2);
    for(int i = 0; i < c; i ++) {
        if(ids1.at(i) != ids2.at(i))
            return ids1.at(i) < ids2.at(i);
    }
    return c1 < c2;
}

struct clip_intersection_info
{
    vec2            pt;
    clip_edge*      subject_edge = nullptr;
    float           t_in_clip_edge = -1.f;
    float           t_in_subject_edge = -1.f;
    int             serial_id = -1;

public:
    clip_intersection_info() {}
    clip_intersection_info(const vec2& p, clip_edge* e, float t1, float t2)
    {
        pt = p;
        subject_edge = e;
        t_in_clip_edge = t1;
        t_in_subject_edge = t2;
    }
    bool operator<(const clip_intersection_info& that) const
    {
        assert(subject_edge && that.subject_edge);
        if(subject_edge != that.subject_edge)
            return clip_edge_ahead_of(*subject_edge, *that.subject_edge);
        return t_in_subject_edge < that.t_in_subject_edge;
    }
};

typedef list<clip_intersection_info> clip_intersection_list;

static inline void setup_intersection_order(clip_intersection_list& ci)
{
    ci.sort();
    int sid = 0;
    for(auto& info : ci)
        info.serial_id = sid ++;
    ci.sort([](const clip_intersection_info& a, const clip_intersection_info& b)->bool { return a.t_in_clip_edge < b.t_in_clip_edge; });
}

static void query_intersection_infos(clip_intersection_list& ci, const vec2& p0, const vec2& p1, clip_rtree& rt)
{
    clip_edge_ptrs pie;     /* potentially intersected edges */
    rt.query(p0, p1, pie);
    if(pie.empty())
        return;
    vec3 coef;
    get_linear_coefficient(coef, p0, vec2().sub(p1, p0));
    for(clip_edge* e : pie) {
        assert(e);
        /* no sef, skip. */
        auto* sef = e->get_sef();
        if(!sef) {
            assert(!"unexpected.");
            continue;
        }
        switch(sef->get_type())
        {
        case clip_src_edge::st_line:
            {
                vec2 p;
                /* must've been intersected */
                intersectp_linear_linear(p, p0, sef->get_begin_point(), vec2().sub(p1, p0), vec2().sub(sef->get_end_point(), sef->get_begin_point()));
                float t1 = linear_reparameterize(p0, p1, p);
                float t2 = linear_reparameterize(sef->get_begin_point(), sef->get_end_point(), p);
                t1 = tolerance_correct(t1);
                t2 = tolerance_correct(t2);
                if(t1 >= 0.f && t1 <= 1.f && t2 >= 0.f && t2 <= 1.f) {
                    if(!is_fuzzy_neighbour(t1, t2))
                        ci.push_back(clip_intersection_info(p, e, t1, t2));
                }
                break;
            }
        case clip_src_edge::st_quad:
            {
                vec3 para[2];
                get_quad_parameter_equation(para, sef->get_begin_point(), sef->get_point(1), sef->get_end_point());
                float t[2];
                int c = intersection_quad_linear(t, para, coef);
                for(int i = 0; i < c; i ++) {
                    float t2 = gs_max(gs_min(t[i], 1.f), 0.f);
                    vec2 p;
                    eval_quad(p, para, t2);
                    float t1 = linear_reparameterize(p0, p1, p);
                    t1 = tolerance_correct(t1);
                    if(!is_fuzzy_neighbour(p, p0) &&
                        !is_fuzzy_neighbour(p, p1) &&
                        !is_fuzzy_neighbour(p, sef->get_begin_point()) &&
                        !is_fuzzy_neighbour(p, sef->get_end_point())
                        )
                        ci.push_back(clip_intersection_info(p, e, t1, t2));
                }
                break;
            }
        case clip_src_edge::st_cubic:
            {
                vec4 para[2];
                get_cubic_parameter_equation(para, sef->get_begin_point(), sef->get_point(1), sef->get_point(2), sef->get_end_point());
                float t[3];
                int c = intersection_cubic_linear(t, para, coef);
                for(int i = 0; i < c; i ++) {
                    float t2 = gs_max(gs_min(t[i], 1.f), 0.f);
                    vec2 p;
                    eval_cubic(p, para, t2);
                    float t1 = linear_reparameterize(p0, p1, p);
                    t1 = tolerance_correct(t1);
                    if(!is_fuzzy_neighbour(p, p0) &&
                        !is_fuzzy_neighbour(p, p1) &&
                        !is_fuzzy_neighbour(p, sef->get_begin_point()) &&
                        !is_fuzzy_neighbour(p, sef->get_end_point())
                        )
                        ci.push_back(clip_intersection_info(p, e, t1, t2));
                }
                break;
            }
        }
    }
    setup_intersection_order(ci);
}

static void query_intersection_infos(clip_intersection_list& ci, const vec2& p0, const vec2& p1, const vec2& p2, clip_rtree& rt)
{
    rectf rc;
    get_quad_bound_box(rc, p0, p1, p2);
    clip_edge_ptrs pie;
    rt.query(rc, pie);
    if(pie.empty())
        return;
    vec3 para[2];
    get_quad_parameter_equation(para, p0, p1, p2);
    vec2 cp[] = { p0, p1, p2 };
    for(clip_edge* e : pie) {
        assert(e);
        auto* sef = e->get_sef();
        if(!sef) {
            assert(!"unexpected.");
            continue;
        }
        switch(sef->get_type())
        {
        case clip_src_edge::st_line:
            {
                vec3 coef;
                get_linear_coefficient(coef, sef->get_begin_point(), vec2().sub(sef->get_end_point(), sef->get_begin_point()));
                float t[2];
                int c = intersection_quad_linear(t, para, coef);
                for(int i = 0; i < c; i ++) {
                    float t1 = gs_max(gs_min(t[i], 1.f), 0.f);
                    vec2 p;
                    eval_quad(p, para, t1);
                    float t2 = linear_reparameterize(sef->get_begin_point(), sef->get_end_point(), p);
                    t2 = tolerance_correct(t2);
                    if(!is_fuzzy_neighbour(p, p0) &&
                        !is_fuzzy_neighbour(p, p1) &&
                        !is_fuzzy_neighbour(p, sef->get_begin_point()) &&
                        !is_fuzzy_neighbour(p, sef->get_end_point())
                        )
                        ci.push_back(clip_intersection_info(p, e, t1, t2));
                }
                break;
            }
        case clip_src_edge::st_quad:
            {
                vec3 para2[2];
                get_quad_parameter_equation(para2, sef->get_begin_point(), sef->get_point(1), sef->get_end_point());
                float t[4][2];
                int c = intersection_quad_quad(t, para, para2);
                for(int i = 0; i < c; i ++) {
                    vec2 p;
                    eval_quad(p, para, t[i][0]);
                    float t1 = gs_max(gs_min(t[i][0], 1.f), 0.f);
                    float t2 = gs_max(gs_min(t[i][1], 1.f), 0.f);
                    if(!is_fuzzy_neighbour(p, p0) &&
                        !is_fuzzy_neighbour(p, p1) &&
                        !is_fuzzy_neighbour(p, sef->get_begin_point()) &&
                        !is_fuzzy_neighbour(p, sef->get_end_point())
                        )
                        ci.push_back(clip_intersection_info(p, e, t1, t2));
                }
                break;
            }
        case clip_src_edge::st_cubic:
            {
                vec2 cp2[] = { sef->get_begin_point(), sef->get_point(1), sef->get_point(2), sef->get_end_point() };
                vec4 para2[2];
                get_cubic_parameter_equation(para2, cp2[0], cp2[1], cp2[2], cp2[3]);
                vec2 p[6];
                int c = intersectp_cubic_quad(p, cp2, cp, 0.2f);
                for(int i = 0; i < c; i ++) {
                    float t1 = quad_reparameterize(para, p[i]);
                    float t2 = cubic_reparameterize(para2, p[i]);
                    t1 = gs_max(gs_min(t1, 1.f), 0.f);
                    t2 = gs_max(gs_min(t2, 1.f), 0.f);
                    if(!is_fuzzy_neighbour(p[i], p0) &&
                        !is_fuzzy_neighbour(p[i], p1) &&
                        !is_fuzzy_neighbour(p[i], sef->get_begin_point()) &&
                        !is_fuzzy_neighbour(p[i], sef->get_end_point())
                        )
                        ci.push_back(clip_intersection_info(p[i], e, t1, t2));
                }
                break;
            }
        }
    }
    setup_intersection_order(ci);
}

static void query_intersection_infos(clip_intersection_list& ci, const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3,clip_rtree& rt)
{
    rectf rc;
    get_cubic_bound_box(rc, p0, p1, p2, p3);
    clip_edge_ptrs pie;
    rt.query(rc, pie);
    if(pie.empty())
        return;
    vec4 para[2];
    get_cubic_parameter_equation(para, p0, p1, p2, p3);
    vec2 cp[] = { p0, p1, p2, p3 };
    for(clip_edge* e : pie) {
        assert(e);
        auto* sef = e->get_sef();
        if(!sef) {
            assert(!"unexpected.");
            continue;
        }
        switch(sef->get_type())
        {
        case clip_src_edge::st_line:
            {
                vec3 coef;
                get_linear_coefficient(coef, sef->get_begin_point(), vec2().sub(sef->get_end_point(), sef->get_begin_point()));
                float t[3];
                int c = intersection_cubic_linear(t, para, coef);
                for(int i = 0; i < c; i ++) {
                    float t1 = gs_max(gs_min(t[i], 1.f), 0.f);
                    vec2 p;
                    eval_cubic(p, para, t1);
                    float t2 = linear_reparameterize(sef->get_begin_point(), sef->get_end_point(), p);
                    t2 = tolerance_correct(t2);
                    if(!is_fuzzy_neighbour(p, p0) &&
                        !is_fuzzy_neighbour(p, p1) &&
                        !is_fuzzy_neighbour(p, sef->get_begin_point()) &&
                        !is_fuzzy_neighbour(p, sef->get_end_point())
                        )
                        ci.push_back(clip_intersection_info(p, e, t1, t2));
                }
                break;
            }
        case clip_src_edge::st_quad:
            {
                vec2 cp2[] = { sef->get_begin_point(), sef->get_point(1), sef->get_end_point() };
                vec3 para2[2];
                get_quad_parameter_equation(para2, cp2[0], cp2[1], cp2[2]);
                vec2 p[6];
                int c = intersectp_cubic_quad(p, cp, cp2, 0.2f);
                for(int i = 0; i < c; i ++) {
                    float t1 = cubic_reparameterize(para, p[i]);
                    float t2 = quad_reparameterize(para2, p[i]);
                    t1 = gs_max(gs_min(t1, 1.f), 0.f);
                    t2 = gs_max(gs_min(t2, 1.f), 0.f);
                    if(!is_fuzzy_neighbour(p[i], p0) &&
                        !is_fuzzy_neighbour(p[i], p1) &&
                        !is_fuzzy_neighbour(p[i], sef->get_begin_point()) &&
                        !is_fuzzy_neighbour(p[i], sef->get_end_point())
                        )
                        ci.push_back(clip_intersection_info(p[i], e, t1, t2));
                }
                break;
            }
        case clip_src_edge::st_cubic:
            {
                vec2 cp2[] = { sef->get_begin_point(), sef->get_point(1), sef->get_point(2), sef->get_end_point() };
                vec4 para2[2];
                get_cubic_parameter_equation(para2, cp2[0], cp2[1], cp2[2], cp2[3]);
                vec2 p[9];
                int c = intersectp_cubic_cubic(p, cp, cp2, 0.2f);
                for(int i = 0; i < c; i ++) {
                    float t1 = cubic_reparameterize(para, p[i]);
                    float t2 = cubic_reparameterize(para2, p[i]);
                    t1 = gs_max(gs_min(t1, 1.f), 0.f);
                    t2 = gs_max(gs_min(t2, 1.f), 0.f);
                    if(!is_fuzzy_neighbour(p[i], p0) &&
                        !is_fuzzy_neighbour(p[i], p1) &&
                        !is_fuzzy_neighbour(p[i], sef->get_begin_point()) &&
                        !is_fuzzy_neighbour(p[i], sef->get_end_point())
                        )
                        ci.push_back(clip_intersection_info(p[i], e, t1, t2));
                }
                break;
            }
        }
    }
    setup_intersection_order(ci);
}

class clip_proc_intersections
{
public:
    struct ep_map_key
    {
        clip_intersection_info* ep = nullptr;
        bool            forward = false;

    public:
        ep_map_key() {}
        ep_map_key(clip_intersection_info* p, bool f) { ep = p, forward = f; }
        bool operator==(const ep_map_key& that) const { return ep == that.ep && forward == that.forward; }
    };

    struct ep_map_hasher
    {
    public:
        size_t operator()(const ep_map_key& k) const { return hash_bytes((const byte*)&k, 5); }
    };

    struct ep_strip_node
    {
        clip_strip*     strip = nullptr;
        clip_intersection_info* ep = nullptr;

    public:
        ep_strip_node() {}
        ep_strip_node(clip_strip* s, clip_intersection_info* i) { strip = s, ep = i; }
    };

    typedef unordered_map<ep_map_key, clip_edge*, ep_map_hasher> ep_map;
    typedef unordered_multimap<clip_edge*, clip_intersection_info*> edge_ips_map;
    typedef map<int, ep_strip_node> ep_strip_map;

protected:
    clipper&            _clipper;

public:
    clip_proc_intersections(clipper& c): _clipper(c)
    {
    }
    int proc_line(clip_strip* strip, clip_intersection_list& cil, int eid, const vec2& p0, const vec2& p1)
    {
        assert(!cil.empty());
        ep_map subjects_epm, clip_epm;
        build_epm_subjects(subjects_epm, cil);
        auto* next = build_epm_clip_line(clip_epm, cil, strip, eid, p0, p1);
        assert(next);
        reorganize_strips(strip, cil, subjects_epm, clip_epm);
        int nextid = next->get_ids().front();
        return ++ nextid;
    }
    int proc_quad(clip_strip* strip, clip_intersection_list& cil, int eid, const vec2& p0, const vec2& p1, const vec2& p2)
    {
        assert(!cil.empty());
        ep_map subjects_epm, clip_epm;
        build_epm_subjects(subjects_epm, cil);
        auto* next = build_epm_clip_quad(clip_epm, cil, strip, eid, p0, p1, p2);
        assert(next);
        reorganize_strips(strip, cil, subjects_epm, clip_epm);
        int nextid = next->get_ids().front();
        return ++ nextid;
    }
    int proc_cubic(clip_strip* strip, clip_intersection_list& cil, int eid, const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3)
    {
        assert(!cil.empty());
        ep_map subjects_epm, clip_epm;
        build_epm_subjects(subjects_epm, cil);
        auto* next = build_epm_clip_cubic(clip_epm, cil, strip, eid, p0, p1, p2, p3);
        assert(next);
        reorganize_strips(strip, cil, subjects_epm, clip_epm);
        int nextid = next->get_ids().front();
        return ++ nextid;
    }
    clip_strip* add_coarse_strip()
    {
        clip_strip* strip = new clip_strip;
        _clipper._coarse_strips.insert(strip);
        return strip;
    }
    void build_epm_subjects(ep_map& epm, clip_intersection_list& cil)
    {
        assert(!cil.empty());
        edge_ips_map ipsm;
        for(clip_intersection_info& info : cil)
            ipsm.emplace(info.subject_edge, &info);
        auto i = ipsm.begin();
        while(i != ipsm.end()) {
            clip_edge* e = i->first;
            assert(e);
            _clipper._rtree.remove(e, e->get_sef()->get_bound_rect());  /* update rtree */
            auto j = ipsm.upper_bound(e);
            vector<clip_intersection_info*> iis;
            for(; i != j; ++ i)
                iis.push_back(i->second);
            std::sort(iis.begin(), iis.end(), [](clip_intersection_info* a, clip_intersection_info* b)->bool { return a->t_in_subject_edge < b->t_in_subject_edge; });
            auto* sef = e->get_sef();
            assert(sef);
            switch(sef->get_type())
            {
            case clip_src_edge::st_line:
                build_epm_line(epm, e, iis);
                break;
            case clip_src_edge::st_quad:
                build_epm_quad(epm, e, iis);
                break;
            case clip_src_edge::st_cubic:
                build_epm_cubic(epm, e, iis);
                break;
            }
        }
    }
    clip_edge* build_epm_clip_line(ep_map& epm, clip_intersection_list& cil, clip_strip* strip, int eid, const vec2& p0, const vec2& p1)
    {
        assert(!cil.empty() && strip);
        auto* end = strip->get_end();
        assert(end && end->get_dest() == p0);
        auto i = cil.begin();
        auto* e = _clipper.add_line_edge(p0, i->pt);
        assert(e);
        e->set_id(eid ++);
        strip->push_back(e);
        epm.emplace(ep_map_key(&*i, false), e);
        for(auto j = std::next(i); j != cil.end(); i = j ++) {
            e = _clipper.add_line_edge(i->pt, j->pt);
            assert(e);
            e->set_id(eid ++);
            strip->push_back(e);
            epm.emplace(ep_map_key(&*i, true), e);
            epm.emplace(ep_map_key(&*j, false), e);
        }
        e = _clipper.add_line_edge(i->pt, p1);
        assert(e);
        e->set_id(eid);
        strip->push_back(e);
        epm.emplace(ep_map_key(&*i, true), e);
        return e;
    }
    clip_edge* build_epm_clip_quad(ep_map& epm, clip_intersection_list& cil, clip_strip* strip, int eid, const vec2& p0, const vec2& p1, const vec2& p2)
    {
        assert(!cil.empty() && strip);
        auto* end = strip->get_end();
        assert(end && end->get_dest() == p0);
        auto i = cil.begin();
        vec2 ocp[3] = { p0, p1, p2 };
        vec2 scp[5];
        split_quad_bezier(scp, ocp, i->t_in_clip_edge);
        auto* e = _clipper.add_quad_edge(scp[0], scp[1], scp[2]);
        assert(e);
        e->set_id(eid ++);
        strip->push_back(e);
        epm.emplace(ep_map_key(&*i, false), e);
        memcpy(ocp, scp + 2, sizeof(ocp));
        for(auto j = std::next(i); j != cil.end(); i = j ++) {
            vec3 para[2];
            get_quad_parameter_equation(para, ocp[0], ocp[1], ocp[2]);
            float t = quad_reparameterize(para, j->pt);
            split_quad_bezier(scp, ocp, t);
            e = _clipper.add_quad_edge(scp[0], scp[1], scp[2]);
            assert(e);
            e->set_id(eid ++);
            strip->push_back(e);
            epm.emplace(ep_map_key(&*i, true), e);
            epm.emplace(ep_map_key(&*j, false), e);
            memcpy(ocp, scp + 2, sizeof(ocp));
        }
        e = _clipper.add_quad_edge(scp[2], scp[3], scp[4]);
        assert(e);
        e->set_id(eid);
        strip->push_back(e);
        epm.emplace(ep_map_key(&*i, true), e);
        return e;
    }
    clip_edge* build_epm_clip_cubic(ep_map& epm, clip_intersection_list& cil, clip_strip* strip, int eid, const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3)
    {
        assert(!cil.empty() && strip);
        auto* end = strip->get_end();
        assert(end && end->get_dest() == p0);
        auto i = cil.begin();
        vec2 ocp[4] = { p0, p1, p2, p3 };
        vec2 scp[7];
        split_cubic_bezier(scp, ocp, i->t_in_clip_edge);
        auto* e = _clipper.add_cubic_edge(scp[0], scp[1], scp[2], scp[3]);
        assert(e);
        e->set_id(eid ++);
        strip->push_back(e);
        epm.emplace(ep_map_key(&*i, false), e);
        memcpy(ocp, scp + 3, sizeof(ocp));
        for(auto j = std::next(i); j != cil.end(); i = j ++) {
            vec4 para[2];
            get_cubic_parameter_equation(para, ocp[0], ocp[1], ocp[2], ocp[3]);
            float t = cubic_reparameterize(para, j->pt);
            split_cubic_bezier(scp, ocp, t);
            e = _clipper.add_cubic_edge(scp[0], scp[1], scp[2], scp[3]);
            assert(e);
            e->set_id(eid ++);
            strip->push_back(e);
            epm.emplace(ep_map_key(&*i, true), e);
            epm.emplace(ep_map_key(&*j, false), e);
            memcpy(ocp, scp + 3, sizeof(ocp));
        }
        e = _clipper.add_cubic_edge(scp[3], scp[4], scp[5], scp[6]);
        assert(e);
        e->set_id(eid);
        strip->push_back(e);
        epm.emplace(ep_map_key(&*i, true), e);
        return e;
    }
    void reorganize_strips(clip_strip* strip, clip_intersection_list& cil, ep_map& subject_epm, ep_map& clip_epm)
    {
        assert(strip);
        if(cil.empty())
            return;
        auto i = cil.rbegin();
        assert(i != cil.rend());
        auto* first_strip = add_coarse_strip();
        assert(first_strip);
        split_strip(strip, first_strip, &*i, subject_epm, clip_epm);
        if(cil.size() == 1) {
            first_strip->close_finish();
            return;
        }
        ep_strip_map esm;
        esm.emplace(i->serial_id, ep_strip_node(first_strip, &*i));
        reorganize_strips(first_strip, esm, cil, std::next(i), subject_epm, clip_epm);
    }
    void reorganize_strips(clip_strip* last_strip, ep_strip_map& esm, clip_intersection_list& cil, const clip_intersection_list::reverse_iterator& i, ep_map& epm1, ep_map& epm2)
    {
        auto j = std::next(i);
        if(j == cil.rend()) {    /* close all */
            if(esm.size() == 1) {
                assert(last_strip == esm.begin()->second.strip);
                auto* next_strip = add_coarse_strip();
                assert(next_strip);
                split_strip(last_strip, next_strip, &*i, epm1, epm2);
                next_strip->close_finish();
            }
            else {
                assert(esm.size() == 2);
                clip_strip* other_strip = nullptr;
                auto* strip1 = esm.begin()->second.strip;
                if(strip1 == last_strip)
                    other_strip = (++esm.begin())->second.strip;
                else {
                    assert(last_strip == (++esm.begin())->second.strip);
                    other_strip = strip1;
                }
                assert(other_strip);
                split_strip(last_strip, other_strip, &*i, epm1, epm2);
                other_strip->close_finish();
            }
            return;
        }
        auto* next_strip = add_coarse_strip();
        assert(next_strip);
        split_strip(last_strip, next_strip, &*j, epm1, epm2);
        auto f = esm.find(j->serial_id - 1);
        if(f != esm.end()) {
            f->second.strip->close_finish();
            esm.erase(f);
        }
        esm.emplace(j->serial_id, ep_strip_node(next_strip, &*j));
        reorganize_strips(next_strip, esm, cil, j, epm2, epm1);
    }
    static void split_strip(clip_strip* strip1, clip_strip* strip2, clip_intersection_info* info, ep_map& epm1, ep_map& epm2)
    {
        assert(strip1 && strip2 && info);
        auto* closing_tail = epm2.find(ep_map_key(info, true))->second;
        auto* closing_front = epm1.find(ep_map_key(info, false))->second;
        auto* opening_front = epm2.find(ep_map_key(info, false))->second;
        auto* opening_tail = epm1.find(ep_map_key(info, true))->second;

        closing_front->tracing();
        closing_tail->tracing();
        opening_front->tracing();
        opening_tail->tracing();

        connect_edge(closing_front, closing_tail);
        connect_edge(opening_front, opening_tail);
        strip1->set_end(closing_tail);
        strip2->set_begin(opening_front);
        strip2->set_end(opening_tail);
        opening_front->set_loop(strip2);
    }
    static void replace_head(clip_edge* o, clip_edge* n)
    {
        assert(o && n);
        auto* head = o->get_prev();
        if(head) {
            disconnect_edge(head, o);
            connect_edge(head, n);
        }
        else if(auto* lp = o->get_loop()) {
            assert(lp->get_begin() == o);
            lp->set_begin(n);
            o->set_loop(nullptr);
            n->set_loop(lp);
        }
    }
    static void replace_tail(clip_edge* o, clip_edge* n)
    {
        assert(o && n);
        auto* tail = o->get_next();
        if(tail) {
            disconnect_edge(o, tail);
            connect_edge(n, tail);
        }
        else {
            auto* lp = o->get_loop();
            if(lp && lp->is_strip()) {
                auto* strip = static_cast<clip_strip*>(lp);
                assert(strip->get_end() == o);
                strip->set_end(n);
            }
        }
    }
    /* todo: add_edge null? */
    void build_epm_line(ep_map& epm, clip_edge* e, const vector<clip_intersection_info*>& iis)
    {
        assert(e && !iis.empty());
        auto* sef = e->get_sef();
        assert(sef && sef->get_type() == clip_src_edge::st_line);
        auto i = iis.begin();
        auto* se = _clipper.add_line_edge(sef->get_begin_point(), (*i)->pt);
        assert(se);
        se->set_id(e->get_ids(), 0);
        replace_head(e, se);
        epm.emplace(ep_map_key(*i, false), se);
        auto* laste = se;
        int eid = 1;
        for(auto j = std::next(i); j != iis.end(); i = j ++) {
            se = _clipper.add_line_edge((*i)->pt, (*j)->pt);
            assert(se);
            se->set_id(e->get_ids(), eid ++);
            connect_edge(laste, se);
            epm.emplace(ep_map_key(*i, true), se);
            epm.emplace(ep_map_key(*j, false), se);
            laste = se;
        }
        se = _clipper.add_line_edge((*i)->pt, sef->get_end_point());
        assert(se);
        se->set_id(e->get_ids(), eid);
        connect_edge(laste, se);
        epm.emplace(ep_map_key(*i, true), se);
        replace_tail(e, se);
    }
    void build_epm_quad(ep_map& epm, clip_edge* e, const vector<clip_intersection_info*>& iis)
    {
        /* todo: tolerance for intersection to be appeared in the end point of the subject path segment */
        assert(e && !iis.empty());
        auto* sef = e->get_sef();
        assert(sef && sef->get_type() == clip_src_edge::st_quad);
        auto i = iis.begin();
        vec2 ocp[3] = { sef->get_begin_point(), sef->get_point(1), sef->get_end_point() };
        vec2 scp[5];
        split_quad_bezier(scp, ocp, (*i)->t_in_subject_edge);
        auto* se = _clipper.add_quad_edge(scp[0], scp[1], scp[2]);
        assert(se);
        se->set_id(e->get_ids(), 0);
        replace_head(e, se);
        epm.emplace(ep_map_key(*i, false), se);
        memcpy(ocp, scp + 2, sizeof(ocp));
        auto* laste = se;
        int eid = 1;
        for(auto j = std::next(i); j != iis.end(); i = j ++) {
            vec3 para[2];
            get_quad_parameter_equation(para, ocp[0], ocp[1], ocp[2]);
            float t = quad_reparameterize(para, (*j)->pt);
            split_quad_bezier(scp, ocp, t);
            se = _clipper.add_quad_edge(scp[0], scp[1], scp[2]);
            assert(se);
            se->set_id(e->get_ids(), eid ++);
            connect_edge(laste, se);
            epm.emplace(ep_map_key(*i, true), se);
            epm.emplace(ep_map_key(*j, false), se);
            memcpy(ocp, scp + 2, sizeof(ocp));
            laste = se;
        }
        se = _clipper.add_quad_edge(scp[2], scp[3], scp[4]);
        assert(se);
        se->set_id(e->get_ids(), eid);
        connect_edge(laste, se);
        epm.emplace(ep_map_key(*i, true), se);
        replace_tail(e, se);
    }
    void build_epm_cubic(ep_map& epm, clip_edge* e, const vector<clip_intersection_info*>& iis)
    {
        /* todo: tolerance for intersection to be appeared in the end point of the subject path segment */
        assert(e && !iis.empty());
        auto* sef = e->get_sef();
        assert(sef && sef->get_type() == clip_src_edge::st_cubic);
        auto i = iis.begin();
        vec2 ocp[4] = { sef->get_begin_point(), sef->get_point(1), sef->get_point(2), sef->get_end_point() };
        vec2 scp[7];
        split_cubic_bezier(scp, ocp, (*i)->t_in_subject_edge);
        auto* se = _clipper.add_cubic_edge(scp[0], scp[1], scp[2], scp[3]);
        assert(se);
        se->set_id(e->get_ids(), 0);
        replace_head(e, se);
        epm.emplace(ep_map_key(*i, false), se);
        memcpy(ocp, scp + 3, sizeof(ocp));
        auto* laste = se;
        int eid = 1;
        for(auto j = std::next(i); j != iis.end(); i = j ++) {
            vec4 para[2];
            get_cubic_parameter_equation(para, ocp[0], ocp[1], ocp[2], ocp[3]);
            float t = cubic_reparameterize(para, (*j)->pt);
            split_cubic_bezier(scp, ocp, t);
            se = _clipper.add_cubic_edge(scp[0], scp[1], scp[2], scp[3]);
            assert(se);
            se->set_id(e->get_ids(), eid ++);
            connect_edge(laste, se);
            epm.emplace(ep_map_key(*i, true), se);
            epm.emplace(ep_map_key(*j, false), se);
            memcpy(ocp, scp + 3, sizeof(ocp));
            laste = se;
        }
        se = _clipper.add_cubic_edge(scp[3], scp[4], scp[5], scp[6]);
        assert(se);
        se->set_id(e->get_ids(), eid);
        connect_edge(laste, se);
        epm.emplace(ep_map_key(*i, true), se);
        replace_tail(e, se);
    }
    static clip_edge* find_subject_edge_positive(ep_map& epm, clip_intersection_info* info)
    {
        assert(info);
        auto f = epm.find(ep_map_key(info, true));
        return f != epm.end() ? f->second : nullptr;
    }
    static clip_edge* find_subject_edge_negative(ep_map& epm, clip_intersection_info* info)
    {
        assert(info);
        auto f = epm.find(ep_map_key(info, false));
        return f != epm.end() ? f->second : nullptr;
    }
};

clip_src_edge_line::clip_src_edge_line(const vec2& p0, const vec2& p1)
{
    _pt[0] = p0;
    _pt[1] = p1;
    _bound.set_by_pts(p0, p1);
}

const vec2& clip_src_edge_line::get_point(int i) const
{
    assert(i >= 0 && i <= 1);
    return _pt[i];
}

const vec2& clip_src_edge_line::get_point_rev(int i) const
{
    assert(i >= 0 && i <= 1);
    return _pt[1 - i];
}

void clip_src_edge_line::tracing() const
{
    trace(_t("@moveTo %f, %f;\n"), _pt[0].x, _pt[0].y);
    trace(_t("@lineTo %f, %f;\n"), _pt[1].x, _pt[1].y);
}

clip_src_edge_quad::clip_src_edge_quad(const vec2& p0, const vec2& p1, const vec2& p2)
{
    _pt[0] = p0;
    _pt[1] = p1;
    _pt[2] = p2;
    get_quad_bound_box(_bound, p0, p1, p2);
}

const vec2& clip_src_edge_quad::get_point(int i) const
{
    assert(i >= 0 && i <= 2);
    return _pt[i];
}

const vec2& clip_src_edge_quad::get_point_rev(int i) const
{
    assert(i >= 0 && i <= 2);
    return _pt[2 - i];
}

void clip_src_edge_quad::tracing() const
{
    trace(_t("@moveTo %f, %f;\n"), _pt[0].x, _pt[0].y);
    trace(_t("@quadTo %f, %f, %f, %f;\n"), _pt[1].x, _pt[1].y, _pt[2].x, _pt[2].y);
}

clip_src_edge_cubic::clip_src_edge_cubic(const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3)
{
    _pt[0] = p0;
    _pt[1] = p1;
    _pt[2] = p2;
    _pt[3] = p3;
    get_cubic_bound_box(_bound, p0, p1, p2, p3);
}

const vec2& clip_src_edge_cubic::get_point(int i) const
{
    assert(i >= 0 && i <= 3);
    return _pt[i];
}

const vec2& clip_src_edge_cubic::get_point_rev(int i) const
{
    assert(i >= 0 && i <= 3);
    return _pt[3 - i];
}

void clip_src_edge_cubic::tracing() const
{
    trace(_t("@moveTo %f, %f;\n"), _pt[0].x, _pt[0].y);
    trace(_t("@cubicTo %f, %f, %f, %f, %f, %f;\n"), _pt[1].x, _pt[1].y, _pt[2].x, _pt[2].y, _pt[3].x, _pt[3].y);
}

clip_edge::~clip_edge()
{
    if(_sef) {
        delete _sef;
        _sef = nullptr;
    }
}

void clip_edge::set_src_edge(clip_src_edge* e, bool rev)
{
    assert(e);
    if(_sef)
        delete _sef;
    _sef = rev ? new clip_src_edge_fetcher_rev(e) : new clip_src_edge_fetcher(e);
}

void clip_edge::setup_src_edge(clip_src_edge* src)
{
    set_src_edge(src, false);
    assert(_symmetric);
    _symmetric->set_src_edge(src, true);
}

void clip_edge::set_id(int id)
{
    assert(_ids.empty());
    _ids.push_back(id);
}

void clip_edge::set_id(const vector<int>& ids, int id)
{
    _ids = ids;
    _ids.push_back(id);
}

clip_loop* clip_edge::find_host_loop() const
{
    if(!_prev)
        return nullptr;
    if(_loop)
        return _loop;
    for(auto* last = _prev; last && last != this; last = last->get_prev()) {
        if(auto* loop = last->get_loop()) {
            assert(loop->get_begin() == last);
            return loop;
        }
    }
    return nullptr;
}

void clip_loop::fix_children()
{
    for_each([this](clip_edge* e) { e->set_loop(this); });
}

void clip_loop::fix_edge_id()
{
    int id = 0;
    for_each([&id](clip_edge* e) { e->set_id(id ++); });
}

void clip_loop::tracing() const
{
    if(!_begin)
        return;
    trace(_t("@!\n"));
    for_each([](const clip_edge* e) { e->tracing(); });
    trace(_t("@@\n"));
}

void clip_strip::fix_children()
{
    for_each([this](clip_edge* e) { e->set_loop(this); });
}

void clip_strip::fix_edge_id()
{
    int id = 0;
    for_each([&id](clip_edge* e) { e->set_id(id ++); });
}

void clip_strip::tracing() const
{
    if(!_begin)
        return;
    trace(_t("@!\n"));
    for_each([](const clip_edge* e) { e->tracing(); });
    trace(_t("@@\n"));
}

void clip_strip::fix_end(clip_edge* e)
{
    assert(e);
    if(_begin != _end)
        set_end(e);
    else {
        set_begin(e);
        set_end(e);
    }
    e->set_loop(nullptr);
}

void clip_strip::init(clip_edge* e)
{
    assert(!_begin && !_end && e);
    _begin = _end = e;
    e->set_loop(this);
}

void clip_strip::setup(clip_edge* e)
{
    assert(!_begin && !_end && e);
    _begin = _end = e;
    for(; _begin->get_prev(); _begin = _begin->get_prev());
    for(; _end->get_next(); _end = _end->get_next());
    _begin->set_loop(this);
}

void clip_strip::push_front(clip_edge* e)
{
    assert(e);
    if(!_begin)
        return init(e);
    e->set_loop(this);
    _begin->set_loop(nullptr);
    connect_edge(e, _begin);
    _begin = e;
}

void clip_strip::push_back(clip_edge* e)
{
    assert(e);
    if(!_begin)
        return init(e);
    connect_edge(_end, e);
    _end = e;
}

bool clip_strip::try_finish()
{
    if(_begin && _end && (_begin->get_org() == _end->get_dest())) {
        finish();
        return true;
    }
    return false;
}

void clip_strip::finish()
{
    assert(_begin && _end);
    connect_edge(_end, _begin);
    set_complete(true);
}

void clip_strip::close_finish()
{
    set_end(get_begin()->get_prev());
    set_complete(true);
}

void clipper::reset()
{
    _edge_holdings.clear();
    _rtree.destroy();
    for(auto* e : _src_edge_holdings)
        delete e;
    _src_edge_holdings.clear();
    for(auto* s : _coarse_strips)
        delete s;
    _coarse_strips.clear();
}

void clipper::add_path(const painter_path& path)
{
    if(path.empty())
        return;
    int size = path.size();
    if(path.get_node(0)->get_tag() != painter_path::pt_moveto) {
        set_error(_t("bad path."));
        return;
    }
    for(int i = 0; i < size; ) {
        int next = clip_coarse(path, i);
        if(next < 0) {
            set_error(_t("unexpected error."));
            return;
        }
        i = next;
    }
}

void clipper::tracing() const
{
    trace(_t("@!\n"));
    for(const clip_strip* s : _coarse_strips) {
        //assert(s && s->is_complete());
        s->tracing();
    }
    trace(_t("@@\n"));
}

/* This step clips an arbitrary path to several loops, but can't guarantee these loops were not intersected.
 * from $path to _coarse_strips
 */
int clipper::clip_coarse(const painter_path& path, int start)
{
    int size = path.size();
    if(path.empty() || start >= size)
        return -1;
    if(path.get_node(start)->get_tag() != painter_path::pt_moveto) {
        set_error(_t("The path start should be \'moveTo\' node."));
        return -1;
    }
    _rtree.destroy();
    int eid = 0;
    const painter_node* last_node = path.get_node(start);
    assert(last_node);
    clip_strip* strip = new clip_strip;
    assert(strip);
    _coarse_strips.insert(strip);
    int i = start + 1;
    for( ; i < size; i ++) {
        const painter_node* node = path.get_node(i);
        assert(node);
        if(node->get_tag() == painter_path::pt_moveto)
            break;
        switch(node->get_tag())
        {
        case painter_path::pt_lineto:
            eid = proc_line_edge(strip, eid, last_node->get_point(), node);
            break;
        case painter_path::pt_quadto:
            eid = proc_quad_edge(strip, eid, last_node->get_point(), static_cast<const painter_path::quad_to_node*>(node));
            break;
        case painter_path::pt_cubicto:
            eid = proc_cubic_edge(strip, eid, last_node->get_point(), static_cast<const painter_path::cubic_to_node*>(node));
            break;
        }
        last_node = node;

        tracing();
    }
    strip->try_finish();
    return i;
}

int clipper::proc_line_edge(clip_strip* strip, int eid, const vec2& last_pt, const painter_node* node)
{
    assert(strip && node);
    if(last_pt == node->get_point())
        return eid;
    clip_intersection_list cil;
    query_intersection_infos(cil, last_pt, node->get_point(), _rtree);
    if(cil.empty()) {
        auto* edge = add_line_edge(last_pt, node->get_point());
        assert(edge);
        strip->push_back(edge);
        edge->set_id(eid);
        return ++ eid;
    }
    clip_proc_intersections cpi(*this);
    return cpi.proc_line(strip, cil, eid, last_pt, node->get_point());
}

int clipper::proc_quad_edge(clip_strip* strip, int eid, const vec2& last_pt, const painter_path::quad_to_node* node)
{
    assert(strip && node);
    clip_intersection_list cil;
    query_intersection_infos(cil, last_pt, node->get_control(), node->get_point(), _rtree);
    if(cil.empty()) {
        auto* edge = add_quad_edge(last_pt, node->get_control(), node->get_point());
        assert(edge);
        strip->push_back(edge);
        edge->set_id(eid);
        return ++ eid;
    }
    clip_proc_intersections cpi(*this);
    return cpi.proc_quad(strip, cil, eid, last_pt, node->get_control(), node->get_point());
}

int clipper::proc_cubic_edge(clip_strip* strip, int eid, const vec2& last_pt, const painter_path::cubic_to_node* node)
{
    assert(strip && node);
    vec2 cp[4] = { last_pt, node->get_control1(), node->get_control2(), node->get_point() };
    float t[2];
    if(get_self_intersection(t, cp[0], cp[1], cp[2], cp[3])) {
        vec2 scp[10];
        split_cubic_bezier(scp, cp, t[0], t[1]);
        /* 3, 4, 5, 6 should be a loop */
        clip_strip* sloop = new clip_strip;
        assert(sloop);
        _coarse_strips.insert(sloop);
        auto* edge = add_cubic_edge(scp[3], scp[4], scp[5], scp[6]);    // todo
        assert(edge);
        sloop->push_back(edge);
        sloop->try_finish();
        scp[6] = scp[3];    /* correction */
        /* rest */
        eid = proc_cubic_edge_nosi(strip, eid, scp[0], scp[1], scp[2], scp[3]);
        return proc_cubic_edge_nosi(strip, eid, scp[6], scp[7], scp[8], scp[9]);
    }
    return proc_cubic_edge_nosi(strip, eid, cp[0], cp[1], cp[2], cp[3]);
}

int clipper::proc_cubic_edge_nosi(clip_strip* strip, int eid, const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3)
{
    assert(strip);
    clip_intersection_list cil;
    query_intersection_infos(cil, p0, p1, p2, p3, _rtree);
    if(cil.empty()) {
        auto* edge = add_cubic_edge(p0, p1, p2, p3);
        assert(edge);
        strip->push_back(edge);
        edge->set_id(eid);
        return ++ eid;
    }
    clip_proc_intersections cpi(*this);
    return cpi.proc_cubic(strip, cil, eid, p0, p1, p2, p3);
}

clip_edge* clipper::add_edge(const vec2& last_pt, const vec2& pt)
{
    assert(last_pt && pt);
    _edge_holdings.push_back(clip_edge(nullptr));
    clip_edge* e0 = &_edge_holdings.back();
    _edge_holdings.push_back(clip_edge(nullptr));
    clip_edge* e1 = &_edge_holdings.back();
    assert(e0 && e1);
    e0->set_symmetric(e1);
    e1->set_symmetric(e0);
    e0->set_org(last_pt);
    e1->set_org(pt);
    return e0;
}

clip_edge* clipper::add_line_edge(const vec2& p0, const vec2& p1)
{
    /* if(p0 == p1)
        return nullptr; */
    auto* edge = add_edge(p0, p1);
    assert(edge);
    auto* src_edge = create_src_edge_line(_src_edge_holdings, p0, p1);
    assert(src_edge);
    edge->setup_src_edge(src_edge);
    _rtree.insert_line(edge, p0, p1);
    return edge;
}

clip_edge* clipper::add_quad_edge(const vec2& p0, const vec2& p1, const vec2& p2)
{
    if(is_approx_line(p0, p1, p2, 0.5f))
        return add_line_edge(p0, p2);
    auto* edge = add_edge(p0, p2);
    assert(edge);
    auto* src_edge = create_src_edge_quad(_src_edge_holdings, p0, p1, p2);
    assert(src_edge);
    edge->setup_src_edge(src_edge);
    _rtree.insert(edge, src_edge->get_bound_rect());
    return edge;
}

clip_edge* clipper::add_cubic_edge(const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3)
{
    if(is_approx_line(p0, p1, p2, 0.5f) && is_approx_line(p1, p2, p3, 0.5f))
        return add_line_edge(p0, p3);
    auto* edge = add_edge(p0, p3);
    assert(edge);
    auto* src_edge = create_src_edge_cubic(_src_edge_holdings, p0, p1, p2, p3);
    assert(src_edge);
    edge->setup_src_edge(src_edge);
    _rtree.insert(edge, src_edge->get_bound_rect());
    return edge;
}

__ariel_end__
