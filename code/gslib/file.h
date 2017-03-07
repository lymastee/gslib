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

#ifndef file_d50c3538_0c5e_49a3_bc6f_5d6bc6144520_h
#define file_d50c3538_0c5e_49a3_bc6f_5d6bc6144520_h

#include <stdio.h>
#include <gslib/config.h>
#include <gslib/string.h>

__gslib_begin__

typedef FILE*   fileptr;
typedef fpos_t  fpos;

class file
{
protected:
    fileptr     _file;

public:
    file(): _file(0) {}
    file(const gchar* name, const gchar* mode): _file(0) { open(name, mode); }
    ~file() { close(); }

#ifndef _UNICODE

    void open(const gchar* name, const gchar* mode) { fopen_s(&_file, name, mode); }
    void open_text(const gchar* name, bool readonly)
    {
        readonly ? fopen_s(&_file, name, _t("r")) :
            fopen_s(&_file, name, _t("w"));
    }
    int read(gchar buf[], int len)
    {
        assert(buf && len);
        return fgets(buf, len, _file) ? strlen(buf) : 0;
    }
    int write(const gchar str[])
    {
        assert(str);
        return (int)fputs(str, _file);
    }

#else
    
    void open(const gchar* name, const gchar* mode) { _wfopen_s(&_file, name, mode); }
    void open_text(const gchar* name, bool readonly)
    {
        readonly ? _wfopen_s(&_file, name, _t("r,ccs=UNICODE")) :
            _wfopen_s(&_file, name, _t("w,ccs=UNICODE"));
    }
    int read(gchar buf[], int len)
    {
        assert(buf && len);
        return fgetws(buf, len, _file) ? wcslen(buf) : 0;
    }
    int write(const gchar str[])
    {
        assert(str);
        return fputws(str, _file);
    }

#endif

    bool is_valid() const { return _file != 0; }
    void flush() { fflush(_file); }
    void rewind() { ::rewind(_file); }
    int current() const { return ftell(_file); }
    int seek(int offset, int sps) { return fseek(_file, offset, sps); }
    int size()
    {
        int old = current();
        seek(0, SEEK_END);
        int pos = current();
        seek(old, SEEK_SET);
        return pos;
    }
    void close()
    {
        if (_file) {
            fclose(_file);
            _file = 0;
        }
    }
    int read(string& buf, int start, int len)
    {
        int lento = start + len;
        if(lento && buf.size() < lento)
            buf.resize(lento);
        return read(&buf.front() + start, len);
    }
    void read_all(string& buf)
    {
        int total = size() + 1;     /* a weird problem about fgetws. */
        if(buf.size() < total)
            buf.resize(total);
        for(int i = 0, j; j = read(buf, i, total-i); i += j);
        /* right trim */
        int p = buf.find_last_not_of(_t('\0'));
        if(p != string::npos) {
            p += 1;
            if(p <= buf.length())
                buf.resize(p);
        }
    }
    int write(const string& buf) { return write(&buf.front()); }
    int get(byte buf[], int len) { return (int)fread_s(buf, len, 1, len, _file); }
    int get(word buf[], int len) { return (int)fread_s(buf, len, 2, len, _file); }
    int get(dword buf[], int len) { return (int)fread_s(buf, len, 4, len, _file); }
    int get(qword buf[], int len) { return (int)fread_s(buf, len, 8, len, _file); }
    int put(const byte buf[], int len) { return (int)fwrite(buf, 1, len, _file); }
    int put(const word buf[], int len) { return (int)fwrite(buf, 2, len, _file); }
    int put(const dword buf[], int len) { return (int)fwrite(buf, 4, len, _file); }
    int put(const qword buf[], int len) { return (int)fwrite(buf, 8, len, _file); }

public:
    template<int _szelem>
    int put_elem(const void* buf, int len) { !!error!! }
    template<>
    int put_elem<1>(const void* buf, int len) { return put((const byte*)buf, len); }
    template<>
    int put_elem<2>(const void* buf, int len) { return put((const word*)buf, len); }
    template<>
    int put_elem<4>(const void* buf, int len) { return put((const dword*)buf, len); }
    template<>
    int put_elem<8>(const void* buf, int len) { return put((const qword*)buf, len); }
};

__gslib_end__

#endif
