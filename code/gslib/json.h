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

#ifndef json_58ea0e36_7aaa_49a1_b466_3bbbb2483c5b_h
#define json_58ea0e36_7aaa_49a1_b466_3bbbb2483c5b_h

#include <gslib/std.h>

__gslib_begin__

enum json_tag
{
    jst_array,
    jst_table,
    jst_pair,
    jst_value,
};

class json_node_value;

class __gs_novtable json_node abstract
{
public:
    virtual ~json_node() {}
    virtual json_tag get_tag() const = 0;
    virtual const string& get_name() const = 0;
    virtual json_node* duplicate() const = 0;   /* gs_del */
};

struct json_node_hash:
    public std::unary_function<json_node*, size_t>
{
public:
    size_t operator()(const json_node* p) const
    {
        assert(p);
        return string_hash(p->get_name());
    }
};

struct json_node_equalto:
    public std::binary_function<json_node*, json_node*, bool>
{
public:
    bool operator()(const json_node* p1, const json_node* p2) const
    {
        assert(p1 && p2);
        return p1->get_name() == p2->get_name();
    }
};

typedef unordered_set<json_node*, json_node_hash, json_node_equalto> json_node_map;
typedef vector<json_node*> json_node_list;

class json_node_array:
    public json_node
{
public:
    virtual ~json_node_array();
    virtual json_tag get_tag() const override { return jst_array; }
    virtual const string& get_name() const override { return _name; }
    virtual json_node* duplicate() const override;

protected:
    string              _name;
    json_node_list      _array;

public:
    void set_name(const gchar* str) { _name.assign(str); }
    void set_name(const gchar* str, int len) { _name.assign(str, len); }
    void set_name(const string& str) { _name = str; }
    bool is_empty() const { return _array.empty(); }
    int get_childs() const { return (int)_array.size(); }
    json_node* at(int i) const { return _array.at(i); }
    int parse(const gchar* src, int len);
    json_node_list& get_container() { return _array; }
};

class json_node_table:
    public json_node
{
public:
    virtual ~json_node_table();
    virtual json_tag get_tag() const override { return jst_table; }
    virtual const string& get_name() const override { return _name; }
    virtual json_node* duplicate() const override;

protected:
    string              _name;
    json_node_map       _table;

public:
    void set_name(const gchar* str) { _name.assign(str); }
    void set_name(const gchar* str, int len) { _name.assign(str, len); }
    void set_name(const string& str) { _name = str; }
    bool is_empty() const { return _table.empty(); }
    int get_childs() const { return (int)_table.size(); }
    json_node* find(const string& name) const;
    int parse(const gchar* src, int len);
    json_node_map& get_container() { return _table; }
};

class json_node_value:
    public json_node
{
public:
    virtual json_tag get_tag() const override { return jst_value; }
    virtual const string& get_name() const override { return _strval; }
    virtual json_node* duplicate() const override;

protected:
    string              _strval;

public:
    void set_value_string(const gchar* str) { _strval.assign(str); }
    void set_value_string(const gchar* str, int len) { _strval.assign(str, len); }
    void set_value_string(const string& str) { _strval = str; }
    int parse(const gchar* src, int len);
};

class json_node_pair:
    public json_node
{
public:
    virtual json_tag get_tag() const override { return jst_pair; }
    virtual const string& get_name() const override { return _name; }
    virtual json_node* duplicate() const override;

protected:
    string              _name;
    json_node_value     _value;

public:
    void set_name(const gchar* str) { _name.assign(str); }
    void set_name(const gchar* str, int len) { _name.assign(str, len); }
    void set_name(const string& str) { _name = str; }
    void set_value(const gchar* str) { _value.set_value_string(str); }
    void set_value(const gchar* str, int len) { _value.set_value_string(str, len); }
    void set_value(const string& str) { _value.set_value_string(str); }
    const json_node_value& get_value() const { return _value; }
    const string& get_value_string() const { return _value.get_name(); }
    int parse(const gchar* src, int len);
};

class json_key:
    public json_node_pair
{
public:
    json_key(const string& name) { _name = name; }
    json_key(const gchar* str) { _name.assign(str); }
    json_key(const gchar* str, int len) { _name.assign(str, len); }
};

class json_parser
{
public:
    json_parser() { _root = 0; }
    ~json_parser() { destroy(); }
    bool parse(const gchar* src, int len);
    bool parse(const gchar* filename);
    void destroy();
    json_node* get_root() const { return _root; }

protected:
    json_node*          _root;
};

__gslib_end__

#endif
