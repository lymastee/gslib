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

#ifndef gsm_129e9ac0_6a88_49a3_8bf1_a95a8edf7da8_h
#define gsm_129e9ac0_6a88_49a3_8bf1_a95a8edf7da8_h

#include <gslib/xml.h>
#include <gslib/json.h>
#include <gslib/uuid.h>
#include <gslib/vdir.h>

/*
 * Make tool introductions.
 * see gsm_help();
 */

__gslib_begin__

enum gsm_project_type
{
    gpt_executable,
    gpt_static_library,
    gpt_dynamic_library,
};

struct gsm_project;
typedef list<string> gsm_str_list;
typedef list<gsm_project> gsm_proj_list;

struct gsm_config
{
    string              source_dir;
    string              target_dir;
    string              spec_compiler;
    bool                is_debug;
};

struct gsm_globals
{
    string              compiler;           /* $compiler */
    string              os_name;            /* $os */
    string              os_version;         /* $os_ver */
    string              source_dir;         /* $src_dir */
    string              target_dir;         /* $tar_dir */
    string              target_name;        /* $tar_name */
    bool                is_debug;           /* $debug */
};

struct gsm_project
{
    string              name;
    uuid                project_id;
    string              output_name;
    gsm_project_type    project_type;
    gsm_str_list        definitions;
    gsm_str_list        libraries;
    string              project_dir;
    string              intermediate_dir;
    string              output_dir;
    gsm_str_list        include_dirs;
    gsm_str_list        library_dirs;
    gsm_str_list        sources;
    json_node_table     ext_sheet;
};

extern void gsm_help(string& str);
extern void gsm_syntax_help(string& str);
extern bool gsm_setup_globals(gsm_globals& globals, const gsm_config& cfgs);
extern bool gsm_prepare_projects(gsm_proj_list& proj_list, vdir& vd, const gsm_globals& globals);
extern bool gsm_generate_projects(vdir& vd, const gsm_globals& globals);
extern bool gsm_finalize_projects(vdir& vd, const gsm_proj_list& projects, const gsm_globals& globals);

__gslib_end__

#endif
