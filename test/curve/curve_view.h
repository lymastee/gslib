#pragma once

#include <ariel/classicstyle.h>
#include "editor.h"

__ariel_begin__

typedef classic_style::button my_button;
//typedef classic_style::menu my_menu;

// 555, 骚不动了，这里的动态虚表dsm取地址时有点问题，以后再调，先这么搞一下
class curve_view;
class my_menu:
    public classic_style::menu
{
public:
    my_menu(wsys_manager* m): classic_style::menu(m) {}
    void set_host(curve_view* cv) { _host = cv; }
    virtual bool on_menu_command(classic_style::menu_cmd_item* item) override;

private:
    curve_view*         _host = nullptr;
};

class curve_view:
    public widget
{
public:
    curve_view(wsys_manager* m): gs::ariel::widget(m) {}
    virtual bool create(widget* ptr, const gchar* name, const rect& rc, uint style) override;
    virtual void draw(painter* paint) override;
    virtual void on_press(uint um, unikey uk, const point& pt) override;
    virtual void on_click(uint um, unikey uk, const point& pt) override;
    virtual void on_hover(uint um, const point& pt) override;
    virtual void on_keydown(uint um, unikey uk) override;
    void on_click_btn_reset(uint um, unikey uk, const point& pt);
    bool on_menu_cmd(classic_style::menu_cmd_item* item);

protected:
    curve_editor        _editor;
    my_menu*            _popup_menu = nullptr;
    bool                _is_left_pressed = false;
    cv_draw_extra       _cde = cde_nothing;

private:
    void setup_sub_widgets();
    void show_popup_at(const point& p);
};

__ariel_end__
