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

#ifndef basis_40d2b174_3729_49b6_a8d6_5b7ac75a2483_h
#define basis_40d2b174_3729_49b6_a8d6_5b7ac75a2483_h

#include <gslib/std.h>
#include <gslib/string.h>
#include <gslib/tree.h>
#include <ariel/widget.h>

namespace ui_editor {

using gs::rect;
using gs::rectf;
using gs::point;
using gs::pointf;
using gs::string;
using gs::list;
using gs::unordered_map;
using gs::gchar;
using gs::uint;
using gs::real;
using gs::real32;
using gs::ariel::painter;
using gs::ariel::wsys_manager;
using gs::ariel::unikey;
typedef gs::ariel::widget core_widget;

struct ui_proc_node;
typedef unordered_map<string, string> ui_prop_map;
typedef unordered_map<string, ui_proc_node> ui_proc_map;

struct ui_proc_node
{
    string              args;
    string              injection;
};

// class ui_editor_configs
// {
// 
// };

struct ui_macro
{
    string              name;
    string              definition;

public:
    ui_macro(const string& n): name(n) {}
    ui_macro(const string& n, const string& d): name(n), definition(d) {}
};

typedef list<string> ui_includes;
typedef list<ui_macro> ui_macros;

class ui_src_entry
{
public:
    void add_include(const string& str);
    void add_include_group(const string& strgroup);
    void add_macro(const string& name) { _macro_list.push_back(ui_macro(name)); }
    void add_macro(const string& name, const string& def) { _macro_list.push_back(ui_macro(name, def)); }
    void set_preproc(const string& preproc) { _preproc_injections = preproc; }
    void set_postproc(const string& postproc) { _postproc_injections = postproc; }
    void set_output(const string& str) { _output_name = str; }
    void set_export(const string& str) { _export_name = str; }
    const ui_includes& get_includes() const { return _include_list; }
    const ui_macros& get_macros() const { return _macro_list; }
    const string& get_preproc() const { return _preproc_injections; }
    const string& get_postproc() const { return _postproc_injections; }
    const string& get_output() const { return _output_name; }
    const string& get_export() const { return _export_name; }

protected:
    ui_includes         _include_list;
    ui_macros           _macro_list;
    string              _preproc_injections;
    string              _postproc_injections;
    string              _output_name;       /* for output file name */
    string              _export_name;       /* for export class name */
};

class ui_editor_context
{
public:
    typedef unordered_map<string, int> var_index_map;

public:
    ui_src_entry& get_src_entry() { return _src_entry; }
    const ui_src_entry& const_src_entry() const { return _src_entry; }
    int acquire_variable_index(const string& vartype);
    int query_variable_index(const string& vartype) const;
    int acquire_variable_name(string& name, const string& vartype);

protected:
    ui_src_entry        _src_entry;
    var_index_map       _index_map;
};

class ui_node
{
public:
    rect                position;
    string              type;
    string              name;
    string              variable;
    bool                is_instance;
    ui_prop_map         prop_map;
    ui_proc_map         proc_map;
    core_widget*        assoc_widget;

public:
    ui_node(): is_instance(false), assoc_widget(nullptr) {}
};

typedef gs::_treenode_cpy_wrapper<ui_node> ui_node_wrapper;
typedef gs::tree<ui_node, ui_node_wrapper> ui_tree;

typedef core_widget* (__stdcall *pfn_create_widget)(wsys_manager* wsys, core_widget* parent, ui_node& uinode);

class ui_transparent_layer:
    public core_widget
{
public:
    ui_transparent_layer(wsys_manager* m);
    void install_reflection_widget(core_widget* refw);
    core_widget* get_reflection_widget() const { return _reflection_widget; }

public:
    virtual ~ui_transparent_layer();
    virtual void on_reflection_close();
    virtual void on_reflection_show(bool b);
    virtual void on_reflection_enable(bool b);
    virtual void on_reflection_move(const rect& rc);
    virtual void on_reflection_char(uint um, uint ch) {}
    virtual void on_reflection_caret() {}
    virtual void on_reflection_capture(bool b) {}
    virtual void on_reflection_focus(bool b) {}
    virtual void on_reflection_scroll(const point& pt, real32 scr, bool vert) {}
    virtual void on_reflection_accelerator(unikey key, uint mask) {}

protected:
    core_widget*        _reflection_widget;

protected:
    void sync_size_with_reflection();
};

class ui_edit_control;

class ui_editor_layer:
    public ui_transparent_layer
{
public:
    enum size_orient
    {
        so_none,
        so_n,
        so_s,
        so_e,
        so_w,
        so_ne,
        so_nw,
        so_se,
        so_sw,
    };

public:
    ui_editor_layer(wsys_manager* m);
    void initialize(ui_node& uinode);
    void install_reflection_widget(core_widget* refw);
    ui_node* get_ui_node() const { return _assoc_ui_node; }

public:
    virtual void on_reflection_press(uint um, unikey uk, const point& pt);
    virtual void on_reflection_click(uint um, unikey uk, const point& pt);
    virtual void on_reflection_hover(uint um, const point& pt);
    virtual void on_reflection_leave(uint um, const point& pt);
    virtual void on_reflection_keydown(uint um, unikey uk);
    virtual void on_reflection_keyup(uint um, unikey uk);

protected:
    ui_node*            _assoc_ui_node;
    ui_edit_control*    _edit_ctl;

protected:
    void refresh_cursor(const point& pt);
    size_orient get_size_orient(const point& pt, int accuracy = 3) const;
    void try_install_edit_control(const point& pt);
    void try_uninstall_edit_control();
    void refresh_edit_control(uint um, const point& pt);
};

extern bool ui_create_widgets(wsys_manager* wsys, core_widget* root_layer, ui_tree& uicore);
//extern bool ui_;

}

#endif
