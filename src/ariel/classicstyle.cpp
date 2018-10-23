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

#include <ariel/classicstyle.h>
#include <ariel/raster.h>
#include <ariel/scene.h>
#include <ariel/rendersys.h>
#include <ariel/textureop.h>

__ariel_begin__

namespace classic_style {

const uuid uuid_menu_item(_t("5dca28c1-47c4-4ea8-8fac-440f2b57c2bc"));

static const style_sheet_pair __widget_style_sheet_pairs[] =
{
    std::make_pair(sst_color, _t("fill_color")),
    std::make_pair(sst_color, _t("stroke_color")),
    std::make_pair(sst_float, _t("fill_opacity")),
    std::make_pair(sst_float, _t("stroke_opacity")),
};

static const style_sheet_pair __background_style_sheet_pairs[] =
{
    std::make_pair(sst_color, _t("background_color")),
    std::make_pair(sst_string, _t("background_image")),
};

static const style_sheet_pair __button_style_sheet_pairs[] =
{
    std::make_pair(sst_color, _t("normal_fill_color")),
    std::make_pair(sst_color, _t("normal_stroke_color")),
    std::make_pair(sst_color, _t("hover_fill_color")),
    std::make_pair(sst_color, _t("hover_stroke_color")),
    std::make_pair(sst_color, _t("press_fill_color")),
    std::make_pair(sst_color, _t("press_stroke_color")),
    std::make_pair(sst_float, _t("normal_fill_opacity")),
    std::make_pair(sst_float, _t("normal_stroke_opacity")),
    std::make_pair(sst_float, _t("hover_fill_opacity")),
    std::make_pair(sst_float, _t("hover_stroke_opacity")),
    std::make_pair(sst_float, _t("press_fill_opacity")),
    std::make_pair(sst_float, _t("press_stroke_opacity")),
    std::make_pair(sst_string, _t("caption")),
    std::make_pair(sst_string, _t("caption_font_name")),
    std::make_pair(sst_integer, _t("caption_font_size")),
    std::make_pair(sst_string, _t("caption_font_color")),
};

static const style_sheet_pair __edit_style_sheet_pairs[] =
{
    std::make_pair(sst_color, _t("normal_fill_color")),
    std::make_pair(sst_color, _t("normal_stroke_color")),
    std::make_pair(sst_float, _t("normal_fill_opacity")),
    std::make_pair(sst_float, _t("normal_stroke_opacity")),
    std::make_pair(sst_color, _t("focused_fill_color")),
    std::make_pair(sst_color, _t("focused_stroke_color")),
    std::make_pair(sst_float, _t("focused_fill_opacity")),
    std::make_pair(sst_float, _t("focused_stroke_opacity")),
    std::make_pair(sst_string, _t("text_font_name")),
    std::make_pair(sst_integer, _t("text_font_size")),
    std::make_pair(sst_color, _t("text_font_color")),
    std::make_pair(sst_float, _t("text_font_opacity")),
};

static const style_sheet_pair __menu_style_sheet_pairs[] =
{
    std::make_pair(sst_color, _t("border_color")),
    std::make_pair(sst_color, _t("separator_color")),
    std::make_pair(sst_integer, _t("separator_space")),
    std::make_pair(sst_integer, _t("text_horizontal_margin")),
    std::make_pair(sst_integer, _t("text_vertical_margin")),
    std::make_pair(sst_integer, _t("caption_reserved_space")),
    std::make_pair(sst_integer, _t("accel_reserved_space")),
};

static const style_sheet_pair __menu_cmd_style_sheet_pairs[] =
{
    std::make_pair(sst_string, _t("caption")),
    std::make_pair(sst_string, _t("accel_key")),
};

static const style_sheet_pair __menu_sub_style_sheet_pairs[] =
{
    std::make_pair(sst_string, _t("caption")),
};

static const style_sheet_pair __menubar_button_style_sheet_pairs[] =
{
    std::make_pair(sst_string, _t("caption")),
};

widget_style_sheet::widget_style_sheet()
{
    _fill_color = color(245, 246, 247);
    _stroke_color = color(245, 246, 247, 0);
}

bool widget_style_sheet::get_value(const string& name, string& value)
{
    if(name == __widget_style_sheet_pairs[0].second)
        return from_color(value, _fill_color);
    else if(name == __widget_style_sheet_pairs[1].second)
        return from_color(value, _stroke_color);
    else if(name == __widget_style_sheet_pairs[2].second)
        return from_float(value, (float)_fill_color.alpha / 255.f);
    else if(name == __widget_style_sheet_pairs[3].second)
        return from_float(value, (float)_stroke_color.alpha / 255.f);
    return false;
}

void widget_style_sheet::set_value(const string& name, const string& value)
{
    if(name == __widget_style_sheet_pairs[0].second)
        verify(to_color(_fill_color, value));
    else if(name == __widget_style_sheet_pairs[1].second)
        verify(to_color(_stroke_color, value));
    else if(name == __widget_style_sheet_pairs[2].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for widget_style_sheet.");
            return;
        }
        _fill_color.alpha = round(f * 255.f);
    }
    else if(name == __widget_style_sheet_pairs[3].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for widget_style_sheet.");
            return;
        }
        _stroke_color.alpha = round(f * 255.f);
    }
    else {
        assert(!"unexpected style sheet name.");
        return;
    }
}

int widget_style_sheet::get_content_size() const
{
    return _countof(__widget_style_sheet_pairs);
}

style_sheet_type widget_style_sheet::get_content_type(int index) const
{
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return sst_unknown;
    }
    return __widget_style_sheet_pairs[index].first;
}

const string& widget_style_sheet::get_content_name(int index) const
{
    static const string err(_t("unknown name."));
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return err;
    }
    return __widget_style_sheet_pairs[index].second;
}

background_style_sheet::background_style_sheet()
{
    _bkground_color = color(245, 246, 247);
}

bool background_style_sheet::get_value(const string& name, string& value)
{
    if(name == __background_style_sheet_pairs[0].second)
        return from_color(value, _bkground_color);
    else if(name == __background_style_sheet_pairs[1].second) {
        value = _bkground_image;
        return true;
    }
    return false;
}

void background_style_sheet::set_value(const string& name, const string& value)
{
    if(name == __background_style_sheet_pairs[0].second)
        verify(to_color(_bkground_color, value));
    else if(name == __background_style_sheet_pairs[1].second)
        _bkground_image = value;
    else {
        assert(!"unexpected style sheet name.");
        return;
    }
}

int background_style_sheet::get_content_size() const
{
    return _countof(__background_style_sheet_pairs);
}

style_sheet_type background_style_sheet::get_content_type(int index) const
{
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return sst_unknown;
    }
    return __background_style_sheet_pairs[index].first;
}

const string& background_style_sheet::get_content_name(int index) const
{
    static const string err(_t("unknown name."));
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return err;
    }
    return __background_style_sheet_pairs[index].second;
}

button_style_sheet::button_style_sheet()
{
    _normal_fill_color = color(245, 246, 247);
    _normal_stroke_color = color(245, 246, 247, 0);
    _hover_fill_color = color(232, 239, 247);
    _hover_stroke_color = color(164, 206, 249);
    _press_fill_color = color(201, 224, 247);
    _press_stroke_color = color(98, 162, 228);
    _caption_font_name.assign(_t("Tahoma"));
    _caption_font_size = 14;
    _caption_font_color = color(0, 0, 0);
}

bool button_style_sheet::get_value(const string& name, string& value)
{
    if(name == __button_style_sheet_pairs[0].second)
        return from_color(value, _normal_fill_color);
    else if(name == __button_style_sheet_pairs[1].second)
        return from_color(value, _normal_stroke_color);
    else if(name == __button_style_sheet_pairs[2].second)
        return from_color(value, _hover_fill_color);
    else if(name == __button_style_sheet_pairs[3].second)
        return from_color(value, _hover_stroke_color);
    else if(name == __button_style_sheet_pairs[4].second)
        return from_color(value, _press_fill_color);
    else if(name == __button_style_sheet_pairs[5].second)
        return from_color(value, _press_stroke_color);
    else if(name == __button_style_sheet_pairs[6].second)
        return from_float(value, (float)_normal_fill_color.alpha / 255.f);
    else if(name == __button_style_sheet_pairs[7].second)
        return from_float(value, (float)_normal_stroke_color.alpha / 255.f);
    else if(name == __button_style_sheet_pairs[8].second)
        return from_float(value, (float)_hover_fill_color.alpha / 255.f);
    else if(name == __button_style_sheet_pairs[9].second)
        return from_float(value, (float)_hover_stroke_color.alpha / 255.f);
    else if(name == __button_style_sheet_pairs[10].second)
        return from_float(value, (float)_press_fill_color.alpha / 255.f);
    else if(name == __button_style_sheet_pairs[11].second)
        return from_float(value, (float)_press_stroke_color.alpha / 255.f);
    else if(name == __button_style_sheet_pairs[12].second) {
        value = _caption;
        return true;
    }
    else if(name == __button_style_sheet_pairs[13].second) {
        value = _caption_font_name;
        return true;
    }
    else if(name == __button_style_sheet_pairs[14].second)
        return from_integer(value, _caption_font_size);
    else if(name == __button_style_sheet_pairs[15].second)
        return from_color(value, _caption_font_color);
    return false;
}

void button_style_sheet::set_value(const string& name, const string& value)
{
    if(name == __button_style_sheet_pairs[0].second)
        verify(to_color(_normal_fill_color, value));
    else if(name == __button_style_sheet_pairs[1].second)
        verify(to_color(_normal_stroke_color, value));
    else if(name == __button_style_sheet_pairs[2].second)
        verify(to_color(_hover_fill_color, value));
    else if(name == __button_style_sheet_pairs[3].second)
        verify(to_color(_hover_stroke_color, value));
    else if(name == __button_style_sheet_pairs[4].second)
        verify(to_color(_press_fill_color, value));
    else if(name == __button_style_sheet_pairs[5].second)
        verify(to_color(_press_stroke_color, value));
    else if(name == __button_style_sheet_pairs[6].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for button_style_sheet.");
            return;
        }
        _normal_fill_color.alpha = round(f * 255.f);
    }
    else if(name == __button_style_sheet_pairs[7].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for button_style_sheet.");
            return;
        }
        _normal_stroke_color.alpha = round(f * 255.f);
    }
    else if(name == __button_style_sheet_pairs[8].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for button_style_sheet.");
            return;
        }
        _hover_fill_color.alpha = round(f * 255.f);
    }
    else if(name == __button_style_sheet_pairs[9].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for button_style_sheet.");
            return;
        }
        _hover_stroke_color.alpha = round(f * 255.f);
    }
    else if(name == __button_style_sheet_pairs[10].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for button_style_sheet.");
            return;
        }
        _press_fill_color.alpha = round(f * 255.f);
    }
    else if(name == __button_style_sheet_pairs[11].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for button_style_sheet.");
            return;
        }
        _press_stroke_color.alpha = round(f * 255.f);
    }
    else if(name == __button_style_sheet_pairs[12].second)
        _caption = value;
    else if(name == __button_style_sheet_pairs[13].second)
        _caption_font_name = value;
    else if(name == __button_style_sheet_pairs[14].second) {
        int i;
        verify(to_integer(i, value));
        _caption_font_size = i;
    }
    else if(name == __button_style_sheet_pairs[15].second)
        verify(to_color(_caption_font_color, value));
    else {
        assert(!"unexpected style sheet name.");
        return;
    }
}

int button_style_sheet::get_content_size() const
{
    return _countof(__button_style_sheet_pairs);
}

style_sheet_type button_style_sheet::get_content_type(int index) const
{
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return sst_unknown;
    }
    return __button_style_sheet_pairs[index].first;
}

const string& button_style_sheet::get_content_name(int index) const
{
    static const string err(_t("unknown name."));
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return err;
    }
    return __button_style_sheet_pairs[index].second;
}

edit_style_sheet::edit_style_sheet()
{
    _normal_fill_color = color(255, 255, 255);
    _normal_stroke_color = color(23, 23, 23);
    _focused_fill_color = color(255, 255, 255);
    _focused_stroke_color = color(0, 120, 215);
    _text_font_name.assign(_t("simsun"));
    _text_font_size = 14;
    _text_font_color = color(0, 0, 0);
}

bool edit_style_sheet::get_value(const string& name, string& value)
{
    if(name == __edit_style_sheet_pairs[0].second)
        return from_color(value, _normal_fill_color);
    else if(name == __edit_style_sheet_pairs[1].second)
        return from_color(value, _normal_stroke_color);
    else if(name == __edit_style_sheet_pairs[2].second)
        return from_float(value, (float)_normal_fill_color.alpha / 255.f);
    else if(name == __edit_style_sheet_pairs[3].second)
        return from_float(value, (float)_normal_stroke_color.alpha / 255.f);
    else if(name == __edit_style_sheet_pairs[4].second)
        return from_color(value, _focused_fill_color);
    else if(name == __edit_style_sheet_pairs[5].second)
        return from_color(value, _focused_stroke_color);
    else if(name == __edit_style_sheet_pairs[6].second)
        return from_float(value, (float)_focused_fill_color.alpha / 255.f);
    else if(name == __edit_style_sheet_pairs[7].second)
        return from_float(value, (float)_focused_stroke_color.alpha / 255.f);
    else if(name == __edit_style_sheet_pairs[8].second) {
        value = _text_font_name;
        return true;
    }
    else if(name == __edit_style_sheet_pairs[9].second)
        return from_integer(value, _text_font_size);
    else if(name == __edit_style_sheet_pairs[10].second)
        return from_color(value, _text_font_color);
    else if(name == __edit_style_sheet_pairs[11].second)
        return from_float(value, (float)_text_font_color.alpha / 255.f);
    return false;
}

void edit_style_sheet::set_value(const string& name, const string& value)
{
    if(name == __edit_style_sheet_pairs[0].second)
        verify(to_color(_normal_fill_color, value));
    else if(name == __edit_style_sheet_pairs[1].second)
        verify(to_color(_normal_stroke_color, value));
    else if(name == __edit_style_sheet_pairs[2].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for edit_style_sheet.");
            return;
        }
        _normal_fill_color.alpha = round(f * 255.f);
    }
    else if(name == __edit_style_sheet_pairs[3].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for edit_style_sheet.");
            return;
        }
        _normal_stroke_color.alpha = round(f * 255.f);
    }
    else if(name == __edit_style_sheet_pairs[4].second)
        verify(to_color(_focused_fill_color, value));
    else if(name == __edit_style_sheet_pairs[5].second)
        verify(to_color(_focused_stroke_color, value));
    else if(name == __edit_style_sheet_pairs[6].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for edit_style_sheet.");
            return;
        }
        _focused_fill_color.alpha = round(f * 255.f);
    }
    else if(name == __edit_style_sheet_pairs[7].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for edit_style_sheet.");
            return;
        }
        _focused_stroke_color.alpha = round(f * 255.f);
    }
    else if(name == __edit_style_sheet_pairs[8].second)
        _text_font_name = value;
    else if(name == __edit_style_sheet_pairs[9].second)
        verify(to_integer(_text_font_size, value));
    else if(name == __edit_style_sheet_pairs[10].second)
        verify(to_color(_text_font_color, value));
    else if(name == __edit_style_sheet_pairs[11].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for edit_style_sheet.");
            return;
        }
        _text_font_color.alpha = round(f * 255.f);
    }
    else {
        assert(!"unexpected style sheet name.");
        return;
    }
}

int edit_style_sheet::get_content_size() const
{
    return _countof(__edit_style_sheet_pairs);
}

style_sheet_type edit_style_sheet::get_content_type(int index) const
{
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return sst_unknown;
    }
    return __edit_style_sheet_pairs[index].first;
}

const string& edit_style_sheet::get_content_name(int index) const
{
    static const string err(_t("unknown name."));
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return err;
    }
    return __edit_style_sheet_pairs[index].second;
}

menu_style_sheet::menu_style_sheet()
{
    _border_color = color(53, 53, 53);
    _separator_color = color(190, 195, 203);
    _separator_space = 1;
    _text_horizontal_margin = 5;
    _text_vertical_margin = 5;
    _caption_reserved_space = 40;
    _accel_reserved_space = 20;
    _caption_font_name.assign(_t("Tahoma"));
    _caption_font_size = 14;
    _normal_fill_color = color(255, 255, 255);
    _hover_fill_color = _press_fill_color;
    _hover_stroke_color = _press_stroke_color;
}

bool menu_style_sheet::get_value(const string& name, string& value)
{
    if(name == __menu_style_sheet_pairs[0].second)
        return from_color(value, _border_color);
    else if(name == __menu_style_sheet_pairs[1].second)
        return from_color(value, _separator_color);
    else if(name == __menu_style_sheet_pairs[2].second)
        return from_integer(value, _separator_space);
    else if(name == __menu_style_sheet_pairs[3].second)
        return from_integer(value, _text_horizontal_margin);
    else if(name == __menu_style_sheet_pairs[4].second)
        return from_integer(value, _text_vertical_margin);
    else if(name == __menu_style_sheet_pairs[5].second)
        return from_integer(value, _caption_reserved_space);
    else if(name == __menu_style_sheet_pairs[6].second)
        return from_integer(value, _accel_reserved_space);
    return button_style_sheet::get_value(name, value);
}

void menu_style_sheet::set_value(const string& name, const string& value)
{
    if(name == __menu_style_sheet_pairs[0].second)
        verify(to_color(_border_color, value));
    else if(name == __menu_style_sheet_pairs[1].second)
        verify(to_color(_separator_color, value));
    else if(name == __menu_style_sheet_pairs[2].second)
        verify(to_integer(_separator_space, value));
    else if(name == __menu_style_sheet_pairs[3].second)
        verify(to_integer(_text_horizontal_margin, value));
    else if(name == __menu_style_sheet_pairs[4].second)
        verify(to_integer(_text_vertical_margin, value));
    else if(name == __menu_style_sheet_pairs[5].second)
        verify(to_integer(_caption_reserved_space, value));
    else if(name == __menu_style_sheet_pairs[6].second)
        verify(to_integer(_accel_reserved_space, value));
    else
        button_style_sheet::set_value(name, value);
}

int menu_style_sheet::get_content_size() const
{
    return _countof(__button_style_sheet_pairs) + _countof(__menu_style_sheet_pairs);
}

style_sheet_type menu_style_sheet::get_content_type(int index) const
{
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return sst_unknown;
    }
    return index < _countof(__menu_style_sheet_pairs) ? __menu_style_sheet_pairs[index].first :
        __button_style_sheet_pairs[index - _countof(__menu_style_sheet_pairs)].first;
}

const string& menu_style_sheet::get_content_name(int index) const
{
    static const string err(_t("unknown name."));
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return err;
    }
    return index < _countof(__menu_style_sheet_pairs) ? __menu_style_sheet_pairs[index].second :
        __button_style_sheet_pairs[index - _countof(__menu_style_sheet_pairs)].second;
}

menu_cmd_style_sheet::menu_cmd_style_sheet()
{
}

bool menu_cmd_style_sheet::get_value(const string& name, string& value)
{
    if(name == __menu_cmd_style_sheet_pairs[0].second) {
        value = _caption;
        return true;
    }
    else if(name == __menu_cmd_style_sheet_pairs[1].second)
        return from_accel_key(value, _accel_key);
    else {
        assert(!"unexpected style sheet name.");
        return false;
    }
}

void menu_cmd_style_sheet::set_value(const string& name, const string& value)
{
    if(name == __menu_cmd_style_sheet_pairs[0].second)
        _caption = value;
    else if(name == __menu_cmd_style_sheet_pairs[1].second)
        verify(to_accel_key(_accel_key, value));
    else {
        assert(!"unexpected style sheet name.");
        return;
    }
}

int menu_cmd_style_sheet::get_content_size() const
{
    return _countof(__menu_cmd_style_sheet_pairs);
}

style_sheet_type menu_cmd_style_sheet::get_content_type(int index) const
{
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return sst_unknown;
    }
    return __menu_cmd_style_sheet_pairs[index].first;
}

const string& menu_cmd_style_sheet::get_content_name(int index) const
{
    static const string err(_t("unknown name."));
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return err;
    }
    return __menu_cmd_style_sheet_pairs[index].second;
}

menu_sub_style_sheet::menu_sub_style_sheet()
{
}

bool menu_sub_style_sheet::get_value(const string& name, string& value)
{
    if(name == __menu_sub_style_sheet_pairs[0].second) {
        value = _caption;
        return true;
    }
    else {
        assert(!"unexpected style sheet name.");
        return false;
    }
}

void menu_sub_style_sheet::set_value(const string& name, const string& value)
{
    if(name == __menu_sub_style_sheet_pairs[0].second)
        _caption = value;
    else {
        assert(!"unexpected style sheet name.");
        return;
    }
}

int menu_sub_style_sheet::get_content_size() const
{
    return _countof(__menu_sub_style_sheet_pairs);
}

style_sheet_type menu_sub_style_sheet::get_content_type(int index) const
{
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return sst_unknown;
    }
    return __menu_sub_style_sheet_pairs[0].first;
}

const string& menu_sub_style_sheet::get_content_name(int index) const
{
    static const string err(_t("unknown name."));
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return err;
    }
    return __menu_sub_style_sheet_pairs[0].second;
}

menubar_style_sheet::menubar_style_sheet()
{
    /* todo */
}

menubar_button_style_sheet::menubar_button_style_sheet()
{
}

bool menubar_button_style_sheet::get_value(const string& name, string& value)
{
    if(name == __menubar_button_style_sheet_pairs[0].second) {
        value = _caption;
        return true;
    }
    else {
        assert(!"unexpected style sheet name.");
        return false;
    }
}

void menubar_button_style_sheet::set_value(const string& name, const string& value)
{
    if(name == __menubar_button_style_sheet_pairs[0].second)
        _caption = value;
    else {
        assert(!"unexpected style sheet name.");
        return;
    }
}

int menubar_button_style_sheet::get_content_size() const
{
    return _countof(__menubar_button_style_sheet_pairs);
}

style_sheet_type menubar_button_style_sheet::get_content_type(int index) const
{
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return sst_unknown;
    }
    return __menubar_button_style_sheet_pairs[0].first;
}

const string& menubar_button_style_sheet::get_content_name(int index) const
{
    static const string err(_t("unknown name."));
    if(index < 0 || index >= get_content_size()) {
        assert(!"bad index for style sheet.");
        return err;
    }
    return __menubar_button_style_sheet_pairs[0].second;
}

void widget::draw(painter* paint)
{
    assert(paint);
    paint->set_brush(_normal_brush);
    paint->set_pen(_normal_pen);
    paint->draw_rect(get_rectf());
}

void widget::flush_style()
{
    setup_brush_by_color(_normal_brush, _fill_color);
    setup_pen_by_color(_normal_pen, _stroke_color);
}

void background::draw(painter* paint)
{
    assert(paint);
    if(_bktex.get())
        paint->draw_image(_bktex.get(), 0.f, 0.f);
}

void background::flush_style()
{
    auto* rsys = scene::get_singleton_ptr()->get_rendersys();
    assert(rsys);
    rsys->set_background_color(_bkground_color);
    if(!_bkground_image.empty()) {
        auto* tex = textureop(rsys).load(_bkground_image);
        assert(tex);
        _bktex.attach(tex);
    }
}

void button::draw(painter* paint)
{
    assert(paint);
    int w, h;
    paint->save();
    paint->set_brush(_current_brush);
    paint->set_pen(_current_pen);
    rectf rc = get_rectf();
    rc.move_to(0.f, 0.f);
    paint->draw_rect(rc);
    paint->restore();
    if(!_caption.empty()) {
        paint->set_font(_caption_font);
        paint->get_text_dimension(_caption.c_str(), w, h);
        paint->draw_text(_caption.c_str(), (get_width() - w) / 2, (get_height() - h) / 2, _caption_font_color, _caption.length());
    }
}

void button::flush_style()
{
    setup_brush_by_color(_normal_brush, _normal_fill_color);
    setup_pen_by_color(_normal_pen, _normal_stroke_color);
    setup_brush_by_color(_hover_brush, _hover_fill_color);
    setup_pen_by_color(_hover_pen, _hover_stroke_color);
    setup_brush_by_color(_press_brush, _press_fill_color);
    setup_pen_by_color(_press_pen, _press_stroke_color);
    setup_font(_caption_font, _caption_font_name, _caption_font_size);
    is_enable() ? set_normal() : set_gray();
}

void button::set_press()
{
    _current_brush = _press_brush;
    _current_pen = _press_pen;
}

void button::set_normal()
{
    _current_brush = _normal_brush;
    _current_pen = _normal_pen;
}

void button::set_hover()
{
    _current_brush = _hover_brush;
    _current_pen = _hover_pen;
}

void button::set_gray()
{
    if(_normal_brush.get_tag() != painter_brush::solid) {
        assert(_normal_brush.get_tag() == painter_brush::none);
        _current_brush = _normal_brush;
    }
    else {
        const color& cr = _normal_brush.get_color();
        int c = ((int)cr.red + cr.green + cr.blue) / 3;
        _current_brush.set_tag(painter_brush::solid);
        _current_brush.set_color(color(c, c, c, cr.alpha));
    }
    if(_normal_pen.get_tag() != painter_pen::solid) {
        assert(_normal_pen.get_tag() == painter_pen::none);
        _current_pen = _normal_pen;
    }
    else {
        const color& cr = _normal_pen.get_color();
        int c = ((int)cr.red + cr.green + cr.blue) / 3;
        _current_pen.set_tag(painter_pen::solid);
        _current_pen.set_color(color(c, c, c, cr.alpha));
    }
}

void edit::draw(painter* paint)
{
    assert(paint);
    set_font(_text_font);
    set_text_color(_text_font_color);
    __super::draw(paint);
}

void edit::flush_style()
{
    setup_brush_by_color(_normal_brush, _normal_fill_color);
    setup_pen_by_color(_normal_pen, _normal_stroke_color);
    setup_brush_by_color(_focused_brush, _focused_fill_color);
    setup_pen_by_color(_focused_pen, _focused_stroke_color);
    setup_font(_text_font, _text_font_name, _text_font_size);
}

void edit::draw_background(painter* paint)
{
    assert(paint);
    paint->save();
    if(is_focused()) {
        paint->set_brush(_focused_brush);
        paint->set_pen(_focused_pen);
    }
    else {
        paint->set_brush(_normal_brush);
        paint->set_pen(_normal_pen);
    }
    paint->draw_rect(get_rectf());
    paint->restore();
}

void* menu_separator::query_interface(const uuid& uid)
{
    return (uid == uuid_menu_item) ? static_cast<menu_item*>(this) : __super::query_interface(uid);
}

void menu_separator::draw(painter* paint)
{
    assert(paint);
    painter_pen pen(painter_pen::solid);
    assert(_menu);
    pen.set_color(_menu->_separator_color);
    paint->set_pen(pen);
    float w = (float)get_width(), h = (float)get_height();
    float pos = 0.5f * (h - 1.f);
    float margin = (float)_menu->_text_horizontal_margin;
    assert(get_parent());
    auto* parent = static_cast<menu_group*>(get_parent());
    float iconw = (float)parent->_icon_width;
    paint->draw_line(vec2(iconw + margin, pos), vec2(w - margin, pos));
}

void* menu_sub_item::query_interface(const uuid& uid)
{
    return (uid == uuid_menu_item) ? static_cast<menu_item*>(this) : __super::query_interface(uid);
}

void menu_sub_item::draw(painter* paint)
{
    assert(paint);
    assert(get_parent());
    /* draw button */
    paint->save();
    if(_sub_menu && _sub_menu->is_visible()) {
        paint->set_brush(_menu->_menu_press_brush);
        paint->set_pen(_menu->_menu_press_pen);
    }
    else {
        paint->set_brush(*_brush_ptr);
        paint->set_pen(*_pen_ptr);
    }
    rectf rc = get_rectf();
    rc.move_to(0.f, 0.f);
    paint->draw_rect(rc);
    paint->restore();
    auto* parent = static_cast<menu_group*>(get_parent());
    float iconw = (float)parent->_icon_width;
    /* draw caption */
    assert(_menu);
    paint->set_font(_menu->_menu_font);
    int w, h;
    paint->get_text_dimension(_caption.c_str(), w, h, _caption.length());
    float x, y;
    x = iconw + _menu->_text_horizontal_margin;
    y = ((float)get_height() - h) * 0.5f;
    paint->draw_text(_caption.c_str(), round(x), round(y), _menu->_caption_font_color, _caption.length());
    /* draw arrow */
    const int arrow_width = 4, arrow_height = 6;
    x = (float)get_width() - arrow_width - _menu->_text_horizontal_margin;
    y = ((float)get_height() - arrow_height) * 0.5f;
    paint->save();
    painter_brush brush(painter_brush::solid);
    brush.set_color(_menu->_caption_font_color);
    paint->set_brush(brush);
    painter_path path;
    path.move_to(x, y);
    path.line_to(x, y + arrow_height);
    path.line_to(x + arrow_width, y + arrow_height / 2);
    path.close_path();
    paint->draw_path(path);
    paint->restore();
}

void menu_sub_item::on_press(uint um, unikey uk, const point& pt)
{
    assert(_menu);
    _brush_ptr = &_menu->_menu_press_brush;
    _pen_ptr = &_menu->_menu_press_pen;
    _menu->on_select_menu_item(this);
}

void menu_sub_item::on_click(uint um, unikey uk, const point& pt)
{
    assert(_menu);
    _brush_ptr = &_menu->_menu_hover_brush;
    _pen_ptr = &_menu->_menu_hover_pen;
    _menu->on_select_menu_item(this);
}

void menu_sub_item::on_hover(uint um, const point& pt)
{
    assert(_menu);
    _brush_ptr = &_menu->_menu_hover_brush;
    _pen_ptr = &_menu->_menu_hover_pen;
    _menu->on_select_menu_item(this);
}

void menu_sub_item::on_leave(uint um, const point& pt)
{
    assert(_menu);
    _brush_ptr = &_menu->_menu_normal_brush;
    _pen_ptr = &_menu->_menu_normal_pen;
}

void menu_sub_item::flush_style()
{
    assert(_menu);
    _brush_ptr = &_menu->_menu_normal_brush;
    _pen_ptr = &_menu->_menu_normal_pen;
}

static void retrieve_text_dimensions(const font& ft, const string& str, int& w, int& h)
{
    if(str.empty()) {
        w = h = 0;
        return;
    }
    auto* fsys = scene::get_singleton_ptr()->get_fontsys();
    assert(fsys);
    fsys->set_font(ft);
    fsys->get_size(str.c_str(), w, h, str.length());
}

void menu_sub_item::get_caption_dimensions(int& w, int& h) const
{
    int tw, th;
    retrieve_text_dimensions(_menu->_menu_font, _caption, tw, th);
    assert(_menu);
    w = tw + _menu->_text_horizontal_margin + _menu->_caption_reserved_space;
    h = th + _menu->_text_vertical_margin + _menu->_text_vertical_margin;
}

void* menu_cmd_item::query_interface(const uuid& uid)
{
    return (uid == uuid_menu_item) ? static_cast<menu_item*>(this) : __super::query_interface(uid);
}

void menu_cmd_item::draw(painter* paint)
{
    assert(paint);
    assert(get_parent());
    auto* parent = static_cast<menu_group*>(get_parent());
    /* draw button */
    paint->save();
    paint->set_brush(*_brush_ptr);
    paint->set_pen(*_pen_ptr);
    rectf rc = get_rectf();
    rc.move_to(0.f, 0.f);
    paint->draw_rect(rc);
    paint->restore();
    /* draw caption */
    assert(_menu);
    paint->set_font(_menu->_menu_font);
    int w, h;
    paint->get_text_dimension(_caption.c_str(), w, h, _caption.length());
    float x, y;
    x = (float)(parent->_icon_width + _menu->_text_horizontal_margin);
    y = ((float)get_height() - h) * 0.5f;
    paint->draw_text(_caption.c_str(), round(x), round(y), _menu->_caption_font_color, _caption.length());
    /* draw accel text */
    if(!_accel_key.is_valid())
        return;
    string str;
    _accel_key.to_string(str);
    paint->get_text_dimension(str.c_str(), w, h, str.length());
    x = (float)(parent->_icon_width + parent->_caption_text_width + _menu->_text_horizontal_margin);
    paint->draw_text(str.c_str(), round(x), round(y), _menu->_caption_font_color, str.length());
}

void menu_cmd_item::on_press(uint um, unikey uk, const point& pt)
{
    assert(_menu);
    _brush_ptr = &_menu->_menu_press_brush;
    _pen_ptr = &_menu->_menu_press_pen;
    _menu->on_select_menu_item(this);
}

void menu_cmd_item::on_click(uint um, unikey uk, const point& pt)
{
    assert(_menu);
    _brush_ptr = &_menu->_menu_hover_brush;
    _pen_ptr = &_menu->_menu_hover_pen;
    _menu->on_select_menu_item(this);
    if(_menu_notify)
        _menu_notify->on_menu_command(this);
}

void menu_cmd_item::on_hover(uint um, const point& pt)
{
    assert(_menu);
    _brush_ptr = &_menu->_menu_hover_brush;
    _pen_ptr = &_menu->_menu_hover_pen;
    _menu->on_select_menu_item(this);
}

void menu_cmd_item::on_leave(uint um, const point& pt)
{
    assert(_menu);
    _brush_ptr = &_menu->_menu_normal_brush;
    _pen_ptr = &_menu->_menu_normal_pen;
}

void menu_cmd_item::flush_style()
{
    assert(_menu);
    _brush_ptr = &_menu->_menu_normal_brush;
    _pen_ptr = &_menu->_menu_normal_pen;
}

void menu_cmd_item::get_caption_dimensions(int& w, int& h) const
{
    int tw, th;
    retrieve_text_dimensions(_menu->_menu_font, _caption, tw, th);
    assert(_menu);
    w = tw + _menu->_text_horizontal_margin + _menu->_caption_reserved_space;
    h = th + _menu->_text_vertical_margin + _menu->_text_vertical_margin;
}

void menu_cmd_item::get_accel_key_dimensions(int& w, int& h) const
{
    if(!_accel_key.is_valid()) {
        w = h = 0;
        return;
    }
    string str;
    _accel_key.to_string(str);
    assert(!str.empty());
    int tw, th;
    retrieve_text_dimensions(_menu->_menu_font, str, tw, th);
    assert(_menu);
    w = tw + _menu->_text_horizontal_margin + _menu->_accel_reserved_space;
    h = th + _menu->_text_vertical_margin + _menu->_text_vertical_margin;
}

void menu_group::draw(painter* paint)
{
    assert(paint);
    paint->save();
    paint->set_brush(painter_brush());
    paint->set_pen(_menu->_menu_border_pen);
    if(_upside_gap <= 0) {
        rectf rc = get_rectf();
        rc.move_to(0.f, 0.f);
        paint->draw_rect(rc);
    }
    else {
        float w = (float)get_width(), h = (float)get_height();
        float gap = (float)_upside_gap;
        painter_path path;
        path.move_to(0.f, 0.f);
        path.line_to(0.f, h);
        path.line_to(w, h);
        path.line_to(w, 0.f);
        if(w > gap)
            path.line_to(gap, 0.f);
        paint->draw_path(path);
    }
    paint->restore();
}

void menu_group::refresh_menu_group_size()
{
    /* retrieve dimensions */
    int item_height = 0, caption_text_width = 0, accel_text_width = 0;
    auto retrieve_dimensions = [&](ariel::widget* w)-> int {
        assert(w);
        auto* item = reinterpret_cast<menu_item*>(w->query_interface(uuid_menu_item));
        assert(item);
        switch(item->get_type())
        {
        case menu_item::cmd_item:
            {
                auto* p = static_cast<menu_cmd_item*>(item);
                int w, h;
                p->get_caption_dimensions(w, h);
                item_height = gs_max(item_height, h);
                caption_text_width = gs_max(caption_text_width, w);
                p->get_accel_key_dimensions(w, h);
                item_height = gs_max(item_height, h);
                accel_text_width = gs_max(accel_text_width, w);
                break;
            }
        case menu_item::sub_item:
            {
                auto* p = static_cast<menu_sub_item*>(item);
                int w, h;
                p->get_caption_dimensions(w, h);
                item_height = gs_max(item_height, h);
                caption_text_width = gs_max(caption_text_width, w);
                break;
            }
        }
        return 0;
    };
    traverse_child_widget(retrieve_dimensions);
    /* setup dimensions */
    _icon_width = 0;
    _caption_text_width = caption_text_width;
    _accel_text_width = accel_text_width;
    int total_width = _icon_width + caption_text_width + accel_text_width;
    /* do arrangement */
    int next_item_pos = 1;
    auto arrange_items = [&](ariel::widget* w)-> int {
        assert(w);
        auto* item = reinterpret_cast<menu_item*>(w->query_interface(uuid_menu_item));
        assert(item);
        switch(item->get_type())
        {
        case menu_item::separator:
            {
                w->move(rect(1, next_item_pos, total_width, _menu->_separator_space));
                next_item_pos += _menu->_separator_space;
                break;
            }
        case menu_item::sub_item:
        case menu_item::cmd_item:
            {
                w->move(rect(1, next_item_pos, total_width, item_height));
                next_item_pos += item_height;
                break;
            }
        }
        return 0;
    };
    traverse_child_widget(arrange_items);
    resize(total_width + 2, next_item_pos + 1);
}

menu_cmd_item* menu_group::add_cmd_item()
{
    auto* p = add_item<menu_cmd_item>(sm_hitable|sm_visible);
    assert(p);
    p->flush_style();
    auto* pmenu = static_cast<menu*>(get_parent());
    assert(pmenu == _menu);
    p->set_cmd_notify(pmenu);
    return p;
}

menu_item* menu_group::hit_item(const point& pt)
{
    auto find_hit_item = [&pt](ariel::widget* w)-> int {
        if(w->get_rect().in_rect(pt))
            return (int)w;
        return 0;
    };
    auto* hit = (ariel::widget*)traverse_child_widget(find_hit_item);
    if(!hit)
        return nullptr;
    auto* item = reinterpret_cast<menu_item*>(hit->query_interface(uuid_menu_item));
    return (item->get_type() == menu_item::separator) ? nullptr : item;
}

template<class _ctor>
_ctor* menu_group::add_item(uint sm)
{
    assert(_manager);
    auto* p = _manager->add_widget<_ctor>(this, nullptr, rect(), sm);
    assert(p);
    p->set_menu(_menu);
    return p;
}

void menu::close()
{
    for(auto* p : _menu_stack)
        p->hide();
    _menu_stack.clear();
    __super::close();
}

void menu::show(bool b)
{
    if(b && !is_visible())
        on_startup_menu();
    else if(!b && is_visible()) {
        for(auto* p : _menu_stack)
            p->hide();
        _menu_stack.clear();
        refresh_menu_size();
    }
    __super::show(b);
}

bool menu::create(widget* ptr, const gchar* name, const rect& rc, uint style)
{
    if(!__super::create(ptr, name, rc, style))
        return false;
    hide();
    return true;
}

void menu::on_press(uint um, unikey uk, const point& pt)
{
    if(auto* item = hit_item(pt)) {
        auto* w = item->to_widget();
        if(w) {
            point pt1 = pt;
            to_global(pt1);
            w->to_local(pt1);
            on_current_hover(w, um, pt1);
            w->on_press(um, uk, pt1);
            return;
        }
    }
    on_current_hover(nullptr, um, pt);
}

void menu::on_click(uint um, unikey uk, const point& pt)
{
    if(auto* item = hit_item(pt)) {
        auto* w = item->to_widget();
        if(w) {
            point pt1 = pt;
            to_global(pt1);
            w->to_local(pt1);
            on_current_hover(w, um, pt1);
            w->on_click(um, uk, pt1);
            return;
        }
    }
    on_current_hover(nullptr, um, pt);
    assert(_manager);
    _manager->set_focus(nullptr);
    capture(false);
    hide();
}

void menu::on_hover(uint um, const point& pt)
{
    if(auto* item = hit_item(pt)) {
        auto* w = item->to_widget();
        if(w) {
            point pt1 = pt;
            to_global(pt1);
            w->to_local(pt1);
            on_current_hover(w, um, pt1);
            w->on_hover(um, pt1);
            return;
        }
    }
    on_current_hover(nullptr, um, pt);
}

void menu::on_keydown(uint um, unikey uk)
{
}

void menu::flush_style()
{
    setup_pen_by_color(_menu_border_pen, _border_color);
    setup_brush_by_color(_menu_normal_brush, _normal_fill_color);
    setup_pen_by_color(_menu_normal_pen, _normal_stroke_color);
    setup_brush_by_color(_menu_hover_brush, _hover_fill_color);
    setup_pen_by_color(_menu_hover_pen, _hover_stroke_color);
    setup_brush_by_color(_menu_press_brush, _press_fill_color);
    setup_pen_by_color(_menu_press_pen, _press_stroke_color);
    setup_font(_menu_font, _caption_font_name, _caption_font_size);
}

void menu::startup()
{
    if(is_visible()) {
        assert(!"the menu was already started.");
        return;
    }
    show(true);
    capture(true);
    focus();
}

void menu::on_sub_menu(menu_sub_item* item)
{
    assert(item && item->get_parent());
    auto* hold = static_cast<menu_group*>(item->get_parent());
    auto* sub_menu = item->get_sub_menu();
    assert(sub_menu);
    auto pos = item->get_rect().top_right();
    pos.offset(hold->get_rect().top_left());
    pos.offset(2, 0);
    sub_menu->move(pos);
    sub_menu->show(true);
    _menu_stack.push_back(sub_menu);
    refresh_menu_size();
}

void menu::on_startup_menu()
{
    if(!_menu_stack.empty()) {
        assert(!"incorrect startup menu.");
        /* try to fix it. */
        for(auto* p : _menu_stack)
            p->hide();
        _menu_stack.clear();
    }
    if(!get_child()) {
        assert(!"incorrect menu: no menu groups found.");
        hide();
        return;
    }
    auto* p = static_cast<menu_group*>(get_child());
    assert(p);
    p->move(point(0, 0));
    p->show(true);
    _menu_stack.push_back(p);
    refresh_menu_size();
}

void menu::refresh_menu_size()
{
    if(_menu_stack.empty()) {
        resize(1, 1);
        return;
    }
    rect u;
    for(auto* p : _menu_stack) {
        assert(p);
        union_rect(u, u, p->get_rect());
    }
    resize(u.width(), u.height());
}

menu_group* menu::register_menu_group()
{
    assert(_manager);
    auto* p = _manager->add_widget<menu_group>(this, nullptr, rect(), sm_hitable|sm_visible);
    assert(p);
    p->set_menu(this);
    p->hide();
    return p;
}

bool menu::unregister_menu_group(menu_group* p)
{
    assert(_manager);
    assert(p && (p->get_parent() == this));
    return _manager->remove_widget(p);
}

menu_item* menu::hit_item(const point& pt)
{
    auto* group = hit_group(pt);
    if(!group)
        return nullptr;
    auto localpt = pt;
    const auto& local_bias = group->get_rect().top_left();
    localpt.offset(-local_bias.x, -local_bias.y);
    return group->hit_item(localpt);
}

menu_group* menu::hit_group(const point& pt)
{
    rect rc = get_rect();
    rc.move_to(point(0, 0));
    if(!rc.in_rect(pt) || _menu_stack.empty())
        return nullptr;
    for(auto i = _menu_stack.rbegin(); i != _menu_stack.rend(); ++ i) {
        if((*i)->get_rect().in_rect(pt))
            return *i;
    }
    return nullptr;
}

menu_group* menu::get_entry_group() const
{
    assert(!_menu_stack.empty());
    return _menu_stack.front();
}

void menu::on_current_hover(ariel::widget* ptr, uint um, const point& pt)
{
    if(_last_hover == ptr)
        return;
    if(_last_hover)
        _last_hover->on_leave(um, pt);
    _last_hover = ptr;
}

void menu::on_select_menu_item(menu_item* item)
{
    assert(item);
    auto* w = item->to_widget();
    assert(w && w->get_parent());
    auto* hold = static_cast<menu_group*>(w->get_parent());
    for( ; !_menu_stack.empty(); _menu_stack.pop_back()) {
        auto* p = _menu_stack.back();
        if(p == hold)
            break;
        p->hide();
    }
    assert(!_menu_stack.empty());
    switch(item->get_type())
    {
    case menu_item::sub_item:
        on_sub_menu(static_cast<menu_sub_item*>(item));
        break;
    }
}

void menubar_button::draw(painter* paint)
{
    paint->save();
    paint->set_brush(*_brush_ptr);
    paint->set_pen(*_pen_ptr);
    float w = (float)get_width(), h = (float)get_height();
    painter_path path;
    path.move_to(w, h);
    path.line_to(w, 0.f);
    path.line_to(0.f, 0.f);
    path.line_to(0.f, h);
    paint->draw_path(path);
    paint->set_pen(painter_pen());
    paint->draw_text(_caption.c_str(), 0, 0, _menubar->_caption_font_color, _caption.length());
    paint->restore();
}

void menubar_button::on_press(uint um, unikey uk, const point& pt)
{
    assert(_menubar);
    _brush_ptr = &_menubar->_menubar_press_brush;
    _pen_ptr = &_menubar->_menubar_press_pen;
}

void menubar_button::on_click(uint um, unikey uk, const point& pt)
{
    assert(_menubar);
    _brush_ptr = &_menubar->_menubar_hover_brush;
    _pen_ptr = &_menubar->_menubar_hover_pen;
}

void menubar_button::on_hover(uint um, const point& pt)
{
    assert(_menubar);
    _brush_ptr = &_menubar->_menubar_hover_brush;
    _pen_ptr = &_menubar->_menubar_hover_pen;
}

void menubar_button::on_leave(uint um, const point& pt)
{
    assert(_menubar);
    _brush_ptr = &_menubar->_menubar_normal_brush;
    _pen_ptr = &_menubar->_menubar_normal_pen;
}

void menubar_button::flush_style()
{
    assert(_menubar);
    set_caption(menubar_button::_caption);
    _brush_ptr = &_menubar->_menubar_normal_brush;
    _pen_ptr = &_menubar->_menubar_normal_pen;
}

void menubar_button::refresh_menubar_button_size()
{
    assert(_menubar && _menu);
    /* setup size */
    int w, h;
    retrieve_text_dimensions(_menubar->_menubar_font, _caption, w, h);
    resize(w, h);
    /* setup menu gap */
    auto* entry = _menu->get_entry_group();
    assert(entry);
    entry->set_upside_gap(w);
}

void menubar::refresh_menubar_size()
{
    int width = 0, height = 0;
    auto travch = [&width, &height](ariel::widget* w)-> int {
        assert(w);
        width += w->get_width();
        height = gs_max(height, w->get_height());
        return 0;
    };
    traverse_child_widget(travch);
    resize(width, height);
}

void menubar::flush_style()
{
    setup_brush_by_color(_menubar_normal_brush, _normal_fill_color);
    setup_pen_by_color(_menubar_normal_pen, _normal_stroke_color);
    setup_brush_by_color(_menubar_hover_brush, _hover_fill_color);
    setup_pen_by_color(_menubar_hover_pen, _hover_stroke_color);
    setup_brush_by_color(_menubar_press_brush, _press_fill_color);
    setup_pen_by_color(_menubar_press_pen, _press_stroke_color);
    setup_font(_menubar_font, _caption_font_name, _caption_font_size);
}

menubar_button* menubar::register_menubar_button()
{
    assert(_manager);
    auto* p = _manager->add_widget<menubar_button>(this, nullptr, rect(), sm_hitable|sm_visible);
    assert(p);
    p->set_menubar(this);
    return p;
}

bool menubar::unregister_menubar_button(menubar_button* p)
{
    assert(_manager);
    assert(p && (p->get_parent() == this));
    return _manager->remove_widget(p);
}

#define menu_script_blanks  _t(" \t\v\r\n\f")

static int skip_blanks(const string& str, int start)
{
    if(start >= str.length())
        return str.length();
    static string blanks(menu_script_blanks);
    return (int)str.find_first_not_of(blanks, start);
}

static int acquire_token(string& tok, const string& str, const string& eos, int start)
{
    int next = (int)str.find_first_of(eos, start);
    if(next == start)
        return next;
    tok.assign(str, start, next - start);
    return next;
}

static menu_group* create_menu_group_from_script(menu* pmenu, const string& script, int& start)
{
    assert(pmenu);
    int next = skip_blanks(script, start);
    if(next >= script.length())
        return nullptr;
    auto* mgroup = pmenu->register_menu_group();
    assert(mgroup);
    for(;;) {
        if(script.at(next) == _t('}')) {
            start = next;
            mgroup->refresh_menu_group_size();
            return mgroup;
        }
        if(script.at(next) == _t('[')) {
            string name;
            int endofname = acquire_token(name, script, _t("]"), next);
            if(endofname >= script.length()) {
                assert(!"unexpected end of menu script.");
                return nullptr;
            }
            if(++endofname < script.length() && script.at(endofname) == _t(';'))
                ++ endofname;
            next = endofname;
            auto* sep = mgroup->add_separator();
            assert(sep);
            continue;
        }
        string name, accelkey;
        int endofname = acquire_token(name, script, _t(","), next);
        if(endofname >= script.length())
            return nullptr;
        next = skip_blanks(script, endofname + 1);
        if(next >= script.length()) {
            assert(!"unexpected end of menu script.");
            return nullptr;
        }
        int endofkey = acquire_token(accelkey, script, _t(":"), next);
        if(endofkey >= script.length())
            return nullptr;
        next = skip_blanks(script, endofkey + 1);
        if(next >= script.length()) {
            assert(!"unexpected end of menu script.");
            return nullptr;
        }
        accel_key key;
        bool has_accel = !accelkey.empty();
        if(has_accel && !key.from_string(accelkey)) {
            assert(!"invalid accelerator key of menu script.");
            return nullptr;
        }
        auto ctl = script.at(next);
        if(ctl == _t('@')) {
            auto* cmd = mgroup->add_cmd_item();
            assert(cmd);
            cmd->set_caption(name);
            if(has_accel)
                cmd->set_accel_key(key);
            next = acquire_token(string(), script, _t(";"), next);
            if(next >= script.length()) {
                assert(!"unexpected end of menu script.");
                return nullptr;
            }
            next ++;
        }
        else if(ctl == _t('{')) {
            auto* subitem = mgroup->add_sub_item();
            assert(subitem);
            subitem->set_caption(name);
            auto* submgroup = create_menu_group_from_script(pmenu, script, ++ next);
            if(!submgroup) {
                assert(!"parse sub menu script failed.");
                return nullptr;
            }
            subitem->set_sub_menu(submgroup);
            assert(script.at(next) == _t('}'));
            if(next + 1 < script.length() && script.at(next) == _t(';'))
                next ++;
            next ++;
        }
        else {
            assert(!"unexpected control text of menu script.");
            return nullptr;
        }
        assert(next <= script.length());
        next = skip_blanks(script, next);
        if(next >= script.length())
            break;
    }
    start = next;
    mgroup->refresh_menu_group_size();
    return mgroup;
}

bool create_menu_from_script(menu* pmenu, const string& script)
{
    int next = 0;
    return create_menu_group_from_script(pmenu, script, next) != nullptr;
}

};

__ariel_end__
