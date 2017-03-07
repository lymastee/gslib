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

#ifndef scene_048bb735_09b4_43d2_9ff5_5e40441c230b_h
#define scene_048bb735_09b4_43d2_9ff5_5e40441c230b_h

#include <gslib/std.h>
#include <ariel/config.h>
#include <ariel/rendersys.h>
#include <ariel/framesys.h>
#include <pink/widget.h>

__ariel_begin__

using pink::widget;
using pink::fontsys;
using pink::wsys_manager;

class rose;
class stage;
typedef list<stage*> stages;

class __gs_novtable stage abstract:
    public frame_listener
{
public:
    stage();
    virtual ~stage() {}
    virtual const gchar* get_name() const = 0;
    virtual bool setup() = 0;
    virtual void draw() = 0;

public:
    template<class _sty> inline
    _sty* as() { return static_cast<_sty*>(this); }
    template<class _sty> inline
    const _sty* as_const() const { return static_cast<const _sty*>(this); }

protected:
    stage*              _prev_presentation_stage;
    stage*              _next_presentation_stage;
    stage*              _prev_notification_stage;
    stage*              _next_notification_stage;

protected:
    void on_next_draw();
    void on_next_frame_start();
    void on_next_frame_end();
    bool on_next_event(const frame_event& event);

public:
    stage* prev_presentation_stage() const { return _prev_presentation_stage; }
    stage* next_presentation_stage() const { return _next_presentation_stage; }
    stage* prev_notification_stage() const { return _prev_notification_stage; }
    stage* next_notification_stage() const { return _next_notification_stage; }

public:
    static void connect_presentation_order(stage* stg1, stage* stg2);
    static void connect_notification_order(stage* stg1, stage* stg2);
    static void connect_presentation_order(stage* stg1, stage* stg2, stage* stg3);
    static void connect_notification_order(stage* stg1, stage* stg2, stage* stg3);
};

class ui_stage:
    public stage
{
public:
    ui_stage(rose* r) { _rose = r; }
    virtual const gchar* get_name() const override { return _t("ui"); }
    virtual bool setup() override;
    virtual void draw() override;
    virtual void on_frame_start() override { on_next_frame_start(); }
    virtual void on_frame_end() override { on_next_frame_end(); }
    virtual bool on_frame_event(const frame_event& event) override;

public:
    wsys_manager* get_wsys_manager() { return &_wsys_manager; }

protected:
    rose*               _rose;
    wsys_manager        _wsys_manager;
};

class scene:
    public frame_listener
{
public:
    static scene* get_singleton_ptr()
    {
        static scene inst;
        return &inst;
    }

private:
    scene();

public:
    ~scene();
    stage* get_stage(const gchar* name);
    wsys_manager* get_ui_system() const { return _uisys; }
    void set_rendersys(rendersys* rsys) { _rendersys = rsys; }
    void set_rose(rose* ptr) { _rose = ptr; }
    void setup();
    void destroy();
    void destroy_all_stages();
    void add_stage(stage* stg) { _stages.push_back(stg); }
    void set_presentation_before(stage* pos, stage* tar);
    void set_presentation_after(stage* pos, stage* tar);
    void set_notification_before(stage* pos, stage* tar);
    void set_notification_after(stage* pos, stage* tar);

protected:
    rendersys*          _rendersys;
    rose*               _rose;
    stages              _stages;
    wsys_manager*       _uisys;
    stage*              _notify;
    stage*              _present;

protected:
    void draw();

public:
    virtual void on_frame_start() override;
    virtual void on_frame_end() override;
    virtual bool on_frame_event(const frame_event& event) override;
};

__ariel_end__

#endif
