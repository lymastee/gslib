#ifndef rpsr_f7f329f9_ae61_4fcf_b0c9_8a96216ed0e0_h
#define rpsr_f7f329f9_ae61_4fcf_b0c9_8a96216ed0e0_h

#include <rathen\config.h>

/*
 * This version of API wrappers were used by the flex & bison due to the cpp generator was still unstable.
 * Any kind of cpp style announcement should not appear in the header file.
 */

#ifndef __cplusplus
#ifdef _UNICODE
typedef wchar_t rchar;
#else
typedef char rchar;
#endif
#else
#include <gslib\string.h>
typedef gs::gchar rchar;
#endif

#ifndef __cplusplus
typedef void rpswrap;
typedef void rpsobj;
typedef void rpsparser;
#else
#include <rathen\parser.h>
typedef gs::rathen::parser::wrapper rpswrap;
typedef gs::rathen::ps_obj rpsobj;
typedef gs::rathen::parser rpsparser;
#endif

/* file: parser.h, line: 14 */
enum rpobj_tag
{
    rpt_root,
    rpt_block,
    rpt_value,
    rpt_expression,
    rpt_operator,
    rpt_function,
    rpt_calling,
    rpt_if_statement,
    rpt_loop_statement,
    rpt_go_statement,
    rpt_stop_statement,
    rpt_exit_statement,
    rpt_variable,
    rpt_constant,
};

enum rpv_tag
{
    rvt_string,
    rvt_integer,
    rvt_float,
};

enum rps_encoding
{
    rpe_ascii,
    rpe_utf8,
    rpe_utf16,
    // ...
};

typedef struct 
{
    rps_encoding    encoding;
    const char*     file;
    char*           source;
    char*           current;
    int             length;
    int             line;
    int             position;
}
rgcontext;

typedef struct
{
    rps_encoding    encoding;
    const rchar*    file;
    char*           source;
    char*           current;
    int             length;
    int             line;
    int             position;
    const rchar*    keyword;
    rpsparser*      parser;
    rpswrap*        wrapper;
}
rpscontext;

#ifdef __cplusplus
extern "C" {
#endif

extern void rathen_yy_setup(rgcontext* context, const char* file, rps_encoding encoding = rpe_utf8);
extern void rathen_yy_ssetup(rgcontext* context, const char* file, char* source, int length, rps_encoding encoding = rpe_utf8);
extern void rathen_yy_release(rgcontext* context);
extern rpscontext* rathen_yy_select(rpscontext* context);
extern void rathen_yy_parse(rpsparser* ps, const char* file);
extern void rathen_yy_sparse(rpsparser* ps, const char* file, char* source, int length);
extern int rathen_yy_read(char* buff, int* bytes, int cap);

extern bool rathen_is_type(const rchar* str, int len = -1);
extern bool rathen_is_keyword(const rchar* str, int len = -1);
extern rpsparser* rathen_create_parser(const rchar* name);
extern void rathen_destroy_parser(rpsparser* rps);
extern rpswrap* rathen_get_root(rpsparser* rps);
extern rpswrap* rathen_create_value(rpscontext* context, const rchar* name, const rchar* tyname, bool ref);
extern rpswrap* rathen_create_function(rpscontext* context, const rchar* name, const rchar* retyname, bool retref);
extern rpswrap* rathen_create_constant(rpscontext* context, const rchar* name, rpv_tag tag, const rchar* vstr);
//extern rpswrap* rathen_create();

#ifdef __cplusplus
};
#endif

#endif
