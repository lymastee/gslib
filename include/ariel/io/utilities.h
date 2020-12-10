/*
 * Copyright (c) 2016-2020 lymastee, All rights reserved.
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

#pragma once

#ifndef utilities_9904f6c1_3075_4335_b065_c7099f52e80b_h
#define utilities_9904f6c1_3075_4335_b065_c7099f52e80b_h

#include <ariel/type.h>
#include <gslib/std.h>
#include <gslib/string.h>
#include <gslib/file.h>

__ariel_begin__

/*
 * Text format elements:
 *  [section] { ... }
 *  [section]:[notation] { ... }
 *
 * where sections could be nested
 */

extern bool io_bad_eof(const string& src, int32 curr);
extern int32 io_skip_blank_charactors(const string& src, int32 start);
extern int32 io_read_section_name(const string& src, string& name, int32 start);
extern int32 io_read_notation(const string& src, string& notation, int32 start);
extern int32 io_skip_section(const string& src, int32 start);
extern int32 io_enter_section(const string& src, int32 start, gchar st = _t('{'));
extern int32 io_read_line_of_section(const string& src, string& line, int32 start);

/*
 * Binary format elements:
 *  #[uint32][string]@[uint32]      section about length(in bytes)
 *  $[uint32][string]               notation string(ASCII only)
 */

using gs::file;

class __gs_novtable io_binary_stream abstract
{
public:
    typedef vector<int32> section_stack;

    enum control_type
    {
        ctl_unknown,
        ctl_section,
        ctl_notation,
        ctl_counter,
        ctl_byte_stream_field,
        ctl_word_stream_field,
        ctl_dword_stream_field,
        ctl_qword_stream_field,
    };

public:
    io_binary_stream(int32 size);
    virtual ~io_binary_stream() {}
    control_type read_control_type();
    bool next_byte_valid() const { return next_n_bytes_valid(1); }
    bool next_word_valid() const { return next_n_bytes_valid(2); }
    bool next_dword_valid() const { return next_n_bytes_valid(4); }
    bool next_qword_valid() const { return next_n_bytes_valid(8); }
    bool next_n_bytes_valid(int32 bytes) const;
    void seek_to(int32 bytes);
    void seek_by(int32 bytes);
    float read_float();
    double read_double();
    int32 read_nstring(string& str);
    int32 read_string(string& str, const string& stopch);
    void enter_section(int32 size) { _section_stack.push_back(size); }
    bool exit_section();
    bool skip_current_section();
    bool skip_next_section();

protected:
    int32                   _size;
    int32                   _current;
    section_stack           _section_stack;

public:
    virtual byte read_byte() = 0;
    virtual word read_word() = 0;
    virtual dword read_dword() = 0;
    virtual qword read_qword() = 0;
    virtual bool read_field_to_buf(void* ptr, int32 bytes) = 0;
    virtual void rewind_to(int32 bytes) = 0;
    virtual int32 current_dev_pos() const = 0;

protected:
    void rewind_by(int32 bytes);
    void take_next_n_bytes(int32 n);
    bool section_stack_valid(int32 bytes) const;
};

class io_binary_memory:
    public io_binary_stream
{
public:
    io_binary_memory(const void* ptr, int32 size);
    virtual byte read_byte() override;
    virtual word read_word() override;
    virtual dword read_dword() override;
    virtual qword read_qword() override;
    virtual bool read_field_to_buf(void* ptr, int32 bytes) override;
    virtual void rewind_to(int32 bytes) override {}
    virtual int32 current_dev_pos() const override { return _current; }

protected:
    const byte*             _mem;
};

class io_binary_file:
    public io_binary_stream
{
public:
    io_binary_file(file& pf);
    virtual byte read_byte() override;
    virtual word read_word() override;
    virtual dword read_dword() override;
    virtual qword read_qword() override;
    virtual bool read_field_to_buf(void* ptr, int32 bytes) override;
    virtual void rewind_to(int32 bytes) override { _file.seek(bytes, SEEK_SET); }
    virtual int32 current_dev_pos() const override { return _file.current(); }

protected:
    file&                   _file;
};

__ariel_end__

#endif
