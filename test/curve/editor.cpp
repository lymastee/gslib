#include "editor.h"

__ariel_begin__

static const float cv_hit_range = 5.f;

curve_creator::~curve_creator()
{
    if(_node) {
        delete _node;
        _node = nullptr;
    }
}

bool curve_creator::add_point(const vec2& p)
{
    if(!_node) {
        _node = new cv_line_node;
        _node->add_point(p);
        return true;
    }
    if(_node->add_point(p))
        return true;
    return switch_to_next_degree(p);
}

bool curve_creator::move_current_point(const vec2& p)
{
    if(!_node)
        return false;
    int c = _node->get_current_id();
    if(c < 0 || c >= _node->get_point_count())
        return false;
    _node->set_point(c, p);
    return true;
}

void curve_creator::draw(painter* paint) const
{
    if(_node)
        _node->draw(paint);
}

void curve_creator::cancel()
{
    if(_node) {
        delete _node;
        _node = nullptr;
    }
}

cv_curve_node* curve_creator::finish()
{
    auto r = _node;
    _node = nullptr;
    if(!r)
        return nullptr;
    if(r->get_type() == cvt_line) {
        if(r->get_current_id() < 1) {   // unfinished line
            delete r;
            return nullptr;
        }
    }
    r->set_current_id(r->get_point_count());
    return r;
}

bool curve_creator::switch_to_next_degree(const vec2& p)
{
    if(!_node)
        return false;
    cv_curve_node* next_degree = nullptr;
    switch(_node->get_type())
    {
    case cvt_line:
        next_degree = new cv_quad_node;
        break;
    case cvt_quad:
        next_degree = new cv_cubic_node;
        break;
    }
    if(!next_degree)
        return false;
    for(int i = 0; i < _node->get_point_count(); i ++)
        next_degree->add_point(_node->get_point(i));
    verify(next_degree->add_point(p));
    delete _node;
    _node = next_degree;
    return true;
}

curve_modifier::curve_modifier(cv_curve_node* n, int id)
{
    assert(n);
    _node = n;
    _current_id = id;
}

void curve_modifier::set_hit_point(const vec2& p)
{
    if(_node)
        _offset = _node->get_point(_current_id) - p;
}

void curve_modifier::move(const vec2& p)
{
    if(_node)
        _node->set_point(_current_id, p + _offset);
}

bool curve_modifier::equal(const curve_hit_point& hp) const
{
    return _node == hp._node && _current_id == hp._id;
}

void curve_modifier::draw(painter* paint) const
{
    assert(paint);
    if(_node) {
        paint->save();
        cv_surround_dot(paint, _node->get_point(_current_id), cv_select_point_cr);
        paint->restore();
    }
}

curve_hit_point::curve_hit_point(cv_curve_node* n, int id)
{
    _node = n;
    _id = id;
}

float curve_hit_point::calc_distance(const vec2& p) const
{
    if(_node) {
        vec2 d;
        d.sub(_node->get_point(_id), p);
        return d.length();
    }
    return FLT_MAX;
}

curve_editor::~curve_editor()
{
    reset();
}

void curve_editor::press_at_point(const vec2& p)
{
    if(_creator.is_creating()) {
        if(_creator.add_point(p))
            return;
        confirm();
    }
    // are we going to launch a modifier?
    bool has_mod_before = false;
    if(_modifier) {
        has_mod_before = true;
        cancel();
    }
    curve_hit_point* f = query_hits(p);
    if(f) {
        _modifier = new curve_modifier(f->_node, f->_id);
        assert(_modifier);
        _modifier->set_hit_point(p);
        return;
    }
    // or another creator
    if(!has_mod_before)
        _creator.add_point(p);
}

void curve_editor::move_point(const vec2& p)
{
    if(_modifier) {
        _modifier->move(p);
        if(_creator.is_creating()) {
            assert(!"conflict.");
            _creator.cancel();
        }
        return;
    }
    _creator.move_current_point(p);
}

void curve_editor::release_at_point(const vec2& p)
{
    if(_modifier) {
        _modifier->move(p);
        return;
    }
    _creator.move_current_point(p);
}

void curve_editor::confirm()
{
    cv_curve_node* node = _creator.finish();
    if(node) {
        _curves.push_back(node);
        prepare_hit_points(node);
    }
}

void curve_editor::cancel()
{
    if(_modifier) {
        delete _modifier;
        _modifier = nullptr;
    }
    _creator.cancel();
}

void curve_editor::draw(painter* paint) const
{
    assert(paint);
    for(const cv_curve_node* node : _curves)
        node->draw(paint);
    _creator.draw(paint);
    if(_modifier)
        _modifier->draw(paint);
}

void curve_editor::draw_extra(painter* paint, cv_draw_extra cde) const
{
    assert(paint);
    for(const cv_curve_node* node : _curves)
        node->draw_extra(paint, cde);
    if(cde == cde_intersections) {
        if(_curves.size() < 2)
            return;
        const cv_curve_node* node1 = _curves.at(0);
        const cv_curve_node* node2 = _curves.at(1);
        cv_draw_intersections(paint, node1, node2);
    }
}

void curve_editor::reset()
{
    if(_modifier) {
        delete _modifier;
        _modifier = nullptr;
    }
    for(cv_curve_node* n : _curves)
        delete n;
    _curves.clear();
    _hitpts.clear();
    _creator.cancel();
}

void curve_editor::prepare_hit_points(cv_curve_node* n)
{
    if(!n)
        return;
    for(int i = 0; i < n->get_point_count(); i ++)
        _hitpts.emplace_back(n, i);
}

curve_hit_point* curve_editor::query_hits(const vec2& p)
{
    float dmin = FLT_MAX;
    curve_hit_point* f = nullptr;
    for(curve_hit_point& hp : _hitpts) {
        float d = hp.calc_distance(p);
        if(d < dmin) {
            dmin = d;
            f = &hp;
        }
    }
    if(dmin <= cv_hit_range)
        return f;
    return nullptr;
}

__ariel_end__
