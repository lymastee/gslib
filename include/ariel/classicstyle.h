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
    virtual int get_content_size() const override;
    virtual style_sheet_type get_content_type(int index) const override;
    virtual const string& get_content_name(int index) const override;

protected:
    color               _fill_color;
    color               _stroke_color;
};

class background_style_sheet:
    public ariel::style_sheet
{
public:
    background_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;
    virtual int get_content_size() const override;
    virtual style_sheet_type get_content_type(int index) const override;
    virtual const string& get_content_name(int index) const override;

protected:
    color               _bkground_color;
    string              _bkground_image;
};

class button_style_sheet:
    public ariel::style_sheet
{
public:
    button_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;
    virtual int get_content_size() const override;
    virtual style_sheet_type get_content_type(int index) const override;
    virtual const string& get_content_name(int index) const override;

protected:
    color               _normal_fill_color;
    color               _normal_stroke_color;
    color               _hover_fill_color;
    color               _hover_stroke_color;
    color               _press_fill_color;
    color               _press_stroke_color;
    string              _caption;
    string              _caption_font_name;
    int                 _caption_font_size;
    color               _caption_font_color;
};

class edit_style_sheet:
    public ariel::style_sheet
{
public:
    edit_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;
    virtual int get_content_size() const override;
    virtual style_sheet_type get_content_type(int index) const override;
    virtual const string& get_content_name(int index) const override;

protected:
    color               _normal_fill_color;
    color               _normal_stroke_color;
    color               _focused_fill_color;
    color               _focused_stroke_color;
    string              _text_font_name;
    int                 _text_font_size;
    color               _text_font_color;
};

class menu_style_sheet:
    public button_style_sheet
{
    friend class menu_separator;
    friend class menu_cmd_item;
    friend class menu_sub_item;
    friend class menu_group;

public:
    menu_style_sheet();
    virtual bool get_value(const string& name, string& value) override;
    virtual void set_value(const string& name, const string& value) override;
    virtual int get_content_size() const override;
    virtual style_sheet_type get_content_type(int index) const override;
    virtual const string& get_content_name(int index) const override;

protected:
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
    virtual int get_content_size() const override;
    virtual style_sheet_type get_content_type(int index) const override;
    virtual const string& get_content_name(int index) const override;

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
    virtual int get_content_size() const override;
    virtual style_sheet_type get_content_type(int index) const override;
    virtual const string& get_content_name(int index) const override;

protected:
    string              _caption;
};

class widget:
    public ariel::widget,
    public widget_style_sheet
{
public:
    widget(wsys_manager* m): ariel::widget(m) {}
    virtual void draw(painter* paint) override;
    virtual void flush_style() override;

protected:
    painter_brush       _normal_brush;
    painter_pen         _normal_pen;
};

class background:
    public ariel::widget,
    public background_style_sheet
{
public:
    background(wsys_manager* m): ariel::widget(m) {}
    virtual void draw(painter* paint) override;
    virtual void flush_style() override;

protected:
    com_ptr<texture2d>  _bktex;
};

class button:
    public ariel::button,
    public button_style_sheet
{
public:
    button(wsys_manager* m): ariel::button(m) {}
    virtual void draw(painter* paint) override;
    virtual void flush_style() override;

protected:
    painter_brush       _normal_brush;
    painter_pen         _normal_pen;
    painter_brush       _hover_brush;
    painter_pen         _hover_pen;
    painter_brush       _press_brush;
    painter_pen         _press_pen;
    font                _caption_font;
    painter_brush       _current_brush;
    painter_pen         _current_pen;

protected:
    virtual void set_press() override;
    virtual void set_normal() override;
    virtual void set_hover() override;
    virtual void set_gray() override;
};

class edit:
    public ariel::edit,
    public edit_style_sheet
{
public:
    edit(wsys_manager* m): ariel::edit(m) {}
    virtual void draw(painter* paint) override;
    virtual void flush_style() override;

protected:
    painter_brush       _normal_brush;
    painter_pen         _normal_pen;
    painter_brush       _focused_brush;
    painter_pen         _focused_pen;
    font                _text_font;

protected:
    virtual void draw_background(painter* paint) override;
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
    string              _caption;
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
    string              _caption;
    accel_key           _accel_key;
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
    menu_group(wsys_manager* m): ariel::widget(m), _menu(nullptr) {}
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

public:
    void set_menu(menu* p) { _menu = p; }
    menu_item* hit_item(const point& pt);

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
    font                _menu_font;
    ariel::widget*      _last_hover;

public:
    menu_item* hit_item(const point& pt);
    menu_group* hit_group(const point& pt);

protected:
    void on_current_hover(ariel::widget* ptr, uint um, const point& pt);
    void on_select_menu_item(menu_item* item);
};

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
