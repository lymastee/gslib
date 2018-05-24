/*
 * Copyright (c) 2016-2018 lymastee, All rights reserved.
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

#ifndef dvt_31727ef2_2dfa_4e64_b93d_deab9c280036_h
#define dvt_31727ef2_2dfa_4e64_b93d_deab9c280036_h

#include <gslib/config.h>

__gslib_begin__

#define method_address(method) \
    []()->uint { __asm { mov eax, method } } ()

template<class _cls>
class vtable_ops:
    public _cls
{
public:
    typedef vtable_ops<_cls> myref;

public:
    myref() {}
    template<class _arg1>
    myref(_arg1 a1): _cls(a1) {}
    template<class _arg1, class _arg2>
    myref(_arg1 a1, _arg2 a2): _cls(a1, a2) {}
    template<class _arg1, class _arg2, class _arg3>
    myref(_arg1 a1, _arg2 a2, _arg3 a3): _cls(a1, a2, a3) {}
    virtual void end_of_vtable() {}

public:
    int get_virtual_method_index(uint m) const
    {
        __asm {
            xor     eax, eax;
            mov     ecx, dword ptr[ecx];
            mov     edx, myref::end_of_vtable;
            mov     ebx, m;
        l_loop:
            cmp     dword ptr[ecx + eax], edx;
            jz      l_exit;
            cmp     dword ptr[ecx + eax], ebx;
            jz      l_exit;
            add     eax, 4;
            jmp     l_loop;
        l_exit:
            shr     eax, 2;
        }
    }
    int size_of_vtable() const
    {
        return get_virtual_method_index(method_address(myref::end_of_vtable));
    }
    void* create_per_instance_vtable(_cls* p)
    {
        assert(p);
        int size = size_of_vtable() * 4;
        byte* ovt = *(byte**)p;
        void* pvt = VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        assert(pvt);
        memcpy(pvt, ovt, size);
        memcpy(p, &pvt, 4);
        DWORD oldpro;
        VirtualProtect(pvt, size, PAGE_EXECUTE_READ, &oldpro);
        return ovt;
    }
    void destroy_per_instance_vtable(_cls* p, void* ovt)
    {
        assert(p && ovt);
        byte* pvt = *(byte**)p;
        int size = size_of_vtable() * 4;
        VirtualFree(pvt, size, MEM_DECOMMIT);
        memcpy(p, &ovt, 4);
    }
    template<class _func>
    uint replace_vtable_method(_cls* p, int index, _func func)
    {
        uint* vt = *(uint**)p;
        vt += index;
        uint r = *vt;
        DWORD oldpro;
        VirtualProtect(vt, 4, PAGE_EXECUTE_READWRITE, &oldpro);
        *vt = (uint)func;
        VirtualProtect(vt, 4, oldpro, &oldpro);
        return r;
    }
};

__gslib_end__

#endif
