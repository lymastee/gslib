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
    std::make_pair(sst_color, _t("fill_color")),
    std::make_pair(sst_color, _t("stroke_color")),
    std::make_pair(sst_float, _t("fill_opacity")),
    std::make_pair(sst_float, _t("stroke_opacity")),
    std::make_pair(sst_string, _t("text_font_name")),
    std::make_pair(sst_integer, _t("text_font_size")),
    std::make_pair(sst_color, _t("text_font_color")),
    std::make_pair(sst_float, _t("text_font_opacity")),
};

static const style_sheet_pair __menu_style_sheet_pairs[] =
{
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

widget_style_sheet::widget_style_sheet()
{
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
}

bool edit_style_sheet::get_value(const string& name, string& value)
{
    if(name == __edit_style_sheet_pairs[0].second)
        return from_color(value, _fill_color);
    else if(name == __edit_style_sheet_pairs[1].second)
        return from_color(value, _stroke_color);
    else if(name == __edit_style_sheet_pairs[2].second)
        return from_float(value, (float)_fill_color.alpha / 255.f);
    else if(name == __edit_style_sheet_pairs[3].second)
        return from_float(value, (float)_stroke_color.alpha / 255.f);
    else if(name == __edit_style_sheet_pairs[4].second) {
        value = _text_font_name;
        return true;
    }
    else if(name == __edit_style_sheet_pairs[5].second)
        return from_integer(value, _text_font_size);
    else if(name == __edit_style_sheet_pairs[6].second)
        return from_color(value, _text_font_color);
    else if(name == __edit_style_sheet_pairs[7].second)
        return from_float(value, (float)_text_font_color.alpha / 255.f);
    return false;
}

void edit_style_sheet::set_value(const string& name, const string& value)
{
    if(name == __edit_style_sheet_pairs[0].second)
        verify(to_color(_fill_color, value));
    else if(name == __edit_style_sheet_pairs[1].second)
        verify(to_color(_stroke_color, value));
    else if(name == __edit_style_sheet_pairs[2].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for edit_style_sheet.");
            return;
        }
        _fill_color.alpha = round(f * 255.f);
    }
    else if(name == __edit_style_sheet_pairs[3].second) {
        float f;
        verify(to_float(f, value));
        if(f < 0.f || f > 1.f) {
            assert(!"invalid opacity for edit_style_sheet.");
            return;
        }
        _stroke_color.alpha = round(f * 255.f);
    }
    else if(name == __edit_style_sheet_pairs[4].second)
        _text_font_name = value;
    else if(name == __edit_style_sheet_pairs[5].second)
        verify(to_integer(_text_font_size, value));
    else if(name == __edit_style_sheet_pairs[6].second)
        verify(to_color(_text_font_color, value));
    else if(name == __edit_style_sheet_pairs[7].second) {
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
}

bool menu_style_sheet::get_value(const string& name, string& value)
{
    if(name == __menu_style_sheet_pairs[0].second)
        return from_color(value, _separator_color);
    else if(name == __menu_style_sheet_pairs[1].second)
        return from_integer(value, _separator_space);
    else if(name == __menu_style_sheet_pairs[2].second)
        return from_integer(value, _text_horizontal_margin);
    else if(name == __menu_style_sheet_pairs[3].second)
        return from_integer(value, _text_vertical_margin);
    else if(name == __menu_style_sheet_pairs[4].second)
        return from_integer(value, _caption_reserved_space);
    else if(name == __menu_style_sheet_pairs[5].second)
        return from_integer(value, _accel_reserved_space);
    return button_style_sheet::get_value(name, value);
}

void menu_style_sheet::set_value(const string& name, const string& value)
{
    if(name == __menu_style_sheet_pairs[0].second)
        verify(to_color(_separator_color, value));
    else if(name == __menu_style_sheet_pairs[1].second)
        verify(to_integer(_separator_space, value));
    else if(name == __menu_style_sheet_pairs[2].second)
        verify(to_integer(_text_horizontal_margin, value));
    else if(name == __menu_style_sheet_pairs[3].second)
        verify(to_integer(_text_vertical_margin, value));
    else if(name == __menu_style_sheet_pairs[4].second)
        verify(to_integer(_caption_reserved_space, value));
    else if(name == __menu_style_sheet_pairs[5].second)
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
    paint->set_brush(_current_brush);
    paint->set_pen(_current_pen);
    paint->draw_rect(get_rectf());
    paint->set_font(_caption_font);
    paint->get_text_dimension(_caption.c_str(), w, h);
    paint->draw_text(_caption.c_str(), (get_width() - w) / 2, (get_height() - h) / 2, _caption_font_color, _caption.length());
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
    setup_brush_by_color(_normal_brush, _fill_color);
    setup_pen_by_color(_normal_pen, _stroke_color);
    setup_font(_text_font, _text_font_name, _text_font_size);
}

void edit::draw_background(painter* paint)
{
    assert(paint);
    paint->save();
    paint->set_brush(_normal_brush);
    paint->set_pen(_normal_pen);
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
    painter_brush brush(painter_brush::solid);
    brush.set_color(_menu->_caption_font_color);
    paint->set_brush(brush);
    painter_path path;
    path.move_to(x, y);
    path.line_to(x, y + arrow_height);
    path.line_to(x + arrow_width, y + arrow_height / 2);
    path.close_path();
    paint->draw_path(path);
}

void menu_sub_item::flush_style()
{
    set_caption(menu_sub_style_sheet::_caption);
}

static void retrieve_text_dimensions(const string& str, int& w, int& h)
{
    if(str.empty()) {
        w = h = 0;
        return;
    }
    auto* fsys = scene::get_singleton_ptr()->get_fontsys();
    assert(fsys);
    fsys->get_size(str.c_str(), w, h, str.length());
}

void menu_sub_item::get_caption_dimensions(int& w, int& h) const
{
    int tw, th;
    retrieve_text_dimensions(_caption, tw, th);
    assert(_menu);
    w = tw + _menu->_text_horizontal_margin + _menu->_caption_reserved_space;
    h = th + _menu->_text_vertical_margin + _menu->_text_vertical_margin;
}

void menu_cmd_item::draw(painter* paint)
{
    assert(paint);
    assert(get_parent());
    auto* parent = static_cast<menu_group*>(get_parent());
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

void* menu_cmd_item::query_interface(const uuid& uid)
{
    return (uid == uuid_menu_item) ? static_cast<menu_item*>(this) : __super::query_interface(uid);
}

void menu_cmd_item::flush_style()
{
    set_caption(menu_cmd_style_sheet::_caption);
    set_accel_key(menu_cmd_style_sheet::_accel_key);
}

void menu_cmd_item::get_caption_dimensions(int& w, int& h) const
{
    int tw, th;
    retrieve_text_dimensions(_caption, tw, th);
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
    retrieve_text_dimensions(str, tw, th);
    assert(_menu);
    w = tw + _menu->_text_horizontal_margin + _menu->_accel_reserved_space;
    h = th + _menu->_text_vertical_margin + _menu->_text_vertical_margin;
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
    _item_height = _icon_width = item_height;
    _caption_text_width = caption_text_width;
    _accel_text_width = accel_text_width;
    int total_width = _icon_width + caption_text_width + accel_text_width;
    /* do arrangement */
    int next_item_pos = 0;
    auto arrange_items = [&](ariel::widget* w)-> int {
        assert(w);
        auto* item = reinterpret_cast<menu_item*>(w->query_interface(uuid_menu_item));
        assert(item);
        switch(item->get_type())
        {
        case menu_item::separator:
            {
                w->move(rect(0, next_item_pos, total_width, _menu->_separator_space));
                next_item_pos += _menu->_separator_space;
                break;
            }
        case menu_item::sub_item:
        case menu_item::cmd_item:
            {
                w->move(rect(0, next_item_pos, total_width, item_height));
                next_item_pos += item_height;
                break;
            }
        }
        return 0;
    };
    traverse_child_widget(arrange_items);
    resize(total_width, next_item_pos);
}

menu_cmd_item* menu_group::add_cmd_item()
{
    auto* p = add_item<menu_cmd_item>(sm_hitable|sm_visible);
    assert(p);
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

void menu::on_press(uint um, unikey uk, const point& pt)
{
}

void menu::on_click(uint um, unikey uk, const point& pt)
{
}

void menu::on_hover(uint um, const point& pt)
{
    if(auto* item = hit_item(pt))
        item->to_widget()->on_hover(um, pt);
}

void menu::on_keydown(uint um, unikey uk)
{
}

void menu::flush_style()
{
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
    for( ; !_menu_stack.empty(); _menu_stack.pop_back()) {
        auto* p = _menu_stack.back();
        if(p == hold)
            break;
        p->hide();
    }
    assert(!_menu_stack.empty());
    auto* sub_menu = item->get_sub_menu();
    assert(sub_menu);
    auto pos = item->get_rect().top_right();
    pos.offset(hold->get_rect().top_left());
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
    auto* p = _manager->add_widget<menu_group>(this, nullptr, rect(), 0);
    assert(p);
    p->set_menu(this);
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
    localpt.offset(-localpt.x, -localpt.y);
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

};

__ariel_end__
