/*
 * Copyright (c) 2016-2021 lymastee, All rights reserved.
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

#include <windows.h>
#include <gslib/dvt.h>

__gslib_begin__

static const int max_vtable_size = 999;

static const byte __notify_code_0[] =
{
    0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:old_func */
    0xff, 0xd0,                             /* call eax */
    0x8d, 0x0d, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea ecx, 0xcccccccc:host */
    0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:action */
    0xff, 0xe0,                             /* jmp eax */
};

static const byte __notify_code_1[] =
{
    0xff, 0x74, 0x24, 0x04,                 /* push dword ptr[esp+4] */
    0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:old_func */
    0xff, 0xd0,                             /* call eax */
    0x8d, 0x0d, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea ecx, 0xcccccccc:host */
    0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:action */
    0xff, 0xe0,                             /* jmp eax */
};

static const byte __notify_code_2[] =
{
    0xff, 0x74, 0x24, 0x08,                 /* push dword ptr[esp+8] */
    0xff, 0x74, 0x24, 0x08,                 /* push dword ptr[esp+8] */
    0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:old_func */
    0xff, 0xd0,                             /* call eax */
    0x8d, 0x0d, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea ecx, 0xcccccccc:host */
    0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:action */
    0xff, 0xe0,                             /* jmp eax */
};

static const byte __notify_code_3[] =
{
    0xff, 0x74, 0x24, 0x0c,                 /* push dword ptr[esp+12] */
    0xff, 0x74, 0x24, 0x0c,                 /* push dword ptr[esp+12] */
    0xff, 0x74, 0x24, 0x0c,                 /* push dword ptr[esp+12] */
    0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:old_func */
    0xff, 0xd0,                             /* call eax */
    0x8d, 0x0d, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea ecx, 0xcccccccc:host */
    0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:action */
    0xff, 0xe0,                             /* jmp eax */
};

static const byte __notify_code_n[] =
{
    0x8d, 0x74, 0x24, 0x04,                 /* lea esi, [esp + 4] */
    0x81, 0xec, 0xcc, 0xcc, 0xcc, 0xcc,     /* sub esp, 0xcccccccc:sizeof_arg_bytes */
    0x8b, 0xfc,                             /* mov edi, esp */
    0x8b, 0xd1,                             /* mov edx, ecx */
    0xb9, 0xcc, 0xcc, 0xcc, 0xcc,           /* mov ecx, 0xcccccccc:sizeof_arg_dwords */
    0xf3, 0xa5,                             /* rep movsd */
    0x8b, 0xca,                             /* mov ecx, edx */
    0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:old_func */
    0xff, 0xd0,                             /* call eax */
    0x8d, 0x0d, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea ecx, 0xcccccccc:host */
    0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:action */
    0xff, 0xe0,                             /* jmp eax */
};

static const byte __reflect_code_n[] =
{
    0x8d, 0x0d, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea ecx, 0xcccccccc:host */
    0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:action */
    0xff, 0xe0,                             /* jmp eax */
};

#if defined(WIN32) || defined(_WINDOWS)
#define alloc_writable_exec_codebytes(len)          VirtualAlloc(nullptr, len, MEM_COMMIT, PAGE_EXECUTE_READWRITE)
#define lock_exec_codebytes(ptr, len, oldprotag)    VirtualProtect(ptr, len, PAGE_EXECUTE_READ, &oldprotag)
#define unlock_exec_codebytes(ptr, len, oldprotag)  VirtualProtect(ptr, len, oldprotag, &oldprotag)
#define dealloc_exec_codebytes(ptr, len)            VirtualFree(ptr, len, MEM_DECOMMIT)
#endif

dvt_detour_code::dvt_detour_code(detour_type dty, int argsize)
{
    _type = dty;
    _argsize = argsize;
    switch(dty)
    {
    case detour_notify:
        if(argsize <= 0) {
            _len = sizeof(__notify_code_0);
            _ptr = (byte*)alloc_writable_exec_codebytes(_len);
            assert(_ptr);
            memcpy(_ptr, __notify_code_0, _len);
        }
        else if(argsize <= 4) {
            _len = sizeof(__notify_code_1);
            _ptr = (byte*)alloc_writable_exec_codebytes(_len);
            assert(_ptr);
            memcpy(_ptr, __notify_code_1, _len);
        }
        else if(argsize <= 8) {
            _len = sizeof(__notify_code_2);
            _ptr = (byte*)alloc_writable_exec_codebytes(_len);
            assert(_ptr);
            memcpy(_ptr, __notify_code_2, _len);
        }
        else if(argsize <= 12) {
            _len = sizeof(__notify_code_3);
            _ptr = (byte*)alloc_writable_exec_codebytes(_len);
            assert(_ptr);
            memcpy(_ptr, __notify_code_3, _len);
        }
        else {
            _len = sizeof(__notify_code_n);
            _ptr = (byte*)alloc_writable_exec_codebytes(_len);
            assert(_ptr);
            memcpy(_ptr, __notify_code_n, _len);
            /* for x86 case */
            int size_in_dwords = (argsize <= 0) ? 0 : (int)(((uint)(argsize - 1)) >> 2) + 1;
            int size_in_bytes = (int)((uint)size_in_dwords << 2);
            *(uint*)(_ptr + 6) = size_in_bytes;
            *(uint*)(_ptr + 15) = size_in_dwords;
        }
        break;
    case detour_reflect:
    default:
        _len = sizeof(__reflect_code_n);
        _ptr = (byte*)alloc_writable_exec_codebytes(_len);
        assert(_ptr);
        memcpy(_ptr, __reflect_code_n, _len);
        break;
    }
}

dvt_detour_code::~dvt_detour_code()
{
    if(_ptr) {
        unlock_exec_codebytes(_ptr, _len, _oldpro);
        dealloc_exec_codebytes(_ptr, _len);
        _ptr = nullptr;
        _len = 0;
    }
}

void dvt_detour_code::finalize(uint old_func, uint host, uint action)
{
    switch(_type)
    {
    case detour_notify:
        if(_argsize <= 0) {
            *(uint*)(_ptr + 2) = old_func;
            *(uint*)(_ptr + 10) = host;
            *(uint*)(_ptr + 16) = action;
        }
        else if(_argsize <= 4) {
            *(uint*)(_ptr + 6) = old_func;
            *(uint*)(_ptr + 14) = host;
            *(uint*)(_ptr + 20) = action;
        }
        else if(_argsize <= 8) {
            *(uint*)(_ptr + 10) = old_func;
            *(uint*)(_ptr + 18) = host;
            *(uint*)(_ptr + 24) = action;
        }
        else if(_argsize <= 12) {
            *(uint*)(_ptr + 14) = old_func;
            *(uint*)(_ptr + 22) = host;
            *(uint*)(_ptr + 28) = action;
        }
        else {
            *(uint*)(_ptr + 25) = old_func;
            *(uint*)(_ptr + 33) = host;
            *(uint*)(_ptr + 39) = action;
        }
        break;
    case detour_reflect:
    default:
        *(uint*)(_ptr + 2) = host;
        *(uint*)(_ptr + 8) = action;
        break;
    }
    lock_exec_codebytes(_ptr, _len, _oldpro);
}

dvt_bridge_code::dvt_bridge_code()
{
    _bridges = nullptr;
    _bridge_size = _jt_stride = 0;
}

dvt_bridge_code::~dvt_bridge_code()
{
    if(_bridges) {
        unsigned long oldpro = 0;
        unlock_exec_codebytes(_bridges, _bridge_size, oldpro);
        dealloc_exec_codebytes(_bridges, _bridge_size);
        _bridges = nullptr;
    }
}

void dvt_bridge_code::install(void* vt, int count)
{
    assert(!_bridges);
    uint* pvt = (uint*)vt;
    _jt_stride = 8;
    _bridge_size = count * _jt_stride;
    _bridges = (byte*)alloc_writable_exec_codebytes(_bridge_size);
    for(int i = 0; i < count; i ++) {
        byte* bptr = (byte*)get_bridge(i);
        bptr[0] = 0x8d, bptr[1] = 0x05;     /* lea eax, ??? */
        *(uint*)(bptr + 2) = pvt[i];
        bptr[6] = 0xff, bptr[7] = 0xe0;     /* jmp eax */
    }
    unsigned long oldpro = 0;
    lock_exec_codebytes(_bridges, _bridge_size, oldpro);
}

void* dvt_bridge_code::get_bridge(int index) const
{
    return (void*)(_bridges + (index * _jt_stride));
}

dvt_sub_holder::~dvt_sub_holder()
{
    if(_subvt && _backvt) {
        switch_to_dvt();    /* ensure we have dvt here */
        dvt_recover_vtable(_subvt, _backvt, _backvtsize);
        _backvt = nullptr;
        _backvtsize = 0;
    }
}

void dvt_sub_holder::switch_to_ovt()
{
    if(_switchvt || !_backvt)
        return;
    byte* pvt = *(byte**)_subvt;
    _switchvt = pvt;
    memcpy(_subvt, &_backvt, 4);
}

void dvt_sub_holder::switch_to_dvt()
{
    if(!_switchvt)
        return;
    memcpy(_subvt, &_switchvt, 4);
    _switchvt = nullptr;
}

void dvt_sub_holder::ensure_dvt_available(void* subvt, uint func)
{
    assert(subvt);
    if(_subvt || _backvt)
        return;
    _subvt = subvt;
    _backvtsize = dvt_get_vtable_size(subvt, func) + 1;         /* include eov */
    _backvt = dvt_create_vtable(subvt, _backvtsize, _bridges);
}

dvt_holder::~dvt_holder()
{
    _subs.clear();
    for(auto* detour : _detours) {
        assert(detour);
        delete detour;
    }
    _detours.clear();
}

dvt_sub_holder& dvt_holder::ensure_sub_holder(void* pthis)
{
    if(!pthis)
        pthis = this;
    auto f = std::lower_bound(_subs.begin(), _subs.end(), pthis, [](const dvt_sub_holder& holder, void* pthis)->bool { return holder.get_subvt() < pthis; });
    if((f == _subs.end()) || (f->get_subvt() != pthis))
        f = _subs.emplace(f, _bridges, pthis == this);
    return *f;
}

dvt_detour_code* dvt_holder::add_detour(dvt_detour_code::detour_type dty, int argsize)
{
    auto* notify = new dvt_detour_code(dty, argsize);
    assert(notify);
    _detours.push_back(notify);
    return notify;
}

void dvt_holder::switch_to_ovt()
{
    for(dvt_sub_holder& subholder : _subs)
        subholder.switch_to_ovt();
}

void dvt_holder::switch_to_dvt()
{
    for(dvt_sub_holder& subholder : _subs)
        subholder.switch_to_dvt();
}

bool dvt_collector::set_delete_later(dvt_holder* holder)
{
    assert(holder);
    if(!holder->is_delete_later()) {
        holder->set_delete_later();
        _holders.push_back(holder);
        return true;
    }
    return false;
}

void dvt_collector::cleanup()
{
    for(auto* p : _holders) {
        assert(p);
        delete p;
    }
    _holders.clear();
}

uint dsm_get_address(uint addr, uint pthis)
{
    assert(addr && pthis);
    byte* pfunc = (byte*)addr;
    if(!pfunc)
        return 0;
    if(pfunc[0] == 0xe9)        /* jump offset from addr + 5 */
        return dsm_get_address(addr + 5 + *(int*)(pfunc + 1), pthis);
    if(pfunc[0] == 0x8d && pfunc[1] == 0x05) {              /* lea eax, ds:[address] */
        if(pfunc[6] == 0xff && pfunc[7] == 0xe0) {          /* jmp eax */
            int abs_addr = *(int*)(pfunc + 2);
            return dsm_get_address(abs_addr, pthis);
        }
        assert(!"unexpected.");
        return 0;
    }
    if(pfunc[0] == 0x8b && pfunc[1] == 0x01) {              /* mov eax, dword ptr[ecx] */
        if(pfunc[2] == 0xff && pfunc[3] == 0x20) {          /* jmp dword ptr[eax] */
            byte* pvt = *(byte**)pthis;
            return dsm_get_address(*(int*)pvt, pthis);
        }
        else if(pfunc[2] == 0xff && pfunc[3] == 0x60) {     /* jmp dword ptr[eax + ?] */
            byte* pvt = *(byte**)pthis;
            pvt += pfunc[4];
            return dsm_get_address(*(int*)pvt, pthis);
        }
        assert(!"unexpected.");
        return 0;
    }
    if(pfunc[0] == 0x55)        /* push ebp, means the address was right. */
        return addr;
    assert(!"unexpected dsm code.");
    return addr;
}

void* dvt_create_vtable(void* pthis, int vtsize, dvt_bridge_code& bridges)
{
    assert(pthis);
    byte* ovt = *(byte**)pthis;
    void** pvt = (void**)VirtualAlloc(nullptr, vtsize * 4, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    assert(pvt);
    bridges.install(ovt, vtsize);
    for(int i = 0; i < vtsize; i ++)
        pvt[i] = bridges.get_bridge(i);
    memcpy(pthis, &pvt, 4);
    DWORD oldpro;
    VirtualProtect(pvt, vtsize * 4, PAGE_EXECUTE_READ, &oldpro);
    return ovt;
}

void dvt_recover_vtable(void* pthis, void* ovt, int vtsize)
{
    assert(pthis && ovt);
    byte* pvt = *(byte**)pthis;
    vtsize *= 4;
    VirtualFree(pvt, vtsize, MEM_DECOMMIT);
    memcpy(pthis, &ovt, 4);
}

int dvt_get_vtable_method_index(void* pthis, uint m, uint eovm)
{
    uint mv = dsm_get_address(m, (uint)pthis);
    uint eov = dsm_get_address(eovm, (uint)pthis);
    uint* ovt = *(uint**)pthis;
    for(int i = 0; i < max_vtable_size; i ++) {
        uint cv = dsm_get_address(ovt[i], (uint)pthis);
        if(cv == mv)
            return i;
        if(cv == eov) {
            assert(!"get index failed.");
            return i;
        }
    }
    return -1;
}

int dvt_get_vtable_size(void* pthis, uint eovm)
{
    uint eov = dsm_get_address(eovm, (uint)pthis);
    uint* ovt = *(uint**)pthis;
    for(int i = 0; i < max_vtable_size; i ++) {
        uint cv = dsm_get_address(ovt[i], (uint)pthis);
        if(cv == eov)
            return i;
    }
    return -1;
}

uint dvt_replace_vtable_method(void* pthis, int index, uint func)
{
    uint* vt = *(uint**)pthis;
    vt += index;
    uint r = *vt;
    DWORD oldpro;
    VirtualProtect(vt, 4, PAGE_EXECUTE_READWRITE, &oldpro);
    *vt = func;
    VirtualProtect(vt, 4, oldpro, &oldpro);
    return r;
}

__gslib_end__
