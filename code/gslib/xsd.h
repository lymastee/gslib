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

#ifndef xsd_284c8467_1f5e_4ae4_b645_077382bf6ab4_h
#define xsd_284c8467_1f5e_4ae4_b645_077382bf6ab4_h

#include <gslib/config.h>
#include <gslib/xml.h>
#include <gslib/std.h>
#include <gslib/xsdtypes.h>

__gslib_begin__

//#define xsd_def_

class xsd_schema;
struct xsd_extern_schema;
class xsd_node;
class xsd_element;
class xsd_attribute;
class xsd_group;
class xsd_attribute_group;
class xsd_simple_type;
class xsd_complex_type;

typedef xml_wrapper xsd_wrapper;
typedef vector<xsd_wrapper*> xsd_wrappers;
typedef xmltree::iterator xsd_iterator;
typedef xmltree::const_iterator xsd_const_iterator;

template<class _ty>
struct xsd_schema_ns_hash:
    public std::unary_function<_ty, size_t>
{
    size_t operator()(_ty t) const
    {
        assert(t);
        return string_hash(t->get_namespace());
    }
};

template<class _ty>
struct xsd_schema_ns_equalto:
    public std::binary_function<_ty, _ty, bool>
{
    bool operator()(_ty t1, _ty t2) const
    {
        assert(t1 && t2);
        return t1->get_namespace() == t2->get_namespace();
    }
};

template<class _ty>
struct xsd_schema_loc_hash:
    public std::unary_function<_ty, size_t>
{
    size_t operator()(_ty t) const
    {
        assert(t);
        return string_hash(t->get_location());
    }
};

template<class _ty>
struct xsd_schema_loc_equalto:
    public std::binary_function<_ty, _ty, bool>
{
    bool operator()(_ty t1, _ty t2) const
    {
        assert(t1 && t2);
        return t1->get_location() == t2->get_location();
    }
};

template<class _ty>
struct xsd_name_hash:
    public std::unary_function<_ty, size_t>
{
    size_t operator()(_ty t) const
    {
        assert(t);
        return string_hash(t->get_name());
    }
};

template<class _ty>
struct xsd_name_equalto:
    public std::binary_function<_ty, _ty, bool>
{
    bool operator()(_ty t1, _ty t2) const
    {
        assert(t1 && t2);
        return t1->get_name() == t2->get_name();
    }
};

typedef vector<xsd_schema*> xsd_schemas;
typedef unordered_multiset<xsd_schema*, xsd_schema_ns_hash<xsd_schema*>, xsd_schema_ns_equalto<xsd_schema*> > xsd_schema_ns_map;
typedef unordered_set<xsd_schema*, xsd_schema_loc_hash<xsd_schema*>, xsd_schema_loc_equalto<xsd_schema*> > xsd_schema_loc_map;
typedef list<xsd_extern_schema> xsd_extern_schemas;
typedef unordered_set<xsd_node*, xsd_name_hash<xsd_node*>, xsd_name_equalto<xsd_node*> > xsd_node_map;
typedef unordered_multiset<xsd_node*, xsd_name_hash<xsd_node*>, xsd_name_equalto<xsd_node*> > xsd_node_multimap;

struct xsd_global
{
    xsd_schemas             packs;
    xsd_schema_ns_map       ns_map;
    xsd_schema_loc_map      loc_map;

public:
    ~xsd_global();

};

struct xsd_cg_config {};

extern xsd_schema* xsd_parse_schema(xsd_global& global, xmltree& dom, const gchar* location);
extern xsd_schema* xsd_parse_schema(xsd_global& global, const gchar* location);
extern void xsd_make_file_name(string& name, const string& str);
extern bool xsd_prepare_name_mangling(xsd_global& global);
extern bool xsd_prepare_translation(xsd_schema* schema, xsd_global& global);
extern bool xsd_translate_declarations(string& output, xsd_schema* schema, xsd_global& global);
extern bool xsd_translate_implementations(string& output, xsd_schema* schema, xsd_global& global);

enum xsd_extern_schema_type
{
    xest_include,
    xest_import,
};

enum xsd_node_type
{
    xnt_key,
    xnt_element,
    xnt_attribute,
    xnt_attribute_group,
    xnt_group,
    xnt_simple_type,
    xnt_complex_type,
};

struct xsd_extern_schema
{
    xsd_extern_schema_type  type;
    string                  ns;
    xsd_schema*             schema;

public:
    xsd_schema* operator->() { return schema; }
    const xsd_schema* operator->() const { return schema; }
};

class xsd_schema
{
protected:
    xsd_wrapper*            _source;
    string                  _namespace;
    string                  _location;
    xsd_extern_schemas      _external;      // include & import
    string                  _xsd_namespace;
    xsd_node_map            _element_map;
    xsd_node_map            _attribute_map;
    xsd_node_map            _attribute_group_map;
    xsd_node_map            _group_map;
    xsd_node_map            _simple_type_map;
    xsd_node_map            _complex_type_map;

public:
    xsd_schema();
    ~xsd_schema();
    const string& get_namespace() const { return _namespace; }
    const string& get_location() const { return _location; }
    const string& get_xsd_namespace() const { return _xsd_namespace; }
    void set_namespace(const string& str) { _namespace = str; }
    void set_location(const string& str) { _location = str; }
    void set_xsd_namespace(const string& str) { _xsd_namespace = str; }
    void set_source(xsd_wrapper* s) { _source = s; }
    xsd_extern_schemas& get_extern_schemas() { return _external; }
    xsd_node_map& get_element_map() { return _element_map; }
    xsd_node_map& get_attribute_map() { return _attribute_map; }
    xsd_node_map& get_attribute_group_map() { return _attribute_group_map; }
    xsd_node_map& get_group_map() { return _group_map; }
    xsd_node_map& get_simple_type_map() { return _simple_type_map; }
    xsd_node_map& get_complex_type_map() { return _complex_type_map; }

protected:
    static xsd_node* find_node(const xsd_node_map& m, const string& name);
    template<class _node>
    static _node* find_and_verify(const xsd_node_map& m, const string& n, xsd_node_type t)
    {
        xsd_node* r = find_node(m, n);
        if(!r) return 0;
        assert(r->get_type() == t);
        return static_cast<_node*>(r);
    }

public:
    xsd_element* find_element_node(const string& name) const { return find_and_verify<xsd_element>(_element_map, name, xnt_element); }
    xsd_attribute* find_attribute_node(const string& name) const { return find_and_verify<xsd_attribute>(_attribute_map, name, xnt_attribute); }
    xsd_group* find_group_node(const string& name) const { return find_and_verify<xsd_group>(_group_map, name, xnt_group); }
    xsd_attribute_group* find_attribute_group_node(const string& name) const { return find_and_verify<xsd_attribute_group>(_attribute_group_map, name, xnt_attribute_group); }
    xsd_simple_type* find_simple_type_node(const string& name) const { return find_and_verify<xsd_simple_type>(_simple_type_map, name, xnt_simple_type); }
    xsd_complex_type* find_complex_type_node(const string& name) const { return find_and_verify<xsd_complex_type>(_complex_type_map, name, xnt_complex_type); }
};

struct xsd_ns_key:
    public xsd_schema
{
    xsd_ns_key(const string& str) { _namespace.assign(str); }
    xsd_ns_key(const gchar* str) { _namespace.assign(str); }
    xsd_ns_key(const gchar* str, int len) { _namespace.assign(str, len); }
};

struct xsd_loc_key:
    public xsd_schema
{
    xsd_loc_key(const string& str) { _location.assign(str); }
    xsd_loc_key(const gchar* str) { _location.assign(str); }
    xsd_loc_key(const gchar* str, int len) { _location.assign(str, len); }
};

class __gs_novtable xsd_node abstract
{
public:
    xsd_node();
    virtual ~xsd_node() {}
    virtual xsd_node_type get_type() const = 0;

public:
    void set_name(const string& str) { _name = str; }
    void set_mangling(const string& str) { _mangling = str; }
    const string& get_name() const { return _name; }
    const string& get_mangling() const { return _mangling; }
    xsd_wrapper* get_source() const { return _source; }
    void set_source(xsd_wrapper* s) { _source = s; }
    void set_translated(bool b) { _translated = b; }
    bool is_translated() const { return _translated; }

protected:
    string                  _name;
    string                  _mangling;
    xsd_wrapper*            _source;
    bool                    _translated;
};

struct xsd_node_key:
    public xsd_node
{
    xsd_node_key(const string& str) { _name.assign(str); }
    xsd_node_key(const gchar* str) { _name.assign(str); }
    xsd_node_key(const gchar* str, int len) { _name.assign(str, len); }
    xsd_node_type get_type() const override { return xnt_key; }
};

class xsd_element:
    public xsd_node
{
public:
    xsd_element();
    ~xsd_element();
    xsd_node_type get_type() const override { return xnt_element; }
    void set_ref(xsd_element* r) { _ref = r; }
    xsd_element* get_ref() const { return _ref; }
    void set_local_type(xsd_node* n) { _local_type = n; }
    xsd_node* get_local_type() const { return _local_type; }
    string& get_type_name() { return _elem_type_name; }
    string& get_entity_name() { return _elem_entity_name; }
    string& get_verificator() { return _elem_verificator; }
    string& get_assigner() { return _elem_assigner; }
    const string& const_type_name() const { return _elem_type_name; }
    const string& const_entity_name() const { return _elem_entity_name; }
    const string& const_verificator() const { return _elem_verificator; }
    const string& const_assigner() const { return _elem_assigner; }

protected:
    xsd_element*            _ref;
    xsd_node*               _local_type;
    string                  _elem_type_name;
    string                  _elem_entity_name;
    string                  _elem_verificator;
    string                  _elem_assigner;
};

class xsd_attribute:
    public xsd_node
{
public:
    xsd_attribute();
    ~xsd_attribute();
    xsd_node_type get_type() const override { return xnt_attribute; }
    void set_ref(xsd_attribute* r) { _ref = 0; }
    xsd_attribute* get_ref() const { return _ref; }
    void set_local_type(xsd_node* n) { _local_type = 0; }
    xsd_node* get_local_type() const { return _local_type; }
    string& get_type_name() { return _attr_type_name; }
    string& get_entity_name() { return _attr_entity_name; }
    string& get_verificator() { return _attr_verificator; }
    string& get_assigner() { return _attr_assigner; }
    const string& const_type_name() const { return _attr_type_name; }
    const string& const_entity_name() const { return _attr_entity_name; }
    const string& const_verificator() const { return _attr_verificator; }
    const string& const_assigner() const { return _attr_assigner; }

protected:
    xsd_attribute*          _ref;
    xsd_node*               _local_type;
    string                  _attr_type_name;
    string                  _attr_entity_name;
    string                  _attr_verificator;
    string                  _attr_assigner;
};

class xsd_group:
    public xsd_node
{
public:
    xsd_node_type get_type() const override { return xnt_group; }

protected:
};

class xsd_attribute_group:
    public xsd_node
{
public:
    xsd_node_type get_type() const override { return xnt_attribute_group; }
};

enum xsd_sts_type
{
    xst_restriction,
    xst_union,
    xst_list,
};

class __gs_novtable xsd_st_substitution abstract
{
public:
    virtual ~xsd_st_substitution() {}
    virtual xsd_sts_type get_type() const = 0;
};

class xsd_st_restriction:
    public xsd_st_substitution
{
public:
    xsd_sts_type get_type() const override { return xst_restriction; }

protected:

};

class xsd_st_union:
    public xsd_st_substitution
{
public:
    xsd_sts_type get_type() const override { return xst_union; }
};

class xsd_st_list:
    public xsd_st_substitution
{
public:
    xsd_sts_type get_type() const override { return xst_list; }
};

class xsd_simple_type:
    public xsd_node
{
public:
    xsd_simple_type();
    ~xsd_simple_type();
    xsd_node_type get_type() const override { return xnt_simple_type; }
    void set_ref(xsd_simple_type* r) { _ref = r; }
    xsd_simple_type* get_ref() const { return _ref; }
    void set_substitution(xsd_st_substitution* sub);
    bool build_assigner(string& code, const string& arg, const string& indent);

protected:
    xsd_simple_type*        _ref;
    xsd_st_substitution*    _substitution;
};

class xsd_complex_type:
    public xsd_node
{
public:
    xsd_node_type get_type() const override { return xnt_complex_type; }
    bool build_assigner(string& code, const string& arg, const string& indent) { return false; }
};

__gslib_end__

#endif
