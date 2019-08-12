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

#include <ariel/style.h>
#include <ariel/widget.h>

__ariel_begin__

const string& get_style_sheet_type_name(style_sheet_type sst)
{
    static const string sst_names[] =
    {
        _t("color"),
        _t("integer"),
        _t("boolean"),
        _t("float"),
        _t("string"),
    };
    return sst_names[sst];
}

style_sheet::style_sheet()
{
}

bool style_sheet::from_color(string& str, const color& cr)
{
    str.format(_t("rgb(%d,%d,%d)"), (int)cr.red, (int)cr.green, (int)cr.blue);
    return true;
}

bool style_sheet::to_color(color& cr, const string& str)
{
    if(str.empty())
        return false;
    if(str.at(0) == _t('#')) {      /* #rrggbb, in hex */
        if(str.size() == 7) {
            cr.red = string(&str.at(1), 2).to_int(16);
            cr.green = string(&str.at(3), 2).to_int(16);
            cr.blue = string(&str.at(5), 2).to_int(16);
            return true;
        }
        return false;
    }
    if(str.compare_cl(_t("rgb"), 3) == 0) {     /* rgb(r,g,b) */
        int r, g, b;
        if(strtool::sscanf(&str.at(3), _t("(%d,%d,%d)"), &r, &g, &b) == 3) {
            cr.red = r;
            cr.green = g;
            cr.blue = b;
            return true;
        }
        return false;
    }
    return false;
}

bool style_sheet::from_integer(string& str, int i)
{
    str.from_int(i);
    return true;
}

bool style_sheet::to_integer(int& i, const string& str)
{
    i = str.to_int(10);
    return true;
}

bool style_sheet::from_boolean(string& str, bool b)
{
    b ? str.assign(_t("true")) : str.assign(_t("false"));
    return true;
}

bool style_sheet::to_boolean(bool& b, const string& str)
{
    if(str == _t("true") || str == _t("1")) {
        b = true;
        return true;
    }
    else if(str == _t("false") || str == _t("0")) {
        b = false;
        return false;
    }
    return false;
}

bool style_sheet::from_float(string& str, float f)
{
    str.from_real(f);
    return true;
}

bool style_sheet::to_float(float& f, const string& str)
{
    f = (float)str.to_real();
    return !isnan(f) && !isinf(f);
}

bool style_sheet::from_accel_key(string& str, const accel_key& k)
{
    k.to_string(str);
    return true;
}

bool style_sheet::to_accel_key(accel_key& k, const string& str)
{
    return k.from_string(str);
}

void style_sheet::setup_brush_by_color(painter_brush& brush, const color& cr)
{
    if(!cr.alpha)
        brush.set_tag(painter_brush::none);
    else {
        brush.set_tag(painter_brush::solid);
        brush.set_color(cr);
    }
}

void style_sheet::setup_pen_by_color(painter_pen& pen, const color& cr)
{
    if(!cr.alpha)
        pen.set_tag(painter_pen::none);
    else {
        pen.set_tag(painter_pen::solid);
        pen.set_color(cr);
    }
}

void style_sheet::setup_font(font& ft, const string& name, int size)
{
    ft.name = name;
    ft.size = size;
}

__ariel_end__
