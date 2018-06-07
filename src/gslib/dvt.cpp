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
