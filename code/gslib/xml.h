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
 
#ifndef xml_f13c2b07_0230_4967_8a16_0c276e19b8a2_h
#define xml_f13c2b07_0230_4967_8a16_0c276e19b8a2_h

#include <gslib/std.h>
#include <gslib/tree.h>

__gslib_begin__

struct xml_kvpair { string key, value; };
typedef xml_kvpair xml_attr;

struct xml_key:
    public xml_kvpair
{
    xml_key(const string& k) { key = k; }
    xml_key(const gchar* c) { key.assign(c); }
};

struct xml_kvp_hash:
    public std::unary_function<xml_kvpair, size_t>
{
    size_t operator()(const xml_kvpair& kvp) const
    { return string_hash(kvp.key); }
};

struct xml_kvp_equalto:
    public std::binary_function<xml_kvpair, xml_kvpair, bool>
{
    bool operator()(const xml_kvpair& p1, const xml_kvpair& p2) const
    { return p1.key == p2.key; }
};

template<class _kvpvsl>
struct xml_kvp_search
{
    static string* find(_kvpvsl& v, const string& k)
    {
        auto i = v.find(xml_key(k));
        return i != v.end() ? &(const_cast<string&>(i->value)) : 0;
    }
    static const string* find(const _kvpvsl& v, const string& k)
    {
        auto i = v.find(xml_key(k));
        return i != v.end() ? &(i->value) : 0;
    }
};

struct __gs_novtable xml_node abstract
{
    typedef unordered_set<xml_kvpair, xml_kvp_hash, xml_kvp_equalto> kvplist;
    typedef kvplist::iterator iterator;
    typedef kvplist::const_iterator const_iterator;

public:
    virtual bool is_value() const = 0;
    virtual void add_attribute(const xml_attr& attr) = 0;
    virtual string* get_attribute(const string& k) = 0;
    virtual const string* get_attribute(const string& k) const = 0;
    virtual void set_attribute(const gchar* k, int len1, const gchar* v, int len2) = 0;
    virtual int get_attribute_count() const = 0;
    virtual const string& get_name() const = 0;
    virtual void set_name(const gchar* str, int len) = 0;
    virtual string to_string() const = 0;   /* for debug */

public:
    string* get_attribute(const gchar* k) { return get_attribute(string(k)); }
    const string* get_attribute(const gchar* k) const { return get_attribute(string(k)); }
    string* get_attribute(const gchar* k, int len) { return get_attribute(string(k, len)); }
    const string* get_attribute(const gchar* k, int len) const { return get_attribute(string(k, len)); }
    bool has_attributes() const { return get_attribute_count() != 0; }
    void set_name(const string& str) { set_name(str.c_str(), str.length()); }
};

struct xml_element:
    public xml_node
{
    string      key;
    kvplist     attrlist;

public:
    bool is_value() const override { return false; }
    void add_attribute(const xml_attr& attr) override { attrlist.insert(attr); }
    string* get_attribute(const string& k) override { return xml_kvp_search<kvplist>::find(attrlist, k); }
    const string* get_attribute(const string& k) const override { return xml_kvp_search<kvplist>::find(attrlist, k); }
    void set_attribute(const gchar* k, int len1, const gchar* v, int len2) override
    {
        assert(k && v);
        string* vstr = get_attribute(string(k, len1));
        if(vstr) {
            vstr->assign(v, len2);
            return;
        }
        xml_attr attr;
        attr.key.assign(k, len1);
        attr.value.assign(v, len2);
        add_attribute(attr);
    }
    void set_attribute(const gchar* k, const gchar* v)
    {
        assert(k && v);
        set_attribute(k, strtool::length(k), v, strtool::length(v));
    }
    int get_attribute_count() const override { return (int)attrlist.size(); }
    const string& get_name() const override { return key; }
    void set_name(const gchar* str, int len) override { key.assign(str, len); }
    kvplist& get_attributes() { return attrlist; }
    const kvplist& const_attributes() const { return attrlist; }
    string to_string() const override;
};

struct xml_value:
    public xml_node
{
    string      value;

public:
    bool is_value() const override { return true; }
    void add_attribute(const xml_attr& attr) override { assert(!"error!"); }
    string* get_attribute(const string& k) override { return 0; }
    const string* get_attribute(const string& k) const override { return 0; }
    void set_attribute(const gchar* k, int len1, const gchar* v, int len2) override { assert(!"error!"); }
    int get_attribute_count() const override { return 0; }
    const string& get_name() const override { return value; }
    void set_name(const gchar* str, int len) override { value.assign(str, len); }
    string to_string() const override;
};

struct xml_enum
{
    typedef xml_node::iterator iterator;
    typedef xml_node::const_iterator const_iterator;

    xml_element*  node;
    iterator    iter;

    xml_enum(xml_node* n)
    {
        assert(n && !n->is_value());
        node = static_cast<xml_element*>(n);
        iter = node->attrlist.begin();
    }
    xml_enum(xml_node* n, iterator i)
    {
        assert(n && !n->is_value());
        node = static_cast<xml_element*>(n);
        iter = i;
    }
    void rewind(iterator i) { iter = i; }
    const xml_attr& get_attribute() const { return *iter; }
    bool next()
    {
        if(iter == node->attrlist.end())
            return false;
        return ++ iter != node->attrlist.end();
    }
    template<class _lambda>
    void for_each(_lambda lam) { for_each(node->attrlist.end(), lam); }
    template<class _lambda>
    void for_each(iterator end, _lambda lam)
    {
        iterator i = iter;
        for( ; i != end; ++ i)
            lam(*i);
    }
    template<class _lambda>
    void const_for_each(_lambda lam) const { const_for_each(node->attrlist.end(), lam); }
    template<class _lambda>
    void const_for_each(const_iterator end, _lambda lam) const
    {
        const_iterator i = iter;
        for( ; i != end; ++ i)
            lam(*i);
    }
};

struct xml_const_enum
{
    typedef xml_node::iterator iterator;
    typedef xml_node::const_iterator const_iterator;

    const xml_element* node;
    const_iterator  iter;

    xml_const_enum(const xml_node* n)
    {
        assert(n && !n->is_value());
        node = static_cast<const xml_element*>(n);
        iter = node->attrlist.begin();
    }
    xml_const_enum(const xml_node* n, const_iterator i)
    {
        assert(n && !n->is_value());
        node = static_cast<const xml_element*>(n);
        iter = i;
    }
    void rewind(const_iterator i) { iter = i; }
    const xml_attr& get_attribute() const { return *iter; }
    bool next()
    {
        if(iter == node->attrlist.end())
            return false;
        return ++ iter != node->attrlist.end();
    }
    template<class _lambda>
    void const_for_each(_lambda lam) const { const_for_each(node->attrlist.end(), lam); }
    template<class _lambda>
    void const_for_each(const_iterator end, _lambda lam) const
    {
        const_iterator i = iter;
        for( ; i != end; ++ i)
            lam(*i);
    }
};

typedef _treenode_wrapper<xml_node> xml_wrapper;

class xmltree:
    public tree<xml_node, xml_wrapper>
{
public:
    enum encodesys
    {
        encodesys_unknown,
        encodesys_ascii,
        encodesys_mbc,          /* multi-byte compatible */
        encodesys_wstr,         /* wide string */
    };
    enum encode
    {
        encode_unknown,
        encode_ansi,
        encode_gb2312,
        encode_utf8,
        encode_utf16,
    };
    encodesys   _encodesys;
    encode      _encode;

protected:
    static const gchar* skip(const gchar* str);
    static const gchar* skip(const gchar* str, const gchar* skp);
    static void replace(string& str, const gchar* s1, const gchar* s2);
    static void filter_entity_reference(string& s);
    static void write_attribute(string& str, const gchar* prefix, const xml_attr& attr, const gchar* postfix);
    static void close_write_item(string& str, const gchar* prefix, const xml_node* node, const gchar* postfix);
    static void begin_write_item(string& str, const gchar* prefix, const xml_node* node, const gchar* postfix);
    static void end_write_item(string& str, const gchar* prefix, const xml_node* node, const gchar* postfix);

protected:
    const gchar* check_header(const gchar* str, string& version, string& encoding);
    const gchar* read_attribute(const gchar* str, string& k, string& v);
    const gchar* read_item(const gchar* str, iterator p);
    void setup_encoding_info(const string& enc);
    void write_header(string& str) const;
    void write_item(string& str, const_iterator i) const;

public:
    xmltree();
    bool load(const gchar* filename);
    bool parse(const gchar* src);
    void output(string& str) const;
    void save(const gchar* filename) const;
    void set_encode(encode enc) { _encode = enc; }

public:
    /*
     * Unique path locater
     * It was designed to access the DOM easier by a specified string path.
     * Usage:
     * 1.basically, we use the string "xx/yy/zz" to specify a path, which means in current position, find a sub node named "xx",
     *   go to the sub node; then find "yy", then "zz", and so on.
     *   the separator could be "/", or backslash "\", as well.
     * 2.about the name selector "$".
     *   you might also write the path as "$xx/$yy/$zz", it was the same as above; the selector "$" was skippable.
     * 3.about the counter "@"
     *   you could add a counter into the path, like "$xx@3/yy/zz", in this case, the selector "$" was unskippable.
     *   you could make a nameless selection, like "$@3/yy/zz", which means in the first jump, it would always choose the third node.
     *   or simply wrote "$/yy/zz", which means it would always choose the first node in the first jump.
     * 4.about the condition specifier "#"
     *   you could add a condition content into the selector;
     *   a condition was defined like: "$xx#aa,bb=cc,!dd|!ee,ff~=gg",
     *   there were two major binary connectors of conditions, "," refer to logic "AND", and "|" refer to logic "OR";
     *   the priority of "OR" was higher than "AND".
     *   if you need a negative condition, use "!" to reverse the logic, and the "!" should always follow "|"
     *   if a condition content was like the above "aa", which means the selected node must have the attribute named "aa"
     *   if a condition content was wrote like "bb=cc", which means the selected node must have an attribute named "bb" and valued "cc"
     *   if a condition content was wrote like "ff~=gg", which means the selected node must have an attribute named "ff" and valued "gg",
     *   the comparison was case insensitive.
     * 5.the "$", "@", "#" could be used as a combo, and the announcement should always be in the specified order, $#@
     */
    iterator unique_path_locater(const gchar* ctlstr) { return unique_path_locater(get_root(), ctlstr); }
    static iterator unique_path_locater(iterator i, const gchar* ctlstr)
    {
        assert(ctlstr);
        string cpyctlstr(ctlstr);
        ctlstr = cpyctlstr.c_str(); /* for trap */
        ctlstr = upl_run_over(i, ctlstr);
        if(ctlstr && !ctlstr[0])
            return i;
        return iterator(0);
    }

public:
    static const gchar* upl_run_once(iterator& i, const gchar* ctlstr);
    static const gchar* upl_run_over(iterator& i, const gchar* ctlstr)
    {
        while(ctlstr && ctlstr[0]) {
            const gchar* ctlret = upl_run_once(i, ctlstr);
            if(!ctlret)
                return ctlstr;
            ctlstr = ctlret;
        }
        return ctlstr;
    }
};

__gslib_end__

#endif
