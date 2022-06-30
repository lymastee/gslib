#include "basis.h"
#include <gslib/utility.h>

__ariel_begin__

static const float cv_dot_radius = 3.f;
static const float cv_surround_radius = 6.f;
static const float cv_dot_overlap_range = 2.f;
static const float cv_tangent_half_len = 150.f;

bool cv_curve_node::add_point(const vec2& p)
{
    int next = _curr + 1;
    if(next >= get_point_count())
        return false;
    if(is_overlapping(p))
        return true;
    set_point(next, p);
    _curr = next;
    return true;
}

bool cv_curve_node::is_overlapping(const vec2& p, int except) const
{
    int size = gs_min(_curr, get_point_count());
    for(int i = 0; i < size; i ++) {
        if(i == except)
            continue;
        float d = vec2().sub(get_point(i), p).length();
        if(d < cv_dot_overlap_range)
            return true;
    }
    return false;
}

void cv_set_pen_color(painter* paint, const color& cr)
{
    assert(paint);
    painter_pen pen(painter_pen::solid, cr);
    paint->set_pen(pen);
}

void cv_set_brush_color(painter* paint, const color& cr)
{
    assert(paint);
    painter_brush brush(painter_brush::solid, cr);
    paint->set_brush(brush);
}

static void cv_create_dot_path(painter_path& path, const vec2& c)
{
    static const float r = cv_dot_radius;
    static const float cos60 = cosf(PI / 3.f) * r, sin60 = sinf(PI / 3.f) * r;
    float x = c.x, y = c.y;
    path.move_to(x - r, y);
    path.line_to(x - cos60, y + sin60);
    path.line_to(x + cos60, y + sin60);
    path.line_to(x + r, y);
    path.line_to(x + cos60, y - sin60);
    path.line_to(x - cos60, y - sin60);
    path.line_to(x - r, y);
}

void cv_draw_dot(painter* paint, const vec2& p)
{
    assert(paint);
    painter_path path;
    cv_create_dot_path(path, p);
    paint->draw_path(path);
}

void cv_draw_dot(painter* paint, const vec2& p, const color& cr)
{
    assert(paint);
    painter_path path;
    cv_create_dot_path(path, p);
    cv_set_brush_color(paint, cr);
    cv_set_no_pen(paint);
    paint->draw_path(path);
}

void cv_surround_dot(painter* paint, const vec2& p, const color& cr)
{
    assert(paint);
    cv_set_no_brush(paint);
    cv_set_pen_color(paint, cr);
    static const float r = cv_surround_radius;
    float x = p.x, y = p.y;
    painter_path path;
    path.move_to(x - r, y);
    path.quad_to(x - r, y + r, x, y + r);
    path.quad_to(x + r, y + r, x + r, y);
    path.quad_to(x + r, y - r, x, y - r);
    path.quad_to(x - r, y - r, x - r, y);
    paint->draw_path(path);
}

void cv_draw_inflections(painter* paint, const vec2 cp[4])
{
    assert(paint);
    vec4 para[2];
    get_cubic_parameter_equation(para, cp[0], cp[1], cp[2], cp[3]);
    vec3 ff[2];
    vec2 sf[2];
    get_first_derivate_factor(ff, para);
    get_second_derivate_factor(sf, para);
    float t[2];
    int c = get_cubic_inflection(t, ff, sf);
    for(int i = 0; i < c; i ++) {
        vec2 p;
        eval_cubic(p, para, t[i]);
        cv_draw_dot(paint, p, cv_inflection_point_cr);
        // draw tangent
        vec3 tds(t[i] * t[i], t[i], 1.f);
        vec2 tv;
        tv.x = tds.dot(ff[0]);
        tv.y = tds.dot(ff[1]);
        tv.normalize();
        tv.scale(cv_tangent_half_len);
        vec2 p1, p2;
        p1 = p + tv;
        p2 = p - tv;
        cv_set_pen_color(paint, cv_tangent_line_cr);
        cv_set_no_brush(paint);
        paint->draw_line(p1, p2);
    }
}

void cv_draw_quad_extrema(painter* paint, const vec2 cp[3])
{
    assert(paint);
    // draw bound box
    rectf rc;
    get_quad_bound_box(rc, cp[0], cp[1], cp[2]);
    cv_set_no_brush(paint);
    cv_set_pen_color(paint, cv_bound_box_cr);
    paint->draw_rect(rc);
    // draw extrema
    vec3 para[2];
    get_quad_parameter_equation(para, cp[0], cp[1], cp[2]);
    vec2 ff[2];
    get_first_derivate_factor(ff, para);
    float tx, ty;
    int c1 = get_quad_extrema(&tx, ff[0]);
    int c2 = get_quad_extrema(&ty, ff[1]);
    if(c1) {
        vec2 p;
        eval_quad(p, para, tx);
        cv_draw_dot(paint, p, cv_extrema_point_cr);
    }
    if(c2) {
        vec2 p;
        eval_quad(p, para, ty);
        cv_draw_dot(paint, p, cv_extrema_point_cr);
    }
}

void cv_draw_cubic_extrema(painter* paint, const vec2 cp[4])
{
    assert(paint);
    // draw bound box
    rectf rc;
    get_cubic_bound_box(rc, cp[0], cp[1], cp[2], cp[3]);
    cv_set_no_brush(paint);
    cv_set_pen_color(paint, cv_bound_box_cr);
    paint->draw_rect(rc);
    // draw extrema
    vec4 para[2];
    get_cubic_parameter_equation(para, cp[0], cp[1], cp[2], cp[3]);
    vec3 ff[2];
    get_first_derivate_factor(ff, para);
    float t[4];
    int c = get_cubic_extrema(t, ff[0]);
    c += get_cubic_extrema(t + c, ff[1]);
    for(int i = 0; i < c; i ++) {
        vec2 p;
        eval_cubic(p, para, t[i]);
        cv_draw_dot(paint, p, cv_extrema_point_cr);
    }
}

void cv_draw_self_intersection(painter* paint, const vec2 cp[4])
{
    assert(paint);
    float ts[2];
    if(!get_self_intersection(ts, cp[0], cp[1], cp[2], cp[3]))
        return;
    vec4 para[2];
    get_cubic_parameter_equation(para, cp[0], cp[1], cp[2], cp[3]);
    vec2 p;
    eval_cubic(p, para, ts[0]);
    cv_draw_dot(paint, p, cv_intersect_point_cr);
}

void cv_draw_cubic_to_quad(painter* paint, const vec2 cp[4])
{
    assert(paint);
    vector<vec2> quads;
    cubic_to_quad_bezier(quads, cp, 2.f);  // change this tolerance to see difference results
    if(quads.empty())
        return;
    paint->save();
    // draw control path
    painter_path ctls;
    ctls.move_to(quads.front());
    for(size_t i = 1; i < quads.size(); i ++)
        ctls.line_to(quads.at(i));
    cv_set_pen_color(paint, cv_quad_control_path_cr);
    cv_set_no_brush(paint);
    paint->draw_path(ctls);
    // draw path
    painter_path path;
    path.move_to(quads.front());
    for(size_t i = 1; i + 1 < quads.size(); i += 2)
        path.quad_to(quads.at(i), quads.at(i + 1));
    cv_set_pen_color(paint, cv_cubic_to_quad_cr);
    cv_set_no_brush(paint);
    paint->draw_path(path);
    // draw dots;
    cv_set_no_pen(paint);
    cv_set_brush_color(paint, cv_control_point_cr);
    for(const vec2& p : quads)
        cv_draw_dot(paint, p);
    paint->restore();
}

static void collect_intersections(vector<vec2>& pts, const cv_line_node* node1, const cv_line_node* node2)
{
    const vec2& p1 = node1->get_point(0);
    const vec2& p2 = node1->get_point(1);
    const vec2& q1 = node2->get_point(0);
    const vec2& q2 = node2->get_point(1);
    vec2 d1, d2;
    d1.sub(p2, p1);
    d2.sub(q2, q1);
    if(is_parallel(d1, d2))
        return;
    vec2 p;
    intersectp_linear_linear(p, p1, q1, d1, d2);
    pts.push_back(p);
}

static void collect_intersections(vector<vec2>& pts, const cv_line_node* node1, const cv_quad_node* node2)
{
    const vec2& p1 = node1->get_point(0);
    const vec2& p2 = node1->get_point(1);
    const vec2& q1 = node2->get_point(0);
    const vec2& q2 = node2->get_point(1);
    const vec2& q3 = node2->get_point(2);
    vec3 linear;
    get_linear_coefficient(linear, p1, vec2().sub(p2, p1));
    vec3 quad[2];
    get_quad_parameter_equation(quad, q1, q2, q3);
    float t[2];
    int c = intersection_quad_linear(t, quad, linear);
    for(int i = 0; i < c; i ++) {
        vec2 p;
        eval_quad(p, quad, t[i]);
        pts.push_back(p);
    }
}

static void collect_intersections(vector<vec2>& pts, const cv_line_node* node1, const cv_cubic_node* node2)
{
    const vec2& p1 = node1->get_point(0);
    const vec2& p2 = node1->get_point(1);
    const vec2& q1 = node2->get_point(0);
    const vec2& q2 = node2->get_point(1);
    const vec2& q3 = node2->get_point(2);
    const vec2& q4 = node2->get_point(3);
    vec3 linear;
    get_linear_coefficient(linear, p1, vec2().sub(p2, p1));
    vec4 cubic[2];
    get_cubic_parameter_equation(cubic, q1, q2, q3, q4);
    float t[3];
    int c = intersection_cubic_linear(t, cubic, linear);
    for(int i = 0; i < c; i ++) {
        vec2 p;
        eval_cubic(p, cubic, t[i]);
        pts.push_back(p);
    }
}

static void collect_intersections(vector<vec2>& pts, const cv_quad_node* node1, const cv_quad_node* node2)
{
    const vec2& p1 = node1->get_point(0);
    const vec2& p2 = node1->get_point(1);
    const vec2& p3 = node1->get_point(2);
    const vec2& q1 = node2->get_point(0);
    const vec2& q2 = node2->get_point(1);
    const vec2& q3 = node2->get_point(2);
    vec3 quad1[2], quad2[2];
    get_quad_parameter_equation(quad1, p1, p2, p3);
    get_quad_parameter_equation(quad2, q1, q2, q3);
    float ts[4][2];
    int c = intersection_quad_quad(ts, quad1, quad2);
    for(int i = 0; i < c; i ++) {
        vec2 p;
        eval_quad(p, quad1, ts[i][0]);
        pts.push_back(p);
    }
}

static void collect_intersections(vector<vec2>& pts, const cv_quad_node* node1, const cv_cubic_node* node2)
{
    vec2 p[3] = {
        node1->get_point(0),
        node1->get_point(1),
        node1->get_point(2)
    };
    vec2 q[4] = {
        node2->get_point(0),
        node2->get_point(1),
        node2->get_point(2),
        node2->get_point(3)
    };
    vec2 ip[6];
    int c = intersectp_cubic_quad(ip, q, p, 0.3f);
    for(int i = 0; i < c; i ++)
        pts.push_back(ip[i]);
}

static void collect_intersections(vector<vec2>& pts, const cv_cubic_node* node1, const cv_cubic_node* node2)
{
    vec2 p[4] = {
        node1->get_point(0),
        node1->get_point(1),
        node1->get_point(2),
        node1->get_point(3)
    };
    vec2 q[4] = {
        node2->get_point(0),
        node2->get_point(1),
        node2->get_point(2),
        node2->get_point(3)
    };
    vec2 ip[9];
    int c = intersectp_cubic_cubic(ip, p, q, 0.3f);
    for(int i = 0; i < c; i ++)
        pts.push_back(ip[i]);
}

void cv_draw_intersections(painter* paint, const cv_curve_node* node1, const cv_curve_node* node2)
{
    assert(paint && node1 && node2);
    vector<vec2> pts;
    if(node1->get_type() == cvt_line) {
        if(node2->get_type() == cvt_line)
            collect_intersections(pts, static_cast<const cv_line_node*>(node1), static_cast<const cv_line_node*>(node2));
        else if(node2->get_type() == cvt_quad)
            collect_intersections(pts, static_cast<const cv_line_node*>(node1), static_cast<const cv_quad_node*>(node2));
        else if(node2->get_type() == cvt_cubic)
            collect_intersections(pts, static_cast<const cv_line_node*>(node1), static_cast<const cv_cubic_node*>(node2));
    }
    else if(node1->get_type() == cvt_quad) {
        if(node2->get_type() == cvt_line)
            collect_intersections(pts, static_cast<const cv_line_node*>(node2), static_cast<const cv_quad_node*>(node1));
        else if(node2->get_type() == cvt_quad)
            collect_intersections(pts, static_cast<const cv_quad_node*>(node1), static_cast<const cv_quad_node*>(node2));
        else if(node2->get_type() == cvt_cubic)
            collect_intersections(pts, static_cast<const cv_quad_node*>(node1), static_cast<const cv_cubic_node*>(node2));
    }
    else if(node1->get_type() == cvt_cubic) {
        if(node2->get_type() == cvt_line)
            collect_intersections(pts, static_cast<const cv_line_node*>(node2), static_cast<const cv_cubic_node*>(node1));
        else if(node2->get_type() == cvt_quad)
            collect_intersections(pts, static_cast<const cv_quad_node*>(node2), static_cast<const cv_cubic_node*>(node1));
        else if(node2->get_type() == cvt_cubic)
            collect_intersections(pts, static_cast<const cv_cubic_node*>(node1), static_cast<const cv_cubic_node*>(node2));
    }
    paint->save();
    cv_set_no_pen(paint);
    cv_set_brush_color(paint, cv_intersect_point_cr);
    for(const vec2& p : pts)
        cv_draw_dot(paint, p);
    paint->restore();
}

__ariel_end__