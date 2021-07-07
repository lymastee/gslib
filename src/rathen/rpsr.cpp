#include <rathen\buildin.h>
#include <rathen\basis.h>
#include <rathen\rpsr.h>
#include <rathen\parser.h>
#include <gslib\file.h>

using namespace gs;
using namespace gs::rathen;

typedef parser::iterator rpsiter;

static rpscontext*  __rps_context = 0;

extern int yyparse();

template<class holder>
rpswrap* _rathen_create_value(rpscontext* context, holder* hdr, type* ty, const rchar* name, bool global, bool ref)
{
    assert(context && hdr && ty && name);
    if(hdr->find_object(name)) {
        set_error(_t("value name has already been taken: %s."), name);
        return 0;
    }
    rpsparser* rps = context->parser;
    assert(rps);
    rpsiter i = rps->birth<ps_value>(rpsiter(context->wrapper));
    ps_value* v = static_cast<ps_value*>(i.get_ptr());
    v->set_type(ty);
    v->set_name(name);
    v->set_global(global);
    v->set_ref(ref);
    hdr->add_indexing(v);
    return i._vptr;
}

template<class holder>
rpswrap* _rathen_create_function(rpscontext* context, holder* hdr, type* rety, const rchar* name, bool retref)
{
    assert(context && hdr && rety);
    if(hdr->find_object(name)) {
        set_error(_t("function name has already been taken: %s."), name);
        return 0;
    }
    rpsparser* rps = context->parser;
    assert(rps);
    rpsiter i = rps->birth<ps_function>(rpsiter(context->wrapper));
    ps_function* f = static_cast<ps_function*>(i.get_ptr());
    f->set_retv_type(rety);
    f->set_name(name);
    f->set_retv_ref(retref);
    f->set_parent(hdr);
    hdr->add_indexing(f);
    return i._vptr;
}

template<class holder>
rpswrap* _rathen_create_constant(rpscontext* context, holder* hdr, rpv_tag tag, const rchar* name, const rchar* vstr)
{
    assert(context && hdr && vstr);
    if(name && hdr->find_object(name)) {
        set_error(_t("constant name has already been taken: %s."), name);
        return 0;
    }
    rpsparser* rps = context->parser;
    assert(rps);
    rpsiter i = rps->birth<ps_constant>(rpsiter(context->wrapper));
    ps_constant* c = static_cast<ps_constant*>(i.get_ptr());
    switch(tag)
    {
    case rvt_string:
        c->set_string(vstr);
        break;
    case rvt_float:
        c->set_float((real32)(string(vstr).to_real()));
        break;
    case rvt_integer:
        c->set_integer(string(vstr).to_int());
        break;
    }
    if(name && name[0]) {
        c->set_name(name);
        hdr->add_indexing(c);
    }
    return i._vptr;
}

extern "C" {

void rathen_yy_setup(rgcontext* context, const char* file, rps_encoding encoding)
{
    assert(context && file && encoding == rpe_utf8);
    string buf;
    gs::file f(file, "r,ccs=utf-8");
    f.read_all(buf);
    rathen_yy_ssetup(context, file, (char*)buf.c_str(), buf.length(), rpe_utf8);
}

void rathen_yy_ssetup(rgcontext* context, const char* file, char* source, int length, rps_encoding encoding)
{
    assert(context && file && source && encoding == rpe_utf8);
    memset(context, 0, sizeof(rgcontext));
    context->encoding = encoding;
    context->file = file;
    context->source = source;
    context->current = source;
    context->length = length;
    context->line = 0;
    context->position = 0;
}

void rathen_yy_release(rgcontext* context)
{
    assert(context);
    /* reserved */
}

rpscontext* rathen_yy_select(rpscontext* context)
{
    rpscontext* former = __rps_context;
    __rps_context = context;
    return former;
}

int rathen_yy_read(char* buff, int* bytes, int cap)
{
    assert(__rps_context);
    int c = cap;
    int remaining = __rps_context->length - (__rps_context->current - __rps_context->source);
    if(c > remaining)
        c = remaining;
    memcpy_s(buff, cap, __rps_context->current, c);
    *bytes = c;
    __rps_context->current += c;
    return 0;
}

void rathen_yy_parse(rpsparser* ps, const char* file)
{
    assert(ps && file);
    rpscontext context;
    rpscontext* old = rathen_yy_select(&context);
    rathen_yy_setup((rgcontext*)&context, file);
    context.parser = ps;
    context.wrapper = rathen_get_root(ps);
    yyparse();
    rathen_yy_select(old);
}

void rathen_yy_sparse(rpsparser* ps, const char* file, char* source, int length)
{
    assert(ps && file && source);
    rpscontext context;
    rpscontext* old = rathen_yy_select(&context);
    rathen_yy_ssetup((rgcontext*)&context, file, source, length);
    context.parser = ps;
    context.wrapper = rathen_get_root(ps);
    yyparse();
    rathen_yy_select(old);
}

bool rathen_is_type(const rchar* str, int len)
{
    if(len < 0)
        len = strtool::length(str);
    return _type_manager->find_type(str, len) != 0;
}

bool rathen_is_keyword(const rchar* str, int len)
{
    if(len < 0)
        len = strtool::length(str);
    return _key_manager->get_key(string(str, len)) != 0;
}

rpsparser* rathen_create_parser(const rchar* name)
{
    assert(name);
    parser* p = gs_new(parser);
    assert(p);
    p->set_root(name);
    return p;
}

void rathen_destroy_parser(rpsparser* rps)
{
    assert(rps);
    gs_del(parser, rps);
}

rpswrap* rathen_get_root(rpsparser* rps)
{
    assert(rps);
    return rps->get_root()._vptr;
}

rpswrap* rathen_create_value(rpscontext* context, const rchar* name, const rchar* tyname, bool ref)
{
    assert(context && context->parser && name && tyname);
    if(!context->wrapper)
        context->wrapper = rathen_get_root(context->parser);
    rpsobj* obj = context->wrapper->get_ptr();
    assert(obj);
    type* ty = _type_manager->find_type(tyname);
    if(ty == 0) {
        set_error(_t("unknown type name: %s."), tyname);
        return 0;
    }
    switch(obj->get_tag())
    {
    case ps_obj::pt_root:
        return _rathen_create_value(context, static_cast<ps_root*>(obj), ty, name, true, ref);
    case ps_obj::pt_function:
        return _rathen_create_value(context, static_cast<ps_function*>(obj), ty, name, false, ref);
    case ps_obj::pt_block:
        return _rathen_create_value(context, static_cast<ps_block*>(obj), ty, name, false, ref);
    default:
        set_error(_t("improper position to register a value, parent name: %s?"), obj->get_name().c_str());
        return 0;
    }
}

rpswrap* rathen_create_function(rpscontext* context, const rchar* name, const rchar* retyname, bool retref)
{
    assert(context && context->parser && name && retyname);
    if(!context->wrapper)
        context->wrapper = rathen_get_root(context->parser);
    rpsobj* obj = context->wrapper->get_ptr();
    assert(obj);
    type* ty = _type_manager->find_type(retyname);
    if(ty == 0) {
        set_error(_t("unknown type name: %s."), retyname);
        return 0;
    }
    switch(obj->get_tag())
    {
    case ps_obj::pt_root:
        return _rathen_create_function(context, static_cast<ps_root*>(obj), ty, name, retref);
    case ps_obj::pt_function:
        return _rathen_create_function(context, static_cast<ps_function*>(obj), ty, name, retref);
    case ps_obj::pt_block:
        return _rathen_create_function(context, static_cast<ps_block*>(obj), ty, name, retref);
    default:
        set_error(_t("improper position to register a function, parent name: %s?"), obj->get_name().c_str());
        return 0;
    }
}

rpswrap* rathen_create_constant(rpscontext* context, const rchar* name, rpv_tag tag, const rchar* vstr)
{
    assert(context && context->parser && name && vstr);
    if(!context->wrapper)
        context->wrapper = rathen_get_root(context->parser);
    rpsobj* obj = context->wrapper->get_ptr();
    assert(obj);
    switch(obj->get_tag())
    {
    case ps_obj::pt_root:
        return _rathen_create_constant(context, static_cast<ps_root*>(obj), tag, name, vstr);
    case ps_obj::pt_function:
        return _rathen_create_constant(context, static_cast<ps_function*>(obj), tag, name, vstr);
    case ps_obj::pt_block:
        return _rathen_create_constant(context, static_cast<ps_block*>(obj), tag, name, vstr);
    default:
        set_error(_t("improper position to register a constant, parent name: %s?"), obj->get_name().c_str());
        return 0;
    }
}

};
