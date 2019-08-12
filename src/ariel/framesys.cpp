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

#include <ariel/framesys.h>
#include <ariel/rendersysd3d11.h>
#include <ariel/scene.h>
#include <ariel/rose.h>
#include <gslib/dvt.h>

__ariel_begin__

void frame_dispatcher::on_show(bool b)
{
    frame_event_table<feid_show>::type event;
    event.show = b;
    assert(_listener);
    _listener->on_frame_event(event);
}

void frame_dispatcher::on_create(system_driver* ptr, const rect& rc)
{
    frame_event_table<feid_create>::type event;
    event.driver = ptr;
    event.boundary = rc;
    assert(_listener);
    _listener->on_frame_event(event);
}

void frame_dispatcher::on_close()
{
    frame_event_table<feid_close>::type event;
    assert(_listener);
    _listener->on_frame_event(event);
}

void frame_dispatcher::on_resize(const rect& rc)
{
    frame_event_table<feid_resize>::type event;
    event.boundary = rc;
    assert(_listener);
    _listener->on_frame_event(event);
}

void frame_dispatcher::on_paint(const rect& rc)
{
    frame_event_table<feid_paint>::type event;
    event.boundary = rc;
    assert(_listener);
    _listener->on_frame_event(event);
}

void frame_dispatcher::on_halt()
{
    frame_event_table<feid_halt>::type event;
    assert(_listener);
    _listener->on_frame_event(event);
}

void frame_dispatcher::on_resume()
{
    frame_event_table<feid_resume>::type event;
    assert(_listener);
    _listener->on_frame_event(event);
}

bool frame_dispatcher::on_mouse_down(uint um, unikey uk, const point& pt)
{
    frame_event_table<feid_mouse_down>::type event;
    event.modifier = um;
    event.key = uk;
    event.position = pt;
    assert(_listener);
    return _listener->on_frame_event(event);
}

bool frame_dispatcher::on_mouse_up(uint um, unikey uk, const point& pt)
{
    frame_event_table<feid_mouse_up>::type event;
    event.modifier = um;
    event.key = uk;
    event.position = pt;
    assert(_listener);
    return _listener->on_frame_event(event);
}

bool frame_dispatcher::on_mouse_move(uint um, const point& pt)
{
    frame_event_table<feid_mouse_move>::type event;
    event.modifier = um;
    event.position = pt;
    assert(_listener);
    return _listener->on_frame_event(event);
}

bool frame_dispatcher::on_key_down(uint um, unikey uk)
{
    frame_event_table<feid_key_down>::type event;
    event.modifier = um;
    event.key = uk;
    assert(_listener);
    return _listener->on_frame_event(event);
}

bool frame_dispatcher::on_key_up(uint um, unikey uk)
{
    frame_event_table<feid_key_up>::type event;
    event.modifier = um;
    event.key = uk;
    assert(_listener);
    return _listener->on_frame_event(event);
}

bool frame_dispatcher::on_char(uint um, uint ch)
{
    frame_event_table<feid_char>::type event;
    event.modifier = um;
    event.charactor = ch;
    assert(_listener);
    return _listener->on_frame_event(event);
}

void frame_dispatcher::on_timer(uint tid)
{
    frame_event_table<feid_timer>::type event;
    event.timerid = tid;
    assert(_listener);
    _listener->on_frame_event(event);
}

void frame_dispatcher::do_draw()
{
    frame_event_table<feid_draw>::type event;
    assert(_listener);
    _listener->on_frame_event(event);
}

void frame_dispatcher::on_frame_start()
{
    assert(_listener);
    _listener->on_frame_start();
}

void frame_dispatcher::on_frame_end()
{
    assert(_listener);
    _listener->on_frame_end();
}

framesys::framesys()
{
    _notify = &_dispatcher;
    _rendersys = new rendersys_d3d11;
    _rose = new rose;
    _empty_frame_proc = nullptr;
    memset(&_configs, 0, sizeof(_configs));
    set_frame_strategy(fs_busy_loop);
    set_frame_listener(scene::get_singleton_ptr());
}

framesys::~framesys()
{
    if(_rose) {
        delete _rose;
        _rose = nullptr;
    }
    if(_rendersys) {
        delete _rendersys;
        _rendersys = nullptr;
    }
}

void framesys::initialize(const rect& rc)
{
    system_context ctx;
    ctx.mask = system_context::sct_rectangle;
    ctx.rectangle = rc;
    initialize(ctx);
    assert(_rendersys && _configs.handles);
    rendersys::configs cfgs;
    _rendersys->setup(_configs.handles, cfgs);
    assert(_rose);
    _rose->setup_dimensions(rc.width(), rc.height());
    _rose->setup(_rendersys);
    setup();
}

void framesys::refresh()
{
    _dispatcher.do_draw();
}

void framesys::set_frame_strategy(frame_strategy stt)
{
    _strategy = stt;
    switch(stt)
    {
    case fs_busy_loop:
        _empty_frame_proc = &framesys::empty_frame_busy;
        break;
    case fs_lazy_passive:
        _empty_frame_proc = &framesys::empty_frame_lazy;
        break;
    }
}

void framesys::empty_frame_lazy()
{
    dvt_collector::get_singleton_ptr()->cleanup();
}

void framesys::empty_frame_busy()
{
    _dispatcher.on_frame_start();
    _dispatcher.do_draw();
    _dispatcher.on_frame_end();
    dvt_collector::get_singleton_ptr()->cleanup();
}

__ariel_end__
