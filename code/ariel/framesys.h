/*
 * Copyright (c) 2016-2017 lymastee, All rights reserved.
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
    feid_move,
    feid_key_down,
    feid_key_up,
    feid_char,
    feid_show,
    feid_create,
    feid_close,
    feid_size,
    feid_halt,
    feid_resume,
    feid_custom,
};

struct frame_event
{
    uint                id;
    uint                size;
    void*               ptr;

public:
    frame_event()
    {
        id = feid_invalid;
        size = 0;
        ptr = 0;
    }
    frame_event(uint i, uint s, void* p)
    {
        id = i;
        size = s;
        ptr = p;
    }
};

template<uint _feid, class _fepk>
struct frame_pack:
    public frame_event,
    public _fepk
{
public:
    frame_pack()
    {
        id = _feid;
        size = sizeof(_fepk);
        ptr = static_cast<_fepk*>(this);
    }
};

struct frame_pack_void {};
struct frame_pack_paint { rect boundary; };
struct frame_pack_timer { uint timerid; };
struct frame_pack_show { bool show; };
struct frame_pack_size { rect boundary; };
struct frame_pack_mouse_down
{
    uint            modifier;
    unikey          key;
    point           position;
};
struct frame_pack_mouse_up
{
    uint            modifier;
    unikey          key;
    point           position;
};
struct frame_pack_move
{
    uint            modifier;
    point           position;
};
struct frame_pack_key_down
{
    uint            modifier;
    unikey          key;
};
struct frame_pack_key_up
{
    uint            modifier;
    unikey          key;
};
struct frame_pack_char
{
    uint            modifier;
    uint            charactor;
};
struct frame_pack_create
{
    system_driver*  driver;
    rect            boundary;
};

template<frame_event_id _feid>
struct frame_pack_table;

#define register_frame_pack(feid, fepk) \
    template<> \
    struct frame_pack_table<feid> { \
        typedef frame_pack<feid, fepk> type; \
    };
register_frame_pack(feid_draw, frame_pack_void);
register_frame_pack(feid_timer, frame_pack_timer);
register_frame_pack(feid_paint, frame_pack_paint);
register_frame_pack(feid_mouse_down, frame_pack_mouse_down);
register_frame_pack(feid_mouse_up, frame_pack_mouse_up);
register_frame_pack(feid_move, frame_pack_move);
register_frame_pack(feid_key_down, frame_pack_key_down);
register_frame_pack(feid_key_up, frame_pack_key_up);
register_frame_pack(feid_char, frame_pack_char);
register_frame_pack(feid_show, frame_pack_show);
register_frame_pack(feid_create, frame_pack_create);
register_frame_pack(feid_close, frame_pack_void);
register_frame_pack(feid_size, frame_pack_size);
register_frame_pack(feid_halt, frame_pack_void);
register_frame_pack(feid_resume, frame_pack_void);
#undef register_frame_pack

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
    virtual void on_size(const rect& rc) override;
    virtual void on_paint(const rect& rc) override;
    virtual void on_halt() override;
    virtual void on_resume() override;
    virtual bool on_mouse_down(uint um, unikey uk, const point& pt) override;
    virtual bool on_mouse_up(uint um, unikey uk, const point& pt) override;
    virtual bool on_move(uint um, const point& pt) override;
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
