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

#include <gslib/json.h>
#include <gslib/file.h>
#include <gslib/error.h>

__gslib_begin__

#define json_cst_key(cstr) \
    json_key(cstr, _cststrlen(cstr))

#define jsonblkstr  _t(" \t\v\r\n\f")

static int json_skip_blanks(const gchar* str, int len)
{
    assert(str);
    if(len <= 0)
        return 0;
    static const string blks = jsonblkstr;
    for(int i = 0; i < len; i ++) {
        if(!str[i])     /* end up unexpectedly. */
            return len;
        if(blks.find(str[i]) == string::npos)
            return i;
    }
    return len;
}

static int json_parse_string(string& output, const gchar* str, int len)
{
    assert(str);
    int rs = json_skip_blanks(str, len);
    if(rs == len) {
        set_error(_t("unexpected endings."));
        return -1;
    }
    const gchar* reader = str + rs;
    int rest = len - rs;
    assert(rest > 0);
    if(reader[0] != _t('\"')) {
        set_error(_t("parse string failed."));
        return -1;
    }
    reader ++, rest --;
    while(rest > 0) {
        if(reader[0] == _t('\"')) {
            reader ++;
            rest --;
            return len - rest;
        }
        else if(reader[0] == _t('\\')) {
            reader ++;
            rest --;
            if(rest <= 0) {
                set_error(_t("bad escaping char."));
                return -1;
            }
            switch(reader[0])
            {
            case _t('a'):
                output.push_back(_t('\a'));
                break;
            case _t('b'):
                output.push_back(_t('\b'));
                break;
            case _t('f'):
                output.push_back(_t('\f'));
                break;
            case _t('n'):
                output.push_back(_t('\n'));
                break;
            case _t('r'):
                output.push_back(_t('\r'));
                break;
            case _t('t'):
                output.push_back(_t('\t'));
                break;
            case _t('v'):
                output.push_back(_t('\v'));
                break;
            case _t('\\'):
                output.push_back(_t('\\'));
                break;
            case _t('?'):
                output.push_back(_t('?'));
                break;
            case _t('\''):
                output.push_back(_t('\''));
                break;
            case _t('\"'):
                output.push_back(_t('\"'));
                break;
            case _t('0'):
                output.push_back(_t('\0'));
                break;
            default:
                set_error(_t("unexpected escaping char."));
                return -1;
            }
        }
        else {
            output.push_back(reader[0]);
        }
        reader ++;
        rest --;
    }
    set_error(_t("unclosed string."));
    return -1;
}

/* "??": */
static int json_parse_name(string& name, const gchar* str, int len)
{
    assert(str);
    int rs = json_parse_string(name, str, len);
    if(rs == -1)
        return -1;
    const gchar* reader = str + rs;
    int rest = len - rs;
    rs = json_skip_blanks(reader, rest);
    if((rs == rest) || (reader[rs] != _t(':'))) {
        set_error(_t("unexpected end of name, should be \':\'."));
        return -1;
    }
    rest -= (rs + 1);
    return len - rest;
}

json_node_array::~json_node_array()
{
    for(auto* p : _array) { gs_del(json_node, p); }
    _array.clear();
}

int json_node_array::parse(const gchar* src, int len)
{
    assert(src && (len > 0));
    assert(src[0] == _t('['));
    const gchar* reader = src + 1;
    int rest = len - 1;
    int rs = json_skip_blanks(reader, rest);
    if(rs == rest) {
        set_error(_t("unexpected endings."));
        return -1;
    }
    reader += rs;
    rest -= rs;
    if(reader[0] == _t(']'))    /* empty array, but valid. */
        return len - --rest;
    while(rest > 0) {
        if(reader[0] == _t('{')) {
            auto* ch = gs_new(json_node_table);
            _array.push_back(ch);
            rs = ch->parse(reader, rest);
            if(rs == -1) {
                set_error(_t("parse child object failed."));
                return -1;
            }
        }
        else {
            auto* ch = gs_new(json_node_value);
            _array.push_back(ch);
            rs = ch->parse(reader, rest);
            if(rs == -1) {
                set_error(_t("parse child value failed."));
                return -1;
            }
        }
        reader += rs;
        rest -= rs;
        rs = json_skip_blanks(reader, rest);
        if(rs == rest) {
            set_error(_t("unexpected endings."));
            return -1;
        }
        reader += rs;
        rest -= rs;
        assert(rest > 0);
        if(reader[0] == _t(',')) {
            rs = json_skip_blanks(++reader, --rest);
            if(rs == rest) {
                set_error(_t("unexpected endings."));
                return -1;
            }
            reader += rs;
            rest -= rs;
        }
        else if(reader[0] == _t(']')) {
            rest --;
            assert(rest >= 0);
            break;
        }
        else {
            set_error(_t("unexpected character, should be \',\' or \']\'."));
            return -1;
        }
    }
    if(_array.size() == 0) {
        set_error(_t("array should have at least 1 child."));
        return -1;
    }
    return len - rest;
}

json_node_table::~json_node_table()
{
    for(auto* p : _table) { gs_del(json_node, p); }
    _table.clear();
}

json_node* json_node_table::find(const string& name) const
{
    auto f = _table.find(&json_key(name));
    return (f != _table.end()) ? *f : 0;
}

int json_node_table::parse(const gchar* src, int len)
{
    assert(src && (len > 0));
    assert(src[0] == _t('{'));
    if(len == 1) {
        set_error(_t("unexpected endings."));
        return -1;
    }
    const gchar* reader = src + 1;
    int rest = len - 1;
    int rs = json_skip_blanks(reader, rest);
    if(rs == rest) {
        set_error(_t("unexpected endings."));
        return -1;
    }
    reader += rs;
    rest -= rs;
    if(reader[0] == _t('}'))    /* empty table, but valid. */
        return len - --rest;
    while(rest > 0) {
        string ch_name;
        rs = json_parse_name(ch_name, reader, rest);
        if(rs == -1) {
            set_error(_t("parse name failed."));
            return -1;
        }
        if(rs == rest) {
            set_error(_t("unexpected endings."));
            return -1;
        }
        reader += rs;
        rest -= rs;
        rs = json_skip_blanks(reader, rest);
        if(rs == rest) {
            set_error(_t("unexpected endings."));
            return -1;
        }
        reader += rs;
        rest -= rs;
        if(reader[0] == _t('{')) {
            auto* p = gs_new(json_node_table);
            p->set_name(ch_name);
            _table.insert(p);
            rs = p->parse(reader, rest);
            if(rs == -1)
                return -1;
            if(rs == rest) {
                set_error(_t("unexpected endings."));
                return -1;
            }
        }
        else if(reader[0] == _t('[')) {
            auto* p = gs_new(json_node_array);
            p->set_name(ch_name);
            _table.insert(p);
            rs = p->parse(reader, rest);
            if(rs == -1)
                return -1;
            if(rs == rest) {
                set_error(_t("unexpected endings."));
                return -1;
            }
        }
        else {
            auto* p = gs_new(json_node_pair);
            p->set_name(ch_name);
            _table.insert(p);
            rs = p->parse(reader, rest);
            if(rs == -1)
                return -1;
            if(rs == rest) {
                set_error(_t("unexpected endings."));
                return -1;
            }
        }
        reader += rs;
        rest -= rs;
        assert(rest > 0);
        rs = json_skip_blanks(reader, rest);
        if(rs == rest) {
            set_error(_t("unexpected endings."));
            return -1;
        }
        reader += rs;
        rest -= rs;
        if(reader[0] == _t('}')) {
            rest --;
            assert(rest >= 0);
            break;
        }
        else if(reader[0] == _t(',')) {
            rs = json_skip_blanks(++reader, --rest);
            if(rs == rest) {
                set_error(_t("unexpected endings."));
                return -1;
            }
            reader += rs;
            rest -= rs;
        }
        else {
            set_error(_t("unexpected character, should be \',\' or \'}\'."));
            return -1;
        }
    }
    return len - rest;
}

int json_node_value::parse(const gchar* src, int len)
{
    assert(src);
    int rs = json_skip_blanks(src, len);
    if(rs == len) {
        set_error(_t("unexpected endings."));
        return len;
    }
    const gchar* reader = src + rs;
    int rest = len - rs;
    if(reader[0] == _t('\"')) {
        rs = json_parse_string(_strval, reader, rest);
        if(rs == -1)
            return -1;
        reader += rs;
        rest -= rs;
    }
    else if(_cstrncmp(reader, rest, _t("true"))) {
        _strval.append(_t("true"));
        rs = _cststrlen(_t("true"));
        reader += rs;
        rest -= rs;
    }
    else if(_cstrncmp(reader, rest, _t("false"))) {
        _strval.append(_t("false"));
        rs = _cststrlen(_t("false"));
        reader += rs;
        rest -= rs;
    }
    else if(_cstrncmp(reader, rest, _t("null"))) {
        _strval.append(_t("null"));
        rs = _cststrlen(_t("null"));
        reader += rs;
        rest -= rs;
    }
    else {
        /* a number could start with '-' */
        if(reader[0] == _t('-')) {
            _strval.push_back(_t('-'));
            rs = json_skip_blanks(++reader, --rest);
            if(rs == rest) {
                set_error(_t("unexpected endings."));
                return -1;
            }
            reader += rs;
            rest -= rs;
        }
        /* read numbers */
        static const gchar num_dot[] = _t("0123456789.");
        static const gchar nums[] = _t("0123456789");
        int i = 0;
        for(; i < rest; i ++) {
            auto* t = strtool::_test(reader + i, num_dot);
            if(!t) {
                if(_strval.empty()) {
                    set_error(_t("parse number failed."));
                    return -1;
                }
                return len - (rest + i);
            }
            _strval.push_back(t[0]);
            if(t[0] == _t('.')) {
                i ++;
                break;
            }
        }
        reader += i;
        rest -= i;
        assert(rest >= 0);
        for(i = 0; i < rest; i ++) {
            auto* t = strtool::_test(reader + i, nums);
            if(!t) {
                rest -= i;
                break;
            }
            _strval.push_back(t[0]);
        }
    }
    return len - rest;
}

int json_node_pair::parse(const gchar* src, int len)
{
    assert(src);
    return _value.parse(src, len);
}

json_node* json_node_table::duplicate() const
{
    auto* p = gs_new(json_node_table);
    assert(p);
    p->set_name(_name);
    for(auto* ch : _table)
        p->get_container().insert(ch->duplicate());
    return p;
}

json_node* json_node_array::duplicate() const
{
    auto* p = gs_new(json_node_array);
    assert(p);
    p->set_name(_name);
    for(auto* ch : _array)
        p->get_container().push_back(ch->duplicate());
    return p;
}

json_node* json_node_pair::duplicate() const
{
    auto* p = gs_new(json_node_pair);
    assert(p);
    p->set_name(_name);
    p->_value.set_value_string(_value.get_name());
    return p;
}

json_node* json_node_value::duplicate() const
{
    auto* p = gs_new(json_node_value);
    assert(p);
    p->set_value_string(_strval);
    return p;
}

bool json_parser::parse(const gchar* src, int len)
{
    assert(src && (len > 0));
    int rs = json_skip_blanks(src, len);
    if(rs == len)
        return true;
    assert(!_root);
    const gchar* reader = src + rs;
    int rest = len - rs;
    assert(rest > 0);
    if(reader[0] == _t('{')) {
        auto* p = gs_new(json_node_table);
        _root = p;
        rs = p->parse(reader, rest);
        if(rs == -1)
            return false;
    }
    else if(reader[0] == _t('[')) {
        auto* p = gs_new(json_node_array);
        _root = p;
        rs = p->parse(reader, rest);
        if(rs == -1)
            return false;
    }
    else {
        set_error(_t("expected \'{\' or \'[\'."));
        return false;
    }
    if(rs != rest) {
        reader += rs;
        rest -= rs;
        rs = json_skip_blanks(reader, rest);
        if(rs != rest) {
            set_error(_t("unexpected endings."));
            return false;
        }
    }
    return true;
}

bool json_parser::parse(const gchar* filename)
{
    assert(filename);
    file f;
    f.open_text(filename, true);
    string buf;
    f.read_all(buf);
    return parse(buf.c_str(), buf.length());
}

void json_parser::destroy()
{
    if(_root) {
        gs_del(json_node, _root);
        _root = 0;
    }
}

__gslib_end__
