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

#include <assert.h>
#include <gslib/res.h>

__gslib_begin__

respack* get_default_respack()
{
    static respack inst;
    return &inst;
}

resvdir::resvdir(const gchar* path)
{
    assert(path);
    assert(is_valid_path(path));
    int len = strtool::length(path);
    assert(len > 0);
    for( ; len > 0 && path[len-1] != _t('\\') && path[len-1] != _t('/'); len --);
    assert(path[len-1] == _t('\\') || path[len-1] == _t('/'));
    _dir.assign(path, len);
    _file.assign(path+len);
}

resvdir::resvdir(const gchar* dir, const gchar* file)
{
    assert(dir && file);
    assert(is_valid_dir(dir));
    _dir.assign(dir);
    if(_dir.back() != _t('\\') && _dir.back() != _t('/'))
        _dir.push_back(_t('\\'));
    _file.assign(file);
}

const gchar* resvdir::get_postfix() const
{
    const gchar* str = _file.c_str();
    if(str[0] == 0)
        return 0;
    int pos = (int)_file.rfind(_t('.'));
    if(pos == string::npos || !str[pos+1])
        return 0;
    return str + pos + 1;
}

const gchar* resvdir::get_path(string& path) const
{
    path.assign(_dir);
    path.append(_file);
    return path.c_str();
}

bool resvdir::is_valid_path(const gchar* path)
{
    if(!path)
        return false;
    if(!is_valid_dir(path))
        return false;
    int len = strtool::length(path);
    assert(len > 0);
    if(path[len-1] == _t('\\') || path[len-1] == _t('/'))
        return false;
    return true;
}

bool resvdir::is_valid_dir(const gchar* dir)
{
    return !dir ? false :
        (dir[0] == _t(':') && (dir[1] == _t('\\') || dir[1] == _t('/'))
        );
}

const gchar* resvdir::get_root(const gchar* path)
{
    assert(path);
    assert(is_valid_dir(path));
    return path + 2;
}

const gchar* resvdir::get_next_node(const gchar* path)
{
    assert(path);
    for( ; path[0] && path[0] != _t('\\') && path[0] != _t('/'); path ++);
    return path;
}

resnode::resnode()
{
    _res = 0;
    _len = 0;
    _del = 0;
}

resnode::resnode(const gchar* key)
{
    _key = key;
    _res = 0;
    _len = 0;
    _del = 0;
}

void resnode::destroy()
{
    if(_del && _res) {
        _del(_res, _len);
        _del = 0;
        _len = 0;
        _res = 0;
    }
    _subs.clear();
}

void resnode::assign(resnode& that)
{
    destroy();
    /* keep the key */
    _res = that._res;
    _len = that._len;
    _del = that._del;
    that._res = 0;
    that._len = 0;
    that._del = 0;
    _subs.swap(that._subs);
}

resnode* resnode::add(const gchar* key, res* ptr, int len)
{
    assert(key);
    std::pair<iterator, bool> ret = _subs.insert(resnode(key));
    if(!ret.second)
        return 0;
    resnode* node = const_cast<resnode*>(&(*ret.first));    /* only if you don't change the key */
    node->set_resource(ptr, len);
    return node;
}

resnode* resnode::add(resnode* node)
{
    assert(node && node->get_key());
    resnode* ret = add(node->get_key());
    if(!ret)
        return 0;
    ret->assign(*node);
    return ret;
}

resnode* resnode::find(const gchar* key)
{
    assert(key);
    iterator i = _subs.find(resnode(key));
    return i == _subs.end() ? 0 : const_cast<resnode*>(&(*i));
}

const resnode* resnode::find(const gchar* key) const
{
    assert(key);
    const_iterator i = _subs.find(resnode(key));
    return i == _subs.end() ? 0 : &(*i);
}

resnode* respack::add(resnode* node, const gchar* dir, const gchar* file)
{
    assert(node && dir && file);
    const gchar* next = resvdir::get_next_node(dir);
    string key(dir, (int)(next-dir));
    resnode* nextnode = node->find(key.c_str());
    if(!nextnode)
        nextnode = node->add(key.c_str());
    return !next[0] ? nextnode->add(file) :
        add(nextnode, next+1, file);
}

resnode* respack::find(resnode* node, const gchar* path)
{
    assert(node && path);
    const gchar* next = resvdir::get_next_node(path);
    string key(path, (int)(next-path));
    resnode* nextnode = node->find(key.c_str());
    if(!nextnode)
        return 0;
    return !next[0] ? nextnode : find(nextnode, next+1);
}

const resnode* respack::find(const resnode* node, const gchar* path) const
{
    assert(node && path);
    const gchar* next = resvdir::get_next_node(path);
    string key(path, (int)(next-path));
    const resnode* nextnode = node->find(key.c_str());
    if(!nextnode)
        return 0;
    return !next[0] ? nextnode : find(nextnode, next+1);
}

__gslib_end__
