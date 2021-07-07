/*
 * Copyright (c) 2016-2021 lymastee, All rights reserved.
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
#include <ariel/io/utilities.h>

#define ariel_io_blanks _t(" \t\v\r\n\f")

__ariel_begin__

bool io_bad_eof(const string& src, int32 curr)
{
    if(curr < 0) {
        set_error(_t("An error was already existed.\n"));
        return true;
    }
    if(curr >= src.length()) {
        set_error(_t("Unexpected end of file.\n"));
        return true;
    }
    return false;
}

int32 io_skip_blank_charactors(const string& src, int32 start)
{
    int32 p = (int32)src.find_first_not_of(ariel_io_blanks, start);
    if(p == string::npos)
        return src.length();
    return p;
}

int32 io_read_section_name(const string& src, string& name, int32 start)
{
    int32 p = (int32)src.find_first_of(ariel_io_blanks ":{", start);
    if(p == string::npos)
        return src.length();
    if(p > start)
        name.assign(src.c_str() + start, p - start);
    return p;
}

int32 io_read_notation(const string& src, string& notation, int32 start)
{
    int32 p = (int32)src.find_first_of(ariel_io_blanks "{", start);
    if(p == string::npos)
        return src.length();
    if(p > start)
        notation.assign(src.c_str() + start, p - start);
    return p;
}

int32 io_skip_section(const string& src, int32 start)
{
    int32 next = (int32)src.find_first_of(_t("{}"), start);
    if(next == string::npos)
        return -1;
    if(src.at(next) == _t('}'))
        return ++ next;
    if(++ next >= src.length())
        return -1;
    int layercnt = 1;
    for(;; ++ next) {
        next = src.find_first_of(_t("{}"), start = next);
        if(next == string::npos)
            return -1;
        switch(src.at(next))
        {
        case _t('{'):
            layercnt ++;
            break;
        case _t('}'):
        default:
            layercnt --;
            break;
        }
        if(!layercnt)
            break;
    }
    assert(src.at(next) == _t('}'));
    return ++ next;
}

int32 io_enter_section(const string& src, int32 start, gchar st)
{
    assert(start < src.length());
    if(src.at(start) == st)
        return ++ start;
    int32 next = io_skip_blank_charactors(src, start);
    if(io_bad_eof(src, next) || src.at(next) != st)
        return -1;
    return ++ next;
}

int32 io_read_line_of_section(const string& src, string& line, int32 start)
{
    assert(start < src.length());
    int32 p = (int32)src.find_first_of(_t("\r\n}"), start);
    if(io_bad_eof(src, p))
        return -1;
    if(src.at(p) == _t('}')) {
        set_error(_t("Unexpected end of section.\n"));
        return -1;
    }
    line.assign(src, start, p - start);
    return (int32)src.find_first_not_of(_t("\r\n"), p);
}

io_binary_stream::io_binary_stream(int32 size)
{
    _current = 0;
    _size = size;
}

io_binary_stream::control_type io_binary_stream::read_control_type()
{
    if(!next_byte_valid()) {
        set_error(_t("io_binary_stream::read_control_type failed."));
        return ctl_unknown;
    }
    byte ct1 = read_byte(), ct2;
    switch(ct1)
    {
    case _t('#'):
        return ctl_section;
    case _t('$'):
        return ctl_notation;
    case _t('@'):
        return ctl_counter;
    case _t('%'):
        assert(next_byte_valid());
        ct2 = read_byte();
        switch(ct2)
        {
        case _t('b'):
            return ctl_byte_stream_field;
        case _t('w'):
            return ctl_word_stream_field;
        case _t('d'):
            return ctl_dword_stream_field;
        case _t('q'):
            return ctl_qword_stream_field;
        default:
            return ctl_unknown;
        }
    }
    return ctl_unknown;
}

bool io_binary_stream::section_stack_valid(int32 bytes) const
{
    if(_section_stack.empty())
        return true;
    for(size_t i = 0; i < _section_stack.size(); ++ i) {
        int32 rest = _section_stack.at(i);
        if(rest < bytes)
            return false;
    }
    return true;
}

void io_binary_stream::take_next_n_bytes(int32 n)
{
    _current += n;
    assert(_current <= _size);
    if(_section_stack.empty())
        return;
    for(int i = (int)_section_stack.size() - 1; i >= 0; i --) {
        _section_stack.at(i) -= n;
        assert(_section_stack.at(i) >= 0);
    }
}

bool io_binary_stream::next_n_bytes_valid(int32 bytes) const
{
    if(_size - _current < bytes)
        return false;
    return section_stack_valid(bytes);
}

void io_binary_stream::seek_to(int32 bytes)
{
    seek_by(bytes - _current);
}

void io_binary_stream::seek_by(int32 bytes)
{
    take_next_n_bytes(bytes);
    rewind_to(_current);
}

void io_binary_stream::rewind_by(int32 bytes)
{
    int32 pos = current_dev_pos();
    rewind_to(pos + bytes);
}

float io_binary_stream::read_float()
{
    dword dat = read_dword();
    return *(float*)&dat;
}

double io_binary_stream::read_double()
{
    qword dat = read_qword();
    return *(double*)&dat;
}

int32 io_binary_stream::read_nstring(string& str)
{
    if(!next_dword_valid()) {
        set_error(_t("io_binary_stream: bad format for string.\n"));
        return -1;
    }
    int32 size = (int32)read_dword();
    if(!next_n_bytes_valid(size)) {
        set_error(_t("io_binary_stream: bad format for string.\n"));
        return -1;
    }
    str.resize(size);
    read_field_to_buf(&str.front(), size);
    take_next_n_bytes(size);
    return size;
}

int32 io_binary_stream::read_string(string& str, const string& stopch)
{
    if(stopch.empty())
        return -1;
    str.clear();
    auto sto = current_dev_pos();
    while(next_byte_valid()) {
        gchar c = (gchar)read_byte();
        if(stopch.find(c, 0) != string::npos) {
            seek_by(-1);
            return (int32)str.size();
        }
        str.push_back(c);
    }
    seek_to(sto);
    return -1;
}

bool io_binary_stream::exit_section()
{
    if(_section_stack.empty())
        return false;
    if(_section_stack.back() <= 0) {
        _section_stack.pop_back();
        return true;
    }
    return false;
}

bool io_binary_stream::skip_current_section()
{
    if(_section_stack.empty())
        return false;
    int32 cssz = _section_stack.back();
    if(cssz <= 0) {
        exit_section();
        return true;
    }
    seek_by(cssz);
    verify(exit_section());
    return true;
}

bool io_binary_stream::skip_next_section()
{
    auto ct = read_control_type();
    if(ct == ctl_notation) {
        string cast;
        if(read_string(cast, _t("$@")) <= 0) {
            set_error(_t("skip next section failed.\n"));
            return false;
        }
        ct = read_control_type();
    }
    if(ct != ctl_counter) {
        set_error(_t("skip next section failed.\n"));
        return false;
    }
    int32 sz = (int32)read_dword();
    enter_section(sz);
    return skip_current_section();
}

io_binary_memory::io_binary_memory(const void* ptr, int32 size):
    io_binary_stream(size)
{
    _mem = (const byte*)ptr;
}

byte io_binary_memory::read_byte()
{
    byte r = *(_mem + _current);
    take_next_n_bytes(1);
    return r;
}

word io_binary_memory::read_word()
{
    word r = *(word*)(_mem + _current);
    take_next_n_bytes(2);
    return r;
}

dword io_binary_memory::read_dword()
{
    dword r = *(dword*)(_mem + _current);
    take_next_n_bytes(4);
    return r;
}

qword io_binary_memory::read_qword()
{
    qword r = *(qword*)(_mem + _current);
    take_next_n_bytes(8);
    return r;
}

bool io_binary_memory::read_field_to_buf(void* ptr, int32 bytes)
{
    if(!next_n_bytes_valid(bytes))
        return false;
    memcpy(ptr, _mem + _current, bytes);
    take_next_n_bytes(bytes);
    return true;
}

io_binary_file::io_binary_file(file& pf):
    _file(pf), io_binary_stream(pf.size())
{
}

byte io_binary_file::read_byte()
{
    byte r;
    _file.get(&r, 1);
    take_next_n_bytes(1);
    return r;
}

word io_binary_file::read_word()
{
    word r;
    _file.get(&r, 1);
    take_next_n_bytes(2);
    return r;
}

dword io_binary_file::read_dword()
{
    dword r;
    _file.get(&r, 1);
    take_next_n_bytes(4);
    return r;
}

qword io_binary_file::read_qword()
{
    qword r;
    _file.get(&r, 1);
    take_next_n_bytes(8);
    return r;
}

bool io_binary_file::read_field_to_buf(void* ptr, int32 bytes)
{
    if(!next_n_bytes_valid(bytes))
        return false;
    _file.get((byte*)ptr, bytes);
    take_next_n_bytes(bytes);
    return true;
}

__ariel_end__
