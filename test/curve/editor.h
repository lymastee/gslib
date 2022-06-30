#pragma once

#include "basis.h"

__ariel_begin__

class curve_creator
{
public:
    ~curve_creator();
    bool is_creating() const { return _node != nullptr; }
    bool add_point(const vec2& p);
    bool move_current_point(const vec2& p);
    void draw(painter* paint) const;
    void cancel();
    cv_curve_node* finish();

private:
    cv_curve_node*      _node = nullptr;

private:
    bool switch_to_next_degree(const vec2& p);
};

struct curve_hit_point;

class curve_modifier
{
public:
    curve_modifier(cv_curve_node* n, int id);
    void set_hit_point(const vec2& p);
    void move(const vec2& p);
    bool equal(const curve_hit_point& hp) const;
    void draw(painter* paint) const;

private:
    cv_curve_node*      _node = nullptr;
    int                 _current_id = -1;
    vec2                _offset = vec2(0.f, 0.f);
};

struct curve_hit_point
{
    cv_curve_node*      _node = nullptr;
    int                 _id = -1;

public:
    curve_hit_point(cv_curve_node* n, int id);
    float calc_distance(const vec2& p) const;
};

class curve_editor
{
public:
    ~curve_editor();
    void press_at_point(const vec2& p);
    void move_point(const vec2& p);
    void release_at_point(const vec2& p);
    void confirm();
    void cancel();
    void draw(painter* paint) const;
    void draw_extra(painter* paint, cv_draw_extra cde) const;
    void reset();

private:
    vector<cv_curve_node*> _curves;
    curve_creator _creator;
    curve_modifier* _modifier = nullptr;
    list<curve_hit_point> _hitpts;

private:
    void prepare_hit_points(cv_curve_node* n);
    curve_hit_point* query_hits(const vec2& p);
};

__ariel_end__