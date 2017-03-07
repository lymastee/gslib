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

#include <gslib/error.h>
#include <gslib/xsd.h>
#include <gslib/uuid.h>
#include <sstream>

__gslib_begin__

#define xsd_cststr_info(str) str, _cststrlen(str)

#ifdef _UNICODE
#define xsd_wildcard_str        L"%ls"
#define xsd_wildcard_ch         L"%lc"
#define xsd_wildcard_str_and    L"%ls"L
#define xsd_wildcard_ch_and     L"%lc"L
#define xsd_t                   L
#else
#define xsd_wildcard_str        "%s"
#define xsd_wildcard_ch         "%c"
#define xsd_wildcard_str_and    "%ls"
#define xsd_wildcard_ch_and     "%lc"
#define xsd_t
#endif

static bool xsd_compare_string(const string& str1, const gchar* str2, int len2)
{
    assert(str2);
    return str1.equal(str2, len2);
}

static bool xsd_compare_string(const string& str, const gchar* hdr, int hdrlen, const gchar* bk, int bklen)
{
    assert(hdr && bk);
    int len = str.length();
    if(len != hdrlen + bklen)
        return false;
    const gchar* s = str.c_str();
    return (strtool::compare(s, hdr, hdrlen) == 0) &&
        (strtool::compare(s + hdrlen, bk, bklen) == 0);
}

static bool xsd_compare_string(const string& str, const string& hdr, const gchar* bk, int bklen)
{
    assert(bk);
    return xsd_compare_string(str, hdr.c_str(), hdr.length(), bk, bklen);
}

static bool xsd_get_namespace(string& ns, const gchar* str, int len)
{
    assert(str && (len > 0));
    const gchar* p = strtool::_test(str, _t(":"));
    if(!p)
        return false;
    assert(*p == _t(':'));
    int pos = p - str;
    assert(pos < len);
    ns.assign(str, pos);
    return true;
}

static xsd_node* xsd_find_native_type(xsd_schema* schema, const string& name)
{
    assert(schema);
    if(auto* st = schema->find_simple_type_node(name))
        return st;
    return schema->find_complex_type_node(name);
}

static xsd_node* xsd_find_external_type(xsd_schema* schema, const xsd_global& global, const string& ns, const string& total)
{
    assert(schema);
    auto& ext = schema->get_extern_schemas();
    auto f = std::find_if(ext.begin(), ext.end(), [&ns](xsd_extern_schema& e)->bool { return e.ns == ns; });
    if(f == ext.end())
        return 0;
    string name;
    int bias = ns.length() + 1;
    name.assign(total.c_str() + bias, total.length() - bias);
    auto fr = global.ns_map.equal_range(&xsd_ns_key((*f)->get_namespace()));
    for(auto i = fr.first; (i != global.ns_map.end()) && (i != fr.second); ++ i) {
        if(xsd_node* ft = xsd_find_native_type(*i, name))
            return ft;
    }
    return 0;
}

static void xsd_make_random_stamp(string& stamp)
{
    uuid u(uuid_ver_default);
    u.to_string(stamp);
    stamp.replace(8, 1, 1, _t('_'));
    stamp.replace(13, 1, 1, _t('_'));
    stamp.replace(18, 1, 1, _t('_'));
    stamp.replace(23, 1, 1, _t('_'));
}

static void xsd_make_verificator_name(string& name)
{
    string stamp;
    xsd_make_random_stamp(stamp);
    name.append(_t("verify_"));
    name.append(stamp);
}

static void xsd_make_assigner_name(string& name)
{
    string stamp;
    xsd_make_random_stamp(stamp);
    name.append(_t("assign_"));
    name.append(stamp);
}

static void xsd_camel_to_gs(string& name, const string& str)
{
    const gchar* s = str.c_str();
    while(*s) {
        const gchar* uc = strtool::_test(s, _t("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
        if(uc) {
            int len = uc - s;
            name.append(s, len);
            name.push_back(_t('_'));
            name.push_back(*uc + 32);
            s = uc + 1;
        }
        else {
            name.append(s);
            break;
        }
    }
}

static bool xsd_retrieve_extern_namespace(string& ns, xmltree& dom, xml_node* node)
{
    assert(dom.is_valid() && node);
    // 1.if we have to find the id in DOM root node, the node needs to have namespace
    const string* sns = node->get_attribute(xsd_cststr_info(_t("namespace")));
    if(!sns) {
        set_error(_t("xsd retrieve extern id failed: necessary attribute \'namespace\' missed."));
        return false;
    }
    // 2.try to find the id in DOM root node
    assert(sns);
    auto root = dom.get_root();
    assert(!root->is_value());
    static_cast_as(xml_element*, rtelem, root.get_ptr());
    auto& attrs = rtelem->attrlist;
    for(auto& attr : attrs) {
        if(attr.value == *sns) {
            // the key should be like "xmlns:xx", we would retrieve the id from xx
            if(strtool::compare(attr.key.c_str(), xsd_cststr_info(_t("xmlns")) == 0)) {
                auto pos = attr.key.find_first_of(_t(':'));
                if(pos == -1) {
                    // global namespace? I'm not sure about this
                    assert(attr.value == _t("xmlns"));
                    return true;
                }
                else {
                    pos ++;
                    ns.assign(attr.key, pos, attr.key.length() - pos);
                    return true;
                }
            }
        }
    }
    set_error(_t("xsd retrieve extern id failed: retrieve id failed."));
    return false;
}

static bool xsd_retrieve_xsd_namespace(string& ns, xmltree& dom)
{
    assert(dom.is_valid());
    auto root = dom.get_root();
    assert(!root->is_value());
    static_cast_as(xml_element*, rtelem, root.get_ptr());
    auto& attrs = rtelem->attrlist;
    static const string xsdns(xsd_cststr_info(_t("http://www.w3.org/2001/XMLSchema")));
    for(auto& attr : attrs) {
        if(attr.value == xsdns) {
            if(strtool::compare(attr.key.c_str(), xsd_cststr_info(_t("xmlns"))) == 0) {
                auto pos = attr.key.find_first_of(_t(':'));
                if(pos != -1) {
                    pos ++;
                    ns.assign(attr.key, pos, attr.key.length() - pos);
                    return true;
                }
                else {
                    // I'm not sure about this
                    return true;
                }
            }
        }
    }
    return false;
}

static bool xsd_parse_include(xsd_global& global, xmltree& dom, xsd_schema* schema, xsd_iterator p)
{
    assert(dom.is_valid() && schema && p.is_valid());
    xsd_extern_schemas& ext = schema->get_extern_schemas();
    xsd_wrappers includes;
    for(auto c = p.child(); c.is_valid(); c.to_next()) {
        if(!c->is_value() &&
            xsd_compare_string(c->get_name(), schema->get_xsd_namespace(), xsd_cststr_info(_t(":include")))
            )
            includes.push_back(c.get_wrapper());
    }
    for(xml_wrapper* rp : includes) {
        xml_node* node = rp->get_ptr();
        assert(node);
        const string* loc = node->get_attribute(xsd_cststr_info(_t("schemaLocation")));
        if(!loc) {
            set_error(_t("xsd parse include failed: parser doesn't support include without schemaLocation."));
            return false;
        }
        auto f = global.loc_map.find(&xsd_loc_key(*loc));
        // if the file has been parsed
        if(f != global.loc_map.end()) {
            xsd_extern_schema extschm;
            extschm.type = xest_include;
            extschm.schema = *f;
            if(!xsd_retrieve_extern_namespace(extschm.ns, dom, node))
                return false;
            ext.push_back(extschm);
            continue;
        }
        // otherwise parse it
        xsd_schema* schm = xsd_parse_schema(global, loc->c_str());
        if(!schm) {
            set_error(_t("xsd parse include failed: include file error."));
            return false;
        }
        // verify if the include schema was valid, the targetNamespace must be the same as schema
        if(schema->get_namespace() != schm->get_namespace()) {
            set_error(_t("xsd parse include failed: different target namespace."));
            return false;
        }
        // record it
        xsd_extern_schema extschm;
        extschm.type = xest_include;
        extschm.schema = schm;
        if(!xsd_retrieve_extern_namespace(extschm.ns, dom, node))
            return false;
        ext.push_back(extschm);
    }
    return true;
}

static bool xsd_parse_include(xsd_global& global, xmltree& dom, xsd_schema* schema)
{
    assert(dom.is_valid() && schema);
    auto root = dom.get_root();
    return xsd_parse_include(global, dom, schema, root);
}

static bool xsd_parse_import(xsd_global& global, xmltree& dom, xsd_schema* schema, xsd_iterator p)
{
    assert(dom.is_valid() && p.is_valid());
    xsd_extern_schemas& ext = schema->get_extern_schemas();
    xsd_wrappers imports;
    for(auto c = p.child(); c.is_valid(); c.to_next()) {
        if(!c->is_value() &&
            xsd_compare_string(c->get_name(), schema->get_xsd_namespace(), xsd_cststr_info(_t(":import")))
            )
            imports.push_back(c.get_wrapper());
    }
    for(xml_wrapper* rp : imports) {
        xml_node* node = rp->get_ptr();
        assert(node);
        const string* loc = node->get_attribute(xsd_cststr_info(_t("schemaLocation")));
        if(!loc) {
            set_error(_t("xsd parse import failed: parser doesn't support import without schemaLocation."));
            return false;
        }
        auto f = global.loc_map.find(&xsd_loc_key(*loc));
        // if the file has been parsed
        if(f != global.loc_map.end()) {
            xsd_extern_schema extschm;
            extschm.type = xest_import;
            extschm.schema = *f;
            if(!xsd_retrieve_extern_namespace(extschm.ns, dom, node))
                return false;
            ext.push_back(extschm);
            continue;
        }
        // otherwise parse it
        xsd_schema* schm = xsd_parse_schema(global, loc->c_str());
        if(!schm) {
            set_error(_t("xsd parse import failed: import file error."));
            return false;
        }
        // record it
        xsd_extern_schema extschm;
        extschm.type = xest_import;
        extschm.schema = schm;
        if(!xsd_retrieve_extern_namespace(extschm.ns, dom, node))
            return false;
        ext.push_back(extschm);
    }
    return true;
}

static bool xsd_parse_import(xsd_global& global, xmltree& dom, xsd_schema* schema)
{
    assert(dom.is_valid() && schema);
    auto root = dom.get_root();
    return xsd_parse_import(global, dom, schema, root);
}

template<class _creator>
static xsd_node* xsd_create_schema_content(xsd_iterator i, xsd_node_map& m, _creator creator)
{
    static_cast_as(xml_element*, elem, i.get_ptr());
    const string* name = elem->get_attribute(string(_t("name")));
    if(!name) {
        set_error(_t("xsd parse schema failed: unnamed content."));
        return 0;
    }
    if(m.find(&xsd_node_key(*name)) != m.end()) {
        set_error(_t("xsd parse schema failed: content name already exist."));
        return 0;
    }
    xsd_node* node = creator();
    node->set_name(*name);
    node->set_source(i.get_wrapper());
    m.insert(node);
    return node;
}

static bool xsd_parse_schema_content(xsd_schema* schema, xmltree& dom, xsd_iterator i)
{
    assert(schema && i.is_valid());
    if(i->is_value())
        return false;
    static_cast_as(xml_element*, elem, i.get_ptr());
    assert(elem);
    xsd_node* node = 0;
    if(xsd_compare_string(elem->key, schema->get_xsd_namespace(), xsd_cststr_info(_t(":element"))))
        node = xsd_create_schema_content(i, schema->get_element_map(), []()->xsd_node* { return new xsd_element; });
    else if(xsd_compare_string(elem->key, schema->get_xsd_namespace(), xsd_cststr_info(_t(":attribute"))))
        node = xsd_create_schema_content(i, schema->get_attribute_map(), []()->xsd_node* { return new xsd_attribute; });
    else if(xsd_compare_string(elem->key, schema->get_xsd_namespace(), xsd_cststr_info(_t(":attributeGroup"))))
        node = xsd_create_schema_content(i, schema->get_attribute_group_map(), []()->xsd_node* { return new xsd_attribute_group; });
    else if(xsd_compare_string(elem->key, schema->get_xsd_namespace(), xsd_cststr_info(_t(":group"))))
        node = xsd_create_schema_content(i, schema->get_group_map(), []()->xsd_node* { return new xsd_group; });
    else if(xsd_compare_string(elem->key, schema->get_xsd_namespace(), xsd_cststr_info(_t(":simpleType"))))
        node = xsd_create_schema_content(i, schema->get_simple_type_map(), []()->xsd_node* { return new xsd_simple_type; });
    else if(xsd_compare_string(elem->key, schema->get_xsd_namespace(), xsd_cststr_info(_t(":complexType"))))
        node = xsd_create_schema_content(i, schema->get_complex_type_map(), []()->xsd_node* { return new xsd_complex_type; });
    else
        return false;
    return node != 0;
}

xsd_schema* xsd_parse_schema(xsd_global& global, xmltree& dom, const gchar* location)
{
    assert(dom.is_valid());
    assert(global.loc_map.find(&xsd_loc_key(location)) == global.loc_map.end() &&
        "this file has been parsed!"
        );
    // skip the schema header, just do a simple test
    xsd_iterator root = dom.get_root();
    assert(root);
    if(!xsd_compare_string(root->get_name(), xsd_cststr_info(_t("xsd:schema")))) {
        set_error(_t("xsd parse schema failed: bad name for schema root."));
        return 0;
    }
    // get target namespace
    const string* target = root->get_attribute(xsd_cststr_info(_t("targetNamespace")));
    if(!target) {
        set_error(_t("xsd parse schema failed: no target namespace."));
        return 0;
    }
    xsd_schema* schema = new xsd_schema;
    assert(schema);
    schema->set_namespace(*target);
    schema->set_location(string(location));
    schema->set_source(root.get_wrapper());
    string xsdns;
    if(xsd_retrieve_xsd_namespace(xsdns, dom))
        schema->set_xsd_namespace(xsdns);
    // parse include & import files
    if(!xsd_parse_include(global, dom, schema) || !xsd_parse_import(global, dom, schema)) {
        delete schema;
        return 0;
    }
    // attach to global
    global.packs.push_back(schema);
    global.ns_map.insert(schema);
    global.loc_map.insert(schema);
    // parse schema content
    for(xsd_iterator i = root.child(); i.is_valid(); i.to_next())
        xsd_parse_schema_content(schema, dom, i);
    return schema;
}

xsd_schema* xsd_parse_schema(xsd_global& global, const gchar* location)
{
    assert(location);
    // check if the file has been parsed, the location should be unique
    auto p = global.loc_map.find(&xsd_loc_key(location));
    if(p != global.loc_map.end())
        return *p;
    xmltree dom;
    if(!dom.load(location)) {
        set_error(_t("xsd parse schema failed: load xml file failed."));
        return 0;
    }
    return xsd_parse_schema(global, dom, location);
}

void xsd_make_file_name(string& name, const string& str)
{
    name = str;
    // 1.to lower case
    name.to_lower();
    // 2.trim postfix
    auto lp = name.find_last_of(_t('.'));
    if(lp != -1)
        name.resize(lp);
    // 3.replace - . whitespace with _
    gchar* s = &name.front();
    int p = 0;
    for(;;) {
        const gchar* f = strtool::_test(s + p, _t("-. "));
        if(f) {
            p = f - s;
            s[p] = _t('_');
        }
    }
}

template<class _sel>
static bool xsd_prepare_name_mangling(xsd_global& global, _sel sel)
{
    xsd_node_multimap m;
    for(auto i : global.packs) {
        xsd_node_map& n = sel(i);
        for(auto j : n)
            m.insert(j);
    }
    auto i = m.begin();
    while(i != m.end()) {
        auto p = m.equal_range(&xsd_node_key((*i)->get_name()));
        string name;
        xsd_camel_to_gs(name, (*p.first)->get_name());
        if(p.first == std::prev(p.second)) {
            // no conflict, no name mangling
            (*p.first)->set_mangling(name);
        }
        else {
            int c = 0;
            for(auto j = p.first; j != p.second; ++ j, ++ c) {
                string final(name), postfix;
                postfix.from_int(c);
                final.append(postfix);
                (*j)->set_mangling(final);
            }
        }
        i = p.second;
    }
    return true;
}

bool xsd_prepare_name_mangling(xsd_global& global)
{
    return xsd_prepare_name_mangling(global, [](xsd_schema* schema)->xsd_node_map& { return schema->get_element_map(); }) &&
        xsd_prepare_name_mangling(global, [](xsd_schema* schema)->xsd_node_map& { return schema->get_attribute_map(); }) &&
        xsd_prepare_name_mangling(global, [](xsd_schema* schema)->xsd_node_map& { return schema->get_attribute_group_map(); }) &&
        xsd_prepare_name_mangling(global, [](xsd_schema* schema)->xsd_node_map& { return schema->get_group_map(); }) &&
        xsd_prepare_name_mangling(global, [](xsd_schema* schema)->xsd_node_map& { return schema->get_simple_type_map(); }) &&
        xsd_prepare_name_mangling(global, [](xsd_schema* schema)->xsd_node_map& { return schema->get_complex_type_map(); });
}

static bool xsd_translate_extern_schema(string& output, xsd_schema* schema)
{
    assert(schema);
    xsd_extern_schemas& ext = schema->get_extern_schemas();
    for(auto& es : ext) {
        string inc;
        xsd_make_file_name(inc, es->get_location());
        output.append(_t("#include \""));
        output.append(inc);
        output.append(_t(".h\"\n"));
    }
    return true;
}

static bool xsd_translate_st_restriction(xsd_simple_type* node, xml_element* restri, xsd_global& global, xsd_schema* schema)
{
    assert(node && restri && schema);
    xsd_st_restriction* r = new xsd_st_restriction;
    node->set_substitution(r);
    // 1.
    return true;
}

static bool xsd_translate_st_union(xsd_simple_type* node, xml_element* un, xsd_global& global, xsd_schema* schema)
{
    assert(node && un && schema);
    xsd_st_union* u = new xsd_st_union;
    node->set_substitution(u);
    return true;
}

static bool xsd_translate_st_list(xsd_simple_type* node, xml_element* li, xsd_global& global, xsd_schema* schema)
{
    assert(node && li && schema);
    xsd_st_list* l = new xsd_st_list;
    node->set_substitution(l);
    return true;
}

static bool xsd_translate_simple_type(xsd_simple_type* node, xsd_global& global, xsd_schema* schema)
{
    assert(node && schema);
    if(node->is_translated())
        return true;
    xsd_wrapper* w = node->get_source();
    assert(w);
    xml_element* elem = static_cast<xml_element*>(w->get_ptr());
    assert(elem);
    // parse ref
    const string* r = elem->get_attribute(string(_t("ref")));
    if(r != 0) {
        // verify QName
        xsd_simple_type* nr = schema->find_simple_type_node(*r);
        if(!nr) {
            set_error(xsd_t"xsd translate simple type failed: unknown QName: \""xsd_wildcard_str_and"\".", r->c_str());
            return false;
        }
        // translate all
        if(!nr->is_translated()) {
            if(!xsd_translate_simple_type(nr, global, schema))
                return false;
        }
        for(;;) {
            xsd_simple_type* tref = nr->get_ref();
            if(!tref)
                break;
            nr = tref;
        }
        assert(nr);
        node->set_ref(nr);
        node->set_translated(true);
        return true;
    }
    // must have child
    xsd_iterator c = xsd_iterator(w).child();
    assert(c.is_valid());
    for(; c.is_valid(); c.to_next()) {
        xml_node* n = c.get_ptr();
        if(n->is_value())
            continue;
        xml_element* celem = static_cast<xml_element*>(n);
        const string& name = celem->get_name();
        if(xsd_compare_string(name, schema->get_xsd_namespace(), xsd_cststr_info(_t(":restriction")))) {
            if(!xsd_translate_st_restriction(node, celem, global, schema))
                return false;
        }
        else if(xsd_compare_string(name, schema->get_xsd_namespace(), xsd_cststr_info(_t(":union")))) {
            if(!xsd_translate_st_union(node, celem, global, schema))
                return false;
        }
        else if(xsd_compare_string(name, schema->get_xsd_namespace(), xsd_cststr_info(_t(":list")))) {
            if(!xsd_translate_st_list(node, celem, global, schema))
                return false;
        }
        else continue;
        break;
    }
    set_error(_t("xsd translate simple type failed: unknown error."));
    return false;
}

static bool xsd_translate_simple_types(xsd_global& global, xsd_schema* schema)
{
    assert(schema);
    auto& stypes = schema->get_simple_type_map();
    for(auto* node : stypes) {
        assert(node && (node->get_type() == xnt_simple_type));
        if(!xsd_translate_simple_type(static_cast<xsd_simple_type*>(node), global, schema))
            return false;
    }
    return true;
}

static bool xsd_translate_complex_type(xsd_complex_type* node, xsd_global& global, xsd_schema* schema)
{
    return true;
}

static bool xsd_translate_complex_types(xsd_global& global, xsd_schema* schema)
{
    return true;
}

//bool xsd_translate_element(xsd_node* node, xsd_global& global, xsd_schema* schema);

static bool xsd_translate_attribute(xsd_attribute* node, xsd_global& global, xsd_schema* schema)
{
    assert(node && schema);
    if(node->is_translated())
        return true;

    return true;
}

static bool xsd_translate_attributes(xsd_global& global, xsd_schema* schema)
{
    assert(schema);
    auto& attributes = schema->get_attribute_map();
    for(auto* node : attributes) {
        assert(node && (node->get_type() == xnt_attribute));
        if(!xsd_translate_attribute(static_cast<xsd_attribute*>(node), global, schema))
            return false;
    }
    return true;
}

static bool xsd_translate_element(xsd_element* node, xsd_global& global, xsd_schema* schema)
{
    assert(node && schema);
    if(node->is_translated())
        return true;
    xsd_wrapper* w = node->get_source();
    assert(w);
    xml_element* elem = static_cast<xml_element*>(w->get_ptr());
    assert(elem);
    // parse ref
    const string* r = elem->get_attribute(string(_t("ref")));
    if(r != 0) {
        // verify QName
        xsd_element* nr = schema->find_element_node(*r);
        if(!nr) {
            set_error(xsd_t"xsd translate element failed: unknown QName: \""xsd_wildcard_str_and"\".", r->c_str());
            return false;
        }
        // translate all
        if(!nr->is_translated()) {
            if(!xsd_translate_element(nr, global, schema))
                return false;
        }
        for(;;) {
            xsd_element* tref = nr->get_ref();
            if(!tref)
                break;
            nr = tref;
        }
        assert(nr);
        node->set_ref(nr);
        node->set_translated(true);
        return true;
    }
    // parse type
    xsd_type* builtintype = 0;
    xsd_node* custtype = 0;
    const string* type = elem->get_attribute(string(_t("type")));
    if(!type) {
        bool ptok = false;
        for(auto i = xsd_iterator(w->child()); i.is_valid(); i.to_next()) {
            if(!i->is_value()) {
                if(xsd_compare_string(i->get_name(), schema->get_xsd_namespace(), xsd_cststr_info(_t(":simpleType")))) {
                    xsd_simple_type* st = new xsd_simple_type;
                    node->set_local_type(st);
                    static const string name(_t("unnamed_type"));
                    st->set_name(name);
                    st->set_mangling(name);
                    st->set_source(i.get_wrapper());
                    if(!xsd_translate_simple_type(st, global, schema))
                        return false;
                    custtype = st;
                    ptok = true;
                    break;
                }
                else if(xsd_compare_string(i->get_name(), schema->get_xsd_namespace(), xsd_cststr_info(_t(":complexType")))) {
                    xsd_complex_type* ct = new xsd_complex_type;
                    node->set_local_type(ct);
                    static const string name(_t("unnamed_type"));
                    ct->set_name(name);
                    ct->set_mangling(name);
                    ct->set_source(i.get_wrapper());
                    if(!xsd_translate_complex_type(ct, global, schema))
                        return false;
                    custtype = ct;
                    ptok = true;
                    break;
                }
            }
        }
        if(!ptok) {
            set_error(_t("xsd translate element failed: can't find type."));
            return false;
        }
    }
    else {
        string ns;
        bool fns = xsd_get_namespace(ns, type->c_str(), type->length());
        if(fns && (schema->get_xsd_namespace() == ns)) {
            const gchar* str = type->c_str();
            int bias = ns.length() + 1;
            int len = type->length();
            builtintype = xsd_query_builtin_type(str + bias, len - bias);
            if(!builtintype) {
                set_error(xsd_t"xsd translate element failed: unknown builtin type: \""xsd_wildcard_str_and"\".", type->c_str());
                return false;
            }
            node->get_type_name() = *type;
        }
        else if(!fns) {
            custtype = xsd_find_native_type(schema, *type);
            if(!custtype) {
                set_error(xsd_t"xsd translate element failed: unknown native type: \""xsd_wildcard_str_and"\".", type->c_str());
                return false;
            }
            node->get_type_name() = *type;
        }
        else {
            assert(!ns.empty());
            custtype = xsd_find_external_type(schema, global, ns, *type);
            if(!custtype) {
                set_error(xsd_t"xsd translate element failed: unknown external type: \""xsd_wildcard_str_and"\".", type->c_str());
                return false;
            }
            node->get_type_name() = *type;
        }
    }
    // parse name
    node->get_entity_name() = node->get_mangling();
    // parse verificator
    string& verificator = node->get_verificator();
    if(!verificator.empty())
        verificator.clear();
    string verificator_name;
    xsd_make_verificator_name(verificator_name);
    verificator.format(
#ifdef _UNICODE
        L"bool %ls(xmltree& dom, xmltree::iterator i)\n{\n"
#else
        "bool %s(xmltree& dom, xmltree::iterator i)\n{\n"
#endif
        , verificator_name.c_str()
        );
    verificator.append(_t("    assert(dom.is_mine(i) && i.is_valid());\n"));
    verificator.append(_t("    return true;\n"));
    verificator.append(_t("}\n\n"));
    // parse assigner
    string& assigner = node->get_assigner();
    if(!assigner.empty())
        assigner.clear();
    string assigner_name;
    xsd_make_assigner_name(assigner_name);
    assigner.format(
#ifdef _UNICODE
        L"bool %ls(xmltree::iterator i)\n{\n"
#else
        "bool %s(xmltree::iterator i)\n{\n"
#endif
        , assigner_name.c_str()
        );
    assigner.append(_t("    assert(i.is_valid());\n"));
    if(builtintype) {
        if(!builtintype->build_assigner(assigner, string(_t("i")), string(_t("    ")))) {
            set_error(_t("xsd translate element failed: build builtin assigner failed."));
            return false;
        }
    }
    else {
        assert(custtype);
        switch(custtype->get_type())
        {
        case xnt_simple_type:
            if(!static_cast<xsd_simple_type*>(custtype)->build_assigner(assigner, string(_t("i")), string(_t("    ")))) {
                set_error(_t("xsd translate element failed: build simple type assigner failed."));
                return false;
            }
            break;
        case xnt_complex_type:
            if(!static_cast<xsd_complex_type*>(custtype)->build_assigner(assigner, string(_t("i")), string(_t("    ")))) {
                set_error(_t("xsd translate element failed: build complex type assigner failed."));
                return false;
            }
            break;
        default:
            assert(!"xsd translate element failed: unknown error.");
            return false;
        }
    }
    assigner.append(_t("}\n\n"));
    node->set_translated(true);
    return true;
}

static bool xsd_translate_elements(xsd_global& global, xsd_schema* schema)
{
    assert(schema);
    auto& elements = schema->get_element_map();
    for(auto* node : elements) {
        assert(node && (node->get_type() == xnt_element));
        if(!xsd_translate_element(static_cast<xsd_element*>(node), global, schema))
            return false;
    }
    return true;
}

static bool xsd_translate_group(xsd_group* node, xsd_global& global, xsd_schema* schema)
{
    assert(node && schema);
    return true;
}

static bool xsd_translate_groups(xsd_global& global, xsd_schema* schema)
{
    assert(schema);
    auto& groups = schema->get_group_map();
    for(auto* node : groups) {
        assert(node && (node->get_type() == xnt_group));
        if(!xsd_translate_group(static_cast<xsd_group*>(node), global, schema))
            return false;
    }
    return true;
}

static bool xsd_translate_attribute_group(xsd_node* node, xsd_global& global, xsd_schema* schema)
{
    assert(node && schema);
    return true;
}

static bool xsd_translate_attribute_groups(xsd_global& global, xsd_schema* schema)
{
    assert(schema);
    auto& attrgroups = schema->get_attribute_group_map();
    for(auto* node : attrgroups) {
        assert(node && (node->get_type() == xnt_attribute_group));
        if(!xsd_translate_attribute_group(static_cast<xsd_attribute_group*>(node), global, schema))
            return false;
    }
    return true;
}

bool xsd_prepare_translation(xsd_schema* schema, xsd_global& global)
{
    assert(schema);
    return xsd_translate_elements(global, schema) &&
        xsd_translate_attributes(global, schema) &&
        xsd_translate_groups(global, schema) &&
        xsd_translate_attribute_groups(global, schema) &&
        xsd_translate_simple_types(global, schema) &&
        xsd_translate_complex_types(global, schema);
}

xsd_global::~xsd_global()
{
    for(auto* p : packs) { if(p) delete p; }
    packs.clear();
    ns_map.clear();
    loc_map.clear();
}

xsd_schema::xsd_schema()
{
    _source = 0;
}

template<class _cont>
static void xsd_destroy_container(_cont& c)
{
    for(auto* p : c) { if(p) delete p; }
    c.clear();
}

xsd_schema::~xsd_schema()
{
    xsd_destroy_container(_element_map);
    xsd_destroy_container(_attribute_map);
    xsd_destroy_container(_attribute_group_map);
    xsd_destroy_container(_group_map);
    xsd_destroy_container(_simple_type_map);
    xsd_destroy_container(_complex_type_map);
}

xsd_node* xsd_schema::find_node(const xsd_node_map& m, const string& name)
{
    auto f = m.find(&xsd_node_key(name));
    return f != m.end() ? *f : 0;
}

xsd_node::xsd_node()
{
    _source = 0;
    _translated = false;
}

xsd_element::xsd_element()
{
    _ref = 0;
    _local_type = 0;
}

xsd_element::~xsd_element()
{
    if(_local_type) {
        delete _local_type;
        _local_type = 0;
    }
}

xsd_attribute::xsd_attribute()
{
    _ref = 0;
    _local_type = 0;
}

xsd_attribute::~xsd_attribute()
{
    if(_local_type) {
        delete _local_type;
        _local_type = 0;
    }
}

xsd_simple_type::xsd_simple_type()
{
    _ref = 0;
    _substitution = 0;
}

xsd_simple_type::~xsd_simple_type()
{
    if(_substitution) {
        delete _substitution;
        _substitution = 0;
    }
}

void xsd_simple_type::set_substitution(xsd_st_substitution* sub)
{
    assert(!_substitution);
    _substitution = sub;
}

bool xsd_simple_type::build_assigner(string& code, const string& arg, const string& indent)
{
    return true;
}

__gslib_end__
