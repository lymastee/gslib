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

#include <ariel/rendersys.h>

__ariel_begin__

rendersys::rendersys():
    _bkcr {0.f, 0.f, 0.f, 1.f}
{
}

bool rendersys::is_vsync_enabled(const configs& cfg)
{
    auto f = cfg.find(_t("vsync"));
    if(f == cfg.end())
        return false;
    return f->second == _t("true") || f->second == _t("1");
}


bool rendersys::is_full_screen(const configs& cfg)
{
    auto f = cfg.find(_t("fullscreen"));
    if(f == cfg.end())
        return false;
    return f->second == _t("true") || f->second == _t("1");
}

void rendersys::set_background_color(const color& cr)
{
    _bkcr[0] = (float)cr.red / 255.f;
    _bkcr[1] = (float)cr.green / 255.f;
    _bkcr[2] = (float)cr.blue / 255.f;
    _bkcr[3] = (float)cr.alpha / 255.f;
}

__ariel_end__
