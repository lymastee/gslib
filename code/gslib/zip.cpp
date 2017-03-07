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

#include <io.h>
#include <gslib/zip.h>
#include <gslib/file.h>
#include <ext/zlib/zlib.h>
#include <ext/zlib/contrib/minizip/zip.h>
#include <ext/zlib/contrib/minizip/unzip.h>

__gslib_begin__

typedef char zchar;
typedef _string<char> zstr;

static const zchar* cvtzctlstr(const gchar* str)
{
#ifdef _UNICODE
    if(!str)
        return 0;
    static zstr s;
    s.from(str);
    return s.c_str();
#else
    return str;
#endif
}

void get_zip_path(string& path, const zip_wrapper* w)
{
    assert(w);
    list<const zip_wrapper*> wp;
    for(const zip_wrapper* p = w; p; p = p->parent())
        wp.push_front(p);
    assert(wp.size() > 0);
    if(wp.size() > 1)
        wp.pop_front(); /* no root folders */
    auto first = wp.begin();
    path.append((*first)->get_ptr()->name);
    std::for_each(++ first, wp.end(), [&path](const zip_wrapper* w) {
        assert(w);
        path.append(_t("\\"));
        path.append(w->get_ptr()->name);
    });
}

static zip_wrapper* create_zip_folder(zip_directory& dir, zip_wrapper* wrapper, const gchar* name)
{
    assert(name);
    auto i = dir.birth_tail<zip_folder>(zip_directory::iterator(wrapper));
    i->name = name;
    return i.get_wrapper();
}

static zip_wrapper* create_zip_file(zip_directory& dir, zip_wrapper* wrapper, const gchar* path, const gchar* name)
{
    assert(path && name);
    auto i = dir.birth_tail<zip_file>(zip_directory::iterator(wrapper));
    zip_file* zf = static_cast<zip_file*>(i.get_ptr());
    zf->name = name;
    zf->path = path;
    return i.get_wrapper();
}

#ifdef _UNICODE
#define _gs_finddata_t _wfinddata_t
#define _gs_findfirst _wfindfirst
#define _gs_findnext _wfindnext
#define _gs_findclose _findclose
#else
#define _gs_finddata_t _finddata_t
#define _gs_findfirst _findfirst
#define _gs_findnext _findnext
#define _gs_findclose _findclose
#endif

static void traverse_zip_directory(zip_directory& dir, zip_wrapper* wrapper, const gchar* parent)
{
    assert(wrapper && parent);
    string mode = mkstr(parent, _t("\\*"));
    _gs_finddata_t fd;
    memset(&fd, 0, sizeof(fd));
    intptr_t ff = _gs_findfirst(mode.c_str(), &fd);
    if(ff == -1)
        return;
    int failed = 0;
    do {
        if(fd.attrib & _A_SUBDIR) {
            if(!(_cstrcmp(fd.name, _t(".")) || _cstrcmp(fd.name, _t("..")))) {
                zip_wrapper* next = create_zip_folder(dir, wrapper, fd.name);
                assert(next);
                traverse_zip_directory(dir, next, mkstr3(parent, _t("\\"), fd.name));
            }
        }
        else if((fd.attrib & _A_NORMAL) ||
            (fd.attrib & _A_RDONLY) ||
            (fd.attrib & _A_HIDDEN) ||
            (fd.attrib & _A_ARCH)
            )
            create_zip_file(dir, wrapper, mkstr3(parent, _t("\\"), fd.name), fd.name);
        failed = _gs_findnext(ff, &fd);
    }
    while(!failed);
    _gs_findclose(ff);
}

static bool is_valid_file(const gchar* path)
{
    assert(path);
    file f(path, _t("rb"));
    return f.is_valid();
}

static uint select_max_but_not(uint p1, uint p2, uint neg)
{
    if((p1 == neg) && (p2 == neg))
        return neg;
    else if(p1 == neg)
        return p2;
    else if(p2 == neg)
        return p1;
    return gs_max(p1, p2);
}

static void extract_last_name(string& name, const gchar* path)
{
    assert(path);
    string p = path;
    assert(p.back() != _t('/') || p.back() != _t('\\'));
    uint last_slash = p.rfind(_t('\\'));
    uint last_bkslash = p.rfind(_t('/'));
    uint pos = select_max_but_not(last_slash, last_bkslash, string::npos);
    if(pos == string::npos)
        name = path;
    else {
        pos ++;
        name.assign(path + pos, p.length() - pos);
    }
}

static void get_root_name(string& name, const gchar* path)
{
    assert(path);
    string str;
    extract_last_name(str, path);
    if(str.back() != _t(':')) {
        name = str;
        return;
    }
    /* could be a root directory, then find the first file and set it as the root name. */
    _gs_finddata_t fd;
    memset(&fd, 0, sizeof(fd));
    intptr_t ff = _gs_findfirst(mkstr(path, _t("\\*")), &fd);
    if(ff == -1) {
        assert(!"no file was found.");
        return;
    }
    name = fd.name;
    _gs_findclose(ff);
}

void create_zip_directory(zip_directory& dir, const gchar* path)
{
    assert(path);
    if(is_valid_file(path)) {
        string name;
        extract_last_name(name, path);
        create_zip_file(dir, 0, path, name.c_str());
        return;
    }
    string name;
    get_root_name(name, path);
    zip_wrapper* root = create_zip_folder(dir, 0, name.c_str());
    assert(root);
    traverse_zip_directory(dir, root, path);
}

static void convert_zip_buffer(zip_buffer* buf, vdirfile* src)
{
    assert(buf && src);
    buf->name = src->get_name();
    buf->buffer.attach(&src->vsl);
}

static void convert_zip_folder(zip_folder* folder, vdirnode* src)
{
    assert(folder && src);
    folder->name = src->get_name();
}

static zip_directory::iterator convert_zip_item(zip_directory& dir, zip_directory::iterator i, vdiriter j)
{
    assert(j);
    zip_directory::iterator zi;
    auto tag = j->get_tag();
    switch(tag)
    {
    case vdirnode::dt_file:
        zi = dir.birth<zip_buffer>(i);
        convert_zip_buffer(static_cast<zip_buffer*>(zi.get_ptr()), static_cast<vdirfile*>(j.get_ptr()));
        break;
    case vdirnode::dt_folder:
        zi = dir.birth<zip_folder>(i);
        convert_zip_folder(static_cast<zip_folder*>(zi.get_ptr()), j.get_ptr());
        break;
    }
    auto ch = j.child();
    for(; ch; ch.to_next())
        convert_zip_item(dir, zi, ch);
    return zi;
}

void convert_zip_directory(zip_directory& dir, vdir& vd)
{
    auto i = vd.get_root();
    if(!i)
        return;
    convert_zip_item(dir, dir.get_root(), i);
}

static bool do_zip_file(zip& z, const zip_argument& arg, const string& path, const zip_file* zf)
{
    assert(zf);
    file f(zf->path.c_str(), _t("rb"));
    if(!f.is_valid())
        return false;
    int size = f.size();
    if(size == 0)
        return true;
    byte* buf = new byte[16384];
    int ct = 0;
    z.start(path.c_str(), zf->password.c_str(), zf->comment.c_str());
    for(;;) {
        int s = f.get(buf, 16384);
        z.write(buf, s);
        ct += s;
        if(ct >= size)
            break;
    }
    z.finish();
    delete [] buf;
    return true;
}

static bool do_zip_buffer(zip& z, const zip_argument& arg, const string& path, const zip_buffer* zb)
{
    assert(zb);
    z.start(path.c_str(), zb->password.c_str(), zb->comment.c_str());
    z.write(zb->buffer.get_ptr(), zb->buffer.get_cur());
    z.finish();
    return true;
}

static bool do_zip_node(zip& z, const zip_argument& arg, const zip_wrapper* wrapper)
{
    assert(wrapper);
    const zip_node* node = wrapper->get_ptr();
    assert(node->tag == zip_node::zt_file ||
        node->tag == zip_node::zt_buffer
        );
    string path;
    get_zip_path(path, wrapper);
    switch(node->tag)
    {
    case zip_node::zt_file:
        return do_zip_file(z, arg, path, static_cast<const zip_file*>(node));
    case zip_node::zt_buffer:
        return do_zip_buffer(z, arg, path, static_cast<const zip_buffer*>(node));
    default:
        assert(!"unexpected.");
        return false;
    }
}

bool do_zip(const zip_argument& arg, const zip_directory& dir)
{
    zip z;
    z.create(arg);
    dir.postorder_traversal([&](const zip_wrapper* w) {
        assert(w);
        auto i = zip_directory::const_iterator(w);
        if(i.is_leaf()) {
            bool b = do_zip_node(z, arg, w);
            assert(b && "create zip node failed.");
        }
    });
    z.close(arg.comment);
    return true;
}

bool do_zip(const zip_argument& arg, const gchar* path)
{
    assert(path);
    zip_directory dir;
    create_zip_directory(dir, path);
    return do_zip(arg, dir);
}

void zip::create(const zip_argument& arg)
{
    _arg = arg;
    const zchar* zipname = cvtzctlstr(arg.name);
    assert(zipname && !_zh);
    _zh = zipOpen(zipname, APPEND_STATUS_CREATE);
    assert(_zh);
}

void zip::close(const gchar* comment)
{
    if(!_zh)
        return;
    const zchar* zipcomment = 0;
    if(comment != 0)
        zipcomment = cvtzctlstr(comment);
    zipClose(_zh, zipcomment);
    _zh = 0;
}

bool zip::start(const gchar* name, const gchar* password, const gchar* comment)
{
    assert(name);
    zip_fileinfo zi;
    memset(&zi, 0, sizeof(zi));
    zstr zipname = cvtzctlstr(name);
    zstr zippwd = cvtzctlstr(password);
    int err = zipOpenNewFileInZip(_zh, zipname.c_str(), &zi, 0, 0, 0, 0, cvtzctlstr(comment), Z_DEFLATED, _arg.level);
    return err == ZIP_OK;
}

bool zip::write(const void* buf, int size)
{
    int err = zipWriteInFileInZip(_zh, buf, size);
    return err == ZIP_OK;
}

void zip::finish()
{
    zipCloseFileInZip(_zh);
}

bool view_zip_info(zip_info& info, const gchar* path)
{
    assert(path);
    unzip uz;
    if(!uz.open(path))
        return false;
    bool r = uz.view(info);
    uz.close();
    return r;
}

void do_unzip(const gchar* src, const gchar* dest)
{
    assert(src && dest);
    unzip uz;
    if(!uz.open(src))
        return;
    uz.extract(dest);
    uz.close();
}

static void fix_root_name(vdir& vd, const gchar* src)
{
    assert(src);
    auto r = vd.get_root();
    if(!r)
        return;
    string str = src;
    int len = str.length();
    int p1 = str.find_last_of(_t('\\'));
    int p2 = str.find_last_of(_t('/'));
    int p = gs_max(p1, p2);
    if(p == -1)
        p = 0;
    r->name.assign(src, p, len - p);
}

void do_unzip(const gchar* src, vdir& vd)
{
    assert(src);
    unzip uz;
    if(!uz.open(src))
        return;
    uz.extract(vd);
    uz.close();
    fix_root_name(vd, src);
}

bool unzip::open(const gchar* filename)
{
    assert(filename);
    _zh = unzOpen(cvtzctlstr(filename));
    return _zh != 0;
}

void unzip::close()
{
    if(!_zh)
        return;
    unzClose(_zh);
    _zh = 0;
}

static void write_zip_info(zip_info_node& node, const char* filename, const unz_file_info& fileinfo)
{
    assert(filename);
    node.name.from(filename);
    node.length = fileinfo.compressed_size;
    node.size = fileinfo.uncompressed_size;
    node.ratio = 0.f;
    if(fileinfo.uncompressed_size > 0)
        node.ratio = ((float)fileinfo.uncompressed_size) / fileinfo.compressed_size;
    node.crc = fileinfo.crc;
}

static bool is_back_with_div(const char* name)
{
    assert(name);
    string str;
    str.from(name);
    gchar ch = str.back();
    return ch == _t('/') || ch == _t('\\');
}

bool unzip::view(zip_info& info)
{
    assert(_zh);
    unz_global_info gi;
    int err = unzGetGlobalInfo(_zh, &gi);
    if(err != UNZ_OK)
        return false;
    for(uint i = 0; i < gi.number_entry; i ++) {
        info.push_back(zip_info_node());
        zip_info_node& last = info.back();
        unz_file_info fileinfo;
        char filename[256];
        unzGetCurrentFileInfo(_zh, &fileinfo, filename, sizeof(filename), 0, 0, 0, 0);
        write_zip_info(last, filename, fileinfo);
        unzGoToNextFile(_zh);
    }
    return true;
}

bool unzip::extract(const gchar* path)
{
    assert(path);
    unz_global_info gi;
    int err = unzGetGlobalInfo(_zh, &gi);
    if(err != UNZ_OK)
        return false;
    for(uint i = 0; i < gi.number_entry; i ++) {
        unz_file_info fileinfo;
        char filename[256];
        unzGetCurrentFileInfo(_zh, &fileinfo, filename, sizeof(filename), 0, 0, 0, 0);
        if(!fileinfo.uncompressed_size)     /* might be a folder */
            assert(is_back_with_div(filename));
        else {
            string localname;
            localname.from(filename);
            if(!extract_item(path, localname.c_str(), 0))
                return false;
        }
        unzGoToNextFile(_zh);
    }
    return true;
}

static const gchar* get_last_div(const gchar* src)
{
    assert(src);
    int len = strtool::length(src);
    for(int i = len; i >= 0; i --) {
        if(src[i] == _t('/') || src[i] == _t('\\'))
            return src + i;
    }
    return 0;
}

static void create_file_path(file& f, const gchar* path)
{
    assert(path);
    string p, n;
    const gchar* divpos = get_last_div(path);
    if(!divpos)
        n = path;
    else {
        assert(divpos[0] == _t('/') || divpos[0] == _t('\\'));
        p.assign(path, divpos - path);
        n.assign(++ divpos);
    }
    if(!p.empty()) {
        dirop dop;
        string oldpath;
        dop.get_current_dir(oldpath);
        dop.make_dir(p.c_str());
        dop.change_dir(p.c_str());
        f.open(n.c_str(), _t("wb"));
        dop.change_dir(oldpath.c_str());
        return;
    }
    f.open(n.c_str(), _t("wb"));
}

bool unzip::extract_item(const gchar* path, const gchar* name, const gchar* password)
{
    assert(path && name);
    if(unzLocateFile(_zh, cvtzctlstr(name), 0) != UNZ_OK)
        return false;
    unz_file_info fileinfo;
    char filename[256];
    int err = unzGetCurrentFileInfo(_zh, &fileinfo, filename, sizeof(filename), 0, 0, 0, 0);
    if(err != UNZ_OK)
        return false;
    err = unzOpenCurrentFilePassword(_zh, cvtzctlstr(password));
    if(err != UNZ_OK)
        return false;
    file f;
    string localname;
    localname.from(filename);
    create_file_path(f, mkstr3(path, _t("/"), localname.c_str()));
    byte* buf = new byte[8192];
    int rdsz = 0;
    for(;;) {
        rdsz = unzReadCurrentFile(_zh, buf, 8192);
        if(rdsz <= 0)
            break;
        f.put(buf, rdsz);
    }
    delete [] buf;
    err = unzCloseCurrentFile(_zh);
    return (err == UNZ_OK);
}

bool unzip::extract(vdir& vd)
{
    unz_global_info gi;
    int err = unzGetGlobalInfo(_zh, &gi);
    if(err != UNZ_OK)
        return false;
    vd.create_folder(vd.get_root(), _t("root"));    /* create root */
    for(uint i = 0; i < gi.number_entry; i ++) {
        unz_file_info fileinfo;
        char filename[256];
        unzGetCurrentFileInfo(_zh, &fileinfo, filename, sizeof(filename), 0, 0, 0, 0);
        if(!fileinfo.uncompressed_size)
            assert(is_back_with_div(filename));
        else {
            string localname;
            localname.from(filename);
            if(!extract_item(vd, localname.c_str(), 0))
                return false;
        }
        unzGoToNextFile(_zh);
    }
    return true;
}

static vdirfile* create_virtual_file(vdir& vd, const gchar* path)
{
    assert(path);
    string p, n;
    const gchar* divpos = get_last_div(path);
    if(!divpos)
        n = path;
    else {
        assert(divpos[0] == _t('/') || divpos[0] == _t('\\'));
        p.assign(path, divpos - path);
        n.assign(++ divpos);
    }
    vdirop vdop(vd);
    if(!p.empty()) {
        vdop.make_dir(p.c_str());
        vdop.change_dir(p.c_str());
    }
    vdiriter i = vdop.create_file(n.c_str());
    if(i.is_valid())
        return static_cast<vdirfile*>(i.get_ptr());
    return 0;
}

bool unzip::extract_item(vdir& vd, const gchar* name, const gchar* password)
{
    assert(name);
    if(unzLocateFile(_zh, cvtzctlstr(name), 0) != UNZ_OK)
        return false;
    unz_file_info fileinfo;
    char filename[256];
    int err = unzGetCurrentFileInfo(_zh, &fileinfo, filename, sizeof(filename), 0, 0, 0, 0);
    if(err != UNZ_OK)
        return false;
    err = unzOpenCurrentFilePassword(_zh, cvtzctlstr(password));
    if(err != UNZ_OK)
        return false;
    string localname, localpath(_t("root/"));
    localname.from(filename);
    localpath += localname;
    vdirfile* vf = create_virtual_file(vd, localpath.c_str());
    assert(vf);
    vf->vsl.expand((int)fileinfo.uncompressed_size);
    byte* buf = new byte[8192];
    int rdsz = 0;
    for(;;) {
        rdsz = unzReadCurrentFile(_zh, buf, 8192);
        if(rdsz <= 0)
            break;
        vf->set_file_data(buf, rdsz);
    }
    delete [] buf;
    err = unzCloseCurrentFile(_zh);
    return (err == UNZ_OK);
}

__gslib_end__
