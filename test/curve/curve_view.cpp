#include "curve_view.h"

#define refresh_tag     false

__ariel_begin__

bool my_menu::on_menu_command(classic_style::menu_cmd_item* item)
{
    if(_host)
        return _host->on_menu_cmd(item);
    return false;
}

bool curve_view::create(widget* ptr, const gchar* name, const rect& rc, uint style)
{
    if(__super::create(ptr, name, rc, style)) {
        setup_sub_widgets();
        return true;
    }
    return false;
}

void curve_view::draw(painter* paint)
{
    assert(paint);
    _editor.draw(paint);
    if(_cde != cde_nothing)
        _editor.draw_extra(paint, _cde);
}

void curve_view::on_press(uint um, unikey uk, const point& pt)
{
    __super::on_press(um, uk, pt);
    if(uk == mk_left) {
        _editor.press_at_point(vec2((float)pt.x, (float)pt.y));
        _is_left_pressed = true;
        refresh(refresh_tag);
    }
}

void curve_view::on_click(uint um, unikey uk, const point& pt)
{
    __super::on_click(um, uk, pt);
    if(uk == mk_left) {
        _editor.release_at_point(vec2((float)pt.x, (float)pt.y));
        _is_left_pressed = false;
        refresh(refresh_tag);
    }
    else if(uk == mk_right)
        show_popup_at(pt);
}

void curve_view::on_hover(uint um, const point& pt)
{
    __super::on_hover(um, pt);
    if(_is_left_pressed) {
        _editor.move_point(vec2((float)pt.x, (float)pt.y));
        refresh(refresh_tag);
    }
}

void curve_view::on_keydown(uint um, unikey uk)
{
    switch(uk)
    {
    case uk_cr:
        _editor.confirm();
        return;
    case uk_esc:
        _editor.cancel();
        return;
    }
    __super::on_keydown(um, uk);
}

void curve_view::on_click_btn_reset(uint um, unikey uk, const point& pt)
{
    _editor.reset();
    refresh(refresh_tag);
}

bool curve_view::on_menu_cmd(classic_style::menu_cmd_item* item)
{
    // todo: 这里也有问题，menu cmd在解析时被丢弃了，暂时只能取caption来匹配命令，以后要补上
    const string& cmd = item->get_caption();
    if(cmd == _t("Show Inflections"))
        _cde = cde_inflections;
    else if(cmd == _t("Show Extrema"))
        _cde = cde_extrema;
    else if(cmd == _t("Show Self-intersection"))
        _cde = cde_self_intersection;
    else if(cmd == _t("Show Cubic to quad"))
        _cde = cde_cubic_to_quad;
    else if(cmd == _t("Show Intersections"))
        _cde = cde_intersections;
    else
        _cde = cde_nothing;
    _popup_menu->hide();
    refresh(refresh_tag);
    return true;
}

void curve_view::setup_sub_widgets()
{
    assert(_manager);
    auto btn_reset = _manager->add_widget<my_button>(this, _t("btn_reset"), rect(25, 25, 60, 27), sm_visible | sm_hitable);
    assert(btn_reset);
    btn_reset->set_value(_t("caption"), _t("reset"));
    btn_reset->flush_style();

    connect_notify(btn_reset, &my_button::on_click, this, &curve_view::on_click_btn_reset, 12);
    assert(!_popup_menu);
    _popup_menu = _manager->add_widget<my_menu>(this, _t("popup_menu"), rect(100, 100, 1, 1), sm_hitable | sm_visible);
    assert(_popup_menu);
    _popup_menu->flush_style();
    classic_style::create_menu_from_script(_popup_menu,
        _t("Nothing,:@ShowNothingCmd;")
        _t("Show Inflections,:@ShowInflectionsCmd;")
        _t("Show Extrema,:@ShowExtremaCmd;")
        _t("Show Self-intersection,:@;")
        _t("Show Cubic to quad,:@;")
        _t("Show Intersections,:@;")
        );
    _popup_menu->set_host(this);
    //connect_notify(_popup_menu, &my_menu::on_menu_command, this, &curve_view::on_menu_cmd, 4);
}

void curve_view::show_popup_at(const point& p)
{
    if(!_popup_menu)
        return;
    _popup_menu->move(p);
    _popup_menu->startup();
}

__ariel_end__
