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

#include <gslib/error.h>
#include <gslib/string.h>
#include "io.h"

using namespace gs;

namespace ui_editor {

/*
 * <type name="x" x="0" y="0" width="100" height="100" variable="my_ui_1" is_instance="true">
 *     <prop:some_property>value</prop:some_property>
 *     <type2 name="child" />
 * </type>
 */
static bool make_core_node(ui_node& dstnode, const xml_node& srcnode, ui_editor_context& ctx)
{
    dstnode.type = srcnode.get_name();
    const string* name = srcnode.get_attribute(_cststr(_t("name")));
    const string* x = srcnode.get_attribute(_cststr(_t("x")));
    const string* y = srcnode.get_attribute(_cststr(_t("y")));
    const string* w = srcnode.get_attribute(_cststr(_t("width")));
    const string* h = srcnode.get_attribute(_cststr(_t("height")));
    const string* is_inst = srcnode.get_attribute(_cststr(_t("is_instance")));
    string defname;
    if(!name) {
        static const string default_name(_t("unnamed node"));
        static int unnamed_ctr = 0;
        defname.format(_t("%s%d"), defname.c_str(), unnamed_ctr++);
        name = &defname;
    }
    if(!x || !y || !w || !h) {
        set_error(_t("one or more necessary attributes were missing."));
        return false;
    }
    dstnode.name = *name;
    dstnode.position = rect(x->to_int(), y->to_int(), w->to_int(), h->to_int());
    if(is_inst)
        dstnode.is_instance = is_inst->equal(_t("true"));
    const string* var = srcnode.get_attribute(_cststr(_t("variable")));
    if(var)
        dstnode.variable = *var;
    else
        ctx.acquire_variable_name(dstnode.variable, dstnode.type);
    return true;
}

static bool append_core_node_property(ui_node& dstnode, const xmltree::const_iterator& srciter)
{
    assert(srciter);
    const string& name = srciter->get_name();
    int hdrlen = _cststrlen(_t("prop:"));
    if(name.compare(_t("prop:"), hdrlen) != 0)
        return false;
    const gchar* attrname = name.c_str() + hdrlen;
    auto chd = srciter.child();
    for(; chd; chd = chd.next()) {
        if(chd->is_value()) {
            dstnode.prop_map.emplace(attrname, chd->get_name());
            return true;
        }
    }
    dstnode.prop_map.emplace(attrname, _t(""));
    return true;
}

static bool append_core_node_procedure(ui_node& dstnode, const xmltree::const_iterator& srciter)
{
    assert(srciter);
    const string& name = srciter->get_name();
    int hdrlen = _cststrlen(_t("proc:"));
    if(name.compare(_t("proc:"), hdrlen) != 0)
        return false;
    const string* args = srciter->get_attribute(_t("args"));
    const gchar* procname = name.c_str() + hdrlen;
    auto chd = srciter.child();
    for(; chd; chd = chd.next()) {
        if(chd->is_value()) {
            ui_proc_node node = { args ? *args : string(), chd->get_name() };
            dstnode.proc_map.emplace(procname, node);
            return true;
        }
    }
    ui_proc_node node = { args ? *args : string(), string() };
    dstnode.proc_map.emplace(procname, node);
    return true;
}

static bool analysis_core_sub_nodes(ui_tree& uitree, ui_tree::iterator dstiter, ui_editor_context& ctx, const xmltree& srctree, const xmltree::const_iterator& srciter)
{
    assert(srciter);
    if(srciter.is_leaf())
        return true;
    ui_node& dstnode = *dstiter;
    auto chd = srciter.child();
    assert(chd);
    for(; chd; chd = chd.next()) {
        if(chd->get_name().compare(_cststr(_t("prop:"))) == 0)
            append_core_node_property(dstnode, chd);
        else if(chd->get_name().compare(_cststr(_t("proc:"))) == 0)
            append_core_node_procedure(dstnode, chd);
        else {
            auto subnode = uitree.birth_tail(dstiter);
            make_core_node(*subnode, *chd, ctx);
            analysis_core_sub_nodes(uitree, subnode, ctx, srctree, chd);
        }
    }
    return true;
}

static const string* retrieve_value_from_source(const xmltree& srctree, const xmltree::const_iterator& srciter, const string& label)
{
    assert(srctree.is_valid() && srciter);
    assert(srciter->get_name() == label);
    auto chv = srciter.child();
    if(!chv || !chv->is_value())
        return nullptr;
    return &chv->get_name();
}

static bool convert_include_to_core(ui_src_entry& srcentry, const xmltree& srctree, const xmltree::const_iterator& srciter)
{
    const string* value = retrieve_value_from_source(srctree, srciter, _t("include"));
    if(!value)
        return false;
    srcentry.add_include_group(*value);
    return true;
}

static bool convert_macro_to_core(ui_src_entry& srcentry, const xmltree& srctree, const xmltree::const_iterator& srciter)
{
    assert(srctree.is_valid() && srciter);
    assert(srciter->get_name() == _t("macro"));
    const string* nameattr = srciter->get_attribute(_t("name"));
    if(!nameattr)
        return false;
    auto chv = srciter.child();
    if(!chv) {
        srcentry.add_macro(*nameattr);
        return true;
    }
    if(!chv->is_value())
        return false;
    srcentry.add_macro(*nameattr, chv->get_name());
    return true;
}

static bool convert_output_to_core(ui_src_entry& srcentry, const xmltree& srctree, const xmltree::const_iterator& srciter)
{
    const string* value = retrieve_value_from_source(srctree, srciter, _t("output"));
    if(!value)
        return false;
    srcentry.set_output(*value);
    return true;
}

static bool convert_export_to_core(ui_src_entry& srcentry, const xmltree& srctree, const xmltree::const_iterator& srciter)
{
    const string* value = retrieve_value_from_source(srctree, srciter, _t("export"));
    if(!value)
        return false;
    srcentry.set_export(*value);
    return true;
}

static bool convert_preproc_to_core(ui_src_entry& srcentry, const xmltree& srctree, const xmltree::const_iterator& srciter)
{
    const string* value = retrieve_value_from_source(srctree, srciter, _t("preproc"));
    if(!value)
        return false;
    srcentry.set_preproc(*value);
    return true;
}

static bool convert_postproc_to_core(ui_src_entry& srcentry, const xmltree& srctree, const xmltree::const_iterator& srciter)
{
    const string* value = retrieve_value_from_source(srctree, srciter, _t("postproc"));
    if(!value)
        return false;
    srcentry.set_postproc(*value);
    return true;
}

/*
 * <entry>
 *     <include>a.h;b.h</include>
 *     <macro name="A"/>
 *     <macro name="B">blabla...</macro>
 *     <output>output_name.h</output>
 *     <export>my_custom_ui</export>
 *     <preproc>This is preproc injection.</preproc>
 *     <ui_entry>
 *         <root_widget/>
 *     </ui_entry>
 *     <postproc>This is postproc injection.</postproc>
 * </entry>
 */
static xmltree::const_iterator convert_entry_to_core(ui_src_entry& srcentry, const xmltree& srctree)
{
    if(!srctree.is_valid())
        return xmltree::const_iterator(0);
    auto r = srctree.get_root();
    assert(r);
    if(r->get_name() != _t("entry")) {
        set_error(_t("ui file missing entry."));
        return xmltree::const_iterator(0);
    }
    if(r.is_leaf())
        return xmltree::const_iterator(0);
    xmltree::const_iterator ui_entry(0);
    for(auto c = r.child(); c; c = c.next()) {
        if(c->get_name() == _t("include"))
            convert_include_to_core(srcentry, srctree, c);
        else if(c->get_name() == _t("macro"))
            convert_macro_to_core(srcentry, srctree, c);
        else if(c->get_name() == _t("output"))
            convert_output_to_core(srcentry, srctree, c);
        else if(c->get_name() == _t("export"))
            convert_export_to_core(srcentry, srctree, c);
        else if(c->get_name() == _t("preproc"))
            convert_preproc_to_core(srcentry, srctree, c);
        else if(c->get_name() == _t("postproc"))
            convert_postproc_to_core(srcentry, srctree, c);
        else if(c->get_name() == _t("ui_entry"))
            ui_entry = c;
        else {
            assert(!"unexpected ui element.");
        }
    }
    return ui_entry;
}

bool ui_convert_source_to_core(ui_editor_context& ctx, ui_tree& uitree, const xmltree& src)
{
    if(!src.is_valid())
        return false;
    auto r = src.get_root();
    assert(r);
    auto uientry = convert_entry_to_core(ctx.get_src_entry(), src);
    if(!uientry)
        return false;
    assert(uientry->get_name() == _t("ui_entry"));
    if(uientry.is_leaf())
        return true;
    auto ui1 = uientry.child(); /* select 1st branch */
    assert(ui1);
    auto dstr = uitree.birth_tail(ui_tree::iterator(0));
    make_core_node(*dstr, *ui1, ctx);
    return analysis_core_sub_nodes(uitree, dstr, ctx, src, ui1);
}

static bool convert_prop_map_to_source(xmltree& src, xmltree::iterator srciter, const ui_node& uinode)
{
    for(const auto& prop : uinode.prop_map) {
        auto i = src.birth_tail<xml_element>(srciter);
        assert(i);
        string propname;
        propname.format(_t("prop:%s"), prop.first.c_str());
        i->set_name(propname);
        auto j = src.birth_tail<xml_value>(i);
        assert(j);
        j->set_name(prop.second);
    }
    return true;
}

static bool convert_proc_map_to_source(xmltree& src, xmltree::iterator srciter, const ui_node& uinode)
{
    for(const auto& procnode : uinode.proc_map) {
        auto i = src.birth_tail<xml_element>(srciter);
        assert(i);
        string procname;
        procname.format(_t("proc:%s"), procnode.first.c_str());
        i->set_name(procname);
        if(!procnode.second.args.empty())
            i->set_attribute(_t("args"), procnode.second.args);
        auto j = src.birth_tail<xml_value>(i);
        assert(j);
        j->set_name(procnode.second.injection);
    }
    return true;
}

static bool make_source_node(xmltree& src, xmltree::iterator srciter, const ui_node& uinode)
{
    assert(srciter);
    srciter->set_name(uinode.type);
    srciter->set_attribute(string(_t("name")), uinode.name);
    srciter->set_attribute(string(_t("x")), string().from_int(uinode.position.left));
    srciter->set_attribute(string(_t("y")), string().from_int(uinode.position.top));
    srciter->set_attribute(string(_t("width")), string().from_int(uinode.position.width()));
    srciter->set_attribute(string(_t("height")), string().from_int(uinode.position.height()));
    srciter->set_attribute(string(_t("variable")), uinode.variable);
    srciter->set_attribute(string(_t("is_instance")), uinode.is_instance ? string(_t("true")) : string(_t("false")));
    return convert_proc_map_to_source(src, srciter, uinode) &&
        convert_prop_map_to_source(src, srciter, uinode);
}

static bool convert_sub_nodes_to_source(xmltree& src, xmltree::iterator srciter, const ui_tree::const_iterator& uiit)
{
    assert(srciter && uiit);
    for(auto i = uiit.child(); i; i = uiit.next()) {
        auto j = src.birth_tail<xml_element>(srciter);
        bool ok1 = make_source_node(src, j, *i);
        bool ok2 = convert_sub_nodes_to_source(src, j, i);
        if(!ok1 || !ok2) {
            set_error(_t("error in convert nodes to source."));
            /* just record and continue... */
        }
    }
    return true;
}

static void convert_includes_to_source(xmltree& src, xmltree::iterator parent, const ui_includes& incs)
{
    assert(parent);
    for(const auto& inc : incs) {
        auto title = src.birth_tail<xml_element>(parent);
        assert(title);
        title->set_name(_t("include"));
        auto value = src.birth_tail<xml_value>(title);
        assert(value);
        value->set_name(inc);
    }
}

static void convert_macros_to_source(xmltree& src, xmltree::iterator parent, const ui_macros& macs)
{
    assert(parent);
    for(const auto& mac : macs) {
        auto title = src.birth_tail<xml_element>(parent);
        assert(title);
        title->set_name(_t("macro"));
        title->set_attribute(_t("name"), mac.name);
        if(!mac.definition.empty()) {
            auto value = src.birth_tail<xml_value>(title);
            assert(value);
            value->set_name(mac.definition);
        }
    }
}

static void create_kvpair_to_source(xmltree& src, xmltree::iterator parent, const string& key, const string& value)
{
    assert(!key.empty());
    if(value.empty())
        return;
    auto k = src.birth_tail<xml_element>(parent);
    assert(k);
    k->set_name(key);
    auto v = src.birth_tail<xml_value>(k);
    assert(v);
    v->set_name(value);
}

static xmltree::iterator convert_entry_to_source(xmltree& src, const ui_src_entry& srcentry)
{
    assert(!src.is_valid());
    auto r = src.birth_tail<xml_element>(xmltree::iterator(0));
    assert(r);
    r->set_name(_t("entry"));
    convert_includes_to_source(src, r, srcentry.get_includes());
    convert_macros_to_source(src, r, srcentry.get_macros());
    create_kvpair_to_source(src, r, _t("output"), srcentry.get_output());
    create_kvpair_to_source(src, r, _t("export"), srcentry.get_export());
    create_kvpair_to_source(src, r, _t("preproc"), srcentry.get_preproc());
    create_kvpair_to_source(src, r, _t("postproc"), srcentry.get_postproc());
    return r;
}

bool ui_convert_core_to_source(xmltree& src, const ui_editor_context& ctx, const ui_tree& uitree)
{
    if(!uitree.is_valid())
        return false;
    auto r = uitree.get_root();
    assert(r);
    auto entry = convert_entry_to_source(src, ctx.const_src_entry());
    assert(entry);
    auto uientry = src.birth_tail<xml_element>(entry);
    assert(uientry);
    uientry->set_name(_t("ui_entry"));
    auto ui1 = src.birth_tail<xml_element>(uientry);
    bool ok1 = make_source_node(src, ui1, *r);
    bool ok2 = convert_sub_nodes_to_source(src, ui1, r);
    return ok1 && ok2;
}

};
