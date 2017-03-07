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

#include <ariel/scene.h>
#include <ariel/rose.h>

__ariel_begin__

stage::stage()
{
    _prev_presentation_stage =
        _next_presentation_stage =
        _prev_notification_stage =
        _next_notification_stage = 0;
}

void stage::connect_presentation_order(stage* stg1, stage* stg2)
{
    if(stg1 != 0)
        stg1->_next_presentation_stage = stg2;
    if(stg2 != 0)
        stg2->_prev_presentation_stage = stg1;
}

void stage::connect_notification_order(stage* stg1, stage* stg2)
{
    if(stg1 != 0)
        stg1->_next_notification_stage = stg2;
    if(stg2 != 0)
        stg2->_prev_notification_stage = stg1;
}

void stage::connect_presentation_order(stage* stg1, stage* stg2, stage* stg3)
{
    connect_presentation_order(stg1, stg2);
    connect_presentation_order(stg2, stg3);
}

void stage::connect_notification_order(stage* stg1, stage* stg2, stage* stg3)
{
    connect_notification_order(stg1, stg2);
    connect_notification_order(stg2, stg3);
}

static void detach_presentation_node(stage* stg)
{
    assert(stg);
    stage* prev = stg->prev_presentation_stage();
    stage* next = stg->next_presentation_stage();
    stage::connect_presentation_order(prev, next);
    stage::connect_presentation_order(0, stg, 0);
}

static void detach_notification_node(stage* stg)
{
    assert(stg);
    stage* prev = stg->prev_notification_stage();
    stage* next = stg->next_notification_stage();
    stage::connect_notification_order(prev, next);
    stage::connect_notification_order(0, stg, 0);
}

static void insert_presentation_before(stage* pos, stage* tar)
{
    assert(pos && tar);
    stage::connect_presentation_order(
        pos->prev_presentation_stage(),
        tar, pos
        );
}

static void insert_presentation_after(stage* pos, stage* tar)
{
    assert(pos && tar);
    stage::connect_presentation_order(
        pos, tar,
        pos->next_presentation_stage()
        );
}

static void insert_notification_before(stage* pos, stage* tar)
{
    assert(pos && tar);
    stage::connect_notification_order(
        pos->prev_notification_stage(),
        tar, pos
        );
}

static void insert_notification_after(stage* pos, stage* tar)
{
    assert(pos && tar);
    stage::connect_notification_order(
        pos, tar,
        pos->next_notification_stage()
        );
}

void stage::on_next_draw()
{
    if(_next_presentation_stage)
        _next_presentation_stage->draw();
}

void stage::on_next_frame_start()
{
    if(_next_notification_stage)
        _next_notification_stage->on_frame_start();
}

void stage::on_next_frame_end()
{
    if(_next_notification_stage)
        _next_notification_stage->on_frame_end();
}

bool stage::on_next_event(const frame_event& event)
{
    if(_next_notification_stage) {
        if(_next_notification_stage->on_frame_event(event))
            return true;
    }
    return false;
}

bool ui_stage::setup()
{
    framesys* sys = framesys::get_framesys();
    const frame_configs& cfgs = sys->get_configs();
    _wsys_manager.set_painter(_rose);
    _wsys_manager.set_wsysdrv(sys);
    _wsys_manager.set_dimension(cfgs.width, cfgs.height);
    return true;
}

void ui_stage::draw()
{
    _wsys_manager.refresh();
    _wsys_manager.update();
    on_next_draw();
}

bool ui_stage::on_frame_event(const frame_event& event)
{
    assert(event.id != feid_invalid);
    switch(event.id)
    {
    case feid_timer:
        {
            auto p = static_cast<frame_pack_timer*>(event.ptr);
            _wsys_manager.on_timer(p->timerid);
            return true;
        }
    case feid_paint:
        {
            auto p = static_cast<frame_pack_size*>(event.ptr);
            _wsys_manager.on_paint(p->boundary);
            return true;
        }
    case feid_mouse_down:
        {
            auto p = static_cast<frame_pack_mouse_down*>(event.ptr);
            return _wsys_manager.on_mouse_down(p->modifier, p->key, p->position);
        }
    case feid_mouse_up:
        {
            auto p = static_cast<frame_pack_mouse_up*>(event.ptr);
            return _wsys_manager.on_mouse_up(p->modifier, p->key, p->position);
        }
    case feid_move:
        {
            auto p = static_cast<frame_pack_move*>(event.ptr);
            return _wsys_manager.on_move(p->modifier, p->position);
        }
    case feid_key_down:
        {
            auto p = static_cast<frame_pack_key_down*>(event.ptr);
            return _wsys_manager.on_key_down(p->modifier, p->key);
        }
    case feid_key_up:
        {
            auto p = static_cast<frame_pack_key_up*>(event.ptr);
            return _wsys_manager.on_key_up(p->modifier, p->key);
        }
    case feid_char:
        {
            auto p = static_cast<frame_pack_char*>(event.ptr);
            return _wsys_manager.on_char(p->modifier, p->charactor);
        }
    case feid_show:
        {
            auto p = static_cast<frame_pack_show*>(event.ptr);
            _wsys_manager.on_show(p->show);
            return true;
        }
    case feid_create:
        {
            auto p = static_cast<frame_pack_create*>(event.ptr);
            _wsys_manager.on_create(p->driver, p->boundary);
            return true;
        }
    case feid_close:
        {
            _wsys_manager.on_close();
            return true;
        }
    case feid_size:
        {
            auto p = static_cast<frame_pack_size*>(event.ptr);
            _wsys_manager.on_size(p->boundary);
            return true;
        }
    case feid_halt:
        {
            _wsys_manager.on_halt();
            return true;
        }
    case feid_resume:
        {
            _wsys_manager.on_resume();
            return true;
        }
    }
    return on_next_event(event);
}

scene::scene()
{
    _rendersys = 0;
    _rose = 0;
    _notify = 0;
    _present = 0;
    _uisys = 0;
}

scene::~scene()
{
    destroy();
}

void scene::setup()
{
    assert(_rendersys && _rose);
    ui_stage* ui = gs_new(ui_stage, _rose);
    add_stage(ui);
    ui->setup();
    _notify = _present = ui;
    _uisys = ui->get_wsys_manager();
}

void scene::destroy()
{
    destroy_all_stages();
    _rendersys = 0;
    _rose = 0;
    _uisys = 0;
}

void scene::destroy_all_stages()
{
    std::for_each(_stages.begin(), _stages.end(), [](stage* stg) {
        assert(stg);
        gs_del(stage, stg);
    });
    _stages.clear();
}

stage* scene::get_stage(const gchar* name)
{
    assert(name);
    string str(name);
    auto i = std::find_if(_stages.begin(), _stages.end(), [&str](stage* stg)->bool {
        assert(stg);
        return str == stg->get_name();
    });
    if(i == _stages.end()) {
        assert(!"no such stage.");
        return 0;
    }
    return *i;
}

template<class _fnrouter>
static bool is_my_stage(stage* stg, stage* first, _fnrouter router)
{
    assert(stg && first);
    stage* test = first;
    while(test) {
        if(test == stg)
            return true;
        test = router(test);
    }
    return false;
}

template<class _fnrouter1, class _fnrouter2>
static bool is_connected(stage* stg, _fnrouter1 router1, _fnrouter2 router2)
{
    if(!stg)
        return false;
    return router1(stg) || router2(stg);
}

static bool is_presentation_connected(stage* stg)
{
    return is_connected(stg,
        [](stage* stg)->stage* { return stg->prev_presentation_stage(); },
        [](stage* stg)->stage* { return stg->next_presentation_stage(); }
        );
}

static bool is_notification_connected(stage* stg)
{
    return is_connected(stg,
        [](stage* stg)->stage* { return stg->prev_notification_stage(); },
        [](stage* stg)->stage* { return stg->next_notification_stage(); }
        );
}

void scene::set_presentation_before(stage* pos, stage* tar)
{
    assert(pos && tar);
    assert(is_my_stage(pos, _present, [](stage* stg)->stage* {
        return stg->next_presentation_stage();
    }));
    if(is_presentation_connected(tar))
        detach_presentation_node(tar);
    stage* prev = pos->prev_presentation_stage();
    insert_presentation_before(pos, tar);
    if(!prev) {
        assert(pos == _present);
        _present = tar;
    }
}

void scene::set_presentation_after(stage* pos, stage* tar)
{
    assert(pos && tar);
    assert(is_my_stage(pos, _present, [](stage* stg)->stage* {
        return stg->next_presentation_stage();
    }));
    if(is_presentation_connected(tar))
        detach_presentation_node(tar);
    insert_presentation_after(pos, tar);
}

void scene::set_notification_before(stage* pos, stage* tar)
{
    assert(pos && tar);
    assert(is_my_stage(pos, _notify, [](stage* stg)->stage* {
        return stg->next_notification_stage();
    }));
    if(is_notification_connected(tar))
        detach_notification_node(tar);
    stage* prev = pos->prev_notification_stage();
    insert_notification_before(pos, tar);
    if(!prev) {
        assert(pos == _notify);
        _notify = tar;
    }
}

void scene::set_notification_after(stage* pos, stage* tar)
{
    assert(pos && tar);
    assert(is_my_stage(pos, _notify, [](stage* stg)->stage* {
        return stg->next_notification_stage();
    }));
    if(is_notification_connected(tar))
        detach_notification_node(tar);
    insert_notification_after(pos, tar);
}

void scene::draw()
{
    assert(_rendersys);
    _rendersys->begin_render();
    assert(_present);
    _present->draw();
    _rendersys->end_render();
}

void scene::on_frame_start()
{
    if(_notify)
        _notify->on_frame_start();
}

void scene::on_frame_end()
{
    if(_notify)
        _notify->on_frame_end();
}

bool scene::on_frame_event(const frame_event& event)
{
    if(event.id == feid_draw) {
        draw();
        return true;
    }
    else if(event.id == feid_paint) {
        if(!_rendersys || !_rose || !_present)
            return false;
        draw();
        return true;
    }
    return _notify ? _notify->on_frame_event(event) : false;
}

__ariel_end__
