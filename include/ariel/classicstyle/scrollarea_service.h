/*
 * Copyright (c) 2016-2021 lymastee, All rights reserved.
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

#ifndef scrollarea_service_c9e01511_1d52_4a84_aef7_8e422cca025f_h
#define scrollarea_service_c9e01511_1d52_4a84_aef7_8e422cca025f_h

#include <ariel/painterport.h>
#include <ariel/classicstyle/scrollarea_service.h>

__ariel_begin__

namespace classic_style {

template<class _base_widget>
class __gs_novtable scrollarea_service abstract:
    public _base_widget
{
public:
    scrollarea_service(wsys_manager* m): _base_widget(m)
    {
    }
    void set_scroll_area(const rectf& rc) { _scroll_area = rc; }
    void set_scroll_viewport(const rectf& rc) { _scroll_viewport = rc; }

public:
    virtual void draw(painter* paint) override
    {
    }

protected:
    painterport         _painter;
    rectf               _scroll_area;
    rectf               _scroll_viewport;
};

};

__ariel_end__

#endif
