/*
 * Copyright (c) 2016-2020 lymastee, All rights reserved.
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

#ifndef style_ac50d5be_0f99_48dc_908f_ab8a309b2a27_h
#define style_ac50d5be_0f99_48dc_908f_ab8a309b2a27_h

#include <gslib/string.h>
#include <gslib/std.h>
#include <ariel/type.h>
#include <ariel/painter.h>

__ariel_begin__

enum style_sheet_type
{
    sst_unknown     = -1,
    sst_color       = 0,
    sst_integer,
    sst_boolean,
    sst_float,
    sst_string,
    sst_capacity,
};

extern const string& get_style_sheet_type_name(style_sheet_type sst);
typedef std::pair<style_sheet_type, string> style_sheet_def;
typedef unordered_map<string, int> style_sheet_info_map;

struct accel_key;

class __gs_novtable style_sheet abstract
{
public:
    static const int npos = -1;

public:
    style_sheet(const style_sheet_def* ssp, int len);
    virtual ~style_sheet() {}
    virtual bool get_value(const string& name, string& value) = 0;
    virtual void set_value(const string& name, const string& value) = 0;
    virtual int get_content_size() const { return _ss_length; }
    virtual style_sheet_type get_content_type(int index) const;
    virtual const string& get_content_name(int index) const;
    virtual void flush_style() = 0;

protected:
    const style_sheet_def*  _ss_pairs;
    int                     _ss_length;
    style_sheet_info_map    _ss_info;

public:
    void initialize_style_sheet(const style_sheet_def* ssp, int len);
    int get_style_sheet_index(const string& name) const;

protected:
    static bool from_color(string& str, const color& cr);
    static bool to_color(color& cr, const string& str);
    static bool from_integer(string& str, int i);
    static bool to_integer(int& i, const string& str);
    static bool from_boolean(string& str, bool b);
    static bool to_boolean(bool& b, const string& str);
    static bool from_float(string& str, float f);
    static bool to_float(float& f, const string& str);
    static bool from_accel_key(string& str, const accel_key& k);
    static bool to_accel_key(accel_key& k, const string& str);
    static void setup_brush_by_color(painter_brush& brush, const color& cr);
    static void setup_pen_by_color(painter_pen& pen, const color& cr);
    static void setup_font(font& ft, const string& name, int size);
};

__ariel_end__

#endif
