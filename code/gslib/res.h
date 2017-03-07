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

#ifndef res_92f78d87_1896_40fa_bc63_c52e2793c970_h
#define res_92f78d87_1896_40fa_bc63_c52e2793c970_h

#include <gslib/config.h>
#include <gslib/std.h>

__gslib_begin__

class resvdir
{
public:
    resvdir(const gchar* path);
    resvdir(const gchar* dir, const gchar* file);
    const gchar* get_dir() const { return _dir.c_str(); }
    const gchar* get_file() const { return _file.c_str(); }
    const gchar* get_postfix() const;
    const gchar* get_path(string& path) const;

public:
    static bool is_valid_path(const gchar* path);
    static bool is_valid_dir(const gchar* dir);
    static const gchar* get_root(const gchar* path);
    static const gchar* get_next_node(const gchar* path);

protected:
    string      _dir;
    string      _file;
};

typedef unsigned char res;

class resnode
{
public:
    struct hash
    {
        size_t operator()(const resnode& that) const
        { return string_hash(that._key); }
    };
    struct equal_to
    {
        bool operator()(const resnode& r1, const resnode& r2) const
        { return string_hash(r1._key) == string_hash(r2._key); }
    };
    typedef unordered_set<resnode, hash, equal_to> resset;
    typedef resset::iterator iterator;
    typedef resset::const_iterator const_iterator;
    typedef void (*fndestroy)(res*, int);

public:
    resnode();
    resnode(const gchar* key);
    resnode(const resnode& that) { assign(const_cast<resnode&>(that)); }
    ~resnode() { destroy(); }
    void set_destroy(fndestroy del) { _del = del; }
    void set_resource(res* ptr, int len) { _res = ptr; _len = len; }
    res* get_resource() const { return _res; }
    const gchar* get_key() const { return _key.size() ? _key.c_str() : 0; }
    void destroy();
    void assign(resnode& that);
    bool operator<(const resnode& that) const { return _key < that._key; }
    iterator begin() { return _subs.begin(); }
    const_iterator begin() const { return _subs.end(); }
    iterator end() { return _subs.end(); }
    const_iterator end() const { return _subs.end(); }
    resnode* add(const gchar* key, res* ptr = 0, int len = 0);
    resnode* add(resnode* node);
    resnode* find(const gchar* key);
    const resnode* find(const gchar* key) const;

protected:
    string      _key;
    res*        _res;
    int         _len;
    fndestroy   _del;
    resset      _subs;
};

class respack
{
public:
    typedef resnode::resset resset;
    typedef resnode::iterator iterator;
    typedef resnode::const_iterator const_iterator;
    typedef resnode::fndestroy fndestroy;

public:
    respack(): _root(_t(":")) {}
    resnode* get_root() { return &_root; }
    const resnode* get_root() const { return &_root; }
    resnode* add(const gchar* path) { return add(resvdir(path)); }
    resnode* add(const resvdir& vdir) { return add(get_root(), resvdir::get_root(vdir.get_dir()), vdir.get_file()); }
    resnode* add(resnode* node, const gchar* dir, const gchar* file);
    resnode* find(const gchar* path) { return find(get_root(), resvdir::get_root(path)); }
    resnode* find(resnode* node, const gchar* path);
    const resnode* find(const gchar* path) const { return find(get_root(), resvdir::get_root(path)); }
    const resnode* find(const resnode* node, const gchar* path) const;
    resnode* reg(const gchar* path, res* ptr, int len, fndestroy del = 0)
    {
        if(resnode* node = add(path)) {
            node->set_resource(ptr, len);
            node->set_destroy(del);
            return node;
        }
        return 0;
    }

protected:
    resnode     _root;
};

extern respack* get_default_respack();

__gslib_end__

#endif
