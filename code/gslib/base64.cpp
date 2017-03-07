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

#include <stdlib.h>
#include <gslib/base64.h>

__gslib_begin__

static const gchar base64_encode_table[] = _t("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
static const byte base64_decode_table[] =
{
    62,         /* + (ASCII 43) */
    0, 0, 0,    /* ,-. */
    63,         /* / */
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61,     /* [0-9] */
    0, 0, 0, 0, 0, 0, 0,    /* :;<=>?@ */
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,   /* [A-Z] */
    0, 0, 0, 0, 0, 0,   /* [\]^_` */
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51  /* [a-z] */
};
#define base64_padding _t('=')

#pragma warning(disable: 4731)

int base64_encode(string& output, const void* data, int len)
{
    assert(data);
    if(len <= 0)
        return 0;
    div_t d = div(len, 3);
    assert(d.rem < 3);
    int pl = d.rem ? d.quot + 1 : d.quot;
    int pql = pl * 4;
    int l3 = d.quot;
    const gchar* b64t = base64_encode_table;
    output.resize(pql);
    auto* f = &output.front();
#ifndef _UNICODE
    __asm
    {
        push    ebp;                    /* use an extra register */
        mov     esi, data;
        mov     edi, f;
        mov     ecx, l3;
        mov     eax, dword ptr[b64t];   /* speed up for lookup */
    lrpt:
        mov     ebp, dword ptr[esi];    /* store [cast][b3][b2][b1] */
        bswap   ebp;                    /* turn to [b1][b2][b3][cast] */
                                        /* encode to [ec4][ec3][ec2][ec1] */
        mov     ebx, ebp;               /* encode [ec4] */
        shr     ebx, 8;
        and     ebx, 0x3f;
        mov     dh, byte ptr[eax+ebx];
        mov     ebx, ebp;               /* encode [ec3] */
        shr     ebx, 14;
        and     ebx, 0x3f;
        mov     dl, byte ptr[eax+ebx];
        shl     edx, 16;                /* make dx writable */
        mov     ebx, ebp;               /* encode [ec2] */
        shr     ebx, 20;
        and     ebx, 0x3f;
        mov     dh, byte ptr[eax+ebx];
        mov     ebx, ebp;               /* encode [ec1] */
        shr     ebx, 26;
        mov     dl, byte ptr[eax+ebx];
        mov     [edi], edx;             /* store edx */
        add     esi, 3;
        add     edi, 4;
        loop    lrpt;
        pop     ebp;                    /* retrieve ebp */
        mov     ecx, d.rem;
        cmp     ecx, 1;
        jl      lpad0;                  /* no padding */
        jg      lpad2;
                                        /* actually pad1 */
        mov     edx, 0x3d3d0000;
        xor     ecx, ecx;
        mov     ch, byte ptr[esi];
        mov     ebx, ecx;
        shr     ebx, 4;
        and     ebx, 0x3f;
        mov     dh, byte ptr[eax+ebx];
        shr     ecx, 10;
        and     ecx, 0x3f;
        mov     dl, byte ptr[eax+ecx];
        mov     [edi], edx;
        jmp     lpad0;
    lpad2:
        mov     edx, 0x3d00;
        mov     ecx, dword ptr[esi];
        bswap   ecx;
        xor     cx, cx;
        mov     ebx, ecx;
        shr     ebx, 14;
        and     ebx, 0x3f;
        mov     dl, byte ptr[eax+ebx];
        shl     edx, 16;                /* make dx writable */
        mov     ebx, ecx;
        shr     ebx, 20;
        and     ebx, 0x3f;
        mov     dh, byte ptr[eax+ebx];
        shr     ecx, 26;
        and     ecx, 0x3f;
        mov     dl, byte ptr[eax+ecx];
        mov     [edi], edx;
    lpad0:
    }
#else
    __asm
    {
        push    ebp;
        mov     esi, data;
        mov     edi, f;
        mov     ecx, l3;
        mov     eax, dword ptr[b64t];
    lrpt:
        mov     ebp, dword ptr[esi];
        bswap   ebp;
        mov     ebx, ebp;
        shr     ebx, 19;
        and     ebx, 0x7e;
        mov     dx, word ptr[eax+ebx];
        shl     edx, 16;
        mov     ebx, ebp;
        shr     ebx, 25;
        and     ebx, 0x7e;
        mov     dx, word ptr[eax+ebx];
        mov     [edi], edx;
        mov     ebx, ebp;
        shr     ebx, 7;
        and     ebx, 0x7e;
        mov     dx, word ptr[eax+ebx];
        shl     edx, 16;
        mov     ebx, ebp;
        shr     ebx, 13;
        and     ebx, 0x7e;
        mov     dx, word ptr[eax+ebx];
        mov     [edi+4], edx;
        add     esi, 3;
        add     edi, 8;
        loop    lrpt;
        pop     ebp;
        mov     ecx, d.rem;
        cmp     ecx, 1;
        jl      lpad0;
        jg      lpad2;
        xor     ecx, ecx;
        mov     ch, byte ptr[esi];
        mov     ebx, ecx;
        shr     ebx, 3;
        and     ebx, 0x7e;
        mov     dx, word ptr[eax+ebx];
        shl     edx, 16;
        shr     ecx, 9;
        and     ecx, 0x7e;
        mov     dx, word ptr[eax+ecx];
        mov     [edi], edx;
        mov     [edi+4], 0x003d003d;        /* == */
        jmp     lpad0;
    lpad2:
        mov     ecx, dword ptr[esi];
        bswap   ecx;
        xor     cx, cx;
        mov     ebx, ecx;
        shr     ebx, 19;
        and     ebx, 0x7e;
        mov     dx, word ptr[eax+ebx];
        shl     edx, 16;
        mov     ebx, ecx;
        shr     ebx, 25;
        and     ebx, 0x7e;
        mov     dx, word ptr[eax+ebx];
        mov     [edi], edx;
        mov     edx, 0x3d0000;
        shr     ecx, 13;
        and     ecx, 0x7e;
        mov     dx, word ptr[eax+ecx];
        mov     [edi+4], edx;
    lpad0:
    }
#endif
    return pql;
}

int base64_decode(vessel& output, const gchar* src, int len)
{
    assert(src);
    div_t d = div(len, 4);
    if(d.rem) {
        assert(!"bad base64 code!");
        return -1;
    }
    int l4 = d.quot;
    int pl = l4 * 3;
    const byte* b64t = base64_decode_table;
    output.flex(pl + 1);    /* extra 1 byte for write */
    output.set_cur(pl);
    auto* f = output.get_ptr();
#ifndef _UNICODE
    __asm
    {
        mov     esi, src;
        mov     edi, f;
        mov     ecx, l4;
        mov     eax, dword ptr[b64t];
    lrpt:
        movzx   ebx, byte ptr[esi];
        sub     ebx, 43;
        movzx   edx, byte ptr[eax+ebx];
        shl     edx, 6;
        movzx   ebx, byte ptr[esi+1];
        sub     ebx, 43;
        movzx   ebx, byte ptr[eax+ebx];
        or      edx, ebx;
        shl     edx, 6;
        movzx   ebx, byte ptr[esi+2];
        sub     ebx, 43;
        movzx   ebx, byte ptr[eax+ebx];
        or      edx, ebx;
        shl     edx, 6;
        movzx   ebx, byte ptr[esi+3];
        sub     ebx, 43;
        movzx   ebx, byte ptr[eax+ebx];
        or      edx, ebx;
        bswap   edx;
        shr     edx, 8;
        mov     [edi], edx;
        add     esi, 4;
        add     edi, 3;
        loop    lrpt;
    }
#else
    __asm
    {
        mov     esi, src;
        mov     edi, f;
        mov     ecx, l4;
        mov     eax, dword ptr[b64t];
    lrpt:
        movzx   ebx, word ptr[esi];
        sub     ebx, 43;
        movzx   edx, byte ptr[eax+ebx];
        shl     edx, 6;
        movzx   ebx, word ptr[esi+2];
        sub     ebx, 43;
        movzx   ebx, byte ptr[eax+ebx];
        or      edx, ebx;
        shl     edx, 6;
        movzx   ebx, word ptr[esi+4];
        sub     ebx, 43;
        movzx   ebx, byte ptr[eax+ebx];
        or      edx, ebx;
        shl     edx, 6;
        movzx   ebx, word ptr[esi+6];
        sub     ebx, 43;
        movzx   ebx, byte ptr[eax+ebx];
        or      edx, ebx;
        bswap   edx;
        shr     edx, 8;
        mov     [edi], edx;
        add     esi, 8;
        add     edi, 3;
        loop    lrpt;
    }
#endif
    /* deal with padding */
    int pad = 0;
    for(const gchar* s = src + len - 1; s >= src; s --) {
        if(*s != base64_padding)
            break;
        pad ++;
    }
    pl -= pad;
    output.set_cur(pl);
    return pl;
}

__gslib_end__
