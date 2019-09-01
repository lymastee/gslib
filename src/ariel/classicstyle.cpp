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

const uuid uuid_style_sheet(_t("a6facff7-20f7-486f-9fc0-78b7f68f06fb"));
const uuid uuid_menu_item(_t("5dca28c1-47c4-4ea8-8fac-440f2b57c2bc"));

static const style_sheet_def __widget_style_sheet_defs[] =
{
    std::make_pair(sst_color, _t("fill_color")),
    std::make_pair(sst_color, _t("stroke_color")),
    std::make_pair(sst_float, _t("fill_opacity")),
    std::make_pair(sst_float, _t("stroke_opacity")),
    std::make_pair(sst_color, _t("disable_fill_color")),
    std::make_pair(sst_color, _t("disable_stroke_color")),
    std::make_pair(sst_float, _t("disable_fill_opacity")),
    std::make_pair(sst_float, _t("disable_stroke_opacity")),
};

static const style_sheet_def __root_widget_style_sheet_defs[] =
{
    std::make_pair(sst_color, _t("background_color")),
    std::make_pair(sst_string, _t("background_image")),
};

static const style_sheet_def __placeholder_style_sheet_defs[] =
{
    std::make_pair(sst_color, _t("fill_color")),
    std::make_pair(sst_color, _t("stroke_color")),
    std::make_pair(sst_float, _t("fill_opacity")),
    std::make_pair(sst_float, _t("stroke_opacity")),
    std::make_pair(sst_string, _t("remark")),
    std::make_pair(sst_string, _t("remark_font_name")),
    std::make_pair(sst_integer, _t("remark_font_size")),
    std::make_pair(sst_color, _t("remark_font_color")),
};

static const style_sheet_def __button_style_sheet_defs[] =
{
    std::make_pair(sst_color, _t("normal_fill_color")),
    std::make_pair(sst_color, _t("normal_stroke_color")),
    std::make_pair(sst_color, _t("hover_fill_color")),
    std::make_pair(sst_color, _t("hover_stroke_color")),
    std::make_pair(sst_color, _t("press_fill_color")),
    std::make_pair(sst_color, _t("press_stroke_color")),
    std::make_pair(sst_color, _t("disable_fill_color")),
    std::make_pair(sst_color, _t("disable_stroke_color")),
    std::make_pair(sst_float, _t("normal_fill_opacity")),
    std::make_pair(sst_float, _t("normal_stroke_opacity")),
    std::make_pair(sst_float, _t("hover_fill_opacity")),
    std::make_pair(sst_float, _t("hover_stroke_opacity")),
    std::make_pair(sst_float, _t("press_fill_opacity")),
    std::make_pair(sst_float, _t("press_stroke_opacity")),
    std::make_pair(sst_float, _t("disable_fill_opacity")),
    std::make_pair(sst_float, _t("disable_stroke_opacity")),
    std::make_pair(sst_string, _t("caption")),
    std::make_pair(sst_string, _t("caption_font_name")),
    std::make_pair(sst_integer, _t("caption_font_size")),
    std::make_pair(sst_string, _t("caption_font_color")),
};

static const style_sheet_def __edit_line_style_sheet_defs[] =
{
    std::make_pair(sst_color, _t("normal_fill_color")),
    std::make_pair(sst_color, _t("normal_stroke_color")),
    std::make_pair(sst_float, _t("normal_fill_opacity")),
    std::make_pair(sst_float, _t("normal_stroke_opacity")),
    std::make_pair(sst_color, _t("focused_fill_color")),
    std::make_pair(sst_color, _t("focused_stroke_color")),
    std::make_pair(sst_float, _t("focused_fill_opacity")),
    std::make_pair(sst_float, _t("focused_stroke_opacity")),
    std::make_pair(sst_color, _t("disable_fill_color")),
    std::make_pair(sst_color, _t("disable_stroke_color")),
    std::make_pair(sst_float, _t("disable_fill_opacity")),
    std::make_pair(sst_float, _t("disable_stroke_opacity")),
    std::make_pair(sst_string, _t("text_font_name")),
    std::make_pair(sst_integer, _t("text_font_size")),
    std::make_pair(sst_color, _t("text_font_color")),
    std::make_pair(sst_float, _t("text_font_opacity")),
};

static const style_sheet_def __menu_style_sheet_defs[] =
{
    /* same with button style */
    std::make_pair(sst_color, _t("normal_fill_color")),
    std::make_pair(sst_color, _t("normal_stroke_color")),
    std::make_pair(sst_color, _t("hover_fill_color")),
    std::make_pair(sst_color, _t("hover_stroke_color")),
    std::make_pair(sst_color, _t("press_fill_color")),
    std::make_pair(sst_color, _t("press_stroke_color")),
    std::make_pair(sst_color, _t("disable_fill_color")),
    std::make_pair(sst_color, _t("disable_stroke_color")),
    std::make_pair(sst_float, _t("normal_fill_opacity")),
    std::make_pair(sst_float, _t("normal_stroke_opacity")),
    std::make_pair(sst_float, _t("hover_fill_opacity")),
    std::make_pair(sst_float, _t("hover_stroke_opacity")),
    std::make_pair(sst_float, _t("press_fill_opacity")),
    std::make_pair(sst_float, _t("press_stroke_opacity")),
    std::make_pair(sst_float, _t("disable_fill_opacity")),
    std::make_pair(sst_float, _t("disable_stroke_opacity")),
    std::make_pair(sst_string, _t("caption")),
    std::make_pair(sst_string, _t("caption_font_name")),
    std::make_pair(sst_integer, _t("caption_font_size")),
    std::make_pair(sst_string, _t("caption_font_color")),
    /* extensions */
    std::make_pair(sst_color, _t("border_color")),
    std::make_pair(sst_color, _t("separator_color")),
    std::make_pair(sst_integer, _t("separator_space")),
    std::make_pair(sst_integer, _t("text_horizontal_margin")),
    std::make_pair(sst_integer, _t("text_vertical_margin")),
    std::make_pair(sst_integer, _t("caption_reserved_space")),
    std::make_pair(sst_integer, _t("accel_reserved_space")),
};

static const style_sheet_def __menu_cmd_style_sheet_defs[] =
{
    std::make_pair(sst_string, _t("caption")),
    std::make_pair(sst_string, _t("accel_key")),
};

static const style_sheet_def __menu_sub_style_sheet_defs[] =
{
    std::make_pair(sst_string, _t("caption")),
};

static const style_sheet_def __menubar_button_style_sheet_defs[] =
{
    std::make_pair(sst_string, _t("caption")),
};

static const style_sheet_def __tree_view_style_sheet_defs[] =
{
    std::make_pair(sst_color, _t("view_fill_color")),
    std::make_pair(sst_color, _t("view_stroke_color")),
    std::make_pair(sst_float, _t("view_fill_opacity")),
    std::make_pair(sst_float, _t("view_stroke_opacity")),
    std::make_pair(sst_integer, _t("item_indent")),
    std::make_pair(sst_integer, _t("item_spacing")),
    std::make_pair(sst_color, _t("connect_line_color")),
};

widget_style_sheet::widget_style_sheet():
    ariel::style_sheet(__widget_style_sheet_defs, _countof(__widget_style_sheet_defs))
{
    _fill_color = color(245, 246, 247);
    _stroke_color = color(245, 246, 247, 0);
    _disable_fill_color = color(245, 246, 247);
    _disable_stroke_color = color(0, 0, 0);
}

bool widget_style_sheet::get_value(const string& name, string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        return from_color(value, _fill_color);
    case 1:
        return from_color(value, _stroke_color);
    case 2:
        return from_float(value, (float)_fill_color.alpha / 255.f);
    case 3:
        return from_float(value, (float)_stroke_color.alpha / 255.f);
    case 4:
        return from_color(value, _disable_fill_color);
    case 5:
        return from_color(value, _disable_stroke_color);
    case 6:
        return from_float(value, (float)_disable_fill_color.alpha / 255.f);
    case 7:
        return from_float(value, (float)_disable_stroke_color.alpha / 255.f);
    default:
        return false;
    }
}

void widget_style_sheet::set_value(const string& name, const string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        verify(to_color(_fill_color, value));
        break;
    case 1:
        verify(to_color(_stroke_color, value));
        break;
    case 2:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for widget_style_sheet.");
                return;
            }
            _fill_color.alpha = round(f * 255.f);
            break;
        }
    case 3:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for widget_style_sheet.");
                return;
            }
            _stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 4:
        verify(to_color(_disable_fill_color, value));
        break;
    case 5:
        verify(to_color(_disable_stroke_color, value));
        break;
    case 6:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for widget_style_sheet.");
                return;
            }
            _disable_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 7:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for widget_style_sheet.");
                return;
            }
            _disable_stroke_color.alpha = round(f * 255.f);
            break;
        }
    default:
        assert(!"unexpected style sheet name.");
        break;
    }
}

root_widget_style_sheet::root_widget_style_sheet():
    ariel::style_sheet(__root_widget_style_sheet_defs, _countof(__root_widget_style_sheet_defs))
{
    _bkground_color = color(245, 246, 247);
}

bool root_widget_style_sheet::get_value(const string& name, string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        return from_color(value, _bkground_color);
    case 1:
        value = _bkground_image;
        return true;
    default:
        return false;
    }
}

void root_widget_style_sheet::set_value(const string& name, const string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        verify(to_color(_bkground_color, value));
        break;
    case 1:
        _bkground_image = value;
        break;
    default:
        assert(!"unexpected style sheet name.");
        break;
    }
}

placeholder_style_sheet::placeholder_style_sheet():
    ariel::style_sheet(__placeholder_style_sheet_defs, _countof(__placeholder_style_sheet_defs))
{
    _fill_color = color(255, 255, 255);
    _stroke_color = color(128, 128, 128);
    _remark.assign(_t("ui placeholder"));
    _remark_font_name.assign(_t("Tahoma"));
    _remark_font_size = 10;
    _remark_font_color = color(0, 0, 0);
}

bool placeholder_style_sheet::get_value(const string& name, string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        return from_color(value, _fill_color);
    case 1:
        return from_color(value, _stroke_color);
    case 2:
        return from_float(value, (float)_fill_color.alpha / 255.f);
    case 3:
        return from_float(value, (float)_stroke_color.alpha / 255.f);
    case 4:
        value = _remark;
        return true;
    case 5:
        value = _remark_font_name;
        return true;
    case 6:
        return from_integer(value, _remark_font_size);
    case 7:
        return from_color(value, _remark_font_color);
    default:
        return false;
    }
}

void placeholder_style_sheet::set_value(const string& name, const string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        verify(to_color(_fill_color, value));
        break;
    case 1:
        verify(to_color(_stroke_color, value));
        break;
    case 2:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for widget_style_sheet.");
                return;
            }
            _fill_color.alpha = round(f * 255.f);
            break;
        }
    case 3:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for widget_style_sheet.");
                return;
            }
            _stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 4:
        _remark = value;
        break;
    case 5:
        _remark_font_name = value;
        break;
    case 6:
        verify(to_integer(_remark_font_size, value));
        break;
    case 7:
        verify(to_color(_remark_font_color, value));
        break;
    default:
        assert(!"unexpected style sheet name.");
        break;
    }
}

button_style_sheet::button_style_sheet():
    ariel::style_sheet(__button_style_sheet_defs, _countof(__button_style_sheet_defs))
{
    _normal_fill_color = color(245, 246, 247);
    _normal_stroke_color = color(245, 246, 247, 0);
    _hover_fill_color = color(232, 239, 247);
    _hover_stroke_color = color(164, 206, 249);
    _press_fill_color = color(201, 224, 247);
    _press_stroke_color = color(98, 162, 228);
    _disable_fill_color = color(245, 246, 247);
    _disable_stroke_color = color(0, 0, 0);
    _caption_font_name.assign(_t("Tahoma"));
    _caption_font_size = 10;
    _caption_font_color = color(0, 0, 0);
}

bool button_style_sheet::get_value(const string& name, string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        return from_color(value, _normal_fill_color);
    case 1:
        return from_color(value, _normal_stroke_color);
    case 2:
        return from_color(value, _hover_fill_color);
    case 3:
        return from_color(value, _hover_stroke_color);
    case 4:
        return from_color(value, _press_fill_color);
    case 5:
        return from_color(value, _press_stroke_color);
    case 6:
        return from_color(value, _disable_fill_color);
    case 7:
        return from_color(value, _disable_stroke_color);
    case 8:
        return from_float(value, (float)_normal_fill_color.alpha / 255.f);
    case 9:
        return from_float(value, (float)_normal_stroke_color.alpha / 255.f);
    case 10:
        return from_float(value, (float)_hover_fill_color.alpha / 255.f);
    case 11:
        return from_float(value, (float)_hover_stroke_color.alpha / 255.f);
    case 12:
        return from_float(value, (float)_press_fill_color.alpha / 255.f);
    case 13:
        return from_float(value, (float)_press_stroke_color.alpha / 255.f);
    case 14:
        return from_float(value, (float)_disable_fill_color.alpha / 255.f);
    case 15:
        return from_float(value, (float)_disable_stroke_color.alpha / 255.f);
    case 16:
        value = _caption;
        return true;
    case 17:
        value = _caption_font_name;
        return true;
    case 18:
        return from_integer(value, _caption_font_size);
    case 19:
        return from_color(value, _caption_font_color);
    default:
        return false;
    }
}

void button_style_sheet::set_value(const string& name, const string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        verify(to_color(_normal_fill_color, value));
        break;
    case 1:
        verify(to_color(_normal_stroke_color, value));
        break;
    case 2:
        verify(to_color(_hover_fill_color, value));
        break;
    case 3:
        verify(to_color(_hover_stroke_color, value));
        break;
    case 4:
        verify(to_color(_press_fill_color, value));
        break;
    case 5:
        verify(to_color(_press_stroke_color, value));
        break;
    case 6:
        verify(to_color(_disable_fill_color, value));
        break;
    case 7:
        verify(to_color(_disable_stroke_color, value));
        break;
    case 8:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _normal_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 9:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _normal_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 10:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _hover_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 11:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _hover_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 12:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _press_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 13:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _press_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 14:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _disable_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 15:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _disable_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 16:
        _caption = value;
        break;
    case 17:
        _caption_font_name = value;
        break;
    case 18:
        {
            int i;
            verify(to_integer(i, value));
            _caption_font_size = i;
            break;
        }
    case 19:
        verify(to_color(_caption_font_color, value));
        break;
    default:
        assert(!"unexpected style sheet name.");
        break;
    }
}

edit_line_style_sheet::edit_line_style_sheet():
    ariel::style_sheet(__edit_line_style_sheet_defs, _countof(__edit_line_style_sheet_defs))
{
    _normal_fill_color = color(255, 255, 255);
    _normal_stroke_color = color(23, 23, 23);
    _focus_fill_color = color(255, 255, 255);
    _focus_stroke_color = color(0, 120, 215);
    _disable_fill_color = color(127, 127, 127);
    _disable_stroke_color = color(0, 0, 0);
    _text_font_name.assign(_t("simsun"));
    _text_font_size = 10;
    _text_font_color = color(0, 0, 0);
}

bool edit_line_style_sheet::get_value(const string& name, string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        return from_color(value, _normal_fill_color);
    case 1:
        return from_color(value, _normal_stroke_color);
    case 2:
        return from_float(value, (float)_normal_fill_color.alpha / 255.f);
    case 3:
        return from_float(value, (float)_normal_stroke_color.alpha / 255.f);
    case 4:
        return from_color(value, _focus_fill_color);
    case 5:
        return from_color(value, _focus_stroke_color);
    case 6:
        return from_float(value, (float)_focus_fill_color.alpha / 255.f);
    case 7:
        return from_float(value, (float)_focus_stroke_color.alpha / 255.f);
    case 8:
        return from_color(value, _disable_fill_color);
    case 9:
        return from_color(value, _disable_stroke_color);
    case 10:
        return from_float(value, (float)_disable_fill_color.alpha / 255.f);
    case 11:
        return from_float(value, (float)_disable_stroke_color.alpha / 255.f);
    case 12:
        value = _text_font_name;
        return true;
    case 13:
        return from_integer(value, _text_font_size);
    case 14:
        return from_color(value, _text_font_color);
    case 15:
        return from_float(value, (float)_text_font_color.alpha / 255.f);
    default:
        return false;
    }
}

void edit_line_style_sheet::set_value(const string& name, const string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        verify(to_color(_normal_fill_color, value));
        break;
    case 1:
        verify(to_color(_normal_stroke_color, value));
        break;
    case 2:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for edit_style_sheet.");
                return;
            }
            _normal_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 3:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for edit_style_sheet.");
                return;
            }
            _normal_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 4:
        verify(to_color(_focus_fill_color, value));
        break;
    case 5:
        verify(to_color(_focus_stroke_color, value));
        break;
    case 6:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for edit_style_sheet.");
                return;
            }
            _focus_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 7:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for edit_style_sheet.");
                return;
            }
            _focus_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 8:
        verify(to_color(_disable_fill_color, value));
        break;
    case 9:
        verify(to_color(_disable_stroke_color, value));
        break;
    case 10:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for edit_style_sheet.");
                return;
            }
            _disable_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 11:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for edit_style_sheet.");
                return;
            }
            _disable_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 12:
        _text_font_name = value;
        break;
    case 13:
        verify(to_integer(_text_font_size, value));
        break;
    case 14:
        verify(to_color(_text_font_color, value));
        break;
    case 15:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for edit_style_sheet.");
                return;
            }
            _text_font_color.alpha = round(f * 255.f);
            break;
        }
    default:
        assert(!"unexpected style sheet name.");
        break;
    }
}

menu_style_sheet::menu_style_sheet():
    ariel::style_sheet(__menu_style_sheet_defs, _countof(__menu_style_sheet_defs))
{
    _normal_fill_color = color(245, 246, 247);
    _normal_stroke_color = color(245, 246, 247, 0);
    _hover_fill_color = color(232, 239, 247);
    _hover_stroke_color = color(164, 206, 249);
    _press_fill_color = color(201, 224, 247);
    _press_stroke_color = color(98, 162, 228);
    _disable_fill_color = color(245, 246, 247);
    _disable_stroke_color = color(0, 0, 0);
    _caption_font_color = color(0, 0, 0);
    _border_color = color(53, 53, 53);
    _separator_color = color(190, 195, 203);
    _separator_space = 1;
    _text_horizontal_margin = 5;
    _text_vertical_margin = 5;
    _caption_reserved_space = 40;
    _accel_reserved_space = 20;
    _caption_font_name.assign(_t("Tahoma"));
    _caption_font_size = 10;
    _normal_fill_color = color(255, 255, 255);
    _hover_fill_color = _press_fill_color;
    _hover_stroke_color = _press_stroke_color;
}

bool menu_style_sheet::get_value(const string& name, string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        return from_color(value, _normal_fill_color);
    case 1:
        return from_color(value, _normal_stroke_color);
    case 2:
        return from_color(value, _hover_fill_color);
    case 3:
        return from_color(value, _hover_stroke_color);
    case 4:
        return from_color(value, _press_fill_color);
    case 5:
        return from_color(value, _press_stroke_color);
    case 6:
        return from_color(value, _disable_fill_color);
    case 7:
        return from_color(value, _disable_stroke_color);
    case 8:
        return from_float(value, (float)_normal_fill_color.alpha / 255.f);
    case 9:
        return from_float(value, (float)_normal_stroke_color.alpha / 255.f);
    case 10:
        return from_float(value, (float)_hover_fill_color.alpha / 255.f);
    case 11:
        return from_float(value, (float)_hover_stroke_color.alpha / 255.f);
    case 12:
        return from_float(value, (float)_press_fill_color.alpha / 255.f);
    case 13:
        return from_float(value, (float)_press_stroke_color.alpha / 255.f);
    case 14:
        return from_float(value, (float)_disable_fill_color.alpha / 255.f);
    case 15:
        return from_float(value, (float)_disable_stroke_color.alpha / 255.f);
    case 16:
        value = _caption;
        return true;
    case 17:
        value = _caption_font_name;
        return true;
    case 18:
        return from_integer(value, _caption_font_size);
    case 19:
        return from_color(value, _caption_font_color);
    case 20:
        return from_color(value, _border_color);
    case 21:
        return from_color(value, _separator_color);
    case 22:
        return from_integer(value, _separator_space);
    case 23:
        return from_integer(value, _text_horizontal_margin);
    case 24:
        return from_integer(value, _text_vertical_margin);
    case 25:
        return from_integer(value, _caption_reserved_space);
    case 26:
        return from_integer(value, _accel_reserved_space);
    default:
        return false;
    }
}

void menu_style_sheet::set_value(const string& name, const string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        verify(to_color(_normal_fill_color, value));
        break;
    case 1:
        verify(to_color(_normal_stroke_color, value));
        break;
    case 2:
        verify(to_color(_hover_fill_color, value));
        break;
    case 3:
        verify(to_color(_hover_stroke_color, value));
        break;
    case 4:
        verify(to_color(_press_fill_color, value));
        break;
    case 5:
        verify(to_color(_press_stroke_color, value));
        break;
    case 6:
        verify(to_color(_disable_fill_color, value));
        break;
    case 7:
        verify(to_color(_disable_stroke_color, value));
        break;
    case 8:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _normal_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 9:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _normal_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 10:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _hover_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 11:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _hover_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 12:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _press_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 13:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _press_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 14:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _disable_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 15:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for button_style_sheet.");
                return;
            }
            _disable_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 16:
        _caption = value;
        break;
    case 17:
        _caption_font_name = value;
        break;
    case 18:
        {
            int i;
            verify(to_integer(i, value));
            _caption_font_size = i;
            break;
        }
    case 19:
        verify(to_color(_caption_font_color, value));
        break;
    case 20:
        verify(to_color(_border_color, value));
        break;
    case 21:
        verify(to_color(_separator_color, value));
        break;
    case 22:
        verify(to_integer(_separator_space, value));
        break;
    case 23:
        verify(to_integer(_text_horizontal_margin, value));
        break;
    case 24:
        verify(to_integer(_text_vertical_margin, value));
        break;
    case 25:
        verify(to_integer(_caption_reserved_space, value));
        break;
    case 26:
        verify(to_integer(_accel_reserved_space, value));
        break;
    default:
        assert(!"unexpected style sheet name.");
        break;
    }
}

menu_cmd_style_sheet::menu_cmd_style_sheet():
    ariel::style_sheet(__menu_cmd_style_sheet_defs, _countof(__menu_cmd_style_sheet_defs))
{
}

bool menu_cmd_style_sheet::get_value(const string& name, string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        value = _caption;
        return true;
    case 1:
        return from_accel_key(value, _accel_key);
    default:
        assert(!"unexpected style sheet name.");
        return false;
    }
}

void menu_cmd_style_sheet::set_value(const string& name, const string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        _caption = value;
        break;
    case 1:
        verify(to_accel_key(_accel_key, value));
        break;
    default:
        assert(!"unexpected style sheet name.");
        break;
    }
}

menu_sub_style_sheet::menu_sub_style_sheet():
    ariel::style_sheet(__menu_sub_style_sheet_defs, _countof(__menu_sub_style_sheet_defs))
{
}

bool menu_sub_style_sheet::get_value(const string& name, string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        value = _caption;
        return true;
    default:
        assert(!"unexpected style sheet name.");
        return false;
    }
}

void menu_sub_style_sheet::set_value(const string& name, const string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        _caption = value;
        break;
    default:
        assert(!"unexpected style sheet name.");
        break;
    }
}

menubar_style_sheet::menubar_style_sheet()
{
    /* todo */
}

menubar_button_style_sheet::menubar_button_style_sheet():
    ariel::style_sheet(__menubar_button_style_sheet_defs, _countof(__menubar_button_style_sheet_defs))
{
}

bool menubar_button_style_sheet::get_value(const string& name, string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        value = _caption;
        return true;
    default:
        assert(!"unexpected style sheet name.");
        return false;
    }
}

void menubar_button_style_sheet::set_value(const string& name, const string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        _caption = value;
        break;
    default:
        assert(!"unexpected style sheet name.");
        break;
    }
}

tree_view_style_sheet::tree_view_style_sheet():
    ariel::style_sheet(__tree_view_style_sheet_defs, _countof(__tree_view_style_sheet_defs))
{
    _view_fill_color = color(255, 255, 255);
    _view_stroke_color = color(23, 23, 23);
    _item_indent = 14;
    _item_spacing = 1;
    _connect_line_color = color(127, 127, 127);
}

bool tree_view_style_sheet::get_value(const string& name, string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        return from_color(value, _view_fill_color);
    case 1:
        return from_color(value, _view_stroke_color);
    case 2:
        return from_float(value, (float)_view_fill_color.alpha / 255.f);
    case 3:
        return from_float(value, (float)_view_stroke_color.alpha / 255.f);
    case 4:
        return from_integer(value, _item_indent);
    case 5:
        return from_integer(value, _item_spacing);
    case 6:
        return from_color(value, _connect_line_color);
    default:
        return false;
    }
}

void tree_view_style_sheet::set_value(const string& name, const string& value)
{
    int index = get_style_sheet_index(name);
    switch(index)
    {
    case 0:
        verify(to_color(_view_fill_color, value));
        break;
    case 1:
        verify(to_color(_view_stroke_color, value));
        break;
    case 2:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for tree_view_style_sheet.");
                return;
            }
            _view_fill_color.alpha = round(f * 255.f);
            break;
        }
    case 3:
        {
            float f;
            verify(to_float(f, value));
            if(f < 0.f || f > 1.f) {
                assert(!"invalid opacity for tree_view_style_sheet.");
                return;
            }
            _view_stroke_color.alpha = round(f * 255.f);
            break;
        }
    case 4:
        verify(to_integer(_item_indent, value));
        break;
    case 5:
        verify(to_integer(_item_spacing, value));
        break;
    case 6:
        verify(to_color(_connect_line_color, value));
        break;
    default:
        assert(!"unexpected style sheet name.");
        break;
    }
}

void* widget::query_interface(const uuid& uid)
{
    if(uid == uuid_style_sheet)
        return static_cast<style_sheet*>(this);
    return __super::query_interface(uid);
}

void widget::draw(painter* paint)
{
    assert(paint);
    if(is_enabled()) {
        paint->set_brush(_normal_brush);
        paint->set_pen(_normal_pen);
    }
    else {
        paint->set_brush(_disable_brush);
        paint->set_pen(_disable_pen);
    }
    paint->draw_rect(get_rectf());
}

void widget::flush_style()
{
    setup_brush_by_color(_normal_brush, _fill_color);
    setup_pen_by_color(_normal_pen, _stroke_color);
    setup_brush_by_color(_disable_brush, _disable_fill_color);
    setup_pen_by_color(_disable_pen, _disable_stroke_color);
}

void* root_widget::query_interface(const uuid& uid)
{
    if(uid == uuid_style_sheet)
        return static_cast<style_sheet*>(this);
    return __super::query_interface(uid);
}

void root_widget::draw(painter* paint)
{
    assert(paint);
    if(_bktex.get())
        paint->draw_image(_bktex.get(), 0.f, 0.f);
}

void root_widget::flush_style()
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

void* placeholder::query_interface(const uuid& uid)
{
    if(uid == uuid_style_sheet)
        return static_cast<style_sheet*>(this);
    return __super::query_interface(uid);
}

void placeholder::draw(painter* paint)
{
    assert(paint);
    paint->set_brush(_normal_brush);
    paint->set_pen(_normal_pen);
    paint->draw_rect(get_rectf());
    paint->set_font(_remark_font);
    if(!_remark.empty())
        paint->draw_text(_remark.c_str(), 0, 0, _remark_font_color, _remark.length());
}

void placeholder::flush_style()
{
    setup_brush_by_color(_normal_brush, _fill_color);
    setup_pen_by_color(_normal_pen, _stroke_color);
    setup_font(_remark_font, _remark_font_name, _remark_font_size);
}

void* button::query_interface(const uuid& uid)
{
    if(uid == uuid_style_sheet)
        return static_cast<style_sheet*>(this);
    return __super::query_interface(uid);
}

void button::enable(bool b)
{
    __super::enable(b);
    if(!b) {
        _current_brush = _disable_brush;
        _current_pen = _disable_pen;
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
    setup_brush_by_color(_disable_brush, _disable_fill_color);
    setup_pen_by_color(_disable_pen, _disable_stroke_color);
    setup_font(_caption_font, _caption_font_name, _caption_font_size);
    is_enabled() ? set_normal() : set_gray();
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
    _current_brush = _disable_brush;
    _current_pen = _disable_pen;
}

void* edit_line::query_interface(const uuid& uid)
{
    if(uid == uuid_style_sheet)
        return static_cast<style_sheet*>(this);
    return __super::query_interface(uid);
}

void edit_line::draw(painter* paint)
{
    assert(paint);
    set_font(_text_font);
    set_text_color(_text_font_color);
    __super::draw(paint);
}

void edit_line::flush_style()
{
    setup_brush_by_color(_normal_brush, _normal_fill_color);
    setup_pen_by_color(_normal_pen, _normal_stroke_color);
    setup_brush_by_color(_focus_brush, _focus_fill_color);
    setup_pen_by_color(_focus_pen, _focus_stroke_color);
    setup_brush_by_color(_disable_brush, _disable_fill_color);
    setup_pen_by_color(_disable_pen, _disable_stroke_color);
    setup_font(_text_font, _text_font_name, _text_font_size);
}

void edit_line::draw_background(painter* paint)
{
    assert(paint);
    paint->save();
    if(!is_enabled()) {
        paint->set_brush(_disable_brush);
        paint->set_pen(_disable_pen);
    }
    else if(is_focused()) {
        paint->set_brush(_focus_brush);
        paint->set_pen(_focus_pen);
    }
    else {
        paint->set_brush(_normal_brush);
        paint->set_pen(_normal_pen);
    }
    paint->draw_rect(get_rectf());
    paint->restore();
}

void horizontal_splitter::on_hover(uint um, const point& pt)
{
    __super::on_hover(um, pt);
    assert(_manager);
    _manager->set_cursor(cur_size_ns);
}

void horizontal_splitter::on_dragging(const point& pt)
{
    assert(_is_dragging);
    int ofsy = pt.y - _init_drag_pt.y;
    rect rc = get_rect();
    int y = rc.top + ofsy;
    y = gs_max(y, _min_range);
    y = gs_min(y, _max_range);
    rc.move_to(0, y);
    move(rc);
    if(_slider_notify)
        _slider_notify->on_slider_moved(this);
}

void vertical_splitter::on_hover(uint um, const point& pt)
{
    __super::on_hover(um, pt);
    assert(_manager);
    _manager->set_cursor(cur_size_we);
}

void vertical_splitter::on_dragging(const point& pt)
{
    int ofsx = pt.x - _init_drag_pt.x;
    rect rc = get_rect();
    int x = rc.left + ofsx;
    x = gs_max(x, _min_range);
    x = gs_min(x, _max_range);
    rc.move_to(x, 0);
    move(rc);
    if(_slider_notify)
        _slider_notify->on_slider_moved(this);
}

void container::move(const rect& rc)
{
    bool size_changed = (get_width() != rc.width() || get_height() != rc.height());
    __super::move(rc);
    if(size_changed)
        on_container_size_changed();
}

horizontal_layout_container::horizontal_layout_container(wsys_manager* m):
    container(m)
{
    _left_sub_widget = nullptr;
    _right_sub_widget = nullptr;
    _splitter = nullptr;
    _splitter_width = 5;
    _split_percent = 0.2f;
    _layout_width[0] = _layout_width[1] = 0;
}

void horizontal_layout_container::initialize_container()
{
    assert(!_splitter);
    auto* wsys = get_manager();
    assert(wsys);
    _splitter = wsys->add_widget<vertical_splitter>(this, nullptr, rect(0, 0, _splitter_width, get_height()), sm_visible | sm_hitable);
    assert(_splitter);
    _splitter->set_slider_notify(this);
    _splitter->set_range(0, get_width());
    _splitter->flush_style();
    split_by_percent(_split_percent);
}

void horizontal_layout_container::on_splitter_moved(splitter* sp)
{
    int p = _splitter->get_rect().left;
    float t = (float)p / (float)get_width();
    split_by_percent(t);
}

void horizontal_layout_container::on_container_size_changed()
{
    split_by_percent(_split_percent);
}

void horizontal_layout_container::set_split_percent(float t)
{
    if(t == _split_percent)
        return;
    split_by_percent(t);
}

rect horizontal_layout_container::get_left_sub_rect() const
{
    return rect(0, 0, _layout_width[0], get_height());
}

rect horizontal_layout_container::get_right_sub_rect() const
{
    return rect(_layout_width[0] + _splitter->get_width(), 0, _layout_width[1], get_height());
}

void horizontal_layout_container::split_by_percent(float sp)
{
    _split_percent = sp;
    assert(_splitter);
    int w = get_width();
    int h = get_height();
    int spw = _splitter->get_width();
    int restw = gs_max(0, w - spw);
    _layout_width[0] = round(_split_percent * restw);
    _layout_width[1] = gs_max(0, restw - _layout_width[0]);
    _splitter->move(rect(_layout_width[0], 0, spw, h));
    if(_left_sub_widget)
        _left_sub_widget->move(get_left_sub_rect());
    if(_right_sub_widget)
        _right_sub_widget->move(get_right_sub_rect());
}

vertical_layout_container::vertical_layout_container(wsys_manager* m):
    container(m)
{
    _up_sub_widget = nullptr;
    _down_sub_widget = nullptr;
    _splitter = nullptr;
    _splitter_width = 5;
    _split_percent = 0.2f;
    _layout_height[0] = _layout_height[1] = 0;
}

void vertical_layout_container::initialize_container()
{
    assert(!_splitter);
    auto* wsys = get_manager();
    assert(wsys);
    _splitter = wsys->add_widget<horizontal_splitter>(this, nullptr, rect(0, 0, get_width(), _splitter_width), sm_visible | sm_hitable);
    assert(_splitter);
    _splitter->set_slider_notify(this);
    _splitter->set_range(0, get_height());
    _splitter->flush_style();
    split_by_percent(_split_percent);
}

void vertical_layout_container::on_splitter_moved(splitter* sp)
{
    int p = _splitter->get_rect().top;
    float t = (float)p / (float)get_height();
    split_by_percent(t);
}

void vertical_layout_container::on_container_size_changed()
{
    split_by_percent(_split_percent);
}

void vertical_layout_container::set_split_percent(float t)
{
    if(t == _split_percent)
        return;
    split_by_percent(t);
}

rect vertical_layout_container::get_up_sub_rect() const
{
    return rect(0, 0, get_width(), _layout_height[0]);
}

rect vertical_layout_container::get_down_sub_rect() const
{
    return rect(0, _layout_height[0] + _splitter->get_height(), get_width(), _layout_height[1]);
}

void vertical_layout_container::split_by_percent(float sp)
{
    _split_percent = sp;
    assert(_splitter);
    int w = get_width();
    int h = get_height();
    int sph = _splitter->get_height();
    int resth = gs_max(0, h - sph);
    _layout_height[0] = round(_split_percent * resth);
    _layout_height[1] = gs_max(0, resth - _layout_height[0]);
    _splitter->move(rect(0, _layout_height[0], w, sph));
    if(_up_sub_widget)
        _up_sub_widget->move(get_up_sub_rect());
    if(_down_sub_widget)
        _down_sub_widget->move(get_down_sub_rect());
}

void horizontal_scroller::on_dragging(const point& pt)
{
    int ofsx = pt.x - _init_drag_pt.x;
    rect rc = get_rect();
    int x = rc.left + ofsx;
    x = gs_max(x, _min_range);
    x = gs_min(x, _max_range);
    rc.move_to(x, rc.top);
    move(rc);
    if(_slider_notify)
        _slider_notify->on_slider_moved(this);
}

void vertical_scroller::on_dragging(const point& pt)
{
    assert(_is_dragging);
    int ofsy = pt.y - _init_drag_pt.y;
    rect rc = get_rect();
    int y = rc.top + ofsy;
    y = gs_max(y, _min_range);
    y = gs_min(y, _max_range);
    rc.move_to(rc.left, y);
    move(rc);
    if(_slider_notify)
        _slider_notify->on_slider_moved(this);
}

scroll_button::scroll_button(wsys_manager* m):
    button(m)
{
    _btn_type = btn_up;
    _arrow_fill.set_tag(painter_brush::solid);
    _arrow_fill.set_color(color(0, 0, 0));
}

void scroll_button::enable(bool b)
{
    _arrow_fill.set_color(b ? color(0, 0, 0) : color(128, 128, 128));
}

void scroll_button::draw(painter* paint)
{
    __super::draw(paint);
    float w = (float)get_width(), h = (float)get_height();
    static const float d = 7.f;
    paint->save();
    paint->set_brush(_arrow_fill);
    paint->set_pen(painter_pen(painter_pen::none));
    switch(_btn_type)
    {
    case btn_up:
        draw_up_arrow(paint, rectf((w - d) / 2.f, (h - d / 2.f) / 2.f, d, d / 2.f));
        break;
    case btn_down:
        draw_down_arrow(paint, rectf((w - d) / 2.f, (h - d / 2.f) / 2.f, d, d / 2.f));
        break;
    case btn_left:
        draw_left_arrow(paint, rectf((w - d / 2.f) / 2.f, (h - d) / 2.f, d / 2.f, d));
        break;
    case btn_right:
        draw_right_arrow(paint, rectf((w - d / 2.f) / 2.f, (h - d) / 2.f, d / 2.f, d));
        break;
    }
    paint->restore();
}

void scroll_button::draw_up_arrow(painter* paint, const rectf& rc)
{
    painter_path path;
    pointf c;
    c.add(rc.top_left(), rc.top_right());
    c.scale(0.5f);
    path.move_to(c);
    path.line_to(rc.bottom_left());
    path.line_to(rc.bottom_right());
    path.close_path();
    paint->draw_path(path);
}

void scroll_button::draw_down_arrow(painter* paint, const rectf& rc)
{
    painter_path path;
    pointf c;
    c.add(rc.bottom_left(), rc.bottom_right());
    c.scale(0.5f);
    path.move_to(rc.top_left());
    path.line_to(c);
    path.line_to(rc.top_right());
    path.close_path();
    paint->draw_path(path);
}

void scroll_button::draw_left_arrow(painter* paint, const rectf& rc)
{
    painter_path path;
    pointf c;
    c.add(rc.top_left(), rc.bottom_left());
    c.scale(0.5f);
    path.move_to(rc.top_right());
    path.line_to(c);
    path.line_to(rc.bottom_right());
    path.close_path();
    paint->draw_path(path);
}

void scroll_button::draw_right_arrow(painter* paint, const rectf& rc)
{
    painter_path path;
    pointf c;
    c.add(rc.top_right(), rc.bottom_right());
    c.scale(0.5f);
    path.move_to(rc.top_left());
    path.line_to(rc.bottom_left());
    path.line_to(c);
    path.close_path();
    paint->draw_path(path);
}

scrollbar::scrollbar(wsys_manager* m):
    widget(m)
{
    _scroller = nullptr;
    _inc_button = _dec_button = nullptr;
    _scroll_pace = 2;
    _canvas_range = 1;
    _scroll_ratio = 0.f;
    _min_width = _min_height = 0;
    _hori_margin = _vert_margin = 1;
    _fill_color = color(184, 191, 199);
    _stroke_color = color(184, 191, 199, 0);
}

void scrollbar::move(const rect& rc)
{
    __super::move(
        rect(rc.left,
            rc.top,
            gs_max(_min_width, rc.width()),
            gs_max(_min_height, rc.height())
            )
        );
    on_scrollbar_size_changed();
}

void scrollbar::enable(bool b)
{
    __super::enable(b);
    assert(_scroller && _inc_button && _dec_button);
    _scroller->enable(is_enabled());
    _inc_button->enable(is_enabled());
    _dec_button->enable(is_enabled());
}

void scrollbar::scroll_by(int by)
{
    float t = calc_scroll_ratio(by);
    set_scroll_ratio(t);
}

void scrollbar::on_scrollbar_size_changed()
{
    layout_scrollbar();
}

void scrollbar::on_slider_moved(slider*)
{
    float t = calc_scroll_ratio(0);
    set_scroll_ratio(t);
}

void scrollbar::on_inc_button_clicked(uint um, unikey uk, const point& pt)
{
    scroll_by(_scroll_pace);
}

void scrollbar::on_dec_button_clicked(uint um, unikey uk, const point& pt)
{
    scroll_by(-_scroll_pace);
}

void scrollbar::set_canvas_range(int range)
{
    if(_canvas_range != range) {
        _canvas_range = range;
        layout_scrollbar();
    }
}

void scrollbar::set_scroll_ratio(float t)
{
    if(_scroll_ratio != t) {
        _scroll_ratio = t;
        adjust_scrollbar();
    }
}

void scrollbar::connect_scrollbar_notifications()
{
    assert(_inc_button && _dec_button);
    connect_notify(_inc_button, &scroll_button::on_click, this, &scrollbar::on_inc_button_clicked, 12);
    connect_notify(_dec_button, &scroll_button::on_click, this, &scrollbar::on_dec_button_clicked, 12);
    assert(_scroller);
    _scroller->set_slider_notify(this);
}

horizontal_scrollbar::horizontal_scrollbar(wsys_manager* m):
    scrollbar(m)
{
    _min_width = 36;
    _min_height = 18;
}

void horizontal_scrollbar::initialize_scrollbar(int range)
{
    assert(!_scroller);
    _scroller = _manager->add_widget<horizontal_scroller>(this, nullptr, rect(), sm_visible | sm_hitable);
    _scroller->flush_style();
    assert(!_dec_button);
    _dec_button = _manager->add_widget<scroll_button>(this, nullptr, rect(), sm_visible | sm_hitable);
    _dec_button->set_button_type(scroll_button::btn_left);
    _dec_button->flush_style();
    assert(!_inc_button);
    _inc_button = _manager->add_widget<scroll_button>(this, nullptr, rect(), sm_visible | sm_hitable);
    _inc_button->set_button_type(scroll_button::btn_right);
    _inc_button->flush_style();
    connect_scrollbar_notifications();
    _canvas_range = range;
    layout_scrollbar();
}

void horizontal_scrollbar::adjust_scrollbar()
{
    assert(_scroller && _dec_button && _inc_button);
    _dec_button->enable(is_enabled());
    _inc_button->enable(is_enabled());
    float scroll_range = (float)calc_scroll_range();
    assert(scroll_range >= 0.f);
    float view_range = (float)get_width();
    float scroller_width_ratio = gs_min(1.f, view_range / (float)_canvas_range);
    int scroller_width = round(scroller_width_ratio * scroll_range) - _hori_margin * 2;
    int scroller_height = calc_scroll_button_height();
    _scroller->move(rect(_hori_margin, _vert_margin, scroller_width, scroller_height));
    refresh_scroller_pos();
}

void horizontal_scrollbar::layout_scrollbar()
{
    assert(_scroller && _dec_button && _inc_button);
    int w = get_width();
    int h = get_height();
    int btn_width = calc_scroll_button_width();
    int btn_height = calc_scroll_button_height();
    _dec_button->move(rect(_hori_margin, _vert_margin, btn_width, btn_height));
    _inc_button->move(rect(w - btn_width - _hori_margin, _vert_margin, btn_width, btn_height));
    adjust_scrollbar();
    int start = calc_scroll_range_start();
    int end = start + calc_scroll_range() - _scroller->get_width();
    _scroller->set_range(start, end);
}

float horizontal_scrollbar::calc_scroll_ratio(int bias) const
{
    int sw = _scroller->get_width();
    int sp = _scroller->get_rect().left;
    int start = calc_scroll_range_start();
    int range = calc_scroll_range() - sw;
    int p = sp - start + bias;
    p = gs_min(gs_max(p, 0), range);
    float f = ((float)p) / ((float)range);
    return f;
}

int horizontal_scrollbar::calc_scroll_range() const
{
    int w = get_width();
    return w - (calc_scroll_button_width() + _hori_margin * 2) * 2;
}

int horizontal_scrollbar::calc_scroll_button_width() const
{
    return round(0.8f * calc_scroll_button_height());
}

int horizontal_scrollbar::calc_scroll_button_height() const
{
    int h = get_height();
    return h - _vert_margin * 2;
}

int horizontal_scrollbar::calc_scroll_range_start() const
{
    assert(_dec_button);
    int dock_pos = _dec_button->get_rect().right;
    return dock_pos + _hori_margin;
}

void horizontal_scrollbar::refresh_scroller_pos()
{
    int scroller_width = _scroller->get_width();
    int scroll_range = calc_scroll_range();
    assert(scroller_width <= scroll_range);
    int pos = round(_scroll_ratio * (float)(scroll_range - scroller_width));
    int scroll_start = calc_scroll_range_start();
    _scroller->move(point(scroll_start + pos, _vert_margin));
}

vertical_scrollbar::vertical_scrollbar(wsys_manager* m):
    scrollbar(m)
{
    _min_width = 18;
    _min_height = 36;
}

void vertical_scrollbar::initialize_scrollbar(int range)
{
    assert(!_scroller);
    _scroller = _manager->add_widget<vertical_scroller>(this, nullptr, rect(), sm_visible | sm_hitable);
    _scroller->flush_style();
    assert(!_dec_button);
    _dec_button = _manager->add_widget<scroll_button>(this, nullptr, rect(), sm_visible | sm_hitable);
    _dec_button->set_button_type(scroll_button::btn_up);
    _dec_button->flush_style();
    assert(!_inc_button);
    _inc_button = _manager->add_widget<scroll_button>(this, nullptr, rect(), sm_visible | sm_hitable);
    _inc_button->set_button_type(scroll_button::btn_down);
    _inc_button->flush_style();
    connect_scrollbar_notifications();
    _canvas_range = range;
    layout_scrollbar();
}

void vertical_scrollbar::adjust_scrollbar()
{
    assert(_scroller && _dec_button && _inc_button);
    _dec_button->enable(is_enabled());
    _inc_button->enable(is_enabled());
    float scroll_range = (float)calc_scroll_range();
    assert(scroll_range >= 0.f);
    float view_range = (float)get_height();
    float scroller_height_ratio = gs_min(1.f, view_range / (float)_canvas_range);
    int scroller_width = calc_scroll_button_width();
    int scroller_height = round(scroller_height_ratio * scroll_range) - _vert_margin * 2;
    _scroller->move(rect(_hori_margin, _vert_margin, scroller_width, scroller_height));
    refresh_scroller_pos();
}

void vertical_scrollbar::layout_scrollbar()
{
    assert(_scroller && _dec_button && _inc_button);
    int w = get_width();
    int h = get_height();
    int btn_width = calc_scroll_button_width();
    int btn_height = calc_scroll_button_height();
    _dec_button->move(rect(_hori_margin, _vert_margin, btn_width, btn_height));
    _inc_button->move(rect(_hori_margin, h - btn_height - _vert_margin, btn_width, btn_height));
    adjust_scrollbar();
    int start = calc_scroll_range_start();
    int end = start + calc_scroll_range() - _scroller->get_height();
    _scroller->set_range(start, end);
}

float vertical_scrollbar::calc_scroll_ratio(int bias) const
{
    int sh = _scroller->get_height();
    int sp = _scroller->get_rect().top;
    int start = calc_scroll_range_start();
    int range = calc_scroll_range() - sh;
    int p = sp - start + bias;
    p = gs_min(gs_max(p, 0), range);
    float f = ((float)p) / ((float)range);
    return f;
}

int vertical_scrollbar::calc_scroll_range() const
{
    int h = get_height();
    return h - (calc_scroll_button_height() + _vert_margin * 2) * 2;
}

int vertical_scrollbar::calc_scroll_button_width() const
{
    int w = get_width();
    return w - _hori_margin * 2;
}

int vertical_scrollbar::calc_scroll_button_height() const
{
    return round(0.8f * calc_scroll_button_width());
}

int vertical_scrollbar::calc_scroll_range_start() const
{
    assert(_dec_button);
    int dock_pos = _dec_button->get_rect().bottom;
    return dock_pos + _vert_margin;
}

void vertical_scrollbar::refresh_scroller_pos()
{
    int scroller_height = _scroller->get_height();
    int scroll_range = calc_scroll_range();
    assert(scroller_height <= scroll_range);
    int pos = round(_scroll_ratio * (float)(scroll_range - scroller_height));
    int scroll_start = calc_scroll_range_start();
    _scroller->move(point(_hori_margin, scroll_start + pos));
}

class raw_tree_view
{
public:
    raw_tree_view(tree_view* ptv)
    {
        _view = ptv;
    }

private:
    tree_view*          _view;
};

tree_view::tree_view(wsys_manager* m):
    ariel::widget(m)
{
    _raw_view = new raw_tree_view(this);
}

tree_view::~tree_view()
{
    if(_raw_view) {
        delete _raw_view;
        _raw_view = nullptr;
    }
}

void* tree_view::query_interface(const uuid& uid)
{
    if(uid == uuid_style_sheet)
        return static_cast<style_sheet*>(this);
    return __super::query_interface(uid);
}

void tree_view::draw(painter* paint)
{
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
    if(uid == uuid_menu_item)
        return static_cast<menu_item*>(this);
    else if(uid == uuid_style_sheet)
        return static_cast<style_sheet*>(this);
    return __super::query_interface(uid);
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
    if(uid == uuid_menu_item)
        return static_cast<menu_item*>(this);
    else if(uid == uuid_style_sheet)
        return static_cast<style_sheet*>(this);
    return __super::query_interface(uid);
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

void* menu::query_interface(const uuid& uid)
{
    if(uid == uuid_style_sheet)
        return static_cast<style_sheet*>(this);
    return __super::query_interface(uid);
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
    setup_brush_by_color(_menu_disable_brush, _disable_fill_color);
    setup_pen_by_color(_menu_disable_pen, _disable_stroke_color);
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

void* menubar_button::query_interface(const uuid& uid)
{
    if(uid == uuid_style_sheet)
        return static_cast<style_sheet*>(this);
    return __super::query_interface(uid);
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

void* menubar::query_interface(const uuid& uid)
{
    if(uid == uuid_style_sheet)
        return static_cast<style_sheet*>(this);
    return __super::query_interface(uid);
}

void menubar::flush_style()
{
    setup_brush_by_color(_menubar_normal_brush, _normal_fill_color);
    setup_pen_by_color(_menubar_normal_pen, _normal_stroke_color);
    setup_brush_by_color(_menubar_hover_brush, _hover_fill_color);
    setup_pen_by_color(_menubar_hover_pen, _hover_stroke_color);
    setup_brush_by_color(_menubar_press_brush, _press_fill_color);
    setup_pen_by_color(_menubar_press_pen, _press_stroke_color);
    setup_brush_by_color(_menubar_disable_brush, _disable_fill_color);
    setup_pen_by_color(_menubar_disable_pen, _disable_stroke_color);
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
