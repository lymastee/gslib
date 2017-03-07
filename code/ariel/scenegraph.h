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

#ifndef scenegraph_979fc7df_3f68_4a38_9dea_5eed6d400c98_h
#define scenegraph_979fc7df_3f68_4a38_9dea_5eed6d400c98_h

#include <gslib/type.h>
#include <gslib/tree.h>
#include <ariel/config.h>

__ariel_begin__

class scene_node
{
public:
    //typedef gs::string string;
    typedef const string* sntype;
    typedef void* snptr;
    typedef snptr (*fnaccess)(int);
    typedef void (*fndestroy)(snptr);

protected:
    sntype          _tag;
    snptr           _ptr;
    string          _key;
    fndestroy       _del;

public:
    /* sntype declaration */
#define declare_sntype(snt) \
    static sntype snt;
#include <ariel\snt.h>
#undef declare_sntype

    static void register_sntypes();

public:
    scene_node() { reset(); }
    scene_node(const gchar* name)
    {
        reset();
        set_key(name);
    }
    scene_node(const gchar* name, sntype tag, snptr ptr, fndestroy del = 0)
    {
        set_key(name);
        bind(tag, ptr, del);
    }
    ~scene_node()
    {
        destroy();
        reset();
    }
    void reset()
    {
        _tag = sn_void;
        _ptr = 0;
        _del = 0;
    }
    void destroy() { if(_ptr && _del) _del(_ptr); }
    sntype get_type() const { return _tag; }
    snptr get_ptr() const { return _ptr; }
    const gchar* get_name() const { return _key.c_str(); }
    const string& get_key() const { return _key; }
    void set_destroy(fndestroy del) { _del = del; }
    void set_nodestroy() { set_destroy(0); }
    void set_key(const gchar* key) { _key.assign(key); }
    void bind(sntype tag, snptr ptr, fndestroy del = 0)
    {
        _tag = tag;
        _ptr = ptr;
        set_destroy(del);
    }
    void rebind(sntype tag, snptr ptr, fndestroy del = 0)
    {
        destroy();
        bind(tag, ptr, del);
    }
    scene_node& operator=(const scene_node& that)
    {
        _tag = that._tag;
        _ptr = that._ptr;
        _key = that._key;
        _del = that._del;
        const_cast<scene_node&>(that)._del = 0;
        return *this;
    }
};

struct scene_key
{
    const gchar*    _key;
    scene_key() { _key = 0; }
    scene_key(const gchar* k) { _key = k; }
    scene_key(const scene_node* n) { _key = n->get_name(); }
};

class scene_graph
{
public:
    struct snkey_hash
    {
        size_t operator()(const scene_key& key) const
        { return string_hash(key._key); }
    };
    struct snkey_equal
    {
        bool operator()(const scene_key& k1, const scene_key& k2) const
        { return string_hash(k1._key) == string_hash(k2._key); }
    };

    typedef tree<scene_node> sntree;
    typedef sntree::wrapper wrapper;
    typedef sntree::iterator iterator;
    typedef sntree::const_iterator const_iterator;
    typedef unordered_map<scene_key, scene_node*, snkey_hash, snkey_equal> sntable;
    typedef scene_node::sntype sntype;
    typedef scene_node::snptr snptr;
    typedef scene_node::fndestroy fndestroy;

public:
    template<class castop, int bias, class castp>
    static castop fbop_cast(castp ptr) { return ptr ? reinterpret_cast<castop>(((int)ptr) + bias) : 0; }
    template<class castop, class castp>
    static castop fbop_cast(castp ptr, int bias) { return ptr ? reinterpret_cast<castop>(((int)ptr) + bias) : 0; }
    static int wrapper_node_bias()
    {
        static wrapper inst;
        static const int bias = (int)&inst - ((int)inst.get_ptr());
        return bias;
    }
    static wrapper* node_to_wrapper(scene_node* ptr) { return fbop_cast<wrapper*>(ptr, -wrapper_node_bias()); }
    static const wrapper* node_to_wrapper(const scene_node* ptr) { return fbop_cast<const wrapper*>(ptr, -wrapper_node_bias()); }
    static scene_node* iter_to_node(iterator i) { return i.get_ptr(); }
    static const scene_node* iter_to_node(const_iterator i) { return i.get_ptr(); }
    static iterator node_to_iter(scene_node* ptr) { return iterator(node_to_wrapper(ptr)); }
    static const_iterator node_to_iter(const scene_node* ptr) { return const_iterator(node_to_wrapper(ptr)); }

public:
    scene_graph();
    virtual ~scene_graph() {}

public:
    scene_node* get_root_node() { return iter_to_node(_sntree.get_root()); }
    const scene_node* get_root_node() const { return iter_to_node(_sntree.get_root()); }
    bool is_node_attached(const scene_node* node) const { return node && _sntree.is_mine(node_to_iter(node)); }
    sntree* get_node_tree(const scene_node* node);
    const sntree* get_node_tree(const scene_node* node) const;
    void fix_node(scene_node* old, scene_node* new1);
    void map_node(scene_node* node, const gchar* name);
    void unmap_node(scene_node* node);

public:
    virtual scene_node* create_node(sntype tag = scene_node::sn_void, snptr ptr = 0, fndestroy del = 0);
    virtual scene_node* create_node(const gchar* name, sntype tag = scene_node::sn_void, snptr ptr = 0, fndestroy del = 0);
    virtual scene_node* attach_node(scene_node* parent, scene_node* node);
    virtual scene_node* detach_node(scene_node* node);
    virtual void destroy_node(scene_node* node);
    virtual void clear_miscs();

protected:
    sntree          _sntree;
    sntree          _miscs;
    sntable         _sntable;
};

__ariel_end__

#endif
