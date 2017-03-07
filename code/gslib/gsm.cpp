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

#include <gslib/gsm.h>
#include <gslib/treeop.h>
#include <gslib/error.h>
#include <gslib/dir.h>
#include <gslib/file.h>
#include <windows.h>
#include <winreg.h>

__gslib_begin__

void gsm_help(string& str)
{
    str.assign(_t("Make tool introductions.\n")
          _t("A typical make command line was like:\n")
          _t("  gsmake -debug -compiler \"vs_12\" -src \"../source\"\n")
          _t("These arguments could be disordered.\n")
          _t("The command line input arguments were:\n")
          _t("  -debug                  which means debug\n")
          _t("  -release                which means release\n")
          _t("  -compiler \"xx\"          which means compiler was \"xx\",\n")
          _t("                          compiler options currently supported:\n")
          _t("                          \"vs_12\"     Visual Studio 2013\n")
          _t("                          \"vs_14\"     Visual Studio 2015\n")
          _t("  -src \"xxx\"              which means source directory was \"xxx\",\n")
          _t("                          default by current directory.\n")
          _t("  -tar \"xxx\"              which means target directory was \"xxx\",\n")
          _t("                          default by current directory.\n")
          _t("The command line would be interpreted to a set of macro definitions,\n")
          _t("along with the platforms and the environment settings.\n")
          _t("These predefined macros were:\n")
          _t("  $compiler\n")
          _t("  $src_dir                source directory\n")
          _t("  $tar_dir                target direcotry\n")
          _t("  $tar_name               target name\n")
          _t("  $debug                  true or false\n")
          _t("  $os                     \"windows\"\n")
          _t("  $os_ver                 \"10.0.10586\" or sth. alike\n")
        );
}

void gsm_syntax_help(string& str)
{
}

static string _keyword_includes(_cststr(_t("includes")));
static string _keyword_make_type(_cststr(_t("make_type")));
static string _keyword_project(_cststr(_t("project")));
static string _keyword_if(_cststr(_t("if")));
static string _keyword_proj_name(_cststr(_t("project_name")));
static string _keyword_type(_cststr(_t("type")));
static string _keyword_output_name(_cststr(_t("output_name")));
static string _keyword_add_defs(_cststr(_t("add_definitions")));
static string _keyword_add_libs(_cststr(_t("add_libraries")));
static string _keyword_proj_dir(_cststr(_t("project_directory")));
static string _keyword_int_dir(_cststr(_t("intermediate_directory")));
static string _keyword_output_dir(_cststr(_t("output_directory")));
static string _keyword_inc_dirs(_cststr(_t("add_include_directories")));
static string _keyword_lib_dirs(_cststr(_t("add_library_directories")));
static string _keyword_srcs(_cststr(_t("add_sources")));
static string _keyword_ext_sheet(_cststr(_t("ext_sheet")));
static string _keyword_condition(_cststr(_t("condition")));
static string _keyword_do(_cststr(_t("do")));
static string _enum_type_executable(_cststr(_t("executable")));
static string _enum_type_static_library(_cststr(_t("static_library")));
static string _enum_type_dynamic_library(_cststr(_t("dynamic_library")));

static const gchar* get_postfix(const gchar* src, int len)
{
    assert(src);
    if(len == 0)
        return 0;
    for(int i = len - 1; i >= 0; i --) {
        if(src[i] == _t('.'))
            return src + i;
        else if(src[i] == _t('\\') || src[i] == _t('/'))
            return 0;
    }
    return 0;
}

static const gchar* get_postfix(const string& str)
{
    return str.empty() ? 0 : get_postfix(str.c_str(), str.length());
}

static bool is_absolute_path(const string& path)
{
    return (path.size() >= 2) && (path.at(1) == _t(':'));
}

#ifdef _WIN32
static bool get_nt_version_number(string& str)
{
    HMODULE hmod = LoadLibrary(_t("ntdll.dll"));
    if(hmod == NULL)
        return false;
    typedef void (WINAPI *pfRTLGETNTVERSIONNUMBERS)(DWORD*,DWORD*, DWORD*);
    auto func = (pfRTLGETNTVERSIONNUMBERS)GetProcAddress(hmod, "RtlGetNtVersionNumbers");
    if(func) {
        DWORD major_ver, minor_ver, build_number;
        func(&major_ver, &minor_ver, &build_number);
        build_number &= 0x0fffffff;     /* damn why?... */
        str.format(_t("%u.%u.%u"), major_ver, minor_ver, build_number);
        FreeLibrary(hmod);
        return true;
    }
    FreeLibrary(hmod);
    return false;
}

static bool is_visual_studio_available(const gchar* vn)
{
    assert(vn);
    string qkey(_t("VisualStudio.DTE."));
    qkey.append(vn);
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    HKEY key;
    DWORD dispos;
    LSTATUS r = RegCreateKeyEx(HKEY_CLASSES_ROOT, qkey.c_str(), 0, 0,
        REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE,
        &sa, &key, &dispos
        );
    return (r == NO_ERROR);
}

static bool get_latest_visual_studio_version(string& str)
{
    string qkey(_t("VisualStudio.DTE\\CurVer"));
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    HKEY key;
    DWORD dispos;
    LSTATUS r = RegCreateKeyEx(HKEY_CLASSES_ROOT, qkey.c_str(), 0, 0,
        REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE,
        &sa, &key, &dispos
        );
    if(r != NO_ERROR)
        return false;
    gchar buf[128];
    DWORD size;
    DWORD type = REG_SZ;
    r = RegQueryValueEx(key, 0, 0, &type, (BYTE*)buf, &size);
    if(r != NO_ERROR)
        return false;
    string ver_str(buf);
    /* detect the version info here */
    if(ver_str == _t("VisualStudio.DTE.14.0")) {
        str.assign(_t("vs_14"));
        return true;
    }
    else if(ver_str == _t("VisualStudio.DTE.12.0")) {
        str.assign(_t("vs_12"));
        return true;
    }
    return false;
}
#endif

static bool is_compiler_available(const string& str)
{
#ifdef _WIN32
    if(str == _t("vs_12"))
        return is_visual_studio_available(_t("12.0"));
    else if(str == _t("vs_14"))
        return is_visual_studio_available(_t("14.0"));
#endif
    return false;
}

static bool get_latest_compiler(string& str)
{
#ifdef _WIN32
    return get_latest_visual_studio_version(str);
#else
    return false;
#endif
}

static void gsm_make_path(string& str, const string& src, const string& subpath, const gsm_globals& globals)
{
    if(is_absolute_path(src)) {
        str = src;
        return;
    }
    str = globals.source_dir;
    if(!str.empty() && ((str.back() != _t('/')) && (str.back() != _t('\\'))))
        str.push_back(_t('/'));
    str.append(subpath);
    if(!str.empty() && ((str.back() != _t('/')) && (str.back() != _t('\\'))))
        str.push_back(_t('/'));
    str.append(src);
}

bool gsm_setup_globals(gsm_globals& globals, const gsm_config& cfgs)
{
    globals.is_debug = cfgs.is_debug;
#ifdef _WIN32
    globals.os_name.assign(_t("windows"));
    globals.os_version.clear();
    get_nt_version_number(globals.os_version);
    if(!cfgs.spec_compiler.empty() && is_compiler_available(cfgs.spec_compiler))
        globals.compiler = cfgs.spec_compiler;
    else {
        string compiler;
        if(!get_latest_compiler(compiler)) {
            set_error(_t("no available compiler found."));
            return false;
        }
        globals.compiler = compiler;
    }
#endif
    globals.source_dir = cfgs.source_dir;
    globals.target_dir = cfgs.target_dir;
    /* get current directory. */
    gchar currdir[260];
#ifdef _UNICODE
    _wgetcwd(currdir, _countof(currdir));
#else
    _getcwd(currdir, _countof(currdir));
#endif
    if(globals.source_dir.empty())
        globals.source_dir.assign(currdir);
    if(globals.target_dir.empty())
        globals.target_dir.assign(currdir);
    if(!globals.source_dir.empty() &&
        ((globals.source_dir.back() == _t('/')) || (globals.source_dir.back() == _t('\\')))
        )
        globals.source_dir.pop_back();
    if(!globals.target_dir.empty() &&
        ((globals.target_dir.back() == _t('/')) || (globals.target_dir.back() == _t('\\')))
        )
        globals.target_dir.pop_back();
    /* set target name */
    const string& srcdir = globals.source_dir;
    int f = -1;
    for(int i = srcdir.length() - 1; i >= 0; i --) {
        auto& ch = srcdir.at(i);
        if(ch == _t('/') || ch == _t('\\')) {
            f = i;
            break;
        }
    }
    if(f == -1) {
        set_error(_t("unexpected target path."));
        return false;
    }
    globals.target_name.assign(&srcdir.at(++f));
    return true;
}

bool gsm_proceed_project_statement(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals, const string& subpath, json_node* node);
bool gsm_proceed_if_statement(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals, const string& subpath, json_node* node);
bool gsm_proceed_make_statement(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals, const string& subpath, json_node* node);
bool gsm_prepare_project(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals, const string& subpath, file& fm);

static bool gsm_proceed_includes_statement(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals, const string& subpath, json_node* node)
{
    assert(node);
    if(node->get_tag() != jst_array) {
        set_error(_t("type of includes must be array."));
        return false;
    }
    auto* p = static_cast<json_node_array*>(node);
    int c = p->get_childs();
    for(int i = 0; i < c; i ++) {
        auto* ch = p->at(i);
        assert(ch);
        if(ch->get_tag() != jst_value) {
            set_error(_t("each type of includes should be a value."));
            return false;
        }
        auto& str = ch->get_name();
        string nextsubpath, cvpath;
        auto* path = &str;
        if(!is_absolute_path(str)) {
            nextsubpath = subpath;
            assert(cvpath.empty() || ((nextsubpath.back() != _t('/')) && (nextsubpath.back() != _t('\\'))));
            nextsubpath.push_back(_t('/'));
            nextsubpath.append(str);
            cvpath = globals.source_dir;
            if(!cvpath.empty() && ((cvpath.back() != _t('/')) && (cvpath.back() != _t('\\'))))
                cvpath.push_back(_t('/'));
            cvpath.append(nextsubpath);
            path = &cvpath;
        }
#ifdef _UNICODE
        file fm(path->c_str(), _t("r,ccs=UNICODE"));
#else
        file fm(path->c_str(), _t("r"));
#endif
        if(!fm.is_valid()) {
            set_error(_t("file not existed."));
            return false;
        }
        if(!gsm_prepare_project(proj_list, vd, globals, nextsubpath, fm))
            return false;
    }
    return true;
}

static const string& gsm_expect_postfix_vs(gsm_project_type type)
{
    static const string pf_exe(_cststr(_t(".exe")));
    static const string pf_lib(_cststr(_t(".lib")));
    static const string pf_dll(_cststr(_t(".dll")));
    static const string pf_unknown;
    switch(type)
    {
    case gpt_executable:
        return pf_exe;
    case gpt_static_library:
        return pf_lib;
    case gpt_dynamic_library:
        return pf_dll;
    default:
        set_error(_t("unknown project type."));
        return pf_unknown;
    }
}

static bool gsm_setup_project_type_vs(gsm_project& proj, json_node_table* proj_stat)
{
    assert(proj_stat);
    auto* proj_type = proj_stat->find(_keyword_type);
    if(proj_type) {
        if(proj_type->get_tag() != jst_pair) {
            set_error(_t("project statement error, \"%s\" needed."), _keyword_type.c_str());
            return false;
        }
        auto& proj_type_str = static_cast<json_node_pair*>(proj_type)->get_value_string();
        if(proj_type_str == _enum_type_executable)
            proj.project_type = gpt_executable;
        else if(proj_type_str == _enum_type_static_library)
            proj.project_type = gpt_static_library;
        else if(proj_type_str == _enum_type_dynamic_library)
            proj.project_type = gpt_dynamic_library;
        else {
            set_error(_t("project statement error, unexpected project type."));
            return false;
        }
    }
    /* this will set output_name along with it. */
    auto* output_name = proj_stat->find(_keyword_output_name);
    if(output_name) {
        if(output_name->get_tag() != jst_pair) {
            set_error(_t("project statement error, unexpected project type."));
            return false;
        }
        auto& name = static_cast<json_node_pair*>(output_name)->get_value_string();
        proj.output_name = name;
        const gchar* postfix = get_postfix(proj.output_name);
        if(!postfix) {
            proj.output_name.append(gsm_expect_postfix_vs(proj.project_type));
            return true;
        }
        else {
            auto& expected_postfix = gsm_expect_postfix_vs(proj.project_type);
            if(expected_postfix != postfix) {
                set_error(_t("unexpected postfix."));
                return false;
            }
            return true;
        }
    }
    else {
        if(proj.output_name.empty()) {
            /* default: project_name + postfix */
            proj.output_name = proj.name;
            proj.output_name.append(gsm_expect_postfix_vs(proj.project_type));
        }
        return true;
    }
}

static bool gsm_proceed_predefined_micros(gsm_project& proj, json_node_array* node)
{
    assert(node);
    int c = node->get_childs();
    if(!c)
        return true;
    for(int i = 0; i < c; i ++) {
        auto* ch = node->at(i);
        assert(ch);
        if(ch->get_tag() != jst_value) {
            set_error(_t("proceed predefined micros failed."));
            return false;
        }
        proj.definitions.push_back(static_cast<json_node_value*>(ch)->get_name());
    }
    return true;
}

static bool gsm_proceed_add_libraries(gsm_project& proj, json_node_array* node)
{
    assert(node);
    int c = node->get_childs();
    if(!c)
        return true;
    for(int i = 0; i < c; i ++) {
        auto* ch = node->at(i);
        assert(ch);
        if(ch->get_tag() != jst_value) {
            set_error(_t("proceed add libraries failed."));
            return false;
        }
        proj.libraries.push_back(static_cast<json_node_value*>(ch)->get_name());
    }
    return true;
}

static bool gsm_proceed_project_directory(gsm_project& proj, const string& subpath, json_node* node)
{
    auto set_default_proj_dir = [&proj, &subpath]() {
        if(!proj.project_dir.empty())   /* DONOT overwrite */
            return;
        proj.project_dir = subpath;
    };
    if(!node) {
        set_default_proj_dir();
        return true;
    }
    if(node->get_tag() != jst_pair) {
        set_error(_t("proceed project directory error."));
        return false;
    }
    auto& projdir = static_cast<json_node_pair*>(node)->get_value_string();
    if(projdir.empty()) {
        set_default_proj_dir();
        return true;
    }
    proj.project_dir = projdir;
    return true;
}

static bool gsm_proceed_paths(gsm_project& proj, gsm_str_list& ls, json_node_array* node)
{
    assert(node);
    int c = node->get_childs();
    for(int i = 0; i < c; i ++) {
        auto* n = node->at(i);
        assert(n);
        if(n->get_tag() != jst_value) {
            set_error(_t("proceed paths failed."));
            return false;
        }
        ls.push_back(n->get_name());
    }
    return true;
}

static bool gsm_merge_ext_sheet(gsm_project& proj, json_node_table* node)
{
    assert(node);
    auto& cont = node->get_container();
    auto& dest = proj.ext_sheet.get_container();
    for(auto* ch : cont) {
        assert(ch);
        if(dest.find(ch) == dest.end())
            dest.insert(ch->duplicate());
    }
    return true;
}

static bool gsm_proceed_project_statement(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals, const string& subpath, json_node* node)
{
    assert(node);
    if(node->get_tag() != jst_table) {
        set_error(_t("project statement must be a table."));
        return false;
    }
    auto* p = static_cast<json_node_table*>(node);
    /* find project name. */
    auto* proj_name = p->find(_keyword_proj_name);
    if(!proj_name || (proj_name->get_tag() != jst_pair)) {
        set_error(_t("project statement error, \"%s\" needed."), _keyword_proj_name.c_str());
        return false;
    }
    gsm_project* pproj = 0;
    auto& projname_str = static_cast<json_node_pair*>(proj_name)->get_value_string();
    /* find if project existed */
    for(auto& pp : proj_list) {
        if(pp.name == projname_str) {
            pproj = &pp;
            break;
        }
    }
    /* otherwise create a project */
    if(!pproj) {
        proj_list.push_back(gsm_project());
        pproj = &proj_list.back();
        pproj->name = projname_str;
        pproj->project_id.generate(uuid_v4);    /* generate project id */
    }
    assert(pproj);
    auto& proj = *pproj;
    /* find project type, which is necessary */
    if(globals.compiler == _t("vs_12") ||
        globals.compiler == _t("vs_14")
        ) {
        if(!gsm_setup_project_type_vs(proj, p))     /* along with output_name */
            return false;
    }
    /* predefined micro definitions */
    auto* pm_definition = p->find(_keyword_add_defs);
    if(pm_definition) {
        if(pm_definition->get_tag() != jst_array) {
            set_error(_t("project statement error, \"%s\" incorrect format."), _keyword_add_defs.c_str());
            return false;
        }
        if(!gsm_proceed_predefined_micros(proj, static_cast<json_node_array*>(pm_definition)))
            return false;
    }
    /* add libraries */
    auto* add_libs = p->find(_keyword_add_libs);
    if(add_libs) {
        if(add_libs->get_tag() != jst_array) {
            set_error(_t("project statement error, \"%s\" incorrect format."), _keyword_add_libs.c_str());
            return false;
        }
        if(!gsm_proceed_add_libraries(proj, static_cast<json_node_array*>(add_libs)))
            return false;
    }
    /* project directory */
    auto* proj_dir = p->find(_keyword_proj_dir);
    if(!gsm_proceed_project_directory(proj, subpath, proj_dir))
        return false;
    /* create tag according to project directory & name. */
    vdirop dop(vd);
    dop.make_dir(globals.target_name.c_str());
    dop.change_dir(globals.target_name.c_str());
    dop.make_dir(proj.project_dir.c_str());
    dop.change_dir(proj.project_dir.c_str());
    auto tag = vd.create_tag(dop.get_current_iter(), proj.name.c_str());
    static_cast<vdirtag&>(*tag).set_binding(&proj);
    /* intermediate directory */
    auto* int_dir = p->find(_keyword_int_dir);
    if(int_dir) {
        if(int_dir->get_tag() != jst_pair) {
            set_error(_t("project statement error, \"%s\" incorrect format."), _keyword_int_dir.c_str());
            return false;
        }
        proj.intermediate_dir = static_cast<json_node_pair*>(int_dir)->get_value_string();
    }
    if(proj.intermediate_dir.empty())
        proj.intermediate_dir.assign(_cststr(_t("obj")));
    assert(!proj.intermediate_dir.empty());
    if((proj.intermediate_dir.back() != _t('/')) && (proj.intermediate_dir.back() != _t('\\')))
        proj.intermediate_dir.push_back(_t('\\'));
    /* output directory */
    auto* output_dir = p->find(_keyword_output_dir);
    if(output_dir) {
        if(output_dir->get_tag() != jst_pair) {
            set_error(_t("project statement error, \"%s\" incorrect format."), _keyword_output_dir.c_str());
            return false;
        }
        proj.output_dir = static_cast<json_node_pair*>(output_dir)->get_value_string();
    }
    if(proj.output_dir.empty())
        proj.output_dir.assign(_cststr(_t("bin")));
    assert(!proj.output_dir.empty());
    if((proj.output_dir.back() != _t('/')) && (proj.output_dir.back() != _t('\\')))
        proj.output_dir.push_back(_t('\\'));
    /* add include directories */
    auto* add_inc_dir = p->find(_keyword_inc_dirs);
    if(add_inc_dir) {
        if(add_inc_dir->get_tag() != jst_array) {
            set_error(_t("project statement error, \"%s\" incorrect format."), _keyword_inc_dirs.c_str());
            return false;
        }
        if(!gsm_proceed_paths(proj, proj.include_dirs, static_cast<json_node_array*>(add_inc_dir)))
            return false;
    }
    /* add library directories */
    auto* add_lib_dir = p->find(_keyword_lib_dirs);
    if(add_lib_dir) {
        if(add_lib_dir->get_tag() != jst_array) {
            set_error(_t("project statement error, \"%s\" incorrect format."), _keyword_lib_dirs.c_str());
            return false;
        }
        if(!gsm_proceed_paths(proj, proj.library_dirs, static_cast<json_node_array*>(add_lib_dir)))
            return false;
    }
    /* add sources */
    auto* add_src = p->find(_keyword_srcs);
    if(add_src) {
        if(add_src->get_tag() != jst_array) {
            set_error(_t("project statement error, \"%s\" incorrect format."), _keyword_srcs.c_str());
            return false;
        }
        if(!gsm_proceed_paths(proj, proj.sources, static_cast<json_node_array*>(add_src)))
            return false;
    }
    /* ext_sheet */
    auto* ext_sheet = p->find(_keyword_ext_sheet);
    if(ext_sheet) {
        if(ext_sheet->get_tag() != jst_table) {
            set_error(_t("project statement error, \"%s\" should be table."), _keyword_ext_sheet.c_str());
            return false;
        }
        if(!gsm_merge_ext_sheet(proj, static_cast<json_node_table*>(ext_sheet)))
            return false;
    }
    return true;
}

static bool gsm_proceed_if_statement(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals, const string& subpath, json_node* node)
{
    assert(node);
    return true;
}

static bool gsm_proceed_make_statement(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals, const string& subpath, json_node* node)
{
    assert(node);
    if(node->get_tag() != jst_table) {
        set_error(_t("bad make statement: wrong type."));
        return false;
    }
    auto* p = static_cast<json_node_table*>(node);
    auto* f = p->find(_keyword_make_type);
    if(!f || (f->get_tag() != jst_pair)) {
        set_error(_t("bad make statement: unknown make type."));
        return false;
    }
    auto& val = static_cast<json_node_pair*>(f)->get_value_string();
    if(val == _keyword_includes)
        return gsm_proceed_includes_statement(proj_list, vd, globals, subpath, node);
    else if(val == _keyword_project)
        return gsm_proceed_project_statement(proj_list, vd, globals, subpath, node);
    else if(val == _keyword_if)
        return gsm_proceed_if_statement(proj_list, vd, globals, subpath, node);
    else {
        set_error(_t("bad make statement: unknown make type."));
        return false;
    }
}

static bool gsm_prepare_project(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals, const string& subpath, file& fm)
{
    string buf;
    fm.read_all(buf);
    json_parser jp;
    if(!jp.parse(buf.c_str(), buf.length()))
        return false;
    json_node* root = jp.get_root();
    if(!root)
        return true;
    auto t = root->get_tag();
    if(t == jst_array) {
        auto* p = static_cast<json_node_array*>(root);
        int c = p->get_childs();
        for(int i = 0; i < c; i ++) {
            if(!gsm_proceed_make_statement(proj_list, vd, globals, subpath, p->at(i))) {
                set_error(_t("proceed make statement failed."));
                return false;
            }
        }
        return true;
    }
    else if(t == jst_table) {
        auto* p = static_cast<json_node_table*>(root);
        auto* type = p->find(_keyword_make_type);
        if(!type || (type->get_tag() != jst_pair)) {
            set_error(_t("unknown make type."));
            return false;
        }
        auto* pmt = static_cast<json_node_pair*>(type);
        if(pmt->get_value_string() == _keyword_includes)
            return gsm_proceed_includes_statement(proj_list, vd, globals, subpath, p);
        else if(pmt->get_value_string() == _keyword_project)
            return gsm_proceed_project_statement(proj_list, vd, globals, subpath, p);
        else if(pmt->get_value_string() == _keyword_if)
            return gsm_proceed_if_statement(proj_list, vd, globals, subpath, p);
        return true;
    }
    else {
        set_error(_t("unexpected make file."));
        return false;
    }
}

static bool gsm_prepare_projects(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals, const string& subpath)
{
    string path = globals.source_dir;
    assert(!path.empty());
    if((path.back() != _t('/')) && (path.back() != _t('\\')))
        path.push_back(_t('/'));
    path.append(subpath);
    if((path.back() != _t('/')) && (path.back() != _t('\\')))
        path.push_back(_t('/'));
    /* until now: [config path]/[sub path]/ */
    /* test if there was a "make.txt" file */
    string makefilepath = path;
    makefilepath.append(_cststr(_t("make.txt")));
#ifdef _UNICODE
    file fm(makefilepath.c_str(), _t("r,ccs=UNICODE"));
#else
    file fm(makefilepath.c_str(), _t("r"));
#endif
    if(fm.is_valid()) {
        if(!gsm_prepare_project(proj_list, vd, globals, subpath, fm))
            return false;
    }
    /* [config path]/[sub path]/ + * */
    path.push_back(_t('*'));
    /* recursively */
    _gs_finddata_t fd;
    auto fr = _gs_findfirst(path.c_str(), &fd);
    if(fr == -1)
        return true;
    do {
        if(fd.attrib & _A_SUBDIR) {
            if((strtool::compare(fd.name, _t(".")) == 0) ||
                (strtool::compare(fd.name, _t("..")) == 0)
                )
                continue;
            string nextsubpath = subpath;
            if(!nextsubpath.empty() && (nextsubpath.back() != _t('/')) && (nextsubpath.back() != _t('\\')))
                nextsubpath.push_back(_t('/'));
            nextsubpath.append(fd.name);
            if(!gsm_prepare_projects(proj_list, vd, globals, nextsubpath)) {
                _gs_findclose(fr);
                return false;
            }
        }
    }
    while(!_gs_findnext(fr, &fd));
    _gs_findclose(fr);
    return true;
}

bool gsm_prepare_projects(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals)
{
    if(globals.source_dir.empty()) {
        set_error(_t("bad source directory."));
        return false;
    }
    return gsm_prepare_projects(proj_list, vd, globals, string(_t("")));
}

static void gsm_convert_uuid(string& str, const uuid& uid)
{
    string struid;
    uid.to_string(struid);
    str.push_back(_t('{'));
    str.append(struid);
    str.push_back(_t('}'));
}

static void gsm_convert_project_type_vs(string& str, gsm_project_type type)
{
    switch(type)
    {
    case gpt_executable:
        str.assign(_cststr(_t("Application")));
        break;
    case gpt_static_library:
        str.assign(_cststr(_t("StaticLibrary")));
        break;
    case gpt_dynamic_library:
        str.assign(_cststr(_t("DynamicLibrary")));
        break;
    default:
        assert(!"unknown project type.");
    }
}

static void gsm_inject_ext_sheet_vs(xmltree& proj, const gsm_project& projdesc, const gsm_globals& globals)
{
    auto r = proj.get_root();
    if(!r.is_valid())
        return;
    static const string vs_charset(_cststr(_t("vs_CharacterSet"))),
        vs_platform(_cststr(_t("vs_Platform"))),
        vs_lib_path(_cststr(_t("vs_LibraryPath"))),
        vs_inc_path(_cststr(_t("vs_IncludePath"))),
        vs_exe_path(_cststr(_t("vs_ExecutablePath"))),
        vs_pre_build_event(_cststr(_t("vs_PreBuildEvent"))),
        vs_pre_link_event(_cststr(_t("vs_PreLinkEvent"))),
        vs_post_build_event(_cststr(_t("vs_PostBuildEvent")));
    /* vs_CharacterSet */
    auto fcharset = proj.unique_path_locater(_t("$PropertyGroup@2/%CharacterSetTag%"));
    if(fcharset.is_valid()) {
        fcharset->set_name(_cststr(_t("CharacterSet")));
        auto vcharset = proj.birth<xml_value>(fcharset);
        assert(vcharset);
        auto* fjs = projdesc.ext_sheet.find(vs_charset);
        if(fjs && (fjs->get_tag() == jst_pair))
            vcharset->set_name(static_cast<json_node_pair*>(fjs)->get_value_string());
        else
            vcharset->set_name(_cststr(_t("Unicode")));
    }
    /* vs_Platform */
    auto fplatform = proj.unique_path_locater(_t("ItemGroup/ProjectConfiguration/%PlatformTag%"));
    if(fplatform.is_valid()) {
        fplatform->set_name(_cststr(_t("Platform")));
        auto vplatform = proj.birth<xml_value>(fplatform);
        assert(vplatform);
        auto* fjs = projdesc.ext_sheet.find(vs_platform);
        if(fjs && (fjs->get_tag() == jst_pair))
            vplatform->set_name(static_cast<json_node_pair*>(fjs)->get_value_string());
        else
            vplatform->set_name(_cststr(_t("Win32")));
    }
    /* vs_LibraryPath, vs_IncludePath, vs_ExecutablePath */
    auto fgeneraldirs = proj.unique_path_locater(_t("%GeneralDirTag%"));
    if(fgeneraldirs.is_valid()) {
        auto* fjs1 = projdesc.ext_sheet.find(vs_inc_path);
        auto* fjs2 = projdesc.ext_sheet.find(vs_lib_path);
        auto* fjs3 = projdesc.ext_sheet.find(vs_exe_path);
        if((!fjs1 || (fjs1->get_tag() != jst_pair)) &&
            (!fjs2 || (fjs2->get_tag() != jst_pair)) &&
            (!fjs3 || (fjs3->get_tag() != jst_pair))
            )
            proj.erase(fgeneraldirs);
        else {
            fgeneraldirs->set_name(_cststr(_t("PropertyGroup")));
            if(globals.is_debug)
                fgeneraldirs->set_attribute(_cststr(_t("Condition")), _cststr(_t("\'$(Configuration)|$(Platform)\'==\'Debug|Win32\'")));
            else
                fgeneraldirs->set_attribute(_cststr(_t("Condition")), _cststr(_t("\'$(Configuration)|$(Platform)\'==\'Release|Win32\'")));
            if(fjs1 && ((fjs1->get_tag() == jst_pair))) {
                auto p1 = proj.birth<xml_element>(fgeneraldirs);
                auto p2 = proj.birth<xml_value>(p1);
                p1->set_name(_cststr(_t("IncludePath")));
                p2->set_name(static_cast<json_node_pair*>(fjs1)->get_value_string());
            }
            if(fjs2 && ((fjs2->get_tag() == jst_pair))) {
                auto p1 = proj.birth<xml_element>(fgeneraldirs);
                auto p2 = proj.birth<xml_value>(p1);
                p1->set_name(_cststr(_t("LibraryPath")));
                p2->set_name(static_cast<json_node_pair*>(fjs2)->get_value_string());
            }
            if(fjs3 && ((fjs3->get_tag() == jst_pair))) {
                auto p1 = proj.birth<xml_element>(fgeneraldirs);
                auto p2 = proj.birth<xml_value>(p1);
                p1->set_name(_cststr(_t("ExecutablePath")));
                p2->set_name(static_cast<json_node_pair*>(fjs3)->get_value_string());
            }
        }
    }
    /* vs_PreBuildEvent, vs_PreLinkEvent, vs_PostBuildEvent */
    auto fbuildevent = proj.unique_path_locater(_t("ItemDefinitionGroup/%BuildEventTag%"));
    if(fbuildevent.is_valid()) {
        auto* fprebe = projdesc.ext_sheet.find(vs_pre_build_event);
        auto* fprele = projdesc.ext_sheet.find(vs_pre_link_event);
        auto* fpostbe = projdesc.ext_sheet.find(vs_post_build_event);
        auto make_build_event = [&proj](xmltree::iterator p, json_node_table* fe) {
            assert(p && fe);
            static const string kw_cmd(_cststr(_t("cmd"))),
                kw_msg(_cststr(_t("msg")));
            auto* f1 = fe->find(kw_cmd);
            auto* f2 = fe->find(kw_msg);
            if(f1 && (f1->get_tag() == jst_pair)) {
                auto& cmd = static_cast<json_node_pair*>(f1)->get_value_string();
                auto ch = proj.birth<xml_element>(p);
                auto v = proj.birth<xml_value>(ch);
                ch->set_name(_cststr(_t("Command")));
                v->set_name(cmd);
            }
            if(f2 && (f2->get_tag() == jst_pair)) {
                auto& msg = static_cast<json_node_pair*>(f2)->get_value_string();
                auto ch = proj.birth<xml_element>(p);
                auto v = proj.birth<xml_value>(ch);
                ch->set_name(_cststr(_t("Message")));
                v->set_name(msg);
            }
        };
        if(fpostbe && (fpostbe->get_tag() == jst_table)) {
            auto p = proj.insert_after<xml_element>(fbuildevent);
            assert(p);
            p->set_name(_cststr(_t("PostBuildEvent")));
            make_build_event(p, static_cast<json_node_table*>(fpostbe));
        }
        if(fprele && (fprele->get_tag() == jst_table)) {
            auto p = proj.insert_after<xml_element>(fbuildevent);
            assert(p);
            p->set_name(_cststr(_t("PreLinkEvent")));
            make_build_event(p, static_cast<json_node_table*>(fprele));
        }
        if(fprebe && (fprebe->get_tag() == jst_table)) {
            auto p = proj.insert_after<xml_element>(fbuildevent);
            assert(p);
            p->set_name(_cststr(_t("PreBuildEvent")));
            make_build_event(p, static_cast<json_node_table*>(fprebe));
        }
        proj.erase(fbuildevent);
    }
}

static void gsm_inject_sources_vs(xmltree& proj, const gsm_project& projdesc, const gsm_globals& globals)
{
    auto r = proj.get_root();
    if(!r.is_valid())
        return;
    xmltree::iterator f;
    for(auto i = r.child(); i.is_valid(); i.to_next()) {
        if(i->get_name() == _t("%SourceTag%")) {
            f = i;
            break;
        }
    }
    if(!f.is_valid())
        return;
    auto is_comp_file = [](const gchar* pf)-> bool {
        assert(pf);
        static const string c_pf(_cststr(_t(".c"))),
            cxx_pf(_cststr(_t(".cxx"))),
            cpp_pf(_cststr(_t(".cpp"))),
            cc_pf(_cststr(_t(".cc")));
        return ((c_pf == pf) || (cxx_pf == pf) || (cpp_pf == pf) || (cc_pf == pf));
    };
    auto is_inc_file = [](const gchar* pf)->bool {
        assert(pf);
        static const string h_pf(_cststr(_t(".h"))),
            hpp_pf(_cststr(_t(".hpp")));
        return ((h_pf == pf) || (hpp_pf == pf));
    };
    gsm_str_list inc_list, comp_list, none_list;
    for(auto i = projdesc.sources.begin(); i != projdesc.sources.end(); ++ i) {
        auto& str = *i;
        auto* postfix = get_postfix(str);
        if(!postfix)
            none_list.push_back(str);
        else if(is_comp_file(postfix))
            comp_list.push_back(str);
        else if(is_inc_file(postfix))
            inc_list.push_back(str);
        else
            none_list.push_back(str);
    }
    if(!none_list.empty()) {
        auto p = proj.insert<xml_element>(f);
        p->set_name(_cststr(_t("ItemGroup")));
        for(auto& s : none_list) {
            auto g = proj.birth_tail<xml_element>(p);
            g->set_name(_cststr(_t("None")));
            string str;
            gsm_make_path(str, s, projdesc.project_dir, globals);
            g->set_attribute(_cststr(_t("Include")), str.c_str(), str.length());
        }
    }
    if(!inc_list.empty()) {
        auto p = proj.insert<xml_element>(f);
        p->set_name(_cststr(_t("ItemGroup")));
        for(auto& s : inc_list) {
            auto g = proj.birth_tail<xml_element>(p);
            g->set_name(_cststr(_t("ClInclude")));
            string str;
            gsm_make_path(str, s, projdesc.project_dir, globals);
            g->set_attribute(_cststr(_t("Include")), str.c_str(), str.length());
        }
    }
    if(!comp_list.empty()) {
        auto p = proj.insert<xml_element>(f);
        p->set_name(_cststr(_t("ItemGroup")));
        for(auto& s : comp_list) {
            auto g = proj.birth_tail<xml_element>(p);
            g->set_name(_cststr(_t("ClCompile")));
            string str;
            gsm_make_path(str, s, projdesc.project_dir, globals);
            g->set_attribute(_cststr(_t("Include")), str.c_str(), str.length());
        }
    }
    proj.erase(f);
}

static bool gsm_create_project_file_vs12(xmltree& proj, const gsm_project& projdesc, const gsm_globals& globals)
{
    def_tr$(proj1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("Project")));
        set_attribute(_cststr(_t("DefaultTargets")), _cststr(_t("Build")));
        set_attribute(_cststr(_t("ToolsVersion")), _cststr(_t("12.0")));
        set_attribute(_cststr(_t("xmlns")), _cststr(_t("http://schemas.microsoft.com/developer/msbuild/2003")));
    });
    def_tr$(itemgroup, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ItemGroup")));
        set_attribute(_cststr(_t("Label")), _cststr(_t("ProjectConfigurations")));
    });
    def_tr$(projcfg1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ProjectConfiguration")));
        set_attribute(_cststr(_t("Include")), _cststr(_t("Debug|Win32")));
    });
    def_tr$(cfg1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("Configuration")));
    });
    def_tr$(cfg1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("Debug")));
    });
    def_tr$(platformtag, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("%PlatformTag%")));
    });
    def_tr$(projcfg2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ProjectConfiguration")));
        set_attribute(_cststr(_t("Include")), _cststr(_t("Release|Win32")));
    });
    def_tr$(cfg2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("Configuration")));
    });
    def_tr$(cfg2val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("Release")));
    });
    def_tr$(propgroup1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("PropertyGroup")));
        set_attribute(_cststr(_t("Label")), _cststr(_t("Globals")));
    });
    def_tr$(projguid, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ProjectGuid")));
    });
    def_tr$_v1(projguidval, treeop<xmltree>, xml_value, projdesc, [&](const gsm_project& projcfgs) {
        string str;
        gsm_convert_uuid(str, projcfgs.project_id);
        set_name(str.c_str(), str.length());
    });
    def_tr$(import1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("Import")));
        set_attribute(_cststr(_t("Project")), _cststr(_t("$(VCTargetsPath)\\Microsoft.Cpp.Default.props")));
    });
    def_tr$(propgroup2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("PropertyGroup")));
        set_attribute(_cststr(_t("Condition")), _cststr(_t("\'$(Configuration)|$(Platform)\'==\'Debug|Win32\'")));
        set_attribute(_cststr(_t("Label")), _cststr(_t("Configuration")));
    });
    def_tr$(cfgtype1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ConfigurationType")));
    });
    def_tr$_v1(cfgtype1val, treeop<xmltree>, xml_value, projdesc, [&](const gsm_project& projcfgs) {
        string str;
        gsm_convert_project_type_vs(str, projcfgs.project_type);
        set_name(str.c_str(), str.length());
    });
    def_tr$(dbgopt1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("UseDebugLibraries")));
    });
    def_tr$(dbgopt1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("true")));
    });
    def_tr$(platformts1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("PlatformToolset")));
    });
    def_tr$(platformts1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("v120")));
    });
    def_tr$(charsettag, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("%CharacterSetTag%")));
    });
    def_tr$(propgroup3, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("PropertyGroup")));
        set_attribute(_cststr(_t("Condition")), _cststr(_t("\'$(Configuration)|$(Platform)\'==\'Release|Win32\'")));
        set_attribute(_cststr(_t("Label")), _cststr(_t("Configuration")));
    });
    def_tr$(cfgtype2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ConfigurationType")));
    });
    def_tr$_v1(cfgtype2val, treeop<xmltree>, xml_value, projdesc, [&](const gsm_project& projcfgs) {
        string str;
        gsm_convert_project_type_vs(str, projcfgs.project_type);
        set_name(str.c_str(), str.length());
    });
    def_tr$(dbgopt2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("UseDebugLibraries")));
    });
    def_tr$(dbgopt2val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("false")));
    });
    def_tr$(platformts2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("PlatformToolset")));
    });
    def_tr$(platformts2val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("v120")));
    });
    def_tr$(wpo1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("WholeProgramOptimization")));
    });
    def_tr$(wpo1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("true")));
    });
    def_tr$(import2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("Import")));
        set_attribute(_cststr(_t("Project")), _cststr(_t("$(VCTargetsPath)\\Microsoft.Cpp.props")));
    });
    def_tr$(impgroup1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ImportGroup")));
        set_attribute(_cststr(_t("Label")), _cststr(_t("PropertySheets")));
        set_attribute(_cststr(_t("Condition")), _cststr(_t("\'$(Configuration)|$(Platform)\'==\'Debug|Win32\'")));
    });
    def_tr$(import3, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("Import")));
        set_attribute(_cststr(_t("Project")), _cststr(_t("$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props")));
        set_attribute(_cststr(_t("Condition")), _cststr(_t("exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')")));
        set_attribute(_cststr(_t("Label")), _cststr(_t("LocalAppDataPlatform")));
    });
    def_tr$(impgroup2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ImportGroup")));
        set_attribute(_cststr(_t("Label")), _cststr(_t("PropertySheets")));
        set_attribute(_cststr(_t("Condition")), _cststr(_t("\'$(Configuration)|$(Platform)\'==\'Release|Win32\'")));
    });
    def_tr$(import4, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("Import")));
        set_attribute(_cststr(_t("Project")), _cststr(_t("$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props")));
        set_attribute(_cststr(_t("Condition")), _cststr(_t("exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')")));
        set_attribute(_cststr(_t("Label")), _cststr(_t("LocalAppDataPlatform")));
    });
    def_tr$(propgroup4, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("PropertyGroup")));
        set_attribute(_cststr(_t("Condition")), _cststr(_t("\'$(Configuration)|$(Platform)\'==\'Debug|Win32\'")));
    });
    def_tr$(generaldirtag, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("%GeneralDirTag%")));
    });
    def_tr$(outdir1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("OutDir")));
    });
    def_tr$_v1(outdir1val, treeop<xmltree>, xml_value, projdesc, [&](const gsm_project& projcfgs) {
        set_name(projcfgs.output_dir.c_str(), projcfgs.output_dir.length());
    });
    def_tr$(intdir1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("IntDir")));
    });
    def_tr$_v1(intdir1val, treeop<xmltree>, xml_value, projdesc, [&](const gsm_project& projcfgs) {
        set_name(projcfgs.intermediate_dir.c_str(), projcfgs.intermediate_dir.length());
    });
    def_tr$(propgroup5, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("PropertyGroup")));
        set_attribute(_cststr(_t("Condition")), _cststr(_t("\'$(Configuration)|$(Platform)\'==\'Release|Win32\'")));
    });
    def_tr$(itemdefgroup1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ItemDefinitionGroup")));
        set_attribute(_cststr(_t("Condition")), _cststr(_t("\'$(Configuration)|$(Platform)\'==\'Debug|Win32\'")));
    });
    def_tr$(clcompile1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ClCompile")));
    });
    def_tr$(warnlvl1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("WarningLevel")));
    });
    def_tr$(warnlvl1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("Level3")));
    });
    def_tr$(optimize1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("Optimization")));
    });
    def_tr$(optimize1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("Disabled")));
    });
    def_tr$(sdlcheck1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("SDLCheck")));
    });
    def_tr$(sdlcheck1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("true")));
    });
    def_tr$(addincdir1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("AdditionalIncludeDirectories")));
    });
    def_tr$_v2(addincdir1val, treeop<xmltree>, xml_value, projdesc, globals, [&](const gsm_project& projcfgs, const gsm_globals& globals) {
        string str;
        for(auto& s : projcfgs.include_dirs) {
            string path;
            gsm_make_path(path, s, projcfgs.project_dir, globals);
            str.append(path);
            str.push_back(_t(';'));
        }
        str.append(_cststr(_t("%(AdditionalIncludeDirectories)")));
        set_name(str.c_str(), str.length());
    });
    def_tr$(preprocdefs1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("PreprocessorDefinitions")));
    });
    def_tr$_v1(preprocdefs1val, treeop<xmltree>, xml_value, projdesc, [&](const gsm_project& projcfgs) {
        string str;
        for(auto& s : projcfgs.definitions) {
            str.append(s);
            str.push_back(_t(';'));
        }
        str.append(_cststr(_t("%(PreprocessorDefinitions)")));
        set_name(str.c_str(), str.length());
    });
    def_tr$(link1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("Link")));
    });
    def_tr$(gendbgnfo1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("GenerateDebugInformation")));
    });
    def_tr$(gendbgnfo1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("true")));
    });
    def_tr$(addlibdir1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("AdditionalLibraryDirectories")));
    });
    def_tr$_v2(addlibdir1val, treeop<xmltree>, xml_value, projdesc, globals, [&](const gsm_project& projcfgs, const gsm_globals& globals) {
        string str;
        for(auto& s : projcfgs.library_dirs) {
            string path;
            gsm_make_path(path, s, projcfgs.project_dir, globals);
            str.append(path);
            str.push_back(_t(';'));
        }
        str.append(_cststr(_t("%(AdditionalLibraryDirectories)")));
        set_name(str.c_str(), str.length());
    });
    def_tr$(addlib1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("AdditionalDependencies")));
    });
    def_tr$_v1(addlib1val, treeop<xmltree>, xml_value, projdesc, [&](const gsm_project& projcfgs) {
        string str;
        for(auto& s : projcfgs.libraries) {
            str.append(s);
            str.push_back(_t(';'));
        }
        str.append(_cststr(_t("%(AdditionalDependencies)")));
        set_name(str.c_str(), str.length());
    });
    def_tr$(buildevent1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("%BuildEventTag%")));
    });
    def_tr$(itemdefgroup2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ItemDefinitionGroup")));
        set_attribute(_cststr(_t("Condition")), _cststr(_t("\'$(Configuration)|$(Platform)\'==\'Release|Win32\'")));
    });
    def_tr$(clcompile2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ClCompile")));
    });
    def_tr$(warnlvl2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("WarningLevel")));
    });
    def_tr$(warnlvl2val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("Level3")));
    });
    def_tr$(optimize2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("Optimization")));
    });
    def_tr$(optimize2val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("MaxSpeed")));
    });
    def_tr$(funclvllink1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("FunctionLevelLinking")));
    });
    def_tr$(funclvllink1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("true")));
    });
    def_tr$(intrinfunc1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("IntrinsicFunctions")));
    });
    def_tr$(intrinfunc1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("true")));
    });
    def_tr$(sdlcheck2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("SDLCheck")));
    });
    def_tr$(sdlcheck2val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("true")));
    });
    def_tr$(gendbgnfo2, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("GenerateDebugInformation")));
    });
    def_tr$(gendbgnfo2val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("false")));
    });
    def_tr$(comdatfolding1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("EnableCOMDATFolding")));
    });
    def_tr$(comdatfolding1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("true")));
    });
    def_tr$(optmizeref1, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("OptimizeReferences")));
    });
    def_tr$(optmizeref1val, treeop<xmltree>, xml_value, [&]() {
        set_name(_cststr(_t("true")));
    });
    def_tr$(sourcetag, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("%SourceTag%")));
    });
    def_tr$(import5, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("Import")));
        set_attribute(_cststr(_t("Project")), _cststr(_t("$(VCTargetsPath)\\Microsoft.Cpp.targets")));
    });
    def_tr$(impgroup3, treeop<xmltree>, xml_element, [&]() {
        set_name(_cststr(_t("ImportGroup")));
        set_attribute(_cststr(_t("Label")), _cststr(_t("ExtensionTargets")));
    });
    treeop<xmltree> trop(proj);
    if(globals.is_debug) {
        trop.create(
            tr$(proj1).sub(
                tr$(itemgroup).sub(
                    tr$(projcfg1).sub(
                        tr$(cfg1).sub(tr$(cfg1val))
                        + tr$(platformtag)
                    )
                )
                + tr$(propgroup1).sub(
                    tr$(projguid).sub(tr$(projguidval))
                )
                + tr$(import1)
                + tr$(propgroup2).sub(
                    tr$(cfgtype1).sub(tr$(cfgtype1val))
                    + tr$(dbgopt1).sub(tr$(dbgopt1val))
                    + tr$(platformts1).sub(tr$(platformts1val))
                    + tr$(charsettag)
                )
                + tr$(import2)
                + tr$(impgroup1).sub(
                    tr$(import3)
                )
                + tr$(generaldirtag)
                + tr$(propgroup4).sub(
                    tr$(outdir1).sub(tr$(outdir1val))
                    + tr$(intdir1).sub(tr$(intdir1val))
                )
                + tr$(itemdefgroup1).sub(
                    tr$(clcompile1).sub(
                        tr$(warnlvl1).sub(tr$(warnlvl1val))
                        + tr$(optimize1).sub(tr$(optimize1val))
                        + tr$(sdlcheck1).sub(tr$(sdlcheck1val))
                        + tr$(addincdir1).sub(tr$(addincdir1val))
                        + tr$(preprocdefs1).sub(tr$(preprocdefs1val))
                    )
                    + tr$(link1).sub(
                        tr$(gendbgnfo1).sub(tr$(gendbgnfo1val))
                        + tr$(addlibdir1).sub(tr$(addlibdir1val))
                        + tr$(addlib1).sub(tr$(addlib1val))
                    )
                    + tr$(buildevent1)
                )
                + tr$(sourcetag)
                + tr$(import5)
                + tr$(impgroup3)
            )
        );
    }
    else {
        trop.create(
            tr$(proj1).sub(
                tr$(itemgroup).sub(
                    tr$(projcfg2).sub(
                        tr$(cfg2).sub(tr$(cfg2val))
                        + tr$(platformtag)
                    )
                )
                + tr$(propgroup1).sub(
                    tr$(projguid).sub(tr$(projguidval))
                )
                + tr$(import1)
                + tr$(propgroup3).sub(
                    tr$(cfgtype2).sub(tr$(cfgtype2val))
                    + tr$(dbgopt2).sub(tr$(dbgopt2val))
                    + tr$(platformts2).sub(tr$(platformts2val))
                    + tr$(wpo1).sub(tr$(wpo1val))
                    + tr$(charsettag)
                )
                + tr$(import2)
                + tr$(impgroup2).sub(
                    tr$(import4)
                )
                + tr$(generaldirtag)
                + tr$(propgroup5).sub(
                    tr$(outdir1).sub(tr$(outdir1val))
                    + tr$(intdir1).sub(tr$(intdir1val))
                )
                + tr$(itemdefgroup2).sub(
                    tr$(clcompile2).sub(
                        tr$(warnlvl2).sub(tr$(warnlvl2val))
                        + tr$(optimize2).sub(tr$(optimize2val))
                        + tr$(funclvllink1).sub(tr$(funclvllink1val))
                        + tr$(intrinfunc1).sub(tr$(intrinfunc1val))
                        + tr$(sdlcheck2).sub(tr$(sdlcheck2val))
                        + tr$(addincdir1).sub(tr$(addincdir1val))
                        + tr$(preprocdefs1).sub(tr$(preprocdefs1val))
                    )
                    + tr$(link1).sub(
                        tr$(gendbgnfo2).sub(tr$(gendbgnfo2val))
                        + tr$(comdatfolding1).sub(tr$(comdatfolding1val))
                        + tr$(optmizeref1).sub(tr$(optmizeref1val))
                        + tr$(addlibdir1).sub(tr$(addlibdir1val))
                        + tr$(addlib1).sub(tr$(addlib1val))
                    )
                    + tr$(buildevent1)
                )
                + tr$(sourcetag)
                + tr$(import5)
                + tr$(impgroup3)
            )
        );
    }
    gsm_inject_ext_sheet_vs(proj, projdesc, globals);
    gsm_inject_sources_vs(proj, projdesc, globals);
#ifdef _UNICODE
    proj.set_encode(xmltree::encode_utf16);
#endif
    return true;
}

static bool gsm_create_project_file_vs14(xmltree& proj, const gsm_project& projdesc, const gsm_globals& globals)
{
    if(!gsm_create_project_file_vs12(proj, projdesc, globals))
        return false;
    auto f = proj.unique_path_locater(_t("$PropertyGroup@2/PlatformToolset/$@1"));
    if(!f)
        return false;
    f->set_name(_cststr(_t("v140")));
    return true;
}

static bool gsm_create_project_file(xmltree& proj, const gsm_project& projdesc, const gsm_globals& globals)
{
    if(globals.compiler == _t("vs_12"))
        return gsm_create_project_file_vs12(proj, projdesc, globals);
    else if(globals.compiler == _t("vs_14"))
        return gsm_create_project_file_vs14(proj, projdesc, globals);
    return false;
}

bool gsm_generate_projects(vdir& vd, const gsm_globals& globals)
{
    /* replace vdir tags with vdir files */
    bool sth_failed = false;
    vd.preorder_traversal([&](vdir::wrapper* w) {
        assert(w);
        if(sth_failed)
            return;
        vdirnode* node = w->get_ptr();
        assert(node);
        if(node->get_tag() != vdirnode::dt_tag)
            return;
        auto* tp = static_cast<vdirtag*>(node);
        auto* bd = reinterpret_cast<gsm_project*>(tp->get_binding());
        vdir::wrapper nw;
        nw.born<vdirfile>();
        nw.get_ref().set_name(node->get_name());
        auto* fd = static_cast<vdirfile*>(nw.get_ptr());
        xmltree proj;
        sth_failed = !gsm_create_project_file(proj, *bd, globals);
        if(!sth_failed) {
            string str;
            proj.output(str);
            fd->set_file_data(str.c_str(), str.length() * sizeof(gchar));
            /* assumed that it's visual studio here. */
            string name = fd->get_name();
            name.append(_cststr(_t(".vcxproj")));
            fd->set_name(name);
        }
        w->attach(&nw);
    });
    return !sth_failed;
}

static bool gsm_finalize_projects_vs12(vdir& vd, const gsm_proj_list& projects, const gsm_globals& globals)
{
    if(projects.empty())
        return true;
    /* generate solution uuid */
    uuid uid(uuid_v4);
    string uidstr;
    uid.to_string(uidstr);
    string str;
#ifdef _UNICODE
    str.push_back(0xfeff);  /* BOM */
#endif
    str.append(_cststr(_t("\nMicrosoft Visual Studio Solution File, Format Version 12.00\n")
        _t("# Visual Studio 2013\n")
        _t("VisualStudioVersion = 12.0.0.0\n")
        _t("MinimumVisualStudioVersion = 12.0.0.0\n")
        ));
    for(auto& proj : projects) {
        string s, suid;
        proj.project_id.to_string(suid);
        s.format(_t("Project(\"{%s}\") = \"%s\", \"%s\\%s.vcxproj\", \"{%s}\"\nEndProject\n"), uidstr.c_str(), proj.name.c_str(), proj.project_dir.c_str(), proj.name.c_str(), suid.c_str());
        str.append(s);
    }
    /* record */
    auto r = vd.get_root();
    assert(r.is_valid());
    auto record = vd.birth<vdirfile>(r);
    assert(record.is_valid());
    string solname;
    solname.format(_t("%s.sln"), globals.target_name.c_str());
    auto* p = static_cast<vdirfile*>(record.get_ptr());
    p->set_name(solname);
    p->set_file_data(str.c_str(), str.length() * sizeof(gchar));
    return true;
}

static bool gsm_finalize_projects_vs14(vdir& vd, const gsm_proj_list& projects, const gsm_globals& globals)
{
    if(projects.empty())
        return true;
    /* generate solution uuid */
    uuid uid(uuid_v4);
    string uidstr;
    uid.to_string(uidstr);
    string str;
#ifdef _UNICODE
    str.push_back(0xfeff);  /* BOM */
#endif
    str.append(_cststr(_t("\nMicrosoft Visual Studio Solution File, Format Version 14.00\n")
        _t("# Visual Studio 2015\n")
        _t("VisualStudioVersion = 14.0.0.0\n")
        _t("MinimumVisualStudioVersion = 14.0.0.0\n")
        ));
    for(auto& proj : projects) {
        string s, suid;
        proj.project_id.to_string(suid);
        s.format(_t("Project(\"{%s}\") = \"%s\", \"%s\\%s.vcxproj\", \"{%s}\"\nEndProject\n"), uidstr.c_str(), proj.name.c_str(), proj.project_dir.c_str(), proj.name.c_str(), suid.c_str());
        str.append(s);
    }
    /* record */
    auto r = vd.get_root();
    assert(r.is_valid());
    auto record = vd.birth<vdirfile>(r);
    assert(record.is_valid());
    string solname;
    solname.format(_t("%s.sln"), globals.target_name.c_str());
    auto* p = static_cast<vdirfile*>(record.get_ptr());
    p->set_name(solname);
    p->set_file_data(str.c_str(), str.length() * sizeof(gchar));
    return true;
}

bool gsm_finalize_projects(vdir& vd, const gsm_proj_list& projects, const gsm_globals& globals)
{
    if(globals.compiler == _t("vs_12"))
        return gsm_finalize_projects_vs12(vd, projects, globals);
    else if(globals.compiler == _t("vs_14"))
        return gsm_finalize_projects_vs14(vd, projects, globals);
    return false;
}

__gslib_end__
