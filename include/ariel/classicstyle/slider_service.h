/*
 * Copyright (c) 2016-2019 lymastee, All rights reserved.
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

#pragma once

#ifndef slider_service_a46966fa_b9ea_4ff9_9148_d2856688b8be_h
#define slider_service_a46966fa_b9ea_4ff9_9148_d2856688b8be_h

#include <ariel/widget.h>
#include <ariel/style.h>

__ariel_begin__

namespace classic_style {

class __gs_novtable slider abstract {};

class __gs_novtable slider_notify abstract
{
public:
    virtual void on_slider_moved(slider* p) = 0;
};

template<class _base_widget>
class __gs_novtable slider_service abstract:
    public _base_widget,
    public slider
{
public:
    slider_service(wsys_manager* m): _base_widget(m)
    {
        _slider_notify = nullptr;
        _is_dragging = false;
        _min_range = 0;
        _max_range = 100;
    }
    void set_slider_notify(slider_notify* p) { _slider_notify = p; }
    void set_range(int min_range, int max_range) { _min_range = min_range, _max_range = max_range; }

public:
    virtual void on_press(uint um, unikey uk, const point& pt) override
    {
        __super::on_press(um, uk, pt);
        start_dragging(pt);
    }
    virtual void on_click(uint um, unikey uk, const point& pt) override
    {
        __super::on_click(um, uk, pt);
        end_dragging(pt);
    }
    virtual void on_hover(uint um, const point& pt) override
    {
        __super::on_hover(um, pt);
        if(_is_dragging)
            on_dragging(pt);
    }
    virtual void on_leave(uint um, const point& pt) override
    {
        __super::on_leave(um, pt);
        assert(_manager);
        _manager->reset_cursor();
    }
    virtual void on_dragging(const point& pt) = 0;

protected:
    slider_notify*      _slider_notify;
    bool                _is_dragging;
    point               _init_drag_pt;
    int                 _min_range;
    int                 _max_range;

protected:
    void start_dragging(const point& pt)
    {
        if(_is_dragging)
            return;
        _is_dragging = true;
        _init_drag_pt = pt;
        capture(true);
    }
    void end_dragging(const point& pt)
    {
        capture(false);
        if(!_is_dragging)
            return;
        on_dragging(pt);
        _is_dragging = false;
    }
};

};

__ariel_end__

#endif
