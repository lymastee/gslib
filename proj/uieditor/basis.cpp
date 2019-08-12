/*
 * Copyright (c) 2016-2019 lymastee, All rights reserved.
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
#include <gslib/library.h>
#include <gslib/dvt.h>
#include <ariel/classicstyle.h>
#include "basis.h"

namespace ui_editor {

using gs::library;
using gs::addrptr;
using gs::_set_error;
using gs::vtable_ops;
using gs::dvt_detour_code;
using namespace gs::ariel;

void ui_src_entry::add_include(const string& str)
{
    if(str.empty())
        return;
    _include_list.push_back(str);
}

void ui_src_entry::add_include_group(const string& strgroup)
{
    if(strgroup.empty())
        return;
    size_t p = 0;
    for(;;) {
        auto f = strgroup.find(_t(';'), p);
        if(f == string::npos) {
            add_include(string(&strgroup.at(p), strgroup.length() - p));
            break;
        }
        add_include(string(&strgroup.at(p), f - p));
        p = f + 1;
        if(p >= (size_t)strgroup.length())
            break;
    }
}

int ui_editor_context::acquire_variable_index(const string& vartype)
{
    auto r = _index_map.try_emplace(vartype, 0);
    return r.first->second ++;
}

int ui_editor_context::query_variable_index(const string& vartype) const
{
    auto f = _index_map.find(vartype);
    return f != _index_map.end() ? f->second : 0;
}

int ui_editor_context::acquire_variable_name(string& name, const string& vartype)
{
    int index = acquire_variable_index(vartype);
    name.format(_t("_ui_%s%d"), vartype.c_str(), index);
    return index;
}

ui_transparent_layer::ui_transparent_layer(wsys_manager* m):
    core_widget(m)
{
    _reflection_widget = nullptr;
}

ui_transparent_layer::~ui_transparent_layer()
{
}

void ui_transparent_layer::install_reflection_widget(core_widget* refw)
{
    assert(refw);
    _reflection_widget = refw;
    install_dvt(refw, 2);
    connect_reflect(refw, &core_widget::close, this, &ui_transparent_layer::on_reflection_close);
    connect_reflect(refw, &core_widget::show, this, &ui_transparent_layer::on_reflection_show);
    connect_reflect(refw, &core_widget::enable, this, &ui_transparent_layer::on_reflection_enable);
    void (core_widget::*select_fn_move)(const rect& rc) = &core_widget::move;
    connect_reflect(refw, select_fn_move, this, &ui_transparent_layer::on_reflection_move);
    connect_reflect(refw, &core_widget::on_char, this, &ui_transparent_layer::on_reflection_char);
    connect_reflect(refw, &core_widget::on_caret, this, &ui_transparent_layer::on_reflection_caret);
    connect_reflect(refw, &core_widget::on_capture, this, &ui_transparent_layer::on_reflection_capture);
    connect_reflect(refw, &core_widget::on_focus, this, &ui_transparent_layer::on_reflection_focus);
    connect_reflect(refw, &core_widget::on_scroll, this, &ui_transparent_layer::on_reflection_scroll);
    connect_reflect(refw, &core_widget::on_accelerator, this, &ui_transparent_layer::on_reflection_accelerator);
    /* initially synchronize the size */
    sync_size_with_reflection();
}

void ui_transparent_layer::on_reflection_close()
{
    if(_reflection_widget) {
        _reflection_widget->switch_to_ovt();
        _reflection_widget->close();
        _reflection_widget->switch_to_dvt();
    }
    assert(_manager);
    _manager->remove_widget(this);
}

void ui_transparent_layer::on_reflection_show(bool b)
{
    if(_reflection_widget) {
        _reflection_widget->switch_to_ovt();
        _reflection_widget->show(b);
        _reflection_widget->switch_to_dvt();
    }
    show(b);
}

void ui_transparent_layer::on_reflection_enable(bool b)
{
    if(_reflection_widget) {
        _reflection_widget->switch_to_ovt();
        _reflection_widget->enable(b);
        _reflection_widget->switch_to_dvt();
    }
    enable(b);
}

void ui_transparent_layer::on_reflection_move(const rect& rc)
{
    if(_reflection_widget) {
        _reflection_widget->switch_to_ovt();
        _reflection_widget->move(rc);
        _reflection_widget->switch_to_dvt();
    }
    sync_size_with_reflection();
}

void ui_transparent_layer::sync_size_with_reflection()
{
    if(!_reflection_widget)
        return;
    const rect& rc = _reflection_widget->get_rect();
    move(rc);
    rect refrc = rc;
    refrc.move_to(0, 0);
    _reflection_widget->switch_to_ovt();
    _reflection_widget->move(refrc);
    _reflection_widget->switch_to_dvt();
}

class __gs_novtable ui_edit_control abstract
{
public:
    class indicator_widget:
        public core_widget
    {
    public:
        indicator_widget(wsys_manager* wsys): core_widget(wsys)
        {
            _pen.set_tag(painter_pen::solid);
            _pen.set_color(color(255, 0, 0, 255));
        }
        void set_indicator_rect(const rect& rc)
        {
            _rc.set_rect((float)rc.left, (float)rc.top, (float)rc.width(), (float)rc.height());
            refresh(false);
        }
        rect get_indicator_rect() const
        {
            return rect((int)_rc.left, (int)_rc.top, (int)_rc.width(), (int)_rc.height());
        }
        virtual void draw(painter* paint) override
        {
            paint->save();
            paint->set_brush(painter_brush(painter_brush::none));
            paint->set_pen(_pen);
            paint->draw_rect(_rc);
            paint->restore();
        }

    private:
        painter_pen     _pen;
        rectf           _rc;
    };

public:
    ui_edit_control(ui_editor_layer* editlayer, const point& initpos)
    {
        _indicator = nullptr;
        assert(editlayer);
        _editor = editlayer;
        _init_pos = _current_pos = initpos;
    }
    virtual ~ui_edit_control() {}
    virtual void on_edit_position(uint um, const point& pt) = 0;
    virtual void on_edit_done() = 0;
    virtual void refresh_indicator() = 0;

protected:
    indicator_widget*   _indicator;
    ui_editor_layer*    _editor;
    point               _init_pos;
    point               _current_pos;

protected:
    void install_indicator()
    {
        if(_indicator || !_editor)
            return;
        core_widget* parent = _editor->get_parent();
        if(!parent) {
            set_error(_t("install indicator failed: missing parent widget."));
            return;
        }
        rect rc = parent->get_rect();
        rc.move_to(0, 0);
        wsys_manager* wsys = _editor->get_manager();
        assert(wsys);
        _indicator = wsys->add_widget<indicator_widget>(parent, _t("sizing_ctl_indicator"), rc, sm_visible);
        assert(_indicator);
        refresh_indicator();
    }
    void uninstall_indicator()
    {
        if(!_indicator)
            return;
        wsys_manager* wsys = _indicator->get_manager();
        assert(wsys);
        wsys->remove_widget(_indicator);
        _indicator = nullptr;
    }
};

template<ui_editor_layer::size_orient _size_orient>
class ui_edit_ctl_impl:
    public ui_edit_control
{
public:
    ui_edit_ctl_impl(ui_editor_layer* editlayer, const point& initpos): ui_edit_control(editlayer, initpos)
    {
        install_indicator();
    }
    virtual ~ui_edit_ctl_impl()
    {
        on_edit_done();
        uninstall_indicator();
    }
    virtual void on_edit_position(uint um, const point& pt) override
    {
        _current_pos = pt;
        refresh_indicator();
    }
    virtual void on_edit_done() override
    {
        if(!_indicator || !_editor)
            return;
        const rect& rc = _indicator->get_indicator_rect();
        ui_node* uinode = _editor->get_ui_node();
        assert(uinode);
        uinode->position = rc;
        auto* cw = _editor->get_reflection_widget();
        assert(cw);
        cw->move(rc);
    }
    virtual void refresh_indicator() override {}
};

void ui_edit_ctl_impl<ui_editor_layer::so_n>::refresh_indicator()
{
    assert(_editor);
    static int gap = 1;
    const rect& rc = _editor->get_rect();
    int dy = _current_pos.y - _init_pos.y;
    int y = gs::gs_min(rc.top + dy, rc.bottom - gap);
    rect ind;
    ind.set_ltrb(rc.left, y, rc.right, rc.bottom);
    if(_indicator)
        _indicator->set_indicator_rect(ind);
}

void ui_edit_ctl_impl<ui_editor_layer::so_s>::refresh_indicator()
{
    assert(_editor);
    static int gap = 1;
    const rect& rc = _editor->get_rect();
    int dy = _current_pos.y - _init_pos.y;
    int y = gs::gs_max(rc.bottom + dy, rc.top + gap);
    rect ind;
    ind.set_ltrb(rc.left, rc.top, rc.right, y);
    if(_indicator)
        _indicator->set_indicator_rect(ind);
}

void ui_edit_ctl_impl<ui_editor_layer::so_w>::refresh_indicator()
{
    assert(_editor);
    static int gap = 1;
    const rect& rc = _editor->get_rect();
    int dx = _current_pos.x - _init_pos.x;
    int x = gs::gs_min(rc.left + dx, rc.right - gap);
    rect ind;
    ind.set_ltrb(x, rc.top, rc.right, rc.bottom);
    if(_indicator)
        _indicator->set_indicator_rect(ind);
}

void ui_edit_ctl_impl<ui_editor_layer::so_e>::refresh_indicator()
{
    assert(_editor);
    static int gap = 1;
    const rect& rc = _editor->get_rect();
    int dx = _current_pos.x - _init_pos.x;
    int x = gs::gs_max(rc.right + dx, rc.left + gap);
    rect ind;
    ind.set_ltrb(rc.left, rc.top, x, rc.bottom);
    if(_indicator)
        _indicator->set_indicator_rect(ind);
}

void ui_edit_ctl_impl<ui_editor_layer::so_nw>::refresh_indicator()
{
    assert(_editor);
    static int gap = 1;
    const rect& rc = _editor->get_rect();
    int dx = _current_pos.x - _init_pos.x;
    int dy = _current_pos.y - _init_pos.y;
    int x = gs::gs_min(rc.left + dx, rc.right - gap);
    int y = gs::gs_min(rc.top + dy, rc.bottom - gap);
    rect ind;
    ind.set_ltrb(x, y, rc.right, rc.bottom);
    if(_indicator)
        _indicator->set_indicator_rect(ind);
}

void ui_edit_ctl_impl<ui_editor_layer::so_ne>::refresh_indicator()
{
    assert(_editor);
    static int gap = 1;
    const rect& rc = _editor->get_rect();
    int dx = _current_pos.x - _init_pos.x;
    int dy = _current_pos.y - _init_pos.y;
    int x = gs::gs_max(rc.right + dx, rc.left + gap);
    int y = gs::gs_min(rc.top + dy, rc.bottom - gap);
    rect ind;
    ind.set_ltrb(rc.left, y, x, rc.bottom);
    if(_indicator)
        _indicator->set_indicator_rect(ind);
}

void ui_edit_ctl_impl<ui_editor_layer::so_sw>::refresh_indicator()
{
    assert(_editor);
    static int gap = 1;
    const rect& rc = _editor->get_rect();
    int dx = _current_pos.x - _init_pos.x;
    int dy = _current_pos.y - _init_pos.y;
    int x = gs::gs_min(rc.left + dx, rc.right - gap);
    int y = gs::gs_max(rc.bottom + dy, rc.top + gap);
    rect ind;
    ind.set_ltrb(x, rc.top, rc.right, y);
    if(_indicator)
        _indicator->set_indicator_rect(ind);
}

void ui_edit_ctl_impl<ui_editor_layer::so_se>::refresh_indicator()
{
    assert(_editor);
    static int gap = 1;
    const rect& rc = _editor->get_rect();
    int dx = _current_pos.x - _init_pos.x;
    int dy = _current_pos.y - _init_pos.y;
    int x = gs::gs_max(rc.right + dx, rc.left + gap);
    int y = gs::gs_max(rc.bottom + dy, rc.top + gap);
    rect ind;
    ind.set_ltrb(rc.left, rc.top, x, y);
    if(_indicator)
        _indicator->set_indicator_rect(ind);
}

void ui_edit_ctl_impl<ui_editor_layer::so_none>::refresh_indicator()
{
    assert(_editor);
    int dx = _current_pos.x - _init_pos.x;
    int dy = _current_pos.y - _init_pos.y;
    rect rc = _editor->get_rect();
    rc.offset(dx, dy);
    if(_indicator)
        _indicator->set_indicator_rect(rc);
}

ui_editor_layer::ui_editor_layer(wsys_manager* m):
    ui_transparent_layer(m)
{
    _assoc_ui_node = nullptr;
}

void ui_editor_layer::initialize(ui_node& uinode)
{
    _assoc_ui_node = &uinode;
    _edit_ctl = nullptr;
    assert(uinode.assoc_widget);
    install_reflection_widget(uinode.assoc_widget);
}

void ui_editor_layer::install_reflection_widget(core_widget* refw)
{
    assert(refw);
    install_dvt(refw, 2);
    connect_reflect(refw, &core_widget::on_press, this, &ui_editor_layer::on_reflection_press);
    connect_reflect(refw, &core_widget::on_click, this, &ui_editor_layer::on_reflection_click);
    connect_reflect(refw, &core_widget::on_hover, this, &ui_editor_layer::on_reflection_hover);
    connect_reflect(refw, &core_widget::on_leave, this, &ui_editor_layer::on_reflection_leave);
    connect_reflect(refw, &core_widget::on_keydown, this, &ui_editor_layer::on_reflection_keydown);
    connect_reflect(refw, &core_widget::on_keyup, this, &ui_editor_layer::on_reflection_keyup);
    ui_transparent_layer::install_reflection_widget(refw);
}

void ui_editor_layer::on_reflection_press(uint um, unikey uk, const point& pt)
{
    if(_reflection_widget) {
        _reflection_widget->switch_to_ovt();
        _reflection_widget->on_press(um, uk, pt);
        _reflection_widget->switch_to_dvt();
    }
    try_install_edit_control(pt);
}

void ui_editor_layer::on_reflection_click(uint um, unikey uk, const point& pt)
{
    if(_reflection_widget) {
        _reflection_widget->switch_to_ovt();
        _reflection_widget->on_click(um, uk, pt);
        _reflection_widget->switch_to_dvt();
    }
    try_uninstall_edit_control();
}

void ui_editor_layer::on_reflection_hover(uint um, const point& pt)
{
    if(_reflection_widget) {
        _reflection_widget->switch_to_ovt();
        _reflection_widget->on_hover(um, pt);
        _reflection_widget->switch_to_dvt();
    }
    refresh_cursor(pt);
    refresh_edit_control(um, pt);
}

void ui_editor_layer::on_reflection_leave(uint um, const point& pt)
{
    if(_reflection_widget) {
        _reflection_widget->switch_to_ovt();
        _reflection_widget->on_leave(um, pt);
        _reflection_widget->switch_to_dvt();
    }
    _manager->reset_cursor();
}

void ui_editor_layer::on_reflection_keydown(uint um, unikey uk)
{
    if(_reflection_widget) {
        _reflection_widget->switch_to_ovt();
        _reflection_widget->on_keydown(um, uk);
        _reflection_widget->switch_to_dvt();
    }
}

void ui_editor_layer::on_reflection_keyup(uint um, unikey uk)
{
    if(_reflection_widget) {
        _reflection_widget->switch_to_ovt();
        _reflection_widget->on_keyup(um, uk);
        _reflection_widget->switch_to_dvt();
    }
}

void ui_editor_layer::refresh_cursor(const point& pt)
{
    if(_edit_ctl)
        return;
    auto so = get_size_orient(pt);
    switch(so)
    {
    case so_n:
    case so_s:
        _manager->set_cursor(cur_size_ns);
        break;
    case so_e:
    case so_w:
        _manager->set_cursor(cur_size_we);
        break;
    case so_nw:
    case so_se:
        _manager->set_cursor(cur_size_nwse);
        break;
    case so_ne:
    case so_sw:
        _manager->set_cursor(cur_size_nesw);
        break;
    case so_none:
    default:
        _manager->reset_cursor();
        break;
    }
}

ui_editor_layer::size_orient ui_editor_layer::get_size_orient(const point& pt, int accuracy) const
{
    int dn = abs(pt.y);
    int dw = abs(pt.x);
    int ds = abs(get_height() - pt.y);
    int de = abs(get_width() - pt.x);
    int v = 0, h = 0;
    if(dn <= accuracy && ds <= accuracy)
        v = dn < ds ? -1 : 1;
    else if(dn <= accuracy)
        v = -1;
    else if(ds <= accuracy)
        v = 1;
    if(dw <= accuracy && de <= accuracy)
        h = dw < de ? -1 : 1;
    else if(dw <= accuracy)
        h = -1;
    else if(de <= accuracy)
        h = 1;
    if(!v && !h)
        return so_none;
    if(!v)
        return h < 0 ? so_w : so_e;
    if(!h)
        return v < 0 ? so_n : so_s;
    return v < 0 ? (h < 0 ? so_nw : so_ne) :
        (h < 0 ? so_sw : so_se);
}

void ui_editor_layer::try_install_edit_control(const point& pt)
{
    if(_edit_ctl)
        return;
    auto so = get_size_orient(pt);
    switch(so)
    {
    case so_n:
        _edit_ctl = new ui_edit_ctl_impl<so_n>(this, pt);
        break;
    case so_s:
        _edit_ctl = new ui_edit_ctl_impl<so_s>(this, pt);
        break;
    case so_e:
        _edit_ctl = new ui_edit_ctl_impl<so_e>(this, pt);
        break;
    case so_w:
        _edit_ctl = new ui_edit_ctl_impl<so_w>(this, pt);
        break;
    case so_ne:
        _edit_ctl = new ui_edit_ctl_impl<so_ne>(this, pt);
        break;
    case so_nw:
        _edit_ctl = new ui_edit_ctl_impl<so_nw>(this, pt);
        break;
    case so_se:
        _edit_ctl = new ui_edit_ctl_impl<so_se>(this, pt);
        break;
    case so_sw:
        _edit_ctl = new ui_edit_ctl_impl<so_sw>(this, pt);
        break;
    case so_none:
    default:
        _edit_ctl = new ui_edit_ctl_impl<so_none>(this, pt);
        _manager->set_cursor(cur_size_all);
        break;
    }
}

void ui_editor_layer::try_uninstall_edit_control()
{
    if(_edit_ctl) {
        delete _edit_ctl;
        _edit_ctl = nullptr;
    }
}

void ui_editor_layer::refresh_edit_control(uint um, const point& pt)
{
    if(_edit_ctl)
        _edit_ctl->on_edit_position(um, pt);
}

struct ui_creator_node
{
    library                 loader;
    pfn_create_widget       fn_create_widget;
    /* todo more... */

public:
    ui_creator_node()
    {
        fn_create_widget = nullptr;
    }
};

static void ui_default_map_properties(core_widget* w, ui_node& uinode)
{
    assert(w);
    style_sheet* ss = (style_sheet*)(w->query_interface(classic_style::uuid_style_sheet));
    if(!ss)
        return;
    for(const auto& p : uinode.prop_map)
        ss->set_value(p.first, p.second);
    ss->flush_style();
}

static core_widget* __stdcall ui_default_create_widget(wsys_manager* wsys, core_widget* parent, ui_node& uinode)
{
    assert(wsys && parent);
    core_widget* result = nullptr;
    uint style = sm_visible | sm_hitable;
    if(uinode.type.compare(_cststr(_t("root_widget"))) == 0)
        result = wsys->add_widget<classic_style::root_widget>(parent, uinode.name.c_str(), uinode.position, style);
    else if(uinode.type.compare(_cststr(_t("widget"))) == 0)
        result = wsys->add_widget<classic_style::widget>(parent, uinode.name.c_str(), uinode.position, style);
    else if(uinode.type.compare(_cststr(_t("button"))) == 0)
        result = wsys->add_widget<classic_style::button>(parent, uinode.name.c_str(), uinode.position, style);
    else if(uinode.type.compare(_cststr(_t("edit_line"))) == 0)
        result = wsys->add_widget<classic_style::edit_line>(parent, uinode.name.c_str(), uinode.position, style);
    else {
        /* just warn it. */
        set_error(_t("ui create widget failed, widget type: %s."), uinode.type.c_str());
    }
    if(result) {
        ui_default_map_properties(result, uinode);
        uinode.assoc_widget = result;
        return result;
    }
    return nullptr;
}

static void ui_initialize_default_creator_node(ui_creator_node& node)
{
    node.fn_create_widget = &ui_default_create_widget;
}

class ui_package_manager
{
public:
    typedef unordered_map<string, ui_creator_node> ui_creator_map;

public:
    static ui_package_manager* get_singleton_ptr()
    {
        static ui_package_manager inst;
        return &inst;
    }
    bool query(const string& libpath) const
    {
        auto f = _creator_map.find(libpath);
        return f != _creator_map.end();
    }
    const ui_creator_node& get_default_creator() const { return _default_creator_node; }
    const ui_creator_node* find_creator(const string& libpath) const
    {
        auto f = _creator_map.find(libpath);
        if(f != _creator_map.end())
            return &(f->second);
        return nullptr;
    }
    const ui_creator_node* ensure_get_creator(const string& libpath)
    {
        auto f = _creator_map.find(libpath);
        if(f != _creator_map.end())
            return &(f->second);
        if(!library::existed(libpath))
            return nullptr;
        ui_creator_node& node = _creator_map.emplace(libpath, ui_creator_node()).first->second;
        if(!node.loader.load(libpath)) {
            set_error(_t("invalid library: %s."), libpath.c_str());
            return &node;
        }
        /* initialize creator node */
        node.fn_create_widget = (pfn_create_widget)node.loader.resolve(_t("ui_create_widget"));
        return &node;
    }

protected:
    ui_creator_node         _default_creator_node;
    ui_creator_map          _creator_map;

private:
    ui_package_manager()
    {
        ui_initialize_default_creator_node(_default_creator_node);
    }
};

static core_widget* ui_create_widget(wsys_manager* wsys, core_widget* parent, ui_tree::iterator coreit)
{
    assert(wsys && coreit);
    /*
     * If a widget type has a prefix with a colon, for example like "domain:widget_type1",
     * then the "domain" will be the domain name of the widget type, typically we would find the entry ""
     * from the domain.dll
     */
    const ui_creator_node* creator = nullptr;
    const string& widget_type = coreit->type;
    auto p = widget_type.find(_t(':'));
    if(p == string::npos || p == 0)
        creator = &(ui_package_manager::get_singleton_ptr()->get_default_creator());
    else {
        string libpath(widget_type.c_str(), p);
        libpath.append(_t(".dll"));
        creator = ui_package_manager::get_singleton_ptr()->ensure_get_creator(libpath);
    }
    if(!creator) {
        set_error(_t("find ui creator failed."));
        return nullptr;
    }
    core_widget* cw = nullptr;
    ui_editor_layer* edit_control = nullptr;
    if(!parent->get_parent())
        cw = creator->fn_create_widget(wsys, parent, *coreit);
    else {
        string editor_name(_t("edit@"));
        editor_name.append(coreit->name);
        edit_control = wsys->add_widget<ui_editor_layer>(parent, editor_name.c_str(), rect(), sm_visible | sm_hitable);
        assert(edit_control);
        cw = creator->fn_create_widget(wsys, edit_control, *coreit);
    }
    if(!cw) {
        set_error(_t("create widget failed."));
        return nullptr;
    }
    if(edit_control)
        edit_control->initialize(*coreit);
    /* repeat recursively */
    for(auto ch = coreit.child(); ch; ch = ch.next())
        ui_create_widget(wsys, cw, ch);
    return cw;
}

bool ui_create_widgets(wsys_manager* wsys, core_widget* root_layer, ui_tree& uicore)
{
    assert(wsys);
    if(!uicore.is_valid())
        return false;
    ui_transparent_layer* inner_root = nullptr;
    if(!root_layer) {
        inner_root = wsys->add_widget<ui_transparent_layer>(nullptr, _t("ui_root"), rect(), sm_visible | sm_hitable);
        root_layer = inner_root;
        assert(root_layer);
    }
    auto* cw = ui_create_widget(wsys, root_layer, uicore.get_root());
    if(cw) {
        if(inner_root)
            inner_root->install_reflection_widget(cw);
        return true;
    }
    return false;
}

};
