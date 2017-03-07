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

#include <gslib/vdir.h>
#include <gslib/file.h>
#include <gslib/error.h>
#include <gslib/dir.h>

__gslib_begin__

static const gchar* get_path_node(string& node, const gchar* path)
{
    assert(path && path[0]);
    const gchar* s = strtool::_test(path, _t("/\\"));
    if(!s) {
        node.assign(path);
        return 0;
    }
    node.assign(path, s - path);
    return ++ s;
}

void vdirnode::tracing() const
{
    trace(_t("%s\n"), name.c_str());
}

void dirop::get_current_dir(string& str)
{
    gchar buf[260];
    verify(_gs_getcwd(buf, 260) != 0);
    str.assign(buf);
}

bool dirop::change_dir(const gchar* path)
{
    assert(path);
    int err = _gs_chdir(path);
    return !err;
}

bool dirop::make_dir(const gchar* path)
{
    assert(path);
    string acc;
    const gchar* next = get_path_node(acc, path);
    if(acc.back() != _t(':'))   /* not root */
        _gs_mkdir(acc.c_str());
    if(!next)
        return true;
    path = next;
    while(path && path[0]) {
        string node;
        const gchar* next = get_path_node(node, path);
        acc += _t("\\");
        acc += node;
        _gs_mkdir(acc.c_str());
        path = next;
    }
    return true;
}

bool dirop::remove_dir(const gchar* path)
{
    assert(path);
    int err = _gs_rmdir(path);
    return !err;
}

const gchar* vdirfile::get_postfix() const
{
    int dotpos = name.find_last_of(_t('.'));
    if(dotpos == string::npos)
        return 0;
    return name.c_str() + dotpos + 1;
}

void vdirfile::set_file_data(const void* ptr, int size)
{
    assert(ptr && size > 0);
    vsl.store(ptr, size);
}

void vdirfile::set_file_data(const gchar* filename)
{
    assert(filename);
    file f(filename, _t("rb"));
    if(f.is_valid())
        return;
    int size = f.size();
    vsl.flex(size);
    int ct = 0;
    byte* buf = new byte[4096];
    for(;;) {
        int rdsz = f.get(buf, 4096);
        vsl.store(buf, rdsz);
        ct += rdsz;
        if(ct >= size)
            break;
    }
    delete [] buf;
}

vdiriter vdir::create_folder(vdiriter pos, const gchar* name)
{
    vdiriter i = birth_tail<vdirfolder>(pos);
    assert(i.is_valid());
    i->name.assign(name);
    return i;
}

vdiriter vdir::create_file(vdiriter pos, const gchar* name)
{
    vdiriter i = birth_tail<vdirfile>(pos);
    assert(i.is_valid());
    i->name.assign(name);
    return i;
}

vdiriter vdir::create_tag(vdiriter pos, const gchar* name)
{
    vdiriter i = birth_tail<vdirtag>(pos);
    assert(i.is_valid());
    i->name.assign(name);
    return i;
}

static void append_file_path(string& str, vdirciter i)
{
    if(!i.is_valid())
        return;
    vdirciter p = i.parent();
    if(p.is_valid())
        append_file_path(str, p);
    str += _t('\\');
    str += i->get_name();
}

static void save_virtual_file(const gchar* path, vdirciter i)
{
    assert(path && i.is_valid());
    string sp = path;
    if(sp.back() == _t('/') || sp.back() == _t('\\'))
        sp.pop_back();
    append_file_path(sp, i.parent());
    assert(i->get_tag() == vdirnode::dt_file);
    const vdirfile* vf = static_cast<const vdirfile*>(i.get_ptr());
    dirop dop;
    string oldpath;
    dop.get_current_dir(oldpath);
    dop.make_dir(sp.c_str());
    dop.change_dir(sp.c_str());
    file f(vf->get_name(), _t("wb"));
    f.put((const byte*)vf->vsl.get_ptr(), vf->vsl.get_cur());
    dop.change_dir(oldpath.c_str());
}

void vdir::save(const gchar* path)
{
    assert(path);
    preorder_traversal([&](vdirwrapper* w) {
        assert(w);
        vdirnode* node = w->get_ptr();
        assert(node);
        if(node->get_tag() == vdirnode::dt_file)
            save_virtual_file(path, vdiriter(w));
    });
}

static void get_dir_path(string& str, vdirciter ci)
{
    assert(ci.is_valid());
    vdirciter p = ci.parent();
    if(p.is_valid())
        get_dir_path(str, ci);
    str.append(ci->name);
}

void vdirop::get_current_dir(string& str)
{
    assert(_curr.is_valid());
    get_dir_path(str, _curr);
}

bool vdirop::change_dir(const gchar* path)
{
    assert(path);
    vdiriter i = step_to(_curr, path);
    if(i.is_valid()) {
        _curr = i;
        return true;
    }
    return false;
}

bool vdirop::make_dir(const gchar* path)
{
    assert(path);
    string node;
    vdiriter i = _curr;
    while(path && path[0]) {
        const gchar* next = get_path_node(node, path);
        i = step_or_create(i, node.c_str());
        path = next;
    }
    return true;
}

bool vdirop::remove_dir(const gchar* path)
{
    assert(path);
    vdiriter i = step_to(_curr, path);
    if(!i.is_valid())
        return false;
    if(i == _curr) {
        _curr.to_parent();
        assert(_curr.is_valid() && "root directory cannot be removed.");
        _dir.erase(i);
        return true;
    }
    _dir.erase(i);
    return true;
}

static bool is_reserved_path(const gchar* name)
{
    assert(name);
    int len = strtool::length(name);
    return (strtool::compare(name, _t("."), len) == 0) ||
        (strtool::compare(name, _t(".."), len) == 0);
}

vdiriter vdirop::step_or_create(vdiriter pos, const gchar* name)
{
    assert(name);
    vdiriter i = step_once(pos, name);
    if(i.is_valid())
        return i;
    if(is_reserved_path(name))
        return i;
    return _dir.create_folder(pos, name);
}

vdiriter vdirop::step_once(vdiriter pos, const gchar* name)
{
    assert(name);
    int len = strtool::length(name);
    if(strtool::compare(name, _t("."), len) == 0)
        return pos;
    else if(strtool::compare(name, _t(".."), len) == 0)
        return pos.is_valid() ? pos.parent() : vdiriter(0);
    return locate_folder(pos, name);
}

vdiriter vdirop::step_to(vdiriter pos, const gchar* path)
{
    assert(path);
    string node;
    vdiriter i = pos;
    while(path && path[0]) {
        const gchar* next = get_path_node(node, path);
        vdiriter j = step_once(i, node.c_str());
        if(!j.is_valid())
            return j;
        i = j, path = next;
    }
    return i;
}

vdiriter vdirop::locate_folder(vdiriter pos, const gchar* name)
{
    assert(name);
    if(!pos.is_valid()) {
        pos = _dir.get_root();
        if(pos.is_valid() &&
            pos->get_tag() == vdirnode::dt_folder &&
            pos->name == name
            )
            return pos;
        return vdiriter(0);
    }
    vdiriter i = pos.child();
    for( ; i.is_valid(); i.to_next()) {
        if((i->get_tag() == vdirnode::dt_folder) &&
            i->name.equal(name)
            )
            return i;
    }
    return vdiriter(0);
}

vdiriter vdirop::locate_file(vdiriter pos, const gchar* name)
{
    assert(name);
    if(!pos.is_valid()) {
        pos = _dir.get_root();
        if(pos.is_valid() &&
            pos->get_tag() == vdirnode::dt_file &&
            pos->name == name
            )
            return pos;
        return vdiriter(0);
    }
    vdiriter i = pos.child();
    for( ; i.is_valid(); i.to_next()) {
        if((i->get_tag() == vdirnode::dt_file) &&
            i->name.equal(name)
            )
            return i;
    }
    return vdiriter(0);
}

__gslib_end__
