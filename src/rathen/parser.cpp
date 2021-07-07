#include <rathen\parser.h>

__rathen_begin__

const gchar* parser::get_name() const
{
    const_iterator r = get_root();
    return r.is_valid() ? r->get_name().c_str() : 0;
}

void parser::set_root(const gchar* name)
{
    assert(name);
    iterator r = get_root();
    if(r.is_valid())
        r->set_name(name);
}

__rathen_end__
