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

#include <windows.h>
#include <gslib/dvt.h>

__gslib_begin__

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

#if defined(WIN32) || defined(_WINDOWS)
#define alloc_writable_exec_codebytes(len)          VirtualAlloc(nullptr, len, MEM_COMMIT, PAGE_EXECUTE_READWRITE)
#define lock_exec_codebytes(ptr, len, oldprotag)    VirtualProtect(ptr, len, PAGE_EXECUTE_READ, &oldprotag)
#define unlock_exec_codebytes(ptr, len, oldprotag)  VirtualProtect(ptr, len, oldprotag, &oldprotag)
#define dealloc_exec_codebytes(ptr, len)            VirtualFree(ptr, len, MEM_DECOMMIT)
#endif

notify_code::notify_code(int argsize)
{
    _argsize = argsize;
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
}

notify_code::~notify_code()
{
    if(_ptr) {
        unlock_exec_codebytes(_ptr, _len, _oldpro);
        dealloc_exec_codebytes(_ptr, _len);
        _ptr = nullptr;
        _len = 0;
    }
}

void notify_code::finalize(uint old_func, uint host, uint action)
{
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
    lock_exec_codebytes(_ptr, _len, _oldpro);
}

notify_holder::notify_holder()
{
    _backvt = nullptr;
    _backvtsize = 0;
    _delete_later = false;
}

notify_holder::~notify_holder()
{
    if(_backvt) {
        dvt_recover_vtable(this, _backvt, _backvtsize);
        _backvt = nullptr;
        _backvtsize = 0;
    }
    for(auto* notify : _notifiers) {
        assert(notify);
        delete notify;
    }
    _notifiers.clear();
}

notify_code* notify_holder::add_notifier(int argsize)
{
    auto* notify = new notify_code(argsize);
    assert(notify);
    _notifiers.push_back(notify);
    return notify;
}

bool notify_collector::set_delete_later(notify_holder* holder)
{
    assert(holder);
    if(!holder->is_delete_later()) {
        holder->set_delete_later();
        _holders.push_back(holder);
        return true;
    }
    return false;
}

void notify_collector::cleanup()
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
    if(pfunc[0] == 0x8b && pfunc[1] == 0x01) {          /* mov eax, dword ptr[ecx] */
        assert(pfunc[2] == 0xff && pfunc[3] == 0x60);   /* jmp dword ptr[eax + ?] */
        byte* pvt = *(byte**)pthis;
        pvt += pfunc[4];
        return dsm_get_address(*(int*)pvt, pthis);
    }
    if(pfunc[0] == 0x55)        /* push ebp, means the address was right. */
        return addr;
    assert(!"unexpected dsm code.");
    return addr;
}

void dvt_recover_vtable(void* pthis, void* ovt, int vtsize)
{
    assert(pthis && ovt);
    byte* pvt = *(byte**)pthis;
    vtsize *= 4;
    VirtualFree(pvt, vtsize, MEM_DECOMMIT);
    memcpy(pthis, &ovt, 4);
}

__gslib_end__
