#pragma once

#include <ariel/widget.h>

__ariel_begin__

#define cv_control_point_cr         color(34, 177, 76)          // ÂÌ
#define cv_quad_control_path_cr     color(0, 162, 232)          // À¶
#define cv_cubic_control_path_cr    color(255, 174, 201)        // ·Ûºì
#define cv_curve_path_cr            color(128, 64, 64)          // ºÖ
#define cv_current_point_cr         color(255, 201, 14)         // »Æ
#define cv_select_point_cr          color(255, 127, 39)         // ³È
#define cv_intersect_point_cr       color(255, 28, 36)          // ºì
#define cv_tangent_line_cr          color(195, 195, 195)        // »Ò
#define cv_inflection_point_cr      color(128, 0, 128)          // ×Ï
#define cv_extrema_point_cr         color(112, 146, 190)        // À¶»Ò
#define cv_bound_box_cr             color(205, 205, 205)        // »Ò
#define cv_cubic_to_quad_cr         color(255, 201, 14)         // »Æ

class cv_curve_node;

inline void cv_set_no_pen(painter* paint) { paint->set_pen(painter_pen()); }
inline void cv_set_no_brush(painter* paint) { paint->set_brush(painter_brush()); }
extern void cv_set_pen_color(painter* paint, const color& cr);
extern void cv_set_brush_color(painter* paint, const color& cr);
extern void cv_draw_dot(painter* paint, const vec2& p);
extern void cv_draw_dot(painter* paint, const vec2& p, const color& cr);
extern void cv_surround_dot(painter* paint, const vec2& p, const color& cr);
extern void cv_draw_inflections(painter* paint, const vec2 cp[4]);
extern void cv_draw_quad_extrema(painter* paint, const vec2 cp[3]);
extern void cv_draw_cubic_extrema(painter* paint, const vec2 cp[4]);
extern void cv_draw_self_intersection(painter* paint, const vec2 cp[4]);
extern void cv_draw_cubic_to_quad(painter* paint, const vec2 cp[4]);
extern void cv_draw_intersections(painter* paint, const cv_curve_node* node1, const cv_curve_node* node2);

enum cv_curve_type
{
    cvt_line,
    cvt_quad,
    cvt_cubic,
};

enum cv_draw_extra
{
    cde_nothing,
    cde_inflections,
    cde_extrema,
    cde_self_intersection,
    cde_cubic_to_quad,
    cde_intersections,
};

class __gs_novtable cv_curve_node abstract
{
public:
    virtual ~cv_curve_node() {}
    virtual cv_curve_type get_type() const = 0;
    virtual void draw(painter* paint) const = 0;
    virtual void draw_extra(painter* paint, cv_draw_extra cde) const = 0;
    virtual int get_point_count() const = 0;
    virtual const vec2& get_point(int i) const = 0;
    virtual void set_point(int i, const vec2& p) = 0;

protected:
    int                 _curr = -1;

public:
    void set_current_id(int c) { _curr = c; }
    int get_current_id() const { return _curr; }
    bool add_point(const vec2& p);
    bool is_overlapping(const vec2& p, int except = -1) const;
};

template<cv_curve_type _ct, int _size>
class cv_curve_node_tpl:
    public cv_curve_node
{
public:
    virtual cv_curve_type get_type() const override { return _ct; }
    virtual void draw(painter* paint) const override;
    virtual void draw_extra(painter* paint, cv_draw_extra cde) const override;
    virtual int get_point_count() const override { return _size; }
    virtual const vec2& get_point(int i) const override
    {
        assert(i >= 0 && i < _size);
        return _cp[i];
    }
    virtual void set_point(int i, const vec2& p) override
    {
        assert(i >= 0 && i < _size);
        if(!is_overlapping(p, i))
            _cp[i] = p;
    }

private:
    vec2                _cp[_size];
};

template<>
inline void cv_curve_node_tpl<cvt_line, 2>::draw(painter* paint) const
{
    assert(paint);
    if(_curr < 0)
        return;
    paint->save();
    if(_curr == 0) {
        cv_draw_dot(paint, _cp[0], cv_control_point_cr);
        cv_surround_dot(paint, _cp[0], cv_current_point_cr);
        paint->restore();
        return;
    }
    cv_set_pen_color(paint, cv_curve_path_cr);
    cv_set_no_brush(paint);
    paint->draw_line(_cp[0], _cp[1]);
    // draw dots
    cv_set_brush_color(paint, cv_control_point_cr);
    cv_set_no_pen(paint);
    cv_draw_dot(paint, _cp[0]);
    cv_draw_dot(paint, _cp[1]);
    if(_curr == 1)
        cv_surround_dot(paint, _cp[1], cv_current_point_cr);
    paint->restore();
}

template<>
inline void cv_curve_node_tpl<cvt_line, 2>::draw_extra(painter* paint, cv_draw_extra cde) const
{
    assert(paint);
}

template<>
inline void cv_curve_node_tpl<cvt_quad, 3>::draw(painter* paint) const
{
    assert(paint);
    if(_curr < 0)
        return;
    paint->save();
    if(_curr == 0) {
        cv_draw_dot(paint, _cp[0], cv_control_point_cr);
        cv_surround_dot(paint, _cp[0], cv_current_point_cr);
        paint->restore();
        return;
    }
    if(_curr == 1) {
        cv_set_pen_color(paint, cv_quad_control_path_cr);
        cv_set_no_brush(paint);
        paint->draw_line(_cp[0], _cp[1]);
        // draw dots
        cv_set_brush_color(paint, cv_control_point_cr);
        cv_set_no_pen(paint);
        cv_draw_dot(paint, _cp[0]);
        cv_draw_dot(paint, _cp[1]);
        cv_surround_dot(paint, _cp[1], cv_current_point_cr);
        paint->restore();
        return;
    }
    // draw control path
    cv_set_pen_color(paint, cv_quad_control_path_cr);
    cv_set_no_brush(paint);
    painter_path ctl_path;
    ctl_path.move_to(_cp[0]);
    ctl_path.line_to(_cp[1]);
    ctl_path.line_to(_cp[2]);
    paint->draw_path(ctl_path);
    // draw path
    cv_set_pen_color(paint, cv_curve_path_cr);
    paint->draw_quad(_cp[0], _cp[1], _cp[2]);
    // draw dots
    cv_set_brush_color(paint, cv_control_point_cr);
    cv_set_no_pen(paint);
    cv_draw_dot(paint, _cp[0]);
    cv_draw_dot(paint, _cp[1]);
    cv_draw_dot(paint, _cp[2]);
    if(_curr == 2)
        cv_surround_dot(paint, _cp[2], cv_current_point_cr);
    paint->restore();
}

template<>
inline void cv_curve_node_tpl<cvt_quad, 3>::draw_extra(painter* paint, cv_draw_extra cde) const
{
    assert(paint);
    if(cde == cde_extrema)
        cv_draw_quad_extrema(paint, _cp);
}

template<>
inline void cv_curve_node_tpl<cvt_cubic, 4>::draw(painter* paint) const
{
    assert(paint);
    if(_curr < 0)
        return;
    paint->save();
    if(_curr == 0) {
        cv_draw_dot(paint, _cp[0], cv_control_point_cr);
        cv_surround_dot(paint, _cp[0], cv_current_point_cr);
        paint->restore();
        return;
    }
    // draw control path
    painter_path ctl_path;
    ctl_path.move_to(_cp[0]);
    for(int i = 1; i <= gs_min(_curr, 3); i ++)
        ctl_path.line_to(_cp[i]);
    cv_set_pen_color(paint, cv_cubic_control_path_cr);
    cv_set_no_brush(paint);
    paint->draw_path(ctl_path);
    // draw path
    if(_curr >= 3) {
        cv_set_pen_color(paint, cv_curve_path_cr);
        paint->draw_cubic(_cp[0], _cp[1], _cp[2], _cp[3]);
    }
    // draw dots
    cv_set_brush_color(paint, cv_control_point_cr);
    cv_set_no_pen(paint);
    for(int i = 0; i <= gs_min(_curr, 3); i ++)
        cv_draw_dot(paint, _cp[i]);
    if(_curr <= 3)
        cv_surround_dot(paint, _cp[_curr], cv_current_point_cr);
    paint->restore();
}

template<>
inline void cv_curve_node_tpl<cvt_cubic, 4>::draw_extra(painter* paint, cv_draw_extra cde) const
{
    assert(paint);
    if(cde == cde_inflections)
        cv_draw_inflections(paint, _cp);
    else if(cde == cde_extrema)
        cv_draw_cubic_extrema(paint, _cp);
    else if(cde == cde_self_intersection)
        cv_draw_self_intersection(paint, _cp);
    else if(cde == cde_cubic_to_quad)
        cv_draw_cubic_to_quad(paint, _cp);
}

typedef cv_curve_node_tpl<cvt_line, 2> cv_line_node;
typedef cv_curve_node_tpl<cvt_quad, 3> cv_quad_node;
typedef cv_curve_node_tpl<cvt_cubic, 4> cv_cubic_node;

__ariel_end__
