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

#ifndef vdir_b62819a0_88a2_4eb7_9a92_6d287a152304_h
#define vdir_b62819a0_88a2_4eb7_9a92_6d287a152304_h

#include <gslib/config.h>
#include <gslib/tree.h>

__gslib_begin__

class dirop
{
public:
    virtual void get_current_dir(string& str);
    virtual bool change_dir(const gchar* path);
    virtual bool make_dir(const gchar* path);
    virtual bool remove_dir(const gchar* path);
};

struct vdirnode;
struct vdirfile;
typedef vdirnode vdirfolder;
typedef _treenode_wrapper<vdirnode> vdirwrapper;
typedef tree<vdirnode, vdirwrapper> vdirtree;
typedef vdirtree::iterator vdiriter;
typedef vdirtree::const_iterator vdirciter;

struct vdirnode
{
    enum
    {
        dt_folder,
        dt_file,
        dt_tag,
    };
    string          name;

public:
    vdirnode() {}
    vdirnode(const gchar* n) { name.assign(n); }
    virtual uint get_tag() const { return dt_folder; }
    virtual const gchar* get_name() const { return name.c_str(); }
    void set_name(const gchar* str, int len) { name.assign(str, len); }
    void set_name(const string& str) { name = str; }
    void tracing() const;
};

struct vdirfile:
    public vdirnode
{
    vessel          vsl;

public:
    vdirfile() {}
    vdirfile(const gchar* n): vdirnode(n) {}
    virtual uint get_tag() const override { return dt_file; }

public:
    const gchar* get_postfix() const;
    void set_file_data(const void* ptr, int size);
    void set_file_data(const gchar* filename);
};

struct vdirtag:
    public vdirnode
{
    void*           binding;

public:
    vdirtag() { binding = 0; }
    vdirtag(const gchar* n): vdirnode(n) { binding = 0; }
    virtual uint get_tag() const override { return dt_tag; }
    void set_binding(void* p) { binding = p; }
    void* get_binding() const { return binding; }
};

class vdir:
    public vdirtree
{
public:
    vdiriter create_folder(vdiriter pos, const gchar* name);
    vdiriter create_file(vdiriter pos, const gchar* name);
    vdiriter create_tag(vdiriter pos, const gchar* name);
    void save(const gchar* path);
};

class vdirop:
    public dirop
{
public:
    vdirop(vdir& d): _dir(d) {}
    virtual void get_current_dir(string& str) override;
    virtual bool change_dir(const gchar* path) override;
    virtual bool make_dir(const gchar* path) override;
    virtual bool remove_dir(const gchar* path) override;

protected:
    vdir&           _dir;
    vdiriter        _curr;

public:
    vdiriter step_or_create(vdiriter pos, const gchar* name);
    vdiriter step_once(vdiriter pos, const gchar* name);
    vdiriter step_to(vdiriter pos, const gchar* path);
    vdiriter locate_folder(vdiriter pos, const gchar* name);
    vdiriter locate_file(vdiriter pos, const gchar* name);
    vdiriter create_folder(const gchar* name) { return _dir.create_folder(_curr, name); }
    vdiriter create_file(const gchar* name) { return _dir.create_file(_curr, name); }
    vdiriter get_current_iter() const { return _curr; }
    void rewind_curr() { _curr = _dir.get_root(); }
};

__gslib_end__

#endif
