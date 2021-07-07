#include <rathen\basis.h>

__rathen_begin__

type_manager::type_manager()
{
    regtype<int_type>();
    regtype<bool_type>();
    regtype<void_type>();
    regtype<string_type>();
}

__rathen_end__
