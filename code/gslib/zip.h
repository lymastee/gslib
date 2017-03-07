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

#ifndef zip_efeefa3a_157a_4c4c_83b0_99ec8e0364a1_h
#define zip_efeefa3a_157a_4c4c_83b0_99ec8e0364a1_h

#include <gslib/config.h>
#include <gslib/tree.h>
#include <gslib/vdir.h>

__gslib_begin__

typedef void* zip_handle;

struct zip_argument
{
    int             level;      /* 0-9, default -1 */
    const gchar*    name;
    const gchar*    comment;

    zip_argument() { level = -1, comment = name = 0; }
    zip_argument(const gchar* n) { level = -1, name = n, comment = 0; }
    zip_argument(int l, const gchar* n, const gchar* c) { level = l, name = n, comment = c; }
};

struct zip_node
{
    enum
    {
        zt_folder,
        zt_file,
        zt_buffer,
    };
    uint            tag;
    string          name;
};

typedef _treenode_wrapper<zip_node> zip_wrapper;
typedef tree<zip_node, zip_wrapper> zip_directory;

struct zip_folder:
    public zip_node
{
    zip_folder() { tag = zt_folder; }
};

struct zip_file:
    public zip_node
{
    string          password;
    string          comment;
    string          path;
    zip_file() { tag = zt_file; }
};

struct zip_buffer:
    public zip_node
{
    string          password;
    string          comment;
    vessel          buffer;
    zip_buffer() { tag = zt_buffer; }
};

extern void get_zip_path(string& path, const zip_wrapper* w);
extern void create_zip_directory(zip_directory& dir, const gchar* path);
extern void convert_zip_directory(zip_directory& dir, vdir& vd);
extern bool do_zip(const zip_argument& arg, const zip_directory& dir);
extern bool do_zip(const zip_argument& arg, const gchar* path);

struct zip_info_node
{
    string          name;
    int             length;
    int             size;
    float           ratio;
    uint            crc;
};

typedef list<zip_info_node> zip_info;

extern bool view_zip_info(zip_info& info, const gchar* path);
extern void do_unzip(const gchar* src, const gchar* dest);
extern void do_unzip(const gchar* src, vdir& vd);

class zip
{
public:
    zip() { _zh = 0; }
    ~zip() { close(0); }
    void create(const zip_argument& arg);
    void close(const gchar* comment);
    bool start(const gchar* name, const gchar* password, const gchar* comment);
    bool write(const void* buf, int size);
    void finish();

protected:
    zip_handle      _zh;
    zip_argument    _arg;
};

class unzip
{
public:
    unzip() { _zh = 0; }
    ~unzip() { close(); }
    bool open(const gchar* filename);
    void close();
    bool view(zip_info& info);
    bool extract(const gchar* path);
    bool extract_item(const gchar* path, const gchar* name, const gchar* password);
    bool extract(vdir& vd);
    bool extract_item(vdir& vd, const gchar* name, const gchar* password);

protected:
    zip_handle      _zh;
};

__gslib_end__

#endif
