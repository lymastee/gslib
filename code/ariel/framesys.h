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

#ifndef framesys_300a49a6_c103_4a1f_8166_358e823a432b_h
#define framesys_300a49a6_c103_4a1f_8166_358e823a432b_h

#include <gslib/type.h>
#include <gslib/sysop.h>
#include <ariel/config.h>
#include <ariel/rendersys.h>

__ariel_begin__

using pink::point;
using pink::rect;
using pink::font;

enum frame_strategy
{
    fs_busy_loop,
    fs_lazy_passive,
};

class framesys;
struct frame_event;
class frame_listener;
class rose;

enum frame_event_id
{
    feid_invalid,
    feid_draw,
    feid_timer,
    feid_paint,
    feid_mouse_down,
    feid_mouse_up,
    feid_mouse_move,
    feid_key_down,
    feid_key_up,
    feid_char,
    feid_show,
    feid_create,
    feid_close,
    feid_resize,
    feid_halt,
    feid_resume,
    feid_custom,
};

struct __gs_novtable frame_event abstract
{
    virtual uint get_id() const = 0;
    virtual uint get_event_size() const = 0;
};

struct frame_draw_event:
    public frame_event
{
public:
    virtual uint get_id() const override { return feid_draw; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_close_event:
    public frame_event
{
public:
    virtual uint get_id() const override { return feid_close; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_halt_event:
    public frame_event
{
public:
    virtual uint get_id() const override { return feid_halt; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_resume_event:
    public frame_event
{
public:
    virtual uint get_id() const override { return feid_resume; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_paint_event:
    public frame_event
{
    rect            boundary;

public:
    virtual uint get_id() const override { return feid_paint; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_timer_event:
    public frame_event
{
    uint            timerid;

public:
    virtual uint get_id() const override { return feid_timer; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_show_event:
    public frame_event
{
    bool            show;

public:
    virtual uint get_id() const override { return feid_show; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_resize_event:
    public frame_event
{
    rect            boundary;

public:
    virtual uint get_id() const override { return feid_resize; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_mouse_down_event:
    public frame_event
{
    uint            modifier;
    unikey          key;
    point           position;

public:
    virtual uint get_id() const override { return feid_mouse_down; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_mouse_up_event:
    public frame_event
{
    uint            modifier;
    unikey          key;
    point           position;

public:
    virtual uint get_id() const override { return feid_mouse_up; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_mouse_move_event:
    public frame_event
{
    uint            modifier;
    point           position;

public:
    virtual uint get_id() const override { return feid_mouse_move; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_key_down_event:
    public frame_event
{
    uint            modifier;
    unikey          key;

public:
    virtual uint get_id() const override { return feid_key_down; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_key_up_event:
    public frame_event
{
    uint            modifier;
    unikey          key;

public:
    virtual uint get_id() const override { return feid_key_up; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_char_event:
    public frame_event
{
    uint            modifier;
    uint            charactor;

public:
    virtual uint get_id() const override { return feid_char; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

struct frame_create_event:
    public frame_event
{
    system_driver*  driver;
    rect            boundary;

public:
    virtual uint get_id() const override { return feid_create; }
    virtual uint get_event_size() const override { return sizeof(*this); }
};

template<frame_event_id _feid>
struct frame_event_table;

#define register_frame_event(feid, fevt) \
    template<> \
    struct frame_event_table<feid> { \
        typedef fevt type; \
    };
register_frame_event(feid_draw, frame_draw_event);
register_frame_event(feid_timer, frame_timer_event);
register_frame_event(feid_paint, frame_paint_event);
register_frame_event(feid_mouse_down, frame_mouse_down_event);
register_frame_event(feid_mouse_up, frame_mouse_up_event);
register_frame_event(feid_mouse_move, frame_mouse_move_event);
register_frame_event(feid_key_down, frame_key_down_event);
register_frame_event(feid_key_up, frame_key_up_event);
register_frame_event(feid_char, frame_char_event);
register_frame_event(feid_show, frame_show_event);
register_frame_event(feid_create, frame_create_event);
register_frame_event(feid_close, frame_close_event);
register_frame_event(feid_resize, frame_resize_event);
register_frame_event(feid_halt, frame_halt_event);
register_frame_event(feid_resume, frame_resume_event);
#undef register_frame_event

class __gs_novtable frame_listener abstract
{
public:
    virtual ~frame_listener() {}
    virtual void on_frame_start() = 0;
    virtual void on_frame_end() = 0;
    virtual bool on_frame_event(const frame_event& event) = 0;
};

class frame_dispatcher:
    public system_notify
{
public:
    virtual void on_show(bool b) override;
    virtual void on_create(system_driver* ptr, const rect& rc) override;
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
    frame_listener*     _listener;

public:
    frame_dispatcher() { _listener = 0; }
    void set_listener(frame_listener* listenter) { _listener = listenter; }
    void do_draw();
    void on_frame_start();
    void on_frame_end();
};

struct frame_configs
{
    uint                handles;
    int                 width;
    int                 height;
    /* more to come.. */
};

class framesys:
    public system_driver
{
public:
    friend class frame_dispatcher;
    typedef void (framesys::*fn_empty_frame)();

public:
    void set_frame_strategy(frame_strategy stt);
    void set_notify(system_notify* notify) { _notify = notify; }
    void set_frame_listener(frame_listener* lis) { _dispatcher.set_listener(lis); }
    system_notify* get_notify() const { return _notify; }
    rendersys* get_rendersys() const { return _rendersys; }
    rose* get_rose() const { return _rose; }
    const frame_configs& get_configs() const { return _configs; }
    void initialize(const rect& rc);
    void refresh();

public:
    virtual ~framesys();
    virtual void initialize(const system_context& ctx) override;
    virtual void setup() override;
    virtual void close() override;
    virtual void set_timer(uint tid, int t) override;
    virtual void kill_timer(uint tid) override;
    virtual void update() override;
    virtual void emit(int msgid, void* msg, int size) override;
    virtual void set_ime(point pt, const font& ft) override;

private:
    framesys();

public:
    static int run();
    static framesys* get_framesys()
    {
        static framesys inst;
        return &inst;
    }

protected:
    void idle()
    {
        assert(_empty_frame_proc);
        (this->*_empty_frame_proc)();
    }
    void empty_frame_lazy() {}
    void empty_frame_busy();

protected:
    frame_strategy      _strategy;
    frame_configs       _configs;
    frame_dispatcher    _dispatcher;
    system_notify*      _notify;
    rendersys*          _rendersys;
    rose*               _rose;
    fn_empty_frame      _empty_frame_proc;
};

__ariel_end__

#endif
