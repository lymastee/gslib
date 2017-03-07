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
#include <gslib/file.h>
#include <gslib/xml.h>

__gslib_begin__

#define xmlblkstr  _t(" \t\v\r\n\f")

static int cpsetpos(const gchar* str, const gchar* cpset)
{
    assert(str && cpset);
    const gchar* s = strtool::_test(str, cpset);
    return s ? (s - str) : strtool::length(str);
}

string xml_element::to_string() const
{
    string s, sf;
    sf.format(_t("name[%s], attr{ "), key.c_str());
    for(kvplist::const_iterator i = attrlist.begin(); i != attrlist.end(); ++ i) {
        s.format(_t(" [%s]=[%s],"), i->key.c_str(), i->value.c_str());
        sf.append(s);
    }
    sf.pop_back();
    sf.append(_t(" }"));
    return sf;
}

string xml_value::to_string() const
{
    string s;
    s.format(_t("@value[%s]"), value.c_str());
    return s;
}

xmltree::xmltree()
{
    _encodesys = encodesys_unknown;
    _encode = encode_unknown;
}

const gchar* xmltree::skip(const gchar* str)
{
    if(!str || !str[0])
        return 0;
    static const string blk = xmlblkstr;
    for(;; str ++) {
        if(!str[0])
            return 0;
        if(blk.find(str[0]) == string::npos)
            return str;
    }
    return 0;
}

const gchar* xmltree::skip(const gchar* str, const gchar* skp)
{
    assert(skp);
    if(!str || !str[0])
        return 0;
    const string blk = skp;
    for(;; str ++) {
        if(!str[0])
            return 0;
        if(blk.find(str[0]) == string::npos)
            return str;
    }
    return 0;
}

void xmltree::replace(string& str, const gchar* s1, const gchar* s2)
{
    assert(s1 && s2);
    int n = strtool::length(s1), dis = strtool::length(s2);
    for(int i = 0;; ) {
        int j = str.find(s1, i);
        if(j == string::npos)
            return;
        str.replace(j, n, s2);
        i = j + dis;
    }
}

void xmltree::filter_entity_reference(string& s)
{
    replace(s, _t("&lt;"), _t("<"));
    replace(s, _t("&gt;"), _t(">"));
    replace(s, _t("&quot"), _t("\""));
    replace(s, _t("&apos;"), _t("'"));
    replace(s, _t("&amp;"), _t("&"));
}

void xmltree::write_attribute(string& str, const gchar* prefix, const xml_attr& attr, const gchar* postfix)
{
    /* todo: deal with entity reference */
    if(prefix)
        str.append(prefix);
    str.append(attr.key);
    str.append(_t("=\""));
    str.append(attr.value);
    str.push_back(_t('\"'));
    if(postfix)
        str.append(postfix);
}

void xmltree::close_write_item(string& str, const gchar* prefix, const xml_node* node, const gchar* postfix)
{
    assert(node && !node->is_value());
    static_cast_as(const xml_element*, elem, node);
    /* todo: deal with entity reference */
    if(prefix)
        str.append(prefix);
    str.push_back(_t('<'));
    str.append(elem->key);
    xml_const_enum cenum(node);
    cenum.const_for_each([&str](const xml_attr& attr) { write_attribute(str, _t(" "), attr, 0); });
    str.append(_t("/>"));
    if(postfix)
        str.append(postfix);
}

void xmltree::begin_write_item(string& str, const gchar* prefix, const xml_node* node, const gchar* postfix)
{
    /* todo: deal with entity reference */
    assert(node && !node->is_value());
    static_cast_as(const xml_element*, elem, node);
    if(prefix)
        str.append(prefix);
    str.push_back(_t('<'));
    str.append(elem->key);
    xml_const_enum cenum(node);
    cenum.const_for_each([&str](const xml_attr& attr) { write_attribute(str, _t(" "), attr, 0); });
    str.push_back(_t('>'));
    if(postfix)
        str.append(postfix);
}

void xmltree::end_write_item(string& str, const gchar* prefix, const xml_node* node, const gchar* postfix)
{
    assert(node && !node->is_value());
    static_cast_as(const xml_element*, elem, node);
    if(prefix)
        str.append(prefix);
    str.append(_t("</"));
    str.append(elem->key);
    str.push_back(_t('>'));
    if(postfix)
        str.append(postfix);
}

const gchar* xmltree::check_header(const gchar* str, string& version, string& encoding)
{
    if(!str || !str[0])
        return 0;
    if(!(str = skip(str)) || !_cstrcmprun(str, _t("<?")) ||
        !(str = skip(str)) || !_cstrcmprun(str, _t("xml"))) {
        set_error(_t("check header: bad format."));
        return 0;
    }
    string k;
    if(!(str = read_attribute(str, k, version)) || k != _t("version") ||
        !(str = read_attribute(str, k, encoding)) || k != _t("encoding")
        ) {
        set_error(_t("check header: access of key attribute failed."));
        return 0;
    }
    str = skip(str);
    if(_cstrcmprun(str, _t("?>")))
        return str;
    string standalone;
    const gchar* s = read_attribute(str, k, standalone);
    if(s && (k == _t("standalone")))
        str = s;
    if(!(str = skip(str)) || !_cstrcmprun(str, _t("?>"))) {
        set_error(_t("check header: label close error."));
        return 0;
    }
    return str;
}

const gchar* xmltree::read_attribute(const gchar* str, string& k, string& v)
{
    if(!(str = skip(str))) {
        set_error(_t("attrpair: unexpected end of string."));
        return 0;
    }
    const gchar* endstr = strtool::_test(str, _t("=")xmlblkstr);
    if(!endstr) {
        set_error(_t("attrpair: unexpected end of string."));
        return 0;
    }
    k.assign(str, endstr-str);
    str = endstr;
    if(!(str = skip(str, _t("=")xmlblkstr)) || *str++ != _t('"')) {
        set_error(_t("attrpair: unexpected end of string."));
        return 0;
    }
    if(!(endstr = strtool::find(str, _t("\"")))) {
        set_error(_t("attrpair: unclose quote."));
        return 0;
    }
    v.assign(str, endstr-str);
    filter_entity_reference(v);
    return ++endstr;
}

const gchar* xmltree::read_item(const gchar* str, iterator p)
{
    if(!(str = skip(str))) {
        set_error(_t("itemproc: unexpected end of string."));
        return 0;
    }
    if(str[0] == _t('<')) {
        const gchar* s = ++ str;
        if(_cstrcmp(s, _t("!--")))
            return strtool::find(s, _t("-->"));
        if(!(s = strtool::_test(str, _t("/>")xmlblkstr))) {
            set_error(_t("itemproc: unexpected end of string."));
            return 0;
        }
        iterator n = birth_tail<xml_element>(p);
        assert(n.is_valid());
        n->set_name(str, s-str);
        str = s;
        while(str[0] != _t('/') && str[0] != _t('>')) {
            xml_attr attr;
            if(!(str = read_attribute(str, attr.key, attr.value)))
                return 0;
            n->add_attribute(attr);
            if(!(str = skip(str))) {
                set_error(_t("itemproc: unexpected end of string."));
                return 0;
            }
        }
        if(str[0] == _t('/')) {
            if(str[1] != _t('>')) {
                set_error(_t("itemproc: unclosed item."));
                return 0;
            }
            return str + 2;
        }
        else if(str[0] == _t('>')) {
            for(++ str;;) {
                if(!(str = skip(str))) {
                    set_error(_t("itemproc: unexpected end of string."));
                    return 0;
                }
                if(str[0] == _t('<')) {
                    if(str[1] == _t('/')) {
                        str += 2;
                        int dis = cpsetpos(str, _t(">"));
                        if(!str[dis]) {
                            set_error(_t("itemproc: unexpected end of string."));
                            return 0;
                        }
                        if(n->get_name().compare(str, dis) != 0) {
                            set_error(_t("itemproc: incorrect pair of item."));
                            return 0;
                        }
                        return str += ++ dis;
                    }
                    else {
                        if(!(str = read_item(str, n))) {
                            set_error(_t("itemproc: unexpected end of string."));
                            return 0;
                        }
                    }
                }
                else {
                    if(!(str = read_item(str, n)))
                        return 0;
                }
            }
        }
        set_error(_t("itemproc: unknown error."));
        return 0;
    }
    else {
        const gchar* endstr = strtool::_test(str, _t("<"));
        if(!endstr) {
            set_error(_t("itemproc: unexpected end of string."));
            return 0;
        }
        assert(p);
        iterator n = birth_tail<xml_value>(p);
        n->set_name(str, endstr-str);
        return endstr;
    }
}

void xmltree::setup_encoding_info(const string& enc)
{
    /* guess encoding */
    if(enc.compare_cl(_t("ansi")) == 0) {
        _encode = encode_ansi;
        _encodesys = encodesys_ascii;
    }
    else if(enc.compare_cl(_t("gb2312")) == 0) {
        _encode = encode_gb2312;
        _encodesys = encodesys_mbc;
    }
    else if(enc.compare_cl(_t("utf-8")) == 0) {
        _encode = encode_utf8;
        _encodesys = encodesys_wstr;
    }
    else if(enc.compare_cl(_t("utf-16")) == 0) {
        _encode = encode_utf16;
        _encodesys = encodesys_wstr;
    }
}

void xmltree::write_header(string& str) const
{
    str.append(_t("<?xml version=\"1.0\" encoding=\""));
    switch(_encode)
    {
    case encode_gb2312:
        str.append(_t("GB2312"));
        break;
    case encode_utf8:
        str.append(_t("UTF-8"));
        break;
    case encode_utf16:
        str.append(_t("UTF-16"));
        break;
    case encode_unknown:
    case encode_ansi:
    default:
        str.append(_t("ANSI"));
    }
    str.append(_t("\" standalone=\"yes\"?>"));
}

void xmltree::write_item(string& str, const_iterator i) const
{
    assert(i.is_valid());
    const xml_node* node = i.get_ptr();
    if(node->is_value())
        str.append(node->get_name());
    else if(!i.childs())
        close_write_item(str, 0, node, 0);
    else {
        begin_write_item(str, 0, node, 0);
        for(const_iterator c = i.child(); c.is_valid(); c = c.next())
            write_item(str, c);
        end_write_item(str, 0, node, 0);
    }
}

bool xmltree::load(const gchar* filename)
{
    assert(filename);
    string buf;
    file f;
    f.open_text(filename, true);
    if(!f.is_valid())
        return false;
    f.read_all(buf);
    return parse(buf.c_str());
}

bool xmltree::parse(const gchar* src)
{
    assert(src);
    string version, encoding;
    if(!(src = check_header(src, version, encoding))) {
        if(!get_last_error())
            set_error(_t("check header failed: unknown error."));
        return false;
    }
    setup_encoding_info(encoding);
    return read_item(src, get_root()) != 0;
}

void xmltree::output(string& str) const
{
    write_header(str);
    write_item(str, get_root());
}

void xmltree::save(const gchar* filename) const
{
    assert(filename);
    /* todo: optimize for writing file. */
    string buf;
    output(buf);
    file f;
    f.open_text(filename, false);
    if(!f.is_valid())
        return;
    f.write(buf);
}

struct upl_trap
{
    gchar*          _src;
    gchar           _trapc;
    int             _trapp;

    upl_trap() { _src = 0, _trapc = 0, _trapp = 0; }
    upl_trap(const gchar* str, int p) { capture(str, p); }
    ~upl_trap() { release(); }
    void capture(const gchar* str, int p)
    {
        _src = const_cast<gchar*>(str);
        _trapc = _src[p];
        _trapp = p;
        _src[p] = 0;
    }
    void release()
    {
        if(_src) {
            _src[_trapp] = _trapc;
            _src = 0;
        }
    }
};

struct __gs_novtable upl_condition abstract
{
    enum
    {
        uc_both,
        uc_either,
        uc_negate,
        uc_hit,
        uc_equal,
        uc_clequal,
    };
    virtual ~upl_condition() {}
    virtual uint get_tag() const = 0;
    virtual bool fulfill(const xml_node* node) const = 0;
};

struct upl_condition_both:
    public upl_condition
{
    upl_condition*  _condition[2];

public:
    upl_condition_both() { _condition[0] = _condition[1] = 0; }
    upl_condition_both(upl_condition* c1, upl_condition* c2) { _condition[0] = c1, _condition[1] = c2; }
    void set_condition(int i, upl_condition* c) { _condition[i] = c; }
    upl_condition* get_condition(int i) const { return _condition[i]; }
    virtual uint get_tag() const override { return uc_both; }
    virtual bool fulfill(const xml_node* node) const override
    {
        assert(_condition[0] && _condition[1]);
        return _condition[0]->fulfill(node) && _condition[1]->fulfill(node);
    }
};

struct upl_condition_either:
    public upl_condition
{
    upl_condition*  _condition[2];

public:
    upl_condition_either() { _condition[0] = _condition[1] = 0; }
    upl_condition_either(upl_condition* c1, upl_condition* c2) { _condition[0] = c1, _condition[1] = c2; }
    void set_condition(int i, upl_condition* c) { _condition[i] = c; }
    upl_condition* get_condition(int i) const { return _condition[i]; }
    virtual uint get_tag() const override { return uc_either; }
    virtual bool fulfill(const xml_node* node) const override
    {
        assert(_condition[0] && _condition[1]);
        return _condition[0]->fulfill(node) || _condition[1]->fulfill(node);
    }
};

struct upl_condition_negate:
    public upl_condition
{
    upl_condition*  _condition;

public:
    upl_condition_negate() { _condition = 0; }
    upl_condition_negate(upl_condition* c) { _condition = c; }
    virtual uint get_tag() const override { return uc_negate; }
    virtual bool fulfill(const xml_node* node) const override { return !_condition->fulfill(node); }
};

struct upl_condition_hit:
    public upl_condition
{
    string          _key;

public:
    upl_condition_hit() {}
    upl_condition_hit(const gchar* k) { _key.assign(k); }
    virtual uint get_tag() const override { return uc_hit; }
    virtual bool fulfill(const xml_node* node) const override
    {
        const string* v = node->get_attribute(_key);
        return v != 0;
    }
};

struct upl_condition_equal:
    public upl_condition
{
    string          _key;
    string          _value;

public:
    upl_condition_equal() {}
    upl_condition_equal(const gchar* k, const gchar* v)
    {
        assert(k && v);
        _key.assign(k);
        _value.assign(v);
    }
    virtual uint get_tag() const override { return uc_equal; }
    virtual bool fulfill(const xml_node* node) const override
    {
        const string* v = node->get_attribute(_key);
        if(v == 0)
            return false;
        return *v == _value;
    }
};

struct upl_condition_clequal:
    public upl_condition
{
    string          _key;
    string          _value;

public:
    upl_condition_clequal() {}
    upl_condition_clequal(const gchar* k, const gchar* v)
    {
        assert(k && v);
        _key.assign(k);
        _value.assign(v);
    }
    virtual uint get_tag() const override { return uc_clequal; }
    virtual bool fulfill(const xml_node* node) const override
    {
        const string* v = node->get_attribute(_key);
        if(v == 0)
            return false;
        return strtool::compare_cl(v->c_str(), _value.c_str()) == 0;
    }
};

struct upl_pattern
{
public:
    typedef vector<upl_condition*> upl_conditions;
    typedef xmltree::iterator iterator;
    typedef xmltree::const_iterator const_iterator;

public:
    string          name;
    int             breakpoint;
    mutable int     counter;
    upl_conditions  conditions;
    upl_condition*  condition;
    
public:
    upl_pattern() { breakpoint = 1, counter = 0, condition = 0; }
    ~upl_pattern()
    {
        std::for_each(conditions.begin(), conditions.end(), [](upl_condition* c) { gs_del(upl_condition, c); });
        conditions.clear();
    }
    void build(const gchar* ctlstr)
    {
        const gchar* cs = build_name(ctlstr);
        if(cs == 0) {
            breakpoint = 1;
            counter = 0;
            condition = 0;
            return;
        }
        if(cs[0] == _t('#')) {
            cs = build_condition(cs);
            if(cs == 0) {
                breakpoint = 1;
                counter = 0;
                return;
            }
        }
        if(cs[0] == _t('@')) {
            cs = build_counter(cs);
            return;
        }
    }
    void reset() { counter = 0; }
    bool fulfill(const xml_node* node) const
    {
        assert(node);
        if(!name.empty()) {
            if(name != node->get_name())
                return false;
        }
        if(condition) {
            if(!condition->fulfill(node))
                return false;
        }
        return ++counter >= breakpoint;
    }
    iterator run_over(iterator i) const
    {
        assert(i.is_valid());
        iterator c = i.child();
        for( ; c.is_valid(); c.to_next()) {
            if(fulfill(c.get_ptr()))
                return c;
        }
        return iterator(0);
    }

protected:
    const gchar* build_name(const gchar* ctlstr)
    {
        assert(ctlstr && ctlstr[0]);
        if(ctlstr[0] != _t('$')) {
            name.assign(ctlstr);
            return 0;
        }
        assert(ctlstr[0] == _t('$'));
        ctlstr ++;
        int p = cpsetpos(ctlstr, _t("#@"));
        assert(p >= 0);
        if(p == 0) {
            name.clear();
            return 0;
        }
        name.assign(ctlstr, p);
        return !ctlstr[p] ? 0 : ctlstr + p;
    }
    const gchar* build_condition(const gchar* ctlstr)
    {
        assert(ctlstr && ctlstr[0] == _t('#'));
        assert(!condition);
        ctlstr ++;
        int p = cpsetpos(ctlstr, _t("@"));
        if(ctlstr[p] != _t('@')) {
            upl_condition* c = create_first_condition(ctlstr);
            assert(c);
            condition = c;
            return 0;
        }
        upl_trap trap(ctlstr, p);
        upl_condition* c = create_first_condition(ctlstr);
        assert(c);
        condition = c;
        return ctlstr + p;
    }
    const gchar* build_counter(const gchar* ctlstr)
    {
        assert(ctlstr && ctlstr[0] == _t('@'));
        breakpoint = strtool::to_int(++ ctlstr);
        assert(breakpoint > 0);
        return 0;
    }

private:
    upl_condition* create_first_condition(const gchar* ctlstr)
    {
        assert(ctlstr && ctlstr[0]);
        int p = cpsetpos(ctlstr, _t("|,"));
        upl_trap trap(ctlstr, p);
        upl_condition* c = create_condition(ctlstr);
        assert(c);
        trap.release();
        return (!ctlstr[p]) ? c : create_next_condition(c, ctlstr + p);
    }
    upl_condition* create_next_condition(upl_condition* lastc, const gchar* ctlstr)
    {
        assert(lastc && ctlstr && ctlstr[0]);
        gchar ctl = ctlstr[0];
        ctlstr ++;
        int p = cpsetpos(ctlstr, _t("|,"));
        upl_trap trap(ctlstr, p);
        upl_condition* c1 = create_condition(ctlstr);
        assert(c1);
        if(ctl == _t(',')) {
            upl_condition_both* c2 = gs_new(upl_condition_both, lastc, c1);
            conditions.push_back(c2);
            trap.release();
            return (!ctlstr[p]) ? c2 : create_next_condition(c2, ctlstr + p);
        }
        else if(ctl == _t('|')) {
            if(lastc->get_tag() == upl_condition::uc_both) {
                upl_condition_both* ucb = static_cast<upl_condition_both*>(lastc);
                upl_condition_either* c2 = gs_new(upl_condition_either, ucb->get_condition(1), c1);
                conditions.push_back(c2);
                ucb->set_condition(1, c2);
                trap.release();
                return (!ctlstr[p]) ? lastc : create_next_condition(lastc, ctlstr + p);
            }
            else {
                upl_condition_either* c2 = gs_new(upl_condition_either, lastc, c1);
                conditions.push_back(c2);
                trap.release();
                return (!ctlstr[p]) ? c2 : create_next_condition(c2, ctlstr + p);
            }
        }
        assert(!"unexpected.");
        return 0;
    }
    upl_condition* create_condition(const gchar* ctlstr)
    {
        assert(ctlstr && ctlstr[0]);
        if(ctlstr[0] == _t('!')) {
            upl_condition* c1 = create_condition(ctlstr + 1);
            assert(c1);
            upl_condition_negate* c2 = gs_new(upl_condition_negate, c1);
            conditions.push_back(c2);
            return c2;
        }
        const gchar* pclequal = strtool::find(ctlstr, _t("~="));
        if(pclequal) {
            assert(_cstrcmp(pclequal, _t("~=")));
            upl_trap trap(ctlstr, pclequal - ctlstr);
            upl_condition_clequal* c = gs_new(upl_condition_clequal, ctlstr, pclequal + 2);
            conditions.push_back(c);
            return c;
        }
        const gchar* pequal = strtool::find(ctlstr, _t("="));
        if(pequal) {
            assert(pequal[0] == _t('='));
            upl_trap trap(ctlstr, pequal - ctlstr);
            upl_condition_equal* c = gs_new(upl_condition_equal, ctlstr, pequal + 1);
            conditions.push_back(c);
            return c;
        }
        upl_condition_hit* c = gs_new(upl_condition_hit, ctlstr);
        conditions.push_back(c);
        return c;
    }
};

const gchar* xmltree::upl_run_once(iterator& i, const gchar* ctlstr)
{
    assert(ctlstr && ctlstr[0]);
    assert(i.is_valid());
    int p = cpsetpos(ctlstr, _t("\\/"));
    assert(p > 0);
    if(ctlstr[p] == _t('\\') || ctlstr[p] == _t('/')) {
        upl_trap trap(ctlstr, p);
        upl_pattern pattern;
        pattern.build(ctlstr);
        iterator j = pattern.run_over(i);
        if(j.is_valid()) {
            i = j;
            return ctlstr + p + 1;
        }
        return 0;
    }
    upl_pattern pattern;
    pattern.build(ctlstr);
    iterator j = pattern.run_over(i);
    if(j.is_valid()) {
        i = j;
        return ctlstr + p;
    }
    return 0;
}

__gslib_end__
