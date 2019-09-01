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

#ifndef classicstyle_de3b7d7b_438c_4dee_a91e_89f0051dcf28_h
#define classicstyle_de3b7d7b_438c_4dee_a91e_89f0051dcf28_h

#include <ariel/widget.h>
#include <ariel/style.h>
#include <ariel/classicstyle/slider_service.h>

/*
 * Default style for test and reference, flat style.
 * In this style we do not have picture fills in order to have a better performance.
 */

__ariel_begin__

namespace classic_style {

using gs::uuid;

class widget_style_sheet:
    public ariel::style_sheet
{
public:
    widget_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;

protected:
    color               _fill_color;
    color               _stroke_color;
    color               _disable_fill_color;
    color               _disable_stroke_color;
};

class root_widget_style_sheet:
    public ariel::style_sheet
{
public:
    root_widget_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;

protected:
    color               _bkground_color;
    string              _bkground_image;
};

class placeholder_style_sheet:
    public ariel::style_sheet
{
public:
    placeholder_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;

protected:
    color               _fill_color;
    color               _stroke_color;
    string              _remark;
    string              _remark_font_name;
    int                 _remark_font_size;
    color               _remark_font_color;
};

class button_style_sheet:
    public ariel::style_sheet
{
public:
    button_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;

protected:
    color               _normal_fill_color;
    color               _normal_stroke_color;
    color               _hover_fill_color;
    color               _hover_stroke_color;
    color               _press_fill_color;
    color               _press_stroke_color;
    color               _disable_fill_color;
    color               _disable_stroke_color;
    string              _caption;
    string              _caption_font_name;
    int                 _caption_font_size;
    color               _caption_font_color;
};

class edit_line_style_sheet:
    public ariel::style_sheet
{
public:
    edit_line_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;

protected:
    color               _normal_fill_color;
    color               _normal_stroke_color;
    color               _focus_fill_color;
    color               _focus_stroke_color;
    color               _disable_fill_color;
    color               _disable_stroke_color;
    string              _text_font_name;
    int                 _text_font_size;
    color               _text_font_color;
};

class menu_style_sheet:
    public ariel::style_sheet
{
    friend class menu_separator;
    friend class menu_cmd_item;
    friend class menu_sub_item;
    friend class menu_group;

public:
    menu_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;

protected:
    color               _normal_fill_color;
    color               _normal_stroke_color;
    color               _hover_fill_color;
    color               _hover_stroke_color;
    color               _press_fill_color;
    color               _press_stroke_color;
    color               _disable_fill_color;
    color               _disable_stroke_color;
    string              _caption;
    string              _caption_font_name;
    int                 _caption_font_size;
    color               _caption_font_color;
    color               _border_color;
    color               _separator_color;
    int                 _separator_space;
    int                 _text_horizontal_margin;
    int                 _text_vertical_margin;
    int                 _caption_reserved_space;
    int                 _accel_reserved_space;
};

class menu_cmd_style_sheet:
    public ariel::style_sheet
{
public:
    menu_cmd_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;

protected:
    string              _caption;
    accel_key           _accel_key;
};

class menu_sub_style_sheet:
    public ariel::style_sheet
{
public:
    menu_sub_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;

protected:
    string              _caption;
};

class menubar_style_sheet:
    public button_style_sheet
{
public:
    menubar_style_sheet();
};

class menubar_button_style_sheet:
    public ariel::style_sheet
{
public:
    menubar_button_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;

protected:
    string              _caption;
};

class tree_view_style_sheet:
    public ariel::style_sheet
{
public:
    tree_view_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;

protected:
    color               _view_fill_color;
    color               _view_stroke_color;
    color               _disable_fill_color;
    color               _disable_stroke_color;
    int                 _item_indent;
    int                 _item_spacing;
    color               _connect_line_color;
};

class widget:
    public ariel::widget,
    public widget_style_sheet
{
public:
    widget(wsys_manager* m): ariel::widget(m) {}
    virtual void* query_interface(const uuid& uid) override;
    virtual void draw(painter* paint) override;
    virtual void flush_style() override;

protected:
    painter_brush       _normal_brush;
    painter_pen         _normal_pen;
    painter_brush       _disable_brush;
    painter_pen         _disable_pen;
};

class root_widget:
    public ariel::widget,
    public root_widget_style_sheet
{
public:
    root_widget(wsys_manager* m): ariel::widget(m) {}
    virtual void* query_interface(const uuid& uid) override;
    virtual void draw(painter* paint) override;
    virtual void flush_style() override;

protected:
    com_ptr<texture2d>  _bktex;
};

class placeholder:
    public ariel::widget,
    public placeholder_style_sheet
{
public:
    placeholder(wsys_manager* m): ariel::widget(m) {}
    virtual void* query_interface(const uuid& uid) override;
    virtual void draw(painter* paint) override;
    virtual void flush_style() override;

protected:
    painter_brush       _normal_brush;
    painter_pen         _normal_pen;
    font                _remark_font;
};

class button:
    public ariel::button,
    public button_style_sheet
{
public:
    button(wsys_manager* m): ariel::button(m) {}
    virtual void* query_interface(const uuid& uid) override;
    virtual void enable(bool b) override;
    virtual void draw(painter* paint) override;
    virtual void flush_style() override;

protected:
    painter_brush       _normal_brush;
    painter_pen         _normal_pen;
    painter_brush       _hover_brush;
    painter_pen         _hover_pen;
    painter_brush       _press_brush;
    painter_pen         _press_pen;
    painter_brush       _disable_brush;
    painter_pen         _disable_pen;
    font                _caption_font;
    painter_brush       _current_brush;
    painter_pen         _current_pen;

protected:
    virtual void set_press() override;
    virtual void set_normal() override;
    virtual void set_hover() override;
    virtual void set_gray() override;
};

class edit_line:
    public ariel::edit_line,
    public edit_line_style_sheet
{
public:
    edit_line(wsys_manager* m): ariel::edit_line(m) {}
    virtual void* query_interface(const uuid& uid) override;
    virtual void draw(painter* paint) override;
    virtual void flush_style() override;

protected:
    painter_brush       _normal_brush;
    painter_pen         _normal_pen;
    painter_brush       _focus_brush;
    painter_pen         _focus_pen;
    painter_brush       _disable_brush;
    painter_pen         _disable_pen;
    font                _text_font;

protected:
    virtual void draw_background(painter* paint) override;
};

typedef slider_service<widget> splitter;

class horizontal_splitter:
    public splitter
{
public:
    horizontal_splitter(wsys_manager* m): splitter(m) {}
    virtual void on_hover(uint um, const point& pt) override;
    virtual void on_dragging(const point& pt) override;
};

class vertical_splitter:
    public splitter
{
public:
    vertical_splitter(wsys_manager* m): splitter(m) {}
    virtual void on_hover(uint um, const point& pt) override;
    virtual void on_dragging(const point& pt) override;
};

class __gs_novtable container abstract:
    public ariel::widget,
    public slider_notify
{
public:
    container(wsys_manager* m): ariel::widget(m) {}
    virtual ~container() {}
    virtual void move(const rect& rc) override;
    virtual void initialize_container() = 0;
    virtual void on_splitter_moved(splitter* sp) = 0;
    virtual void on_container_size_changed() = 0;
    virtual void on_slider_moved(slider* p) override { on_splitter_moved(static_cast<splitter*>(p)); }
};

class horizontal_layout_container:
    public container
{
public:
    horizontal_layout_container(wsys_manager* m);
    virtual void initialize_container() override;
    virtual void on_splitter_moved(splitter* sp) override;
    virtual void on_container_size_changed() override;

public:
    void set_left_sub_widget(ariel::widget* p) { _left_sub_widget = p; }
    void set_right_sub_widget(ariel::widget* p) { _right_sub_widget = p; }
    void set_split_percent(float t);
    float get_split_percent() const { return _split_percent; }
    splitter* get_splitter() const { return _splitter; }
    rect get_left_sub_rect() const;
    rect get_right_sub_rect() const;

protected:
    ariel::widget*      _left_sub_widget;
    ariel::widget*      _right_sub_widget;
    splitter*           _splitter;
    int                 _splitter_width;
    float               _split_percent;
    int                 _layout_width[2];

protected:
    void split_by_percent(float sp);
};

class vertical_layout_container:
    public container
{
public:
    vertical_layout_container(wsys_manager* m);
    virtual void initialize_container() override;
    virtual void on_splitter_moved(splitter* sp) override;
    virtual void on_container_size_changed() override;

public:
    void set_up_sub_widget(ariel::widget* p) { _up_sub_widget = p; }
    void set_down_sub_widget(ariel::widget* p) { _down_sub_widget = p; }
    void set_split_percent(float t);
    float get_split_percent() const { return _split_percent; }
    splitter* get_splitter() const { return _splitter; }
    rect get_up_sub_rect() const;
    rect get_down_sub_rect() const;

protected:
    ariel::widget*      _up_sub_widget;
    ariel::widget*      _down_sub_widget;
    splitter*           _splitter;
    int                 _splitter_width;
    float               _split_percent;
    int                 _layout_height[2];

protected:
    void split_by_percent(float sp);
};

typedef slider_service<button> scroller;

class horizontal_scroller:
    public scroller
{
public:
    horizontal_scroller(wsys_manager* m): scroller(m) {}
    virtual void on_dragging(const point& pt) override;
};

class vertical_scroller:
    public scroller
{
public:
    vertical_scroller(wsys_manager* m): scroller(m) {}
    virtual void on_dragging(const point& pt) override;
};

class scroll_button:
    public button
{
public:
    enum button_type
    {
        btn_up,
        btn_down,
        btn_left,
        btn_right,
    };

public:
    scroll_button(wsys_manager* m);
    virtual void enable(bool b) override;
    virtual void draw(painter* paint) override;
    void set_button_type(button_type btntype) { _btn_type = btntype; }

protected:
    painter_brush       _arrow_fill;
    button_type         _btn_type;

protected:
    void draw_up_arrow(painter* paint, const rectf& rc);
    void draw_down_arrow(painter* paint, const rectf& rc);
    void draw_left_arrow(painter* paint, const rectf& rc);
    void draw_right_arrow(painter* paint, const rectf& rc);
};

class __gs_novtable scrollbar abstract:
    public widget,
    public slider_notify
{
public:
    scrollbar(wsys_manager* m);
    virtual ~scrollbar() {}
    virtual void move(const rect& rc) override;
    virtual void enable(bool b) override;
    virtual void scroll_by(int by);
    virtual void initialize_scrollbar(int range) = 0;
    virtual void on_scrollbar_size_changed();
    virtual void on_inc_button_clicked(uint um, unikey uk, const point& pt);
    virtual void on_dec_button_clicked(uint um, unikey uk, const point& pt);
    virtual void set_canvas_range(int range);
    virtual void set_scroll_ratio(float t);
    virtual void adjust_scrollbar() = 0;
    /*
     * The difference between adjust_scrollbar & layout_scrollbar was:
     * adjust_scrollbar happens when only the scroll ratio changes, but the size of the scrollbar remains the same.
     * layout_scrollbar happens when the size of the scrollbar changes.
     */
    virtual void layout_scrollbar() = 0;
    virtual void on_slider_moved(slider* p) override;

public:
    void set_scrollbar_min_width(int w) { _min_width = w; }
    void set_scrollbar_min_height(int h) { _min_height = h; }
    int get_scrollbar_min_width() const { return _min_width; }
    int get_scrollbar_min_height() const { return _min_height; }
    int get_canvas_range() const { return _canvas_range; }
    float get_scroll_ratio() const { return _scroll_ratio; }
    void set_scroll_pace(int pace) { _scroll_pace = pace; }
    int get_scroll_pace() const { return _scroll_pace; }

protected:
    scroller*           _scroller;
    scroll_button*      _inc_button;
    scroll_button*      _dec_button;
    int                 _scroll_pace;
    int                 _canvas_range;
    float               _scroll_ratio;
    int                 _min_width;
    int                 _min_height;
    int                 _hori_margin;
    int                 _vert_margin;

protected:
    virtual float calc_scroll_ratio(int bias) const = 0;
    virtual void refresh_scroller_pos() = 0;
    void connect_scrollbar_notifications();
};

class horizontal_scrollbar:
    public scrollbar
{
public:
    horizontal_scrollbar(wsys_manager* m);
    virtual void initialize_scrollbar(int range) override;
    virtual void adjust_scrollbar() override;
    virtual void layout_scrollbar() override;

protected:
    virtual float calc_scroll_ratio(int bias) const override;
    virtual void refresh_scroller_pos() override;
    int calc_scroll_button_width() const;
    int calc_scroll_button_height() const;
    int calc_scroll_range() const;
    int calc_scroll_range_start() const;
};

class vertical_scrollbar:
    public scrollbar
{
public:
    vertical_scrollbar(wsys_manager* m);
    virtual void initialize_scrollbar(int range) override;
    virtual void adjust_scrollbar() override;
    virtual void layout_scrollbar() override;

protected:
    virtual float calc_scroll_ratio(int bias) const override;
    virtual void refresh_scroller_pos() override;
    int calc_scroll_button_width() const;
    int calc_scroll_button_height() const;
    int calc_scroll_range() const;
    int calc_scroll_range_start() const;
};

class tree_view:
    public ariel::widget,
    public tree_view_style_sheet
{
public:
    friend class raw_tree_view;

public:
    tree_view(wsys_manager* m);
    virtual ~tree_view();
    virtual void* query_interface(const uuid& uid) override;
    virtual void draw(painter* paint) override;

protected:
    raw_tree_view*      _raw_view;
    //ariel::scroller*    _;
    painter_brush       _view_brush;
    painter_pen         _view_pen;
};

class menu;
class menu_group;
class menu_cmd_item;

class __gs_novtable menu_cmd_notify abstract
{
public:
    virtual bool on_menu_command(menu_cmd_item* item) = 0;
};

class __gs_novtable menu_item abstract
{
public:
    enum item_type
    {
        separator,
        sub_item,
        cmd_item,
    };

public:
    menu_item(): _menu(nullptr) {}
    virtual ~menu_item() {}
    virtual item_type get_type() const = 0;
    virtual ariel::widget* to_widget() = 0;

protected:
    menu*               _menu;

public:
    void set_menu(menu* p) { _menu = p; }
};

class menu_separator:
    public menu_item,
    public ariel::widget
{
public:
    menu_separator(wsys_manager* m): ariel::widget(m) {}
    virtual item_type get_type() const override { return separator; }
    virtual ariel::widget* to_widget() override { return this; }
    virtual void* query_interface(const uuid& uid) override;
    virtual void draw(painter* paint) override;
};

class menu_sub_item:
    public menu_item,
    public ariel::widget,
    public menu_sub_style_sheet
{
public:
    menu_sub_item(wsys_manager* m): ariel::widget(m), _sub_menu(nullptr), _brush_ptr(nullptr), _pen_ptr(nullptr) {}
    virtual item_type get_type() const override { return sub_item; }
    virtual ariel::widget* to_widget() override { return this; }
    virtual void* query_interface(const uuid& uid) override;
    virtual void draw(painter* paint) override;
    virtual void on_press(uint um, unikey uk, const point& pt) override;
    virtual void on_click(uint um, unikey uk, const point& pt) override;
    virtual void on_hover(uint um, const point& pt) override;
    virtual void on_leave(uint um, const point& pt) override;
    virtual void flush_style() override;

protected:
    menu_group*         _sub_menu;
    painter_brush*      _brush_ptr;
    painter_pen*        _pen_ptr;

public:
    void set_sub_menu(menu_group* p) { _sub_menu = p; }
    menu_group* get_sub_menu() const { return _sub_menu; }
    void set_caption(const string& str) { _caption = str; }
    const string& get_caption() const { return _caption; }
    void get_caption_dimensions(int& w, int& h) const;
};

class menu_cmd_item:
    public menu_item,
    public ariel::widget,
    public menu_cmd_style_sheet
{
public:
    menu_cmd_item(wsys_manager* m): ariel::widget(m), _menu_notify(nullptr), _brush_ptr(nullptr), _pen_ptr(nullptr) {}
    virtual item_type get_type() const override { return cmd_item; }
    virtual ariel::widget* to_widget() override { return this; }
    virtual void* query_interface(const uuid& uid) override;
    virtual void draw(painter* paint) override;
    virtual void on_press(uint um, unikey uk, const point& pt) override;
    virtual void on_click(uint um, unikey uk, const point& pt) override;
    virtual void on_hover(uint um, const point& pt) override;
    virtual void on_leave(uint um, const point& pt) override;
    virtual void flush_style() override;

protected:
    menu_cmd_notify*    _menu_notify;
    painter_brush*      _brush_ptr;
    painter_pen*        _pen_ptr;

public:
    void set_cmd_notify(menu_cmd_notify* p) { _menu_notify = p; }
    void set_caption(const string& str) { _caption = str; }
    const string& get_caption() const { return _caption; }
    void set_accel_key(const accel_key& k) { _accel_key = k; }
    const accel_key& get_accel_key() const { return _accel_key; }
    void get_caption_dimensions(int& w, int& h) const;
    void get_accel_key_dimensions(int& w, int& h) const;
};

class menu_group:
    public ariel::widget
{
    friend class menu_separator;
    friend class menu_cmd_item;
    friend class menu_sub_item;

public:
    menu_group(wsys_manager* m): ariel::widget(m), _menu(nullptr), _upside_gap(-1) {}
    virtual void draw(painter* paint) override;
    virtual void refresh_menu_group_size();
    virtual menu_separator* add_separator() { return add_item<menu_separator>(sm_visible); }
    virtual menu_sub_item* add_sub_item()
    {
        auto* p = add_item<menu_sub_item>(sm_hitable|sm_visible);
        assert(p);
        p->flush_style();
        return p;
    }
    virtual menu_cmd_item* add_cmd_item();

protected:
    menu*               _menu;
    int                 _item_height;
    int                 _icon_width;
    int                 _caption_text_width;
    int                 _accel_text_width;
    int                 _upside_gap;

public:
    void set_menu(menu* p) { _menu = p; }
    menu_item* hit_item(const point& pt);
    void set_upside_gap(int gap) { _upside_gap = gap; }

protected:
    template<class _ctor>
    _ctor* add_item(uint sm);
};

class menu:
    public ariel::widget,
    public menu_style_sheet,
    public menu_cmd_notify
{
public:
    friend class menu_separator;
    friend class menu_cmd_item;
    friend class menu_sub_item;
    friend class menu_group;
    typedef vector<menu_group*> menu_stack;

public:
    menu(wsys_manager* m): ariel::widget(m), _last_hover(nullptr) {}
    virtual void* query_interface(const uuid& uid) override;
    virtual void close() override;
    virtual void show(bool b) override;
    virtual bool create(widget* ptr, const gchar* name, const rect& rc, uint style) override;
    virtual void on_press(uint um, unikey uk, const point& pt) override;
    virtual void on_click(uint um, unikey uk, const point& pt) override;
    virtual void on_hover(uint um, const point& pt) override;
    virtual void on_keydown(uint um, unikey uk) override;
    virtual void flush_style() override;
    virtual bool on_menu_command(menu_cmd_item* item) override { return false; }
    virtual void startup();
    virtual void on_sub_menu(menu_sub_item* item);
    virtual void on_startup_menu();
    virtual void refresh_menu_size();
    virtual menu_group* register_menu_group();
    virtual bool unregister_menu_group(menu_group* p);

protected:
    menu_stack          _menu_stack;
    painter_pen         _menu_border_pen;
    painter_brush       _menu_normal_brush;
    painter_pen         _menu_normal_pen;
    painter_brush       _menu_hover_brush;
    painter_pen         _menu_hover_pen;
    painter_brush       _menu_press_brush;
    painter_pen         _menu_press_pen;
    painter_brush       _menu_disable_brush;
    painter_pen         _menu_disable_pen;
    font                _menu_font;
    ariel::widget*      _last_hover;

public:
    menu_item* hit_item(const point& pt);
    menu_group* hit_group(const point& pt);
    menu_group* get_entry_group() const;

protected:
    void on_current_hover(ariel::widget* ptr, uint um, const point& pt);
    void on_select_menu_item(menu_item* item);
};

class menubar;

class menubar_button:
    public ariel::widget,
    public menubar_button_style_sheet
{
public:
    menubar_button(wsys_manager* m): ariel::widget(m), _menubar(nullptr), _menu(nullptr), _brush_ptr(nullptr), _pen_ptr(nullptr) {}
    virtual void* query_interface(const uuid& uid) override;
    virtual void draw(painter* paint) override;
    virtual void on_press(uint um, unikey uk, const point& pt) override;
    virtual void on_click(uint um, unikey uk, const point& pt) override;
    virtual void on_hover(uint um, const point& pt) override;
    virtual void on_leave(uint um, const point& pt) override;
    virtual void flush_style() override;
    virtual void refresh_menubar_button_size();

protected:
    menubar*            _menubar;
    menu*               _menu;
    painter_brush*      _brush_ptr;
    painter_pen*        _pen_ptr;

public:
    void set_menubar(menubar* ptr) { _menubar = ptr; }
    void set_menu(menu* ptr) { _menu = ptr; }
    void set_caption(const string& str) { _caption = str; }
    const string& get_caption() const { return _caption; }
};

class menubar:
    public ariel::widget,
    public menubar_style_sheet
{
public:
    friend class menubar_button;

public:
    menubar(wsys_manager* m): ariel::widget(m) {}
    virtual void* query_interface(const uuid& uid) override;
    virtual void flush_style() override;
    virtual void refresh_menubar_size();
    virtual menubar_button* register_menubar_button();
    virtual bool unregister_menubar_button(menubar_button* p);

protected:
    painter_brush       _menubar_normal_brush;
    painter_pen         _menubar_normal_pen;
    painter_brush       _menubar_hover_brush;
    painter_pen         _menubar_hover_pen;
    painter_brush       _menubar_press_brush;
    painter_pen         _menubar_press_pen;
    painter_brush       _menubar_disable_brush;
    painter_pen         _menubar_disable_pen;
    font                _menubar_font;
};

extern const uuid uuid_style_sheet;
extern const uuid uuid_menu_item;

/*
 * name_caption[necessary], accel_key[optional] : @command;
 * name_caption[necessary], accel_key[optional] : { ... };
 * [separator_name];
 */
extern bool create_menu_from_script(menu* pmenu, const string& script);

};

__ariel_end__

#endif
