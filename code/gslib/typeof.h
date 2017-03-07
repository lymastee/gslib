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

#ifndef typeof_027722a0_eb51_4e04_8f59_b9299ee17be6_h
#define typeof_027722a0_eb51_4e04_8f59_b9299ee17be6_h

#include <gslib/type.h>

__gslib_begin__

#define gs_register_typeof_bs(t) \
    template<> \
    struct typeof<t*> { typedef t type; }; \
    template<> \
    struct typeof<const t*> { typedef t type; };

#define gs_register_typeof(q, t) \
    q t; /* for announcement. */ \
    gs_register_typeof_bs(t);

#define gs_register_typeof_ns(ns, q, t) \
    namespace ns { \
        q t; /* for announcement. */ \
    }; \
    gs_register_typeof_bs(ns::t);

/* all pointer typeof operator should be registered here. */
gs_register_typeof_bs(int8);
gs_register_typeof_bs(int16);
gs_register_typeof_bs(int);
gs_register_typeof_bs(int64);
gs_register_typeof_bs(wchar);
gs_register_typeof_bs(real32);
gs_register_typeof_bs(real);
gs_register_typeof_bs(byte);
gs_register_typeof_bs(word);
gs_register_typeof_bs(dword);
gs_register_typeof_bs(qword);
gs_register_typeof_ns(rathen, struct, ps_data);
gs_register_typeof_ns(rathen, struct, ps_ref);
gs_register_typeof_ns(rathen, struct, ps_stdata);
gs_register_typeof_ns(rathen, struct, ps_stref);
/* more to come ... */

__gslib_end__

#endif
