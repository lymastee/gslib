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

#ifndef widget_c3b57c97_cea3_4794_b683_762a7b11af49_h
#define widget_c3b57c97_cea3_4794_b683_762a7b11af49_h

#include <gslib/std.h>
#include <gslib/dvt.h>
#include <ariel/sysop.h>
#include <ariel/painter.h>

__ariel_begin__

class widget;

class wsys_manager;

class widget:
    public notify_holder
{
    friend class wsys_manager;

public:
    widget(wsys_manager* m);
    virtual ~widget();
    virtual const gchar* get_type() const { return _t("widget"); }
    virtual bool create(widget* ptr, const gchar* name, const rect& rc, uint style);
    virtual void close();
    virtual void show(bool b);
    virtual void enable(bool b);
    virtual void move(const rect& rc);
    virtual void refresh(const rect& rc, bool imm = false);
    virtual void draw(painter* paint) {}
    virtual widget* capture(bool b);
    virtual widget* focus();
    virtual bool hit_test(const point& pt) { return (_style & sm_hitable) != 0; }

public:
    enum laytag
    {
        lay_before,
        lay_after,
        lay_first,
        lay_last,
    };
    virtual void lay(widget* ptr, laytag t);

public:
    virtual void on_press(uint um, unikey uk, const point& pt);
    virtual void on_click(uint um, unikey uk, const point& pt);
    virtual void on_hover(uint um, const point& pt);
    virtual void on_leave(uint um, const point& pt);
    virtual void on_keydown(uint um, unikey uk) {}
    virtual void on_keyup(uint um, unikey uk) {}
    virtual void on_char(uint um, uint ch) {}
    virtual void on_caret() {}
    virtual void on_focus(bool b) {}
    virtual void on_scroll(const point& pt, real32 scr, bool vert) {}

protected:
    wsys_manager*   _manager;
    string          _name;
    uint            _style;
    rect            _pos;
    bool            _show;
    bool            _enable;
    point           _htpos;

public:
    const string& get_name() const { return _name; }
    const rect& get_rect() const { return _pos; }
    bool is_visible() const { return _show && (_style & sm_visible); }
    bool is_enable() const { return _enable; }
    point& top_level(point& pt) const;
    rect& top_level(rect& rc) const;
    void move(const point& pt);
    void refresh(bool imm);
    int get_width() const { return _pos.width(); }
    int get_height() const { return _pos.height(); }

protected:
    widget*         _last;
    widget*         _next;
    widget*         _child;
    widget*         _parent;

public:
    widget* get_parent() const { return _parent; }
    widget* get_last() const { return _last; }
    widget* get_next() const { return _next; }
    widget* get_child() const { return _child; }
};

class button:
    public widget
{
public:
    typedef widget superref;
    typedef button self;

public:
    button(wsys_manager* m);
    virtual ~button();
    virtual const gchar* get_type() const override { return _t("button"); }
    virtual void draw(painter* paint) override;
    virtual void enable(bool b) override;
    virtual void on_press(uint um, unikey uk, const point& pt) override;
    virtual void on_click(uint um, unikey uk, const point& pt) override;
    virtual void on_hover(uint um, const point& pt) override;
    virtual void on_leave(uint um, const point& pt) override;

public:
    void set_image(texture2d* img, bool fs = false);
    void set_press();
    void set_normal();
    void set_hover();
    void set_gray();

protected:
    texture2d*      _source;
    texture2d*      _bkground;
    bool            _4states;

public:
    enum btnstate
    {
        bs_none     = 0,
        bs_press,
        bs_hover,
        bs_normal,
    };
    void set_btnstate(btnstate bs) { _btnstate = bs; }
    btnstate get_btnstate() const { return _btnstate; }

protected:
    btnstate        _btnstate;
};

class edit:
    public widget
{
public:
    typedef widget superref;
    typedef edit self;

public:
    edit(wsys_manager* m);
    virtual const gchar* get_type() const override { return _t("edit"); }
    virtual bool create(widget* ptr, const gchar* name, const rect& rc, uint style) override;
    virtual void draw(painter* paint) override;
    virtual void on_press(uint um, unikey uk, const point& pt) override;
    virtual void on_click(uint um, unikey uk, const point& pt) override;
    virtual void on_hover(uint um, const point& pt) override;
    virtual void on_char(uint um, uint ch) override;
    virtual void on_keydown(uint um, unikey uk) override;
    virtual void on_caret() override;
    virtual void on_focus(bool b) override;

protected:
    bool            _caret_on;
    int             _caretpos;
    string          _textbuf;
    int             _sel_start;
    int             _sel_end;

public:
    virtual void set_text(const gchar* str);
    virtual void set_caret(int n);
    virtual void set_select(int start, int end);
    virtual void replace_select(const gchar* str);

public:
    void set_font(const font& ft) { _font = ft; }
    void set_bkground(texture2d* ptr) { _bkground = ptr; }
    void set_text_color(color cr) { _txtcolor = cr; }
    void set_select_color(color cr) { _selcolor = cr; }
    void set_caret_color(color cr) { _crtcolor = cr; }
    const gchar* get_text() const { return _textbuf.c_str(); }
    void del_select();
    int hit_char(point pt);
    int prev_char(int pos);
    int next_char(int pos);
    int prev_logic_char(int pos);
    int next_logic_char(int pos);
    bool no_select() const { return _sel_start == -1 || _sel_start == _sel_end; }

protected:
    texture2d*      _bkground;
    color           _txtcolor;
    color           _selcolor;
    color           _crtcolor;
    font            _font;
    int             _font_idx;

protected:
    int trim_if_overrun(bool alarm);
};

class scroller:
    public button
{
public:
    typedef button superref;
    typedef scroller self;

public:
    scroller(wsys_manager* m);
    void set_scroller(int r1, int r2, bool vts, texture2d* img, bool as = false);
    void set_scroll(real32 s);
    real32 get_scroll() const { return _scrpos; }
    virtual const gchar* get_type() const override { return _t("scroller"); }
    virtual bool create(widget* ptr, const gchar* name, const rect& rc, uint style) override;
    virtual void on_hover(uint um, const point& pt) override;

protected:
    int             _rangemin, _rangemax;
    real32          _scrpos;
    bool            _vtscroll;
};

typedef system_driver wsys_driver;
typedef system_notify wsys_notify;

class timer:
    public notify_holder
{
public:
    timer(wsys_manager* mgr);
    void start(int elapse);
    void start_single(int elapse);
    void set_user_data(uint usd) { _userdata = usd; }
    uint get_user_data() const { return _userdata; }

public:
    virtual ~timer();
    virtual void on_timer(uint tid) {}
    virtual void on_notified();             /* CANNOT be used by connect notify */

protected:
    wsys_driver*    _driver;
    uint            _userdata;
    int             _elapse;
    bool            _single;

private:
    void initialize(wsys_driver* drv);
};

class wsys_manager:
    public wsys_notify
{
public:
    wsys_manager();
    void set_wsysdrv(wsys_driver* drv);
    void set_painter(painter* paint);
    painter* get_painter() const { return _painter; }
    wsys_driver* get_driver() const { return _driver; }
    void initialize(const rect& rc);

protected:
    wsys_driver*    _driver;
    painter*        _painter;
    dirty_list      _dirty;
    int             _width;
    int             _height;
    timer*          _caret;

public:
    void set_dimension(int w, int h);
    int get_width() const { return _width; }
    int get_height() const { return _height; }
    void refresh() { _dirty.set_whole(); }
    void refresh(const rect& rc, bool imm = false);
    void update();
    void update(widget* w);
    widget* hit_test(point pt) { return hit_test(_root, pt); }
    widget* hit_test(widget* ptr, point pt);
    widget* hit_proc(const point& pt, point& pt1);
    widget* set_capture(widget* ptr, bool b);
    widget* set_focus(widget* ptr);
    void on_caret(uint);

public:
    virtual ~wsys_manager();
    virtual void on_show(bool b) override;
    virtual void on_create(wsys_driver* ptr, const rect& rc) override;
    virtual void on_close() override;
    virtual void on_resize(const rect& rc) override;
    virtual void on_paint(const rect& rc) override;
    virtual void on_halt() override;
    virtual void on_resume() override;
    virtual bool on_mouse_down(uint um, unikey uk, const point& pt) override;
    virtual bool on_mouse_up(uint um, unikey uk, const point& pt) override;
    virtual bool on_mouse_move(uint um, const point& pt) override;
    virtual bool on_key_down(uint um, unikey uk) override;
    virtual bool on_key_up(uint um, unikey uk) override;
    virtual bool on_char(uint um, uint ch) override;
    virtual void on_timer(uint tid) override;

protected:
    widget*         _hover;
    widget*         _capture;
    widget*         _focus;
    widget*         _root;

private:
    typedef unordered_map<string, widget*> widget_map;
    widget_map      _widget_map;

public:
    template<class _ctor>
    _ctor* add_widget(widget* ptr, const gchar* name, const rect& rc, uint style)
    {
        if(name && _widget_map.find(name) != _widget_map.end())
            return 0;
        if(!ptr && _root)
            ptr = _root;
        _ctor* p = gs_new(_ctor, this);
        assert(p);
        if(!p->create(ptr, name ? name : _t(""), rc, style)) {
            gs_del(_ctor, p);
            return 0;
        }
        if(name != 0) {
            _widget_map.insert(
                std::make_pair(name, p)
                );
        }
        if(!ptr && !_root)
            _root = p;
        return p;
    }
    widget* get_root() const { return _root; }
    widget* find_widget(const string& name);
    bool remove_widget(widget* ptr);
    bool remove_widget(const string& name);
    bool remove_widget(widget_map::iterator i);

public:
    void set_ime(widget* ptr, point pt, const font& ft);
    void set_clipboard(clipfmt fmt, const void* ptr, int size);
    int get_clipboard(clipfmt fmt, const void*& ptr);
    int get_clipboard(clipboard_list& cl, int c);
};

__ariel_end__

#endif
