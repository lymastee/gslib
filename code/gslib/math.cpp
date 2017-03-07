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

#include <xmmintrin.h>
#include <gslib/type.h>
#include <gslib/math.h>

__gslib_begin__

#define align16 __declspec(align(16))

static const float s1d42 = 1.4210855e-014f;
static const float s3d00 = 3.0f;
static const float s0d50 = 0.5f;
static const float shs0 = 1.f;
static const int spr0 = 0;
static const float align16 shs1[] = { 0.f, 1.f, 0.f, 0.f };
static const float align16 shs2[] = { -3.f, -2.f, 3.f, -1.f };
static const float align16 shs3[] = { 2.f, 1.f, -2.f, 1.f };
static const float align16 crs0[] = { 0.f, 1.f, 0.f, 0.f };
static const float align16 crs1[] = { -0.5f, 0.f, 0.5f, 0.f };
static const float align16 crs2[] = { 1.f, -2.5f, 2.f, -0.5f };
static const float align16 crs3[] = { -0.5f, 1.5f, -1.5f, 0.5f };
static const int s0i0i[] = { 0, INT_MIN, 0, INT_MIN };
static const int s0ii0[] = { 0, INT_MIN, INT_MIN, 0 };
static const int s00ii[] = { 0, 0, INT_MIN, INT_MIN };
static const int si00i[] = { INT_MIN, 0, 0, INT_MIN };
static const int saaaa[] = { INT_MAX, INT_MAX, INT_MAX, INT_MAX };
static const int siiii[] = { INT_MIN, INT_MIN, INT_MIN, INT_MIN };
static const int s1111[] = { 1, 1, 1, 1 };
static const int s2222[] = { 2, 2, 2, 2 };
static const int srrr[] =
{
    INT_MIN, INT_MIN, INT_MIN, INT_MIN,
    0, -1, 0, 0,
    0, 0, -1, 0,
    0, 0, 0, -1,
    -1, -1, -1, 0
};
static const int align16 arrr[] = { -1, -1, -1, 0 };
static const float align16 drrr[] = { -2.f, -2.f, -2.f, 0 };
static const float align16 rrrp[] = { -1.f, -1.f, -1.f, 1.f };
static const float align16 shhhh[] = { 0.5f, 0.5f, 0.5f, 0.5f };
static const float align16 stttt[] = { 3.f, 3.f, 3.f, 3.f };
static const float sf1000[] = { 1.f, 0, 0, 0 };
static const float sf0001[] = { 0, 0, 0, 1.f };
static const float sf0010[] = { 0, 0, 1.f, 0 };
static const float sf0100[] = { 0, 1.f, 0, 0 };
static const float sf1111[] = { 1.f, 1.f, 1.f, 1.f };
static const float sfqrh[] = { -0.5f, -0.5f, -0.5f, -0.5f };

__gs_naked vec2* __stdcall sse2_vec2normalize(vec2* o, const vec2* v)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        push        ecx;
        mov         eax, v;
        fld         s1d42;
        movss       xmm3, dword ptr [eax];
        fstp        v;
        movss       xmm4, dword ptr [eax+4];
        mov         eax, o;
        movaps      xmm0, xmm4;
        mulss       xmm0, xmm4;
        movaps      xmm1, xmm3;
        mulss       xmm1, xmm3;
        addss       xmm1, xmm0;
        movss       xmm0, dword ptr [v];
        comiss      xmm0, xmm1;
        jbe         label1;
        xorps       xmm0, xmm0;
        movss       dword ptr [eax], xmm0;
        movss       dword ptr [eax+4], xmm0;
        jmp         label2;
    label1:
        fld         s3d00;
        movaps      xmm2, xmm1;
        fstp        v;
        rsqrtss     xmm2, xmm2;
        fld         s0d50;
        movss       xmm0, dword ptr [v];
        mulss       xmm1, xmm2;
        fstp        dword ptr [ebp-4];
        mulss       xmm1, xmm2;
        subss       xmm0, xmm1;
        movss       xmm1, dword ptr [ebp-4];
        mulss       xmm1, xmm2;
        mulss       xmm1, xmm0;
        movaps      xmm0, xmm1;
        mulss       xmm0, xmm3;
        movss       dword ptr [eax], xmm0;
        mulss       xmm1, xmm4;
        movss       dword ptr [eax+4], xmm1;
    label2:
        leave;
        ret         8;
    }
}

__gs_naked vec2* __stdcall sse_vec2hermite(vec2* o, const vec2* p1, const vec2* t1, const vec2* p2, const vec2* t2, float s)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        movss       xmm0, dword ptr [s];
        movss       xmm1, xmm0;
        mulss       xmm1, xmm0;
        movss       xmm2, xmm0;
        shufps      xmm0, xmm0, 0;
        mulss       xmm2, xmm1;
        shufps      xmm1, xmm1, 0;
        mulps       xmm0, xmmword ptr [shs1];
        shufps      xmm2, xmm2,0;
        mulps       xmm1, xmmword ptr [shs2];
        addss       xmm0, dword ptr [shs0];
        mulps       xmm2, xmmword ptr [shs3];
        addps       xmm0, xmm1;
        addps       xmm0, xmm2;
        mov         eax, dword ptr [p1];
        movaps      xmm2, xmm0;
        shufps      xmm2, xmm0, 0FAh;
        shufps      xmm0, xmm0, 50h;
        mov         ecx, dword ptr [t1];
        mov         edx, dword ptr [p2];
        movlps      xmm4, qword ptr [eax];
        movhps      xmm4, qword ptr [ecx];
        mov         eax, dword ptr [t2];
        mulps       xmm4, xmm0;
        movlps      xmm6, qword ptr [edx];
        movhps      xmm6, qword ptr [eax];
        mulps       xmm6, xmm2;
        mov         ecx, dword ptr [o];
        addps       xmm4, xmm6;
        movhlps     xmm3, xmm4;
        addps       xmm4, xmm3;
        movlps      qword ptr [ecx], xmm4;
        mov         eax, dword ptr [o];
        pop         ebp;
        ret         18h;
    }
}

__gs_naked vec2* __stdcall sse_vec2catmullrom(vec2* o, const vec2* p1, const vec2* p2, const vec2* p3, const vec2* p4, float s)
{
    __asm
    {
        mov         edi, edi;
        push        ebp;
        mov         ebp, esp;
        movss       xmm0, dword ptr [s];
        movss       xmm1, xmm0;
        mulss       xmm1, xmm0;
        movss       xmm2, xmm0;
        shufps      xmm0, xmm0, 0;
        mulss       xmm2, xmm1;
        shufps      xmm1, xmm1, 0;
        mulps       xmm0, xmmword ptr [crs1];
        shufps      xmm2, xmm2, 0;
        mulps       xmm1, xmmword ptr [crs2];
        addps       xmm0, xmmword ptr [crs0];
        mulps       xmm2, xmmword ptr [crs3];
        addps       xmm0, xmm1;
        addps       xmm0, xmm2;
        mov         eax, dword ptr [p1];
        mov         ecx, dword ptr [p2];
        mov         edx, dword ptr [p3];
        movaps      xmm2, xmm0;
        shufps      xmm2, xmm0, 0FAh;
        shufps      xmm0, xmm0, 50h;
        movlps      xmm4, qword ptr [eax];
        movhps      xmm4, qword ptr [ecx];
        mov         eax, dword ptr [p4];
        mulps       xmm4, xmm0;
        movlps      xmm6, qword ptr [edx];
        movhps      xmm6, qword ptr [eax];
        mulps       xmm6, xmm2;
        addps       xmm4, xmm6;
        movhlps     xmm6, xmm4;
        addps       xmm4, xmm6;
        mov         ecx, dword ptr [o];
        movlps      qword ptr [ecx], xmm4;
        mov         eax, dword ptr [o];
        pop         ebp;
        ret         18h;
    }
}

__gs_naked vec2* __stdcall sse_vec2barycentric(vec2* o, const vec2* p1, const vec2* p2, const vec2* p3, float f, float g)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        xorps       xmm2, xmm2;
        movss       xmm0, dword ptr [f];
        mov         eax, dword ptr [p1];
        shufps      xmm0, xmm0, 0;
        xorps       xmm3, xmm3;
        mov         ecx, dword ptr [p2];
        movhps      xmm2, qword ptr [eax];
        movss       xmm1, dword ptr [g];
        movhps      xmm3, qword ptr [ecx];
        xorps       xmm4, xmm4;
        shufps      xmm1, xmm1,0;
        mov         eax, dword ptr [p3];
        subps       xmm3, xmm2;
        movhps      xmm4, qword ptr [eax];
        mulps       xmm3, xmm0;
        subps       xmm4, xmm2;
        mov         eax, dword ptr [o];
        addps       xmm3, xmm2;
        mulps       xmm4, xmm1;
        addps       xmm3, xmm4;
        movhps      qword ptr [eax], xmm3;
        mov         eax, dword ptr [o];
        pop         ebp;
        ret         18h;
    }
}

__gs_naked vec4* __stdcall sse_vec2transform(vec4* o, const vec2* p, const matrix* m)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         ecx, dword ptr [m];
        mov         eax, dword ptr [p];
        movups      xmm3, xmmword ptr [ecx];
        movss       xmm2, dword ptr [eax];
        shufps      xmm2, xmm2, 0;
        movups      xmm4, xmmword ptr [m];
        mulps       xmm2, xmm3;
        movss       xmm1, dword ptr [eax+4];
        shufps      xmm1, xmm1, 0;
        movups      xmm3, xmmword ptr [ecx+30h];
        mulps       xmm1, xmm4;
        mov         eax, dword ptr [o];
        addps       xmm2, xmm3;
        addps       xmm2, xmm1;
        movups      xmmword ptr [eax], xmm2;
        mov         eax, dword ptr [o];
        pop         ebp;
        ret         0Ch;
    }
}

__gs_naked vec2* __stdcall sse_vec2transformcoord(vec2* o, const vec2* p, const matrix* m)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         ecx, dword ptr [m];
        mov         eax, dword ptr [p];
        movups      xmm3, xmmword ptr [ecx];
        movss       xmm2, dword ptr [eax];
        shufps      xmm2, xmm2, 0;
        movups      xmm4, xmmword ptr [ecx+10h];
        mulps       xmm2, xmm3;
        movss       xmm1, dword ptr [eax+4];
        shufps      xmm1, xmm1, 0;
        movups      xmm3, xmmword ptr [ecx+30h];
        mulps       xmm1, xmm4;
        mov         eax, dword ptr [o];
        addps       xmm2, xmm3;
        addps       xmm2, xmm1;
        movaps      xmm0, xmm2;
        shufps      xmm0, xmm0, 0FFh;
        movaps      xmm4, xmm0;
        rcpps       xmm0, xmm0;
        movaps      xmm1, xmm0;
        mulps       xmm1, xmm0;
        addps       xmm0, xmm0;
        mulps       xmm1, xmm4;
        subps       xmm0, xmm1;
        mulps       xmm2, xmm0;
        movlps      qword ptr [eax], xmm2;
        mov         eax, dword ptr [o];
        pop         ebp;
        ret         0Ch;
    }
}

__gs_naked vec2* __stdcall sse_vec2transformnormal(vec2* o, const vec2* p, const matrix* m)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         ecx, dword ptr [m];
        mov         eax, dword ptr [p];
        movss       xmm2, dword ptr [eax];
        movups      xmm3, xmmword ptr [ecx];
        shufps      xmm2, xmm2, 0;
        movups      xmm4, xmmword ptr [ecx+10h];
        movss       xmm1, dword ptr [eax+4];
        mulps       xmm2, xmm3;
        shufps      xmm1, xmm1, 0;
        mulps       xmm1, xmm4;
        mov         eax, dword ptr [o];
        addps       xmm2, xmm1;
        movlps      qword ptr [eax], xmm2;
        mov         eax, dword ptr [o];
        pop         ebp;
        ret         0Ch;
    }
}

/* __thiscall ecx */
static __gs_naked void* __stdcall aligned_matrix(matrix* mat, uint a, uint b)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        push        ecx;
        mov         eax, dword ptr [ebp+0Ch];
        shl         eax, 2;
        mov         dword ptr [ebp-4], ecx;
        test        eax, eax;
        je          label1;
        mov         edx, dword ptr [ebp+8];
        dec         eax;
        push        esi;
        shr         eax, 2;
        push        edi;
        mov         esi, ecx;
        inc         eax;
    label4:
        cmp         dword ptr [ebp+10h], 0;
        jbe         label2;
        mov         ecx, edx;
        mov         edx, dword ptr [ebp+10h];
        mov         dword ptr [ebp+0Ch], edx;
        mov         edx, dword ptr [ebp+8];
        mov         edi, esi;
    label3:
        movss       xmm0, dword ptr [ecx];
        shufps      xmm0, xmm0,0;
        movaps      xmmword ptr [edi], xmm0;
        add         ecx, 4;
        add         edi, 10h;
        dec         dword ptr [ebp+0Ch];
        jne         label3;
    label2:
        add         edx, 10h;
        add         esi, 40h;
        dec         eax;
        mov         dword ptr [ebp+8], edx;
        jne         label4;
        mov         ecx, dword ptr [ebp-4];
        pop         edi;
        pop         esi;
    label1:
        mov         eax, ecx;
        leave;
        ret         0Ch;
    }
}

__gs_naked vec4* __stdcall sse2_vec2transformarray(vec4* o, uint ostride, const vec2* v, uint vstride, const matrix* m, uint n)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ecx;
        push        ecx;
        and         esp, 0FFFFFFF0h;
        add         esp, 4;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 158h;
        mov         eax, dword ptr [ebx+0Ch];
        mov         ecx, dword ptr [ebx+8];
        mov         edx, dword ptr [ebx+10h];
        mov         dword ptr [ebp-124h], eax;
        mov         eax, dword ptr [ebx+14h];
        push        esi;
        mov         esi, dword ptr [ebx+1Ch];
        push        edi;
        mov         dword ptr [ebp-11Ch], eax;
        mov         eax, dword ptr [ebx+18h];
        mov         dword ptr [ebp-120h], eax;
        push        0Ah;
        mov         eax, esi;
        pop         edi;
        and         eax, 0FFFFFFFCh;
        cmp         edi, esi;
        sbb         edi, edi;
        and         edi, eax;
        sub         esi, edi;
        shr         edi, 2;
        mov         dword ptr [ebp-114h], ecx;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebx+1Ch], esi;
        mov         dword ptr [ebp-12Ch], ecx;
        test        edi, edi;
        je          label1;
        push        4;
        push        4;
        push        dword ptr [ebp-120h];
        lea         ecx, [ebp-110h];
        call        aligned_matrix;
        mov         ecx, dword ptr [ebp-11Ch];
        mov         eax, ecx;
        imul        eax, eax, 3;
        mov         dword ptr [ebp-128h], eax;
        mov         eax, dword ptr [ebp-124h];
        mov         edx, eax;
        imul        edx, edx, 3;
        mov         dword ptr [ebp-130h], edx;
        mov         edx, dword ptr [ebp-118h];
        test        edi, edi;
        je          label2;
        movaps      xmm7, xmmword ptr [ebp-0A0h];
        movaps      xmm2, xmmword ptr [ebp-0B0h];
        movaps      xmm4, xmmword ptr [ebp-0C0h];
        movaps      xmm3, xmmword ptr [ebp-0D0h];
        movaps      xmm6, xmmword ptr [ebp-110h];
        mov         esi, dword ptr [ebp-11Ch];
        shl         ecx, 2;
        mov         dword ptr [ebp-134h], ecx;
        mov         ecx, dword ptr [ebp-114h];
    label3:
        movsd       xmm0, mmword ptr [edx+esi*2];
        movsd       xmm5, mmword ptr [edx];
        movhps      xmm5, qword ptr [esi+edx];
        mov         esi, dword ptr [ebp-128h];
        movhps      xmm0, qword ptr [edx+esi];
        mov         esi, dword ptr [ebp-130h];
        add         edx, dword ptr [ebp-134h];
        movaps      xmm1, xmm5;
        shufps      xmm1, xmm0, 88h;
        shufps      xmm5, xmm0, 0DDh;
        movaps      xmm0, xmm3;
        movaps      xmm3, xmmword ptr [ebp-100h];
        mulps       xmm0, xmm5;
        mulps       xmm6, xmm1;
        addps       xmm6, xmm0;
        addps       xmm6, xmmword ptr [ebp-50h];
        movaps      xmm0, xmm4;
        movaps      xmm4, xmmword ptr [ebp-0F0h];
        mulps       xmm0, xmm5;
        mulps       xmm3, xmm1;
        addps       xmm3, xmm0;
        addps       xmm3, xmmword ptr [ebp-40h];
        movaps      xmm0, xmm2;
        movaps      xmm2, xmmword ptr [ebp-0E0h];
        mulps       xmm4, xmm1;
        mulps       xmm0, xmm5;
        addps       xmm4, xmm0;
        addps       xmm4, xmmword ptr [ebp-30h];
        mulps       xmm2, xmm1;
        movaps      xmm0, xmm7;
        mulps       xmm0, xmm5;
        addps       xmm2, xmm0;
        addps       xmm2, xmmword ptr [ebp-20h];
        movaps      xmm1, xmm6;
        unpcklps    xmm1, xmm3;
        movlps      qword ptr [ecx], xmm1;
        movaps      xmm0, xmm4;
        unpcklps    xmm0, xmm2;
        movlps      qword ptr [ecx+8], xmm0;
        movhps      qword ptr [eax+ecx], xmm1;
        movhps      qword ptr [eax+ecx+8], xmm0;
        unpckhps    xmm6, xmm3;
        movaps      xmm3, xmmword ptr [ebp-0D0h];
        movlps      qword ptr [ecx+eax*2], xmm6;
        unpckhps    xmm4, xmm2;
        movaps      xmm2, xmmword ptr [ebp-0B0h];
        movlps      qword ptr [ecx+eax*2+8], xmm4;
        movhps      qword ptr [ecx+esi], xmm6;
        movaps      xmm6, xmmword ptr [ebp-110h];
        movhps      qword ptr [esi+ecx+8], xmm4;
        movaps      xmm4, xmmword ptr [ebp-0C0h];
        mov         esi, eax;
        shl         esi, 2;
        add         ecx, esi;
        dec         edi;
        mov         esi, dword ptr [ebp-11Ch];
        jne         label3;
        mov         esi, dword ptr [ebx+1Ch];
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jmp         label1;
    label2:
        mov         ecx, dword ptr [ebp-114h];
    label1:
        test        esi, esi;
        je          label4;
        mov         edi, dword ptr [ebp-124h];
    label5:
        push        dword ptr [ebp-120h];
        push        edx;
        push        ecx;
        call        sse_vec2transform;
        mov         edx, dword ptr [ebp-118h];
        mov         ecx, dword ptr [ebp-114h];
        add         edx, dword ptr [ebp-11Ch];
        add         ecx, edi;
        dec         esi;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jne         label5;
    label4:
        mov         ecx, dword ptr [ebp-4];
        mov         eax, dword ptr [ebp-12Ch];
        pop         edi;
        xor         ecx, ebp;
        pop         esi;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         18h;
    }
}

__gs_naked vec2* __stdcall sse2_vec2transformcoordarray(vec2* o, uint ostride, const vec2* v, uint vstride, const matrix* m, uint n)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ecx;
        push        ecx;
        and         esp, 0FFFFFFF0h;
        add         esp, 4;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 158h;
        mov         eax, dword ptr [ebx+0Ch];
        mov         ecx, dword ptr [ebx+8];
        mov         edx, dword ptr [ebx+10h];
        mov         dword ptr [ebp-120h], eax;
        mov         eax, dword ptr [ebx+14h];
        push        esi;
        mov         esi, dword ptr [ebx+1Ch];
        push        edi;
        mov         dword ptr [ebp-11Ch], eax;
        mov         eax, dword ptr [ebx+18h];
        mov         dword ptr [ebp-124h], eax;
        push        0Ah;
        mov         eax, esi;
        pop         edi;
        and         eax, 0FFFFFFFCh;
        cmp         edi, esi;
        sbb         edi, edi;
        and         edi, eax;
        sub         esi, edi;
        shr         edi, 2;
        mov         dword ptr [ebp-114h], ecx;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebx+1Ch], esi;
        mov         dword ptr [ebp-12Ch], ecx;
        test        edi, edi;
        je          label1;
        push        4;
        push        4;
        push        dword ptr [ebp-124h];
        lea         ecx, [ebp-110h];
        call        aligned_matrix;
        mov         ecx, dword ptr [ebp-11Ch];
        mov         edx, dword ptr [ebp-120h];
        imul        edx, edx,3;
        mov         eax, ecx;
        imul        eax, eax,3;
        mov         dword ptr [ebp-130h], edx;
        mov         edx, dword ptr [ebp-118h];
        mov         dword ptr [ebp-128h], eax;
        test        edi, edi;
        je          label2;
        movaps      xmm6, xmmword ptr [ebp-0A0h];
        movaps      xmm7, xmmword ptr [ebp-0C0h];
        movaps      xmm1, xmmword ptr [ebp-0E0h];
        movaps      xmm3, xmmword ptr [ebp-110h];
        mov         esi, dword ptr [ebp-11Ch];
        shl         ecx, 2;
        mov         dword ptr [ebp-134h], ecx;
        mov         ecx, dword ptr [ebp-114h];
    label3:
        movsd       xmm0, mmword ptr [edx+esi*2];
        movsd       xmm5, mmword ptr [edx];
        movhps      xmm5, qword ptr [esi+edx];
        mov         eax, dword ptr [ebp-128h];
        movhps      xmm0, qword ptr [edx+eax];
        mov         eax, dword ptr [ebp-120h];
        mov         esi, dword ptr [ebp-130h];
        add         edx, dword ptr [ebp-134h];
        movaps      xmm4, xmm5;
        shufps      xmm4, xmm0, 88h;
        shufps      xmm5, xmm0, 0DDh;
        mulps       xmm1, xmm4;
        movaps      xmm0, xmm6;
        mulps       xmm0, xmm5;
        addps       xmm1, xmm0;
        addps       xmm1, xmmword ptr [ebp-20h];
        rcpps       xmm2, xmm1;
        movaps      xmm0, xmm2;
        mulps       xmm0, xmm1;
        movaps      xmm1, xmmword ptr [ebp-100h];
        mulps       xmm0, xmm2;
        mulps       xmm3, xmm4;
        addps       xmm2, xmm2;
        subps       xmm2, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0D0h];
        mulps       xmm0, xmm5;
        addps       xmm3, xmm0;
        addps       xmm3, xmmword ptr [ebp-50h];
        mulps       xmm1, xmm4;
        movaps      xmm0, xmm7;
        mulps       xmm0, xmm5;
        addps       xmm1, xmm0;
        addps       xmm1, xmmword ptr [ebp-40h];
        mulps       xmm3, xmm2;
        mulps       xmm1, xmm2;
        movaps      xmm0, xmm3;
        unpcklps    xmm0, xmm1;
        movlps      qword ptr [ecx], xmm0;
        unpckhps    xmm3, xmm1;
        movaps      xmm1, xmmword ptr [ebp-0E0h];
        movhps      qword ptr [ecx+eax], xmm0;
        movlps      qword ptr [ecx+eax*2], xmm3;
        movhps      qword ptr [ecx+esi], xmm3;
        movaps      xmm3, xmmword ptr [ebp-110h];
        mov         esi, eax;
        shl         esi, 2;
        add         ecx, esi;
        dec         edi;
        mov         esi, dword ptr [ebp-11Ch];
        jne         label3;
        mov         esi, dword ptr [ebx+1Ch];
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jmp         label1;
    label2:
        mov         ecx, dword ptr [ebp-114h];
    label1:
        test        esi, esi;
        je          label4;
        mov         edi, dword ptr [ebp-120h];
    label5:
        push        dword ptr [ebp-124h];
        push        edx;
        push        ecx;
        call        sse_vec2transformcoord;
        mov         edx, dword ptr [ebp-118h];
        mov         ecx, dword ptr [ebp-114h];
        add         edx, dword ptr [ebp-11Ch];
        add         ecx, edi;
        dec         esi;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jne         label5;
    label4:
        mov         ecx, dword ptr [ebp-4];
        mov         eax, dword ptr [ebp-12Ch];
        pop         edi;
        xor         ecx, ebp;
        pop         esi;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         18h;
    }
}

__gs_naked vec2* __stdcall sse2_vec2transformnormalarray(vec2* o, uint ostride, const vec2* v, uint vstride, const matrix* m, uint n)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ecx;
        push        ecx;
        and         esp, 0FFFFFFF0h;
        add         esp, 4;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 158h;
        mov         eax, dword ptr [ebx+0Ch];
        mov         ecx, dword ptr [ebx+8];
        mov         edx, dword ptr [ebx+10h];
        mov         dword ptr [ebp-120h], eax;
        mov         eax, dword ptr [ebx+14h];
        push        esi;
        mov         esi, dword ptr [ebx+1Ch];
        push        edi;
        mov         dword ptr [ebp-11Ch], eax;
        mov         eax, dword ptr [ebx+18h];
        mov         dword ptr [ebp-124h], eax;
        push        0Ah;
        mov         eax, esi;
        pop         edi;
        and         eax, 0FFFFFFFCh;
        cmp         edi, esi;
        sbb         edi, edi;
        and         edi, eax;
        sub         esi, edi;
        shr         edi, 2;
        mov         dword ptr [ebp-114h], ecx;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebx+1Ch], esi;
        mov         dword ptr [ebp-12Ch], ecx;
        test        edi, edi;
        je          label1;
        push        2;
        push        2;
        push        dword ptr [ebp-124h];
        lea         ecx, [ebp-110h];
        call        aligned_matrix;
        mov         ecx, dword ptr [ebp-11Ch];
        mov         edx, dword ptr [ebp-120h];
        imul        edx, edx,3;
        mov         eax, ecx;
        imul        eax, eax,3;
        mov         dword ptr [ebp-130h], edx;
        mov         edx, dword ptr [ebp-118h];
        mov         dword ptr [ebp-128h], eax;
        test        edi, edi;
        je          label2;
        movaps      xmm5, xmmword ptr [ebp-0C0h];
        movaps      xmm6, xmmword ptr [ebp-0D0h];
        movaps      xmm7, xmmword ptr [ebp-100h];
        movaps      xmm2, xmmword ptr [ebp-110h];
        mov         esi, dword ptr [ebp-11Ch];
        shl         ecx, 2;
        mov         dword ptr [ebp-134h], ecx;
        mov         ecx, dword ptr [ebp-114h];
    label3:
        movsd       xmm0, mmword ptr [edx+esi*2];
        movsd       xmm3, mmword ptr [edx];
        movhps      xmm3, qword ptr [esi+edx];
        mov         eax, dword ptr [ebp-128h];
        movhps      xmm0, qword ptr [edx+eax];
        mov         eax, dword ptr [ebp-120h];
        mov         esi, dword ptr [ebp-130h];
        add         edx, dword ptr [ebp-134h];
        movaps      xmm4, xmm2;
        movaps      xmm1, xmm3;
        shufps      xmm1, xmm0, 88h;
        shufps      xmm3, xmm0, 0DDh;
        mulps       xmm4, xmm1;
        movaps      xmm0, xmm6;
        mulps       xmm0, xmm3;
        addps       xmm4, xmm0;
        movaps      xmm2, xmm7;
        mulps       xmm2, xmm1;
        movaps      xmm0, xmm5;
        mulps       xmm0, xmm3;
        addps       xmm2, xmm0;
        movaps      xmm0, xmm4;
        unpcklps    xmm0, xmm2;
        movlps      qword ptr [ecx], xmm0;
        movhps      qword ptr [eax+ecx], xmm0;
        unpckhps    xmm4, xmm2;
        movaps      xmm2, xmmword ptr [ebp-110h];
        movlps      qword ptr [ecx+eax*2], xmm4;
        movhps      qword ptr [ecx+esi], xmm4;
        mov         esi, eax;
        shl         esi, 2;
        add         ecx, esi;
        dec         edi;
        mov         esi, dword ptr [ebp-11Ch];
        jne         label3;
        mov         esi, dword ptr [ebx+1Ch];
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jmp         label1;
    label2:
        mov         ecx, dword ptr [ebp-114h];
    label1:
        test        esi, esi;
        je          label4;
        mov         edi, dword ptr [ebp-120h];
    label5:
        push        dword ptr [ebp-124h];
        push        edx;
        push        ecx;
        call        sse_vec2transformnormal;
        mov         edx, dword ptr [ebp-118h];
        mov         ecx, dword ptr [ebp-114h];
        add         edx, dword ptr [ebp-11Ch];
        add         ecx, edi;
        dec         esi;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jne         label5;
    label4:
        mov         ecx, dword ptr [ebp-4];
        mov         eax, dword ptr [ebp-12Ch];
        pop         edi;
        xor         ecx, ebp;
        pop         esi;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         18h;
    }
}

__gs_naked vec3* __stdcall sse2_vec3normalize(vec3* o, const vec3* v)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         eax, dword ptr [ebp+0Ch];
        movss       xmm0, dword ptr [eax];
        movss       xmm1, dword ptr [eax+4];
        movss       xmm2, dword ptr [eax+8];
        movaps      xmm3, xmm0;
        movaps      xmm4, xmm1;
        movaps      xmm5, xmm2;
        mulss       xmm0, xmm0;
        mulss       xmm1, xmm1;
        mulss       xmm2, xmm2;
        addss       xmm0, xmm1;
        addss       xmm0, xmm2;
        mov         ecx, dword ptr [ebp+8];
        comiss      xmm0, dword ptr [s1d42];
        jae         label1;
        xor         eax, eax;
        mov         dword ptr [ecx], eax;
        mov         dword ptr [ecx+4], eax;
        mov         dword ptr [ecx+8], eax;
        jmp         label2;
        nop;
    label1:
        rsqrtss     xmm6, xmm0;
        movaps      xmm7, xmm6;
        mulss       xmm6, xmm0;
        mulss       xmm6, xmm7;
        movss       xmm1, dword ptr [s3d00];
        mulss       xmm7, dword ptr [s0d50];
        subss       xmm1, xmm6;
        mulss       xmm1, xmm7;
        mulss       xmm3, xmm1;
        mulss       xmm4, xmm1;
        mulss       xmm5, xmm1;
        movss       dword ptr [ecx], xmm3;
        movss       dword ptr [ecx+4], xmm4;
        movss       dword ptr [ecx+8], xmm5;
    label2:
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         8;
    }
}

__gs_naked vec3* __stdcall sse_vec3hermite(vec3* o, const vec3* v1, const vec3* t1, const vec3* v2, const vec3* t2, float s)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        movss       xmm0, dword ptr [ebp+1Ch];
        movss       xmm1, xmm0;
        mulss       xmm1, xmm0;
        movss       xmm2, xmm0;
        shufps      xmm0, xmm0,0;
        mulss       xmm2, xmm1;
        shufps      xmm1, xmm1,0;
        mulps       xmm0, xmmword ptr [shs1];
        shufps      xmm2, xmm2,0;
        mulps       xmm1, xmmword ptr [shs2];
        addss       xmm0, dword ptr [shs0];
        mulps       xmm2, xmmword ptr [shs3];
        addps       xmm0, xmm1;
        addps       xmm0, xmm2;
        mov         eax, dword ptr [ebp+0Ch];
        mov         ecx, dword ptr [ebp+10h];
        mov         edx, dword ptr [ebp+14h];
        movaps      xmm1, xmm0;
        shufps      xmm1, xmm0, 55h;
        movaps      xmm2, xmm0;
        shufps      xmm2, xmm0, 0AAh;
        movaps      xmm3, xmm0;
        shufps      xmm3, xmm0, 0FFh;
        shufps      xmm0, xmm0, 0;
        movss       xmm4, dword ptr [eax+8];
        movhps      xmm4, qword ptr [eax];
        mov         eax, dword ptr [ebp+18h];
        mulps       xmm4, xmm0;
        movss       xmm5, dword ptr [ecx+8];
        movhps      xmm5, qword ptr [ecx];
        mulps       xmm5, xmm1;
        movss       xmm6, dword ptr [edx+8];
        movhps      xmm6, qword ptr [edx];
        addps       xmm4, xmm5;
        mulps       xmm6, xmm2;
        addps       xmm4, xmm6;
        movss       xmm7, dword ptr [eax+8];
        movhps      xmm7, qword ptr [eax];
        mov         ecx, dword ptr [ebp+8];
        mulps       xmm7, xmm3;
        addps       xmm4, xmm7;
        movhps      qword ptr [ecx], xmm4;
        movss       dword ptr [ecx+8], xmm4;
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         18h;
    }
}

__gs_naked vec3* __stdcall sse_vec3catmullrom(vec3* o, const vec3* v1, const vec3* v2, const vec3* v3, const vec3* v4, float s)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        movss       xmm0, dword ptr [ebp+1Ch];
        movss       xmm1, xmm0;
        mulss       xmm1, xmm0;
        movss       xmm2, xmm0;
        shufps      xmm0, xmm0, 0;
        mulss       xmm2, xmm1;
        shufps      xmm1, xmm1, 0;
        mulps       xmm0, xmmword ptr [crs1];
        shufps      xmm2, xmm2, 0;
        mulps       xmm1, xmmword ptr [crs2];
        addps       xmm0, xmmword ptr [crs0];
        mulps       xmm2, xmmword ptr [crs3];
        addps       xmm0, xmm1;
        addps       xmm0, xmm2;
        mov         eax, dword ptr [ebp+0Ch];
        mov         ecx, dword ptr [ebp+10h];
        mov         edx, dword ptr [ebp+14h];
        movaps      xmm1, xmm0;
        shufps      xmm1, xmm0, 55h;
        movaps      xmm2, xmm0;
        shufps      xmm2, xmm0, 0AAh;
        movaps      xmm3, xmm0;
        shufps      xmm3, xmm0, 0FFh;
        shufps      xmm0, xmm0, 0;
        movss       xmm4, dword ptr [eax+8];
        movhps      xmm4, qword ptr [eax];
        mov         eax, dword ptr [ebp+18h];
        mulps       xmm4, xmm0;
        movss       xmm5, dword ptr [ecx+8];
        movhps      xmm5, qword ptr [ecx];
        mulps       xmm5, xmm1;
        movss       xmm6, dword ptr [edx+8];
        movhps      xmm6, qword ptr [edx];
        addps       xmm4, xmm5;
        mulps       xmm6, xmm2;
        addps       xmm4, xmm6;
        movss       xmm7, dword ptr [eax+8];
        movhps      xmm7, qword ptr [eax];
        mov         ecx, dword ptr [ebp+8];
        mulps       xmm7, xmm3;
        addps       xmm4, xmm7;
        movhps      qword ptr [ecx], xmm4;
        movss       dword ptr [ecx+8], xmm4;
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         18h;
    }
}

__gs_naked vec3* __stdcall sse_vec3barycentric(vec3* o, const vec3* v1, const vec3* v2, const vec3* v3, float f, float g)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        movss       xmm0, dword ptr [ebp+18h];
        mov         eax, dword ptr [ebp+0Ch];
        shufps      xmm0, xmm0, 0;
        movss       xmm2, dword ptr [eax+8];
        mov         ecx, dword ptr [ebp+10h];
        movhps      xmm2, qword ptr [eax];
        movss       xmm1, dword ptr [ebp+1Ch];
        movss       xmm3, dword ptr [ecx+8];
        movhps      xmm3, qword ptr [ecx];
        shufps      xmm1, xmm1,0;
        mov         eax, dword ptr [ebp+14h];
        subps       xmm3, xmm2;
        movss       xmm4, dword ptr [eax+8];
        movhps      xmm4, qword ptr [eax];
        mulps       xmm3, xmm0;
        subps       xmm4, xmm2;
        mov         eax, dword ptr [ebp+8];
        addps       xmm3, xmm2;
        mulps       xmm4, xmm1;
        addps       xmm3, xmm4;
        movss       dword ptr [eax+8], xmm3;
        movhps      qword ptr [eax], xmm3;
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         18h;
    }
}

__gs_naked vec4* __stdcall sse2_vec3transform(vec4* o, const vec3* v, const matrix* m)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ecx;
        push        ecx;
        and         esp, 0FFFFFFF0h;
        add         esp, 4;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        mov         edx, dword ptr [ebx+10h];
        mov         ecx, dword ptr [ebx+0Ch];
        mov         eax, dword ptr [ebx+8];
        sub         esp, 10h;
        test        dl, 0Fh;
        jne         label1;
        movlps      xmm0, qword ptr [ecx];
        movaps      xmm1, xmm0;
        shufps      xmm1, xmm0, 55h;
        mulps       xmm1, xmmword ptr [edx+10h];
        shufps      xmm0, xmm0, 0;
        mulps       xmm0, xmmword ptr [edx];
        addps       xmm1, xmm0;
        movss       xmm0, dword ptr [ecx+8];
        shufps      xmm0, xmm0, 0;
        mulps       xmm0, xmmword ptr [edx+20h];
        addps       xmm0, xmmword ptr [edx+30h];
        addps       xmm1, xmm0;
        movups      xmmword ptr [eax], xmm1;
        jmp         label2;
    label1:
        movlps      xmm1, qword ptr [ecx];
        movsd       xmm2, mmword ptr [edx+10h];
        movhps      xmm2, qword ptr [edx+18h];
        movaps      xmm0, xmm1;
        shufps      xmm0, xmm1, 55h;
        mulps       xmm2, xmm0;
        movsd       xmm0, mmword ptr [edx];
        movhps      xmm0, qword ptr [edx+8];
        shufps      xmm1, xmm1, 0;
        mulps       xmm0, xmm1;
        movsd       xmm1, mmword ptr [edx+20h];
        movhps      xmm1, qword ptr [edx+28h];
        addps       xmm2, xmm0;
        movss       xmm0, dword ptr [ecx+8];
        shufps      xmm0, xmm0, 0;
        mulps       xmm1, xmm0;
        movsd       xmm0, mmword ptr [edx+30h];
        movhps      xmm0, qword ptr [edx+38h];
        addps       xmm1, xmm0;
        addps       xmm2, xmm1;
        movups      xmmword ptr [eax], xmm2;
    label2:
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         0Ch;
    }
}

__gs_naked vec3* __stdcall sse_vec3transformcoord(vec3* o, const vec3* v, const matrix* m)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ecx;
        push        ecx;
        and         esp, 0FFFFFFF0h;
        add         esp, 4;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        mov         ecx, dword ptr [ebx+10h];
        mov         eax, dword ptr [ebx+0Ch];
        sub         esp, 10h;
        test        cl, 0Fh;
        jne         label1;
        movlps      xmm0, qword ptr [eax];
        movaps      xmm2, xmm0;
        shufps      xmm2, xmm0, 55h;
        mulps       xmm2, xmmword ptr [ecx+10h];
        shufps      xmm0, xmm0, 0;
        mulps       xmm0, xmmword ptr [ecx];
        addps       xmm2, xmm0;
        movss       xmm0, dword ptr [eax+8];
        shufps      xmm0, xmm0, 0;
        mulps       xmm0, xmmword ptr [ecx+20h];
        addps       xmm0, xmmword ptr [ecx+30h];
        addps       xmm2, xmm0;
        jmp         label2;
    label1:
        movlps      xmm1, qword ptr [eax];
        movsd       xmm2, mmword ptr [ecx+10h];
        movhps      xmm2, qword ptr [ecx+18h];
        movaps      xmm0, xmm1;
        shufps      xmm0, xmm1, 55h;
        mulps       xmm2, xmm0;
        movsd       xmm0, mmword ptr [ecx];
        movhps      xmm0, qword ptr [ecx+8];
        shufps      xmm1, xmm1, 0;
        mulps       xmm0, xmm1;
        movsd       xmm1, mmword ptr [ecx+20h];
        movhps      xmm1, qword ptr [ecx+28h];
        addps       xmm2, xmm0;
        movss       xmm0, dword ptr [eax+8];
        shufps      xmm0, xmm0, 0;
        mulps       xmm1, xmm0;
        movsd       xmm0, mmword ptr [ecx+30h];
        movhps      xmm0, qword ptr [ecx+38h];
        addps       xmm1, xmm0;
        addps       xmm2, xmm1;
    label2:
        mov         eax, dword ptr [ebx+8];
        rcpps       xmm1, xmm2;
        movaps      xmm0, xmm1;
        mulps       xmm0, xmm2;
        mulps       xmm0, xmm1;
        addps       xmm1, xmm1;
        subps       xmm1, xmm0;
        shufps      xmm1, xmm1, 0FFh;
        mulps       xmm1, xmm2;
        movlps      qword ptr [eax], xmm1;
        movhlps     xmm1, xmm1;
        movss       dword ptr [eax+8], xmm1;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         0Ch;
    }
}

__gs_naked vec3* __stdcall sse2_vec3transformnormal(vec3* o, const vec3* v, const matrix* m)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ecx;
        push        ecx;
        and         esp, 0FFFFFFF0h;
        add         esp, 4;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        mov         ecx, dword ptr [ebx+10h];
        mov         eax, dword ptr [ebx+0Ch];
        sub         esp, 10h;
        test        cl, 0Fh;
        jne         label1;
        movlps      xmm0, qword ptr [eax];
        movaps      xmm1, xmm0;
        shufps      xmm1, xmm0, 55h;
        mulps       xmm1, xmmword ptr [ecx+10h];
        shufps      xmm0, xmm0, 0;
        mulps       xmm0, xmmword ptr [ecx];
        addps       xmm1, xmm0;
        movss       xmm0, dword ptr [eax+8];
        mov         eax, dword ptr [ebx+8];
        shufps      xmm0, xmm0, 0;
        mulps       xmm0, xmmword ptr [ecx+20h];
        addps       xmm1, xmm0;
        movlps      qword ptr [eax], xmm1;
        movhlps     xmm1, xmm1;
        movss       dword ptr [eax+8], xmm1;
        jmp         label2;
    label1:
        movlps      xmm1, qword ptr [eax];
        movsd       xmm2, mmword ptr [ecx+10h];
        movhps      xmm2, qword ptr [ecx+18h];
        movaps      xmm0, xmm1;
        shufps      xmm0, xmm1, 55h;
        mulps       xmm2, xmm0;
        movsd       xmm0, mmword ptr [ecx];
        movhps      xmm0, qword ptr [ecx+8];
        shufps      xmm1, xmm1, 0;
        mulps       xmm0, xmm1;
        movsd       xmm1, mmword ptr [ecx+20h];
        movhps      xmm1, qword ptr [ecx+28h];
        addps       xmm2, xmm0;
        movss       xmm0, dword ptr [eax+8];
        mov         eax, dword ptr [ebx+8];
        shufps      xmm0, xmm0, 0;
        mulps       xmm1, xmm0;
        addps       xmm2, xmm1;
        movlps      qword ptr [eax], xmm2;
        movhlps     xmm2, xmm2;
        movss       dword ptr [eax+8], xmm2;
    label2:
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         0Ch;
    }
}

__gs_naked vec4* __stdcall sse2_vec3transformarray(vec4* o, uint ostride, const vec3* v, uint vstride, const matrix* m, uint n)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ecx;
        push        ecx;
        and         esp, 0FFFFFFF0h;
        add         esp, 4;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 158h;
        mov         eax, dword ptr [ebx+0Ch];
        mov         ecx, dword ptr [ebx+8];
        mov         edx, dword ptr [ebx+10h];
        mov         dword ptr [ebp-120h], eax;
        mov         eax, dword ptr [ebx+14h];
        push        esi;
        mov         esi, dword ptr [ebx+1Ch];
        push        edi;
        mov         dword ptr [ebp-11Ch], eax;
        mov         eax, dword ptr [ebx+18h];
        mov         dword ptr [ebp-124h], eax;
        push        0Ah;
        mov         eax, esi;
        pop         edi;
        and         eax, 0FFFFFFFCh;
        cmp         edi, esi;
        sbb         edi, edi;
        and         edi, eax;
        sub         esi, edi;
        shr         edi, 2;
        mov         dword ptr [ebp-114h], ecx;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebx+1Ch], esi;
        mov         dword ptr [ebp-12Ch], ecx;
        test        edi, edi;
        je          label1;
        push        4;
        push        4;
        push        dword ptr [ebp-124h];
        lea         ecx, [ebp-110h];
        call        aligned_matrix;
        mov         ecx, dword ptr [ebp-11Ch];
        mov         edx, dword ptr [ebp-120h];
        imul        edx, edx, 3;
        mov         eax, ecx;
        imul        eax, eax, 3;
        mov         dword ptr [ebp-130h], edx;
        mov         edx, dword ptr [ebp-118h];
        mov         dword ptr [ebp-128h], eax;
        test        edi, edi;
        je          label2;
        movaps      xmm3, xmmword ptr [ebp-80h];
        movaps      xmm4, xmmword ptr [ebp-90h];
        movaps      xmm7, xmmword ptr [ebp-110h];
        mov         esi, dword ptr [ebp-11Ch];
        shl         ecx, 2;
        mov         dword ptr [ebp-134h], ecx;
        mov         ecx, dword ptr [ebp-114h];
    label3:
        movsd       xmm0, mmword ptr [edx+esi*2];
        movsd       xmm2, mmword ptr [edx];
        movhps      xmm2, qword ptr [esi+edx];
        movss       xmm1, dword ptr [edx+esi*2+8];
        movss       xmm5, dword ptr [edx+8];
        mov         eax, dword ptr [ebp-128h];
        movhps      xmm0, qword ptr [edx+eax];
        movaps      xmm6, xmm2;
        shufps      xmm2, xmm0, 0DDh;
        shufps      xmm6, xmm0, 88h;
        movss       xmm0, dword ptr [esi+edx+8];
        unpcklps    xmm5, xmm0;
        movss       xmm0, dword ptr [eax+edx+8];
        unpcklps    xmm1, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0D0h];
        mulps       xmm0, xmm2;
        mov         eax, dword ptr [ebp-120h];
        mov         esi, dword ptr [ebp-130h];
        mulps       xmm7, xmm6;
        addps       xmm7, xmm0;
        movaps      xmm0, xmm4;
        movaps      xmm4, xmmword ptr [ebp-100h];
        movlhps     xmm5, xmm1;
        mulps       xmm0, xmm5;
        addps       xmm7, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0C0h];
        addps       xmm7, xmmword ptr [ebp-50h];
        mulps       xmm0, xmm2;
        mulps       xmm4, xmm6;
        addps       xmm4, xmm0;
        movaps      xmm0, xmm3;
        movaps      xmm3, xmmword ptr [ebp-0F0h];
        mulps       xmm0, xmm5;
        addps       xmm4, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0B0h];
        addps       xmm4, xmmword ptr [ebp-40h];
        mulps       xmm0, xmm2;
        add         edx, dword ptr [ebp-134h];
        mulps       xmm3, xmm6;
        addps       xmm3, xmm0;
        movaps      xmm0, xmmword ptr [ebp-70h];
        mulps       xmm0, xmm5;
        addps       xmm3, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0A0h];
        addps       xmm3, xmmword ptr [ebp-30h];
        movaps      xmmword ptr [ebp-150h], xmm2;
        mulps       xmm0, xmmword ptr [ebp-150h];
        movaps      xmm2, xmmword ptr [ebp-0E0h];
        mulps       xmm2, xmm6;
        addps       xmm2, xmm0;
        movaps      xmm0, xmmword ptr [ebp-60h];
        mulps       xmm0, xmm5;
        addps       xmm2, xmm0;
        addps       xmm2, xmmword ptr [ebp-20h];
        movaps      xmm1, xmm7;
        unpcklps    xmm1, xmm4;
        movlps      qword ptr [ecx], xmm1;
        movaps      xmm0, xmm3;
        unpcklps    xmm0, xmm2;
        movlps      qword ptr [ecx+8], xmm0;
        movhps      qword ptr [ecx+eax], xmm1;
        movhps      qword ptr [eax+ecx+8], xmm0;
        unpckhps    xmm7, xmm4;
        movaps      xmm4, xmmword ptr [ebp-90h];
        movlps      qword ptr [ecx+eax*2], xmm7;
        unpckhps    xmm3, xmm2;
        movlps      qword ptr [ecx+eax*2+8], xmm3;
        movhps      qword ptr [ecx+esi], xmm7;
        movaps      xmm7, xmmword ptr [ebp-110h];
        movhps      qword ptr [esi+ecx+8], xmm3;
        movaps      xmm3, xmmword ptr [ebp-80h];
        mov         esi, eax;
        shl         esi, 2;
        add         ecx, esi;
        dec         edi;
        mov         esi, dword ptr [ebp-11Ch];
        jne         label3;
        mov         esi, dword ptr [ebx+1Ch];
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jmp         label1;
    label2:
        mov         ecx, dword ptr [ebp-114h];
    label1:
        test        esi, esi;
        je          label4;
        mov         edi, dword ptr [ebp-120h];
    label5:
        push        dword ptr [ebp-124h];
        push        edx;
        push        ecx;
        call        sse2_vec3transform;
        mov         edx, dword ptr [ebp-118h];
        mov         ecx, dword ptr [ebp-114h];
        add         edx, dword ptr [ebp-11Ch];
        add         ecx, edi;
        dec         esi;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jne         label5;
    label4:
        mov         ecx, dword ptr [ebp-4];
        mov         eax, dword ptr [ebp-12Ch];
        pop         edi;
        xor         ecx, ebp;
        pop         esi;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         18h;
    }
}

__gs_naked vec3* __stdcall sse2_vec3transformcoordarray(vec3* os, uint ostride, const vec3* v, uint vstride, const matrix* m, uint n)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ecx;
        push        ecx;
        and         esp, 0FFFFFFF0h;
        add         esp, 4;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 158h;
        mov         eax, dword ptr [ebx+0Ch];
        mov         ecx, dword ptr [ebx+8];
        mov         edx, dword ptr [ebx+10h];
        mov         dword ptr [ebp-120h], eax;
        mov         eax, dword ptr [ebx+14h];
        push        esi;
        mov         esi, dword ptr [ebx+1Ch];
        push        edi;
        mov         dword ptr [ebp-11Ch], eax;
        mov         eax, dword ptr [ebx+18h];
        mov         dword ptr [ebp-124h], eax;
        push        0Ah;
        mov         eax, esi;
        pop         edi;
        and         eax, 0FFFFFFFCh;
        cmp         edi, esi;
        sbb         edi, edi;
        and         edi, eax;
        sub         esi, edi;
        shr         edi, 2;
        mov         dword ptr [ebp-114h], ecx;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebx+1Ch], esi;
        mov         dword ptr [ebp-12Ch], ecx;
        test        edi, edi;
        je          label1;
        push        4;
        push        4;
        push        dword ptr [ebp-124h];
        lea         ecx, [ebp-110h];
        call        aligned_matrix;
        mov         ecx, dword ptr [ebp-11Ch];
        mov         edx, dword ptr [ebp-120h];
        imul        edx, edx,3;
        mov         eax, ecx;
        imul        eax, eax,3;
        mov         dword ptr [ebp-130h], edx;
        mov         edx, dword ptr [ebp-118h];
        mov         dword ptr [ebp-128h], eax;
        test        edi, edi;
        je          label2;
        movaps      xmm2, xmmword ptr [ebp-60h];
        movaps      xmm4, xmmword ptr [ebp-0A0h];
        movaps      xmm5, xmmword ptr [ebp-0E0h];
        mov         esi, dword ptr [ebp-11Ch];
        shl         ecx, 2;
        mov         dword ptr [ebp-134h], ecx;
        mov         ecx, dword ptr [ebp-114h];
    label3:
        movsd       xmm0, mmword ptr [edx+esi*2];
        movsd       xmm3, mmword ptr [edx];
        movhps      xmm3, qword ptr [esi+edx];
        movss       xmm6, dword ptr [edx+8];
        movss       xmm1, dword ptr [edx+esi*2+8];
        mov         eax, dword ptr [ebp-128h];
        movhps      xmm0, qword ptr [edx+eax];
        movaps      xmm7, xmm3;
        shufps      xmm3, xmm0, 0DDh;
        shufps      xmm7, xmm0, 88h;
        movss       xmm0, dword ptr [esi+edx+8];
        unpcklps    xmm6, xmm0;
        movss       xmm0, dword ptr [eax+edx+8];
        unpcklps    xmm1, xmm0;
        movlhps     xmm6, xmm1;
        movaps      xmm0, xmm4;
        mulps       xmm0, xmm3;
        movaps      xmm4, xmmword ptr [ebp-110h];
        mov         eax, dword ptr [ebp-120h];
        movaps      xmm1, xmm5;
        mulps       xmm1, xmm7;
        addps       xmm1, xmm0;
        movaps      xmm0, xmm2;
        mulps       xmm0, xmm6;
        addps       xmm1, xmm0;
        addps       xmm1, xmmword ptr [ebp-20h];
        movaps      xmm2, xmmword ptr [ebp-100h];
        rcpps       xmm5, xmm1;
        mov         esi, dword ptr [ebp-130h];
        movaps      xmm0, xmm5;
        mulps       xmm0, xmm1;
        mulps       xmm0, xmm5;
        mulps       xmm4, xmm7;
        addps       xmm5, xmm5;
        subps       xmm5, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0D0h];
        mulps       xmm0, xmm3;
        addps       xmm4, xmm0;
        movaps      xmm0, xmmword ptr [ebp-90h];
        mulps       xmm0, xmm6;
        addps       xmm4, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0C0h];
        addps       xmm4, xmmword ptr [ebp-50h];
        mulps       xmm0, xmm3;
        add         edx, dword ptr [ebp-134h];
        movaps      xmmword ptr [ebp-150h], xmm3;
        movaps      xmm3, xmmword ptr [ebp-0F0h];
        mulps       xmm2, xmm7;
        addps       xmm2, xmm0;
        movaps      xmm0, xmmword ptr [ebp-80h];
        mulps       xmm0, xmm6;
        addps       xmm2, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0B0h];
        mulps       xmm0, xmmword ptr [ebp-150h];
        addps       xmm2, xmmword ptr [ebp-40h];
        mulps       xmm3, xmm7;
        addps       xmm3, xmm0;
        movaps      xmm0, xmmword ptr [ebp-70h];
        mulps       xmm0, xmm6;
        addps       xmm3, xmm0;
        addps       xmm3, xmmword ptr [ebp-30h];
        mulps       xmm4, xmm5;
        mulps       xmm2, xmm5;
        mulps       xmm3, xmm5;
        movaps      xmm0, xmm4;
        unpcklps    xmm0, xmm2;
        movlps      qword ptr [ecx], xmm0;
        movhps      qword ptr [eax+ecx], xmm0;
        movaps      xmm1, xmm3;
        unpcklps    xmm1, xmm5;
        movss       dword ptr [ecx+8], xmm1;
        movhlps     xmm1, xmm1;
        movss       dword ptr [eax+ecx+8], xmm1;
        unpckhps    xmm4, xmm2;
        movlps      qword ptr [ecx+eax*2], xmm4;
        movhps      qword ptr [ecx+esi], xmm4;
        unpckhps    xmm3, xmm5;
        movss       dword ptr [ecx+eax*2+8], xmm3;
        movhlps     xmm3, xmm3;
        movss       dword ptr [esi+ecx+8], xmm3;
        mov         esi, eax;
        movaps      xmm2, xmmword ptr [ebp-60h];
        movaps      xmm4, xmmword ptr [ebp-0A0h];
        movaps      xmm5, xmmword ptr [ebp-0E0h];
        shl         esi, 2;
        add         ecx, esi;
        dec         edi;
        mov         esi, dword ptr [ebp-11Ch];
        jne         label3;
        mov         esi, dword ptr [ebx+1Ch];
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jmp         label1;
    label2:
        mov         ecx, dword ptr [ebp-114h];
    label1:
        test        esi, esi;
        je          label4;
        mov         edi, dword ptr [ebp-120h];
    label5:
        push        dword ptr [ebp-124h];
        push        edx;
        push        ecx;
        call        sse_vec3transformcoord;
        mov         edx, dword ptr [ebp-118h];
        mov         ecx, dword ptr [ebp-114h];
        add         edx, dword ptr [ebp-11Ch];
        add         ecx, edi;
        dec         esi;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jne         label5;
    label4:
        mov         ecx, dword ptr [ebp-4];
        mov         eax, dword ptr [ebp-12Ch];
        pop         edi;
        xor         ecx, ebp;
        pop         esi;
        mov         esp,ebp;
        pop         ebp;
        mov         esp,ebx;
        pop         ebx;
        ret         18h;
    }
}

__gs_naked vec3* __stdcall sse2_vec3transformnormalarray(vec3* o, uint ostride, const vec3* v, uint vstride, const matrix* m, uint n)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ecx;
        push        ecx;
        and         esp, 0FFFFFFF0h;
        add         esp, 4;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 158h;
        mov         eax, dword ptr [ebx+0Ch];
        mov         ecx, dword ptr [ebx+8];
        mov         edx, dword ptr [ebx+10h];
        mov         dword ptr [ebp-120h], eax;
        mov         eax, dword ptr [ebx+14h];
        push        esi;
        mov         esi, dword ptr [ebx+1Ch];
        push        edi;
        mov         dword ptr [ebp-11Ch], eax;
        mov         eax, dword ptr [ebx+18h];
        mov         dword ptr [ebp-124h], eax;
        push        0Ah;
        mov         eax, esi;
        pop         edi;
        and         eax, 0FFFFFFFCh;
        cmp         edi, esi;
        sbb         edi, edi;
        and         edi, eax;
        sub         esi, edi;
        shr         edi, 2;
        mov         dword ptr [ebp-114h], ecx;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebx+1Ch], esi;
        mov         dword ptr [ebp-12Ch], ecx;
        test        edi, edi;
        je          label1;
        push        3;
        push        3;
        push        dword ptr [ebp-124h];
        lea         ecx, [ebp-110h];
        call        aligned_matrix;
        mov         ecx, dword ptr [ebp-11Ch];
        mov         edx, dword ptr [ebp-120h];
        imul        edx, edx,3;
        mov         eax, ecx;
        imul        eax, eax,3;
        mov         dword ptr [ebp-130h], edx;
        mov         edx, dword ptr [ebp-118h];
        mov         dword ptr [ebp-128h], eax;
        test        edi, edi;
        je          label2;
        movaps      xmm3, xmmword ptr [ebp-80h];
        movaps      xmm2, xmmword ptr [ebp-90h];
        movaps      xmm6, xmmword ptr [ebp-110h];
        mov         esi, dword ptr [ebp-11Ch];
        shl         ecx, 2;
        mov         dword ptr [ebp-134h], ecx;
        mov         ecx, dword ptr [ebp-114h];
    label3:
        movsd       xmm0, mmword ptr [edx+esi*2];
        movsd       xmm7, mmword ptr [edx];
        movhps      xmm7, qword ptr [esi+edx];
        movss       xmm1, dword ptr [edx+esi*2+8];
        movss       xmm4, dword ptr [edx+8];
        mov         eax, dword ptr [ebp-128h];
        movhps      xmm0, qword ptr [edx+eax];
        movaps      xmm5, xmm7;
        shufps      xmm5, xmm0, 88h;
        shufps      xmm7, xmm0, 0DDh;
        movss       xmm0, dword ptr [esi+edx+8];
        unpcklps    xmm4, xmm0;
        movss       xmm0, dword ptr [eax+edx+8];
        unpcklps    xmm1, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0D0h];
        mov         eax, dword ptr [ebp-120h];
        mulps       xmm0, xmm7;
        mov         esi, dword ptr [ebp-130h];
        movlhps     xmm4, xmm1;
        mulps       xmm6, xmm5;
        addps       xmm6, xmm0;
        movaps      xmm0, xmm2;
        movaps      xmm2, xmmword ptr [ebp-100h];
        mulps       xmm0, xmm4;
        addps       xmm6, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0C0h];
        mulps       xmm0, xmm7;
        add         edx, dword ptr [ebp-134h];
        mulps       xmm2, xmm5;
        addps       xmm2, xmm0;
        movaps      xmm0, xmm3;
        movaps      xmm3, xmmword ptr [ebp-0F0h];
        mulps       xmm0, xmm4;
        addps       xmm2, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0B0h];
        mulps       xmm3, xmm5;
        mulps       xmm0, xmm7;
        addps       xmm3, xmm0;
        movaps      xmm0, xmmword ptr [ebp-70h];
        mulps       xmm0, xmm4;
        addps       xmm3, xmm0;
        movaps      xmm0, xmm6;
        unpcklps    xmm0, xmm2;
        movlps      qword ptr [ecx], xmm0;
        movhps      qword ptr [eax+ecx], xmm0;
        movss       dword ptr [ecx+8], xmm3;
        movaps      xmm1, xmm3;
        unpcklps    xmm1, xmm3;
        unpckhps    xmm6, xmm2;
        movaps      xmm2, xmmword ptr [ebp-90h];
        movhlps     xmm1, xmm1;
        movss       dword ptr [eax+ecx+8], xmm1;
        movlps      qword ptr [ecx+eax*2], xmm6;
        movhps      qword ptr [ecx+esi], xmm6;
        movaps      xmm6, xmmword ptr [ebp-110h];
        unpckhps    xmm3, xmm3;
        movss       dword ptr [ecx+eax*2+8], xmm3;
        movhlps     xmm3, xmm3;
        movss       dword ptr [esi+ecx+8], xmm3;
        movaps      xmm3, xmmword ptr [ebp-80h];
        mov         esi, eax;
        shl         esi, 2;
        add         ecx, esi;
        dec         edi;
        mov         esi, dword ptr [ebp-11Ch];
        jne         label3;
        mov         esi, dword ptr [ebx+1Ch];
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jmp         label1;
    label2:
        mov         ecx, dword ptr [ebp-114h];
    label1:
        test        esi, esi;
        je          label4;
        mov         edi, dword ptr [ebp-120h];
    label5:
        push        dword ptr [ebp-124h];
        push        edx;
        push        ecx;
        call        sse2_vec3transformnormal;
        mov         edx, dword ptr [ebp-118h];
        mov         ecx, dword ptr [ebp-114h];
        add         edx, dword ptr [ebp-11Ch];
        add         ecx, edi;
        dec         esi;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jne         label5;
    label4:
        mov         ecx, dword ptr [ebp-4];
        mov         eax, dword ptr [ebp-12Ch];
        pop         edi;
        xor         ecx, ebp;
        pop         esi;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         18h;
    }
}

__gs_naked vec4* __stdcall sse2_vec4cross(vec4* o, const vec4* v1, const vec4* v2, const vec4* v3)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         eax, dword ptr [ebp+0Ch];
        movsd       xmm7, mmword ptr [eax];
        movhps      xmm7, qword ptr [eax+8];
        mov         eax, dword ptr [ebp+10h];
        movsd       xmm0, mmword ptr [eax];
        movhps      xmm0, qword ptr [eax+8];
        mov         eax, dword ptr [ebp+14h];
        movsd       xmm5, mmword ptr [eax];
        movhps      xmm5, qword ptr [eax+8];
        mov         eax, dword ptr [ebp+8];
        shufps      xmm0, xmm0, 39h;
        movaps      xmm6, xmm5;
        shufps      xmm5, xmm5, 4Eh;
        mulps       xmm6, xmm0;
        movaps      xmm4, xmm5;
        mulps       xmm4, xmm0;
        shufps      xmm5, xmm5, 39h;
        mulps       xmm5, xmm0;
        movaps      xmm2, xmm4;
        shufps      xmm2, xmm4, 39h;
        movaps      xmm0, xmm6;
        shufps      xmm0, xmm6, 4Eh;
        subps       xmm2, xmm0;
        shufps      xmm7, xmm7, 39h;
        movaps      xmm0, xmm7;
        shufps      xmm0, xmm7, 39h;
        movaps      xmm3, xmm5;
        shufps      xmm3, xmm5, 4Eh;
        subps       xmm3, xmm5;
        mulps       xmm3, xmm0;
        shufps      xmm6, xmm6, 39h;
        movaps      xmm1, xmm0;
        shufps      xmm1, xmm0, 39h;
        mulps       xmm7, xmm2;
        addps       xmm3, xmm7;
        subps       xmm4, xmm6;
        mulps       xmm4, xmm1;
        addps       xmm3, xmm4;
        xorps       xmm3, xmmword ptr [s0i0i];
        movups      xmmword ptr [eax], xmm3;
        pop         ebp;
        ret         10h;
    }
}

__gs_naked vec4* __stdcall sse_vec4normalize(vec4* o, const vec4* v)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         eax, dword ptr [ebp+0Ch];
        movss       xmm0, dword ptr [eax];
        movss       xmm1, dword ptr [eax+4];
        movss       xmm2, dword ptr [eax+8];
        movss       xmm3, dword ptr [eax+0Ch];
        movss       xmm4, xmm0;
        movss       xmm5, xmm1;
        movss       xmm6, xmm2;
        movss       xmm7, xmm3;
        mulss       xmm4, xmm0;
        mulss       xmm5, xmm1;
        mulss       xmm6, xmm2;
        addss       xmm4, xmm5;
        mulss       xmm7, xmm3;
        addss       xmm6, xmm7;
        addss       xmm4, xmm6;
        rsqrtss     xmm5, xmm4;
        movss       xmm6, xmm5;
        mulss       xmm5, xmm5;
        mulss       xmm4, xmm5;
        movss       xmm5, dword ptr [s3d00];
        subss       xmm5, xmm4;
        mulss       xmm5, dword ptr [s0d50];
        mulss       xmm5, xmm6;
        mov         eax, dword ptr [ebp+8];
        mulss       xmm0, xmm5;
        mulss       xmm1, xmm5;
        movss       dword ptr [eax], xmm0;
        mulss       xmm2, xmm5;
        movss       dword ptr [eax+4], xmm1;
        mulss       xmm3, xmm5;
        movss       dword ptr [eax+8], xmm2;
        movss       dword ptr [eax+0Ch], xmm3;
        mov         eax,dword ptr [ebp+8];
        pop         ebp;
        ret         8;
    }
}

__gs_naked vec4* __stdcall sse_vec4hermite(vec4* o, const vec4* v1, const vec4* t1, const vec4* v2, const vec4* t2, float s)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        movss       xmm0, dword ptr [ebp+1Ch];
        movss       xmm1, xmm0;
        mulss       xmm1, xmm0;
        movss       xmm2, xmm0;
        shufps      xmm0, xmm0, 0;
        mulss       xmm2, xmm1;
        shufps      xmm1, xmm1, 0;
        mulps       xmm0, xmmword ptr [shs1];
        shufps      xmm2, xmm2, 0;
        mulps       xmm1, xmmword ptr [shs2];
        addss       xmm0, dword ptr [shs0];
        mulps       xmm2, xmmword ptr [shs3];
        addps       xmm0, xmm1;
        addps       xmm0, xmm2;
        mov         eax, dword ptr [ebp+0Ch];
        movaps      xmm1, xmm0;
        shufps      xmm1, xmm0, 55h;
        movaps      xmm2, xmm0;
        mov         ecx, dword ptr [ebp+10h];
        shufps      xmm2, xmm0, 0AAh;
        movaps      xmm3, xmm0;
        shufps      xmm3, xmm0, 0FFh;
        mov         edx, dword ptr [ebp+14h];
        shufps      xmm0, xmm0, 0;
        movups      xmm4, xmmword ptr [eax];
        mov         eax, dword ptr [ebp+18h];
        mulps       xmm4, xmm0;
        movups      xmm5, xmmword ptr [ecx];
        mulps       xmm5, xmm1;
        movups      xmm6, xmmword ptr [edx];
        addps       xmm4, xmm5;
        mulps       xmm6, xmm2;
        addps       xmm4, xmm6;
        movups      xmm7, xmmword ptr [eax];
        mov         ecx, dword ptr [ebp+8];
        mulps       xmm7, xmm3;
        addps       xmm4, xmm7;
        movups      xmmword ptr [ecx], xmm4;
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         18h;
    }
}

__gs_naked vec4* __stdcall sse_vec4catmullrom(vec4* o, const vec4* v1, const vec4* v2, const vec4* v3, const vec4* v4, float s)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        movss       xmm0, dword ptr [ebp+1Ch];
        movss       xmm1, xmm0;
        mulss       xmm1, xmm0;
        movss       xmm2, xmm0;
        shufps      xmm0, xmm0, 0;
        mulss       xmm2, xmm1;
        shufps      xmm1, xmm1, 0;
        mulps       xmm0, xmmword ptr [crs1];
        shufps      xmm2, xmm2, 0;
        mulps       xmm1, xmmword ptr [crs2];
        addps       xmm0, xmmword ptr [crs0];
        mulps       xmm2, xmmword ptr [crs3];
        addps       xmm0, xmm1;
        addps       xmm0, xmm2;
        mov         eax, dword ptr [ebp+0Ch];
        xorps       xmm4, xmm4;
        mov         ecx, dword ptr [ebp+10h];
        mov         edx, dword ptr [ebp+14h];
        movaps      xmm1, xmm0;
        shufps      xmm1, xmm0, 55h;
        movaps      xmm2, xmm0;
        shufps      xmm2, xmm0, 0AAh;
        movaps      xmm3, xmm0;
        shufps      xmm3, xmm0, 0FFh;
        xorps       xmm5, xmm5;
        shufps      xmm0, xmm0, 0;
        movups      xmm4, xmmword ptr [eax];
        mov         eax, dword ptr [ebp+18h];
        mulps       xmm4, xmm0;
        xorps       xmm6, xmm6;
        movups      xmm5, xmmword ptr [ecx];
        mulps       xmm5, xmm1;
        xorps       xmm7, xmm7;
        movups      xmm6, xmmword ptr [edx];
        addps       xmm4, xmm5;
        mulps       xmm6, xmm2;
        addps       xmm4, xmm6;
        movups      xmm7, xmmword ptr [eax];
        mov         ecx, dword ptr [ebp+8];
        mulps       xmm7, xmm3;
        addps       xmm4, xmm7;
        movups      xmmword ptr [ecx], xmm4;
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         18h;
    }
}

__gs_naked vec4* __stdcall sse_vec4barycentric(vec4* o, const vec4* v1, const vec4* v2, const vec4* v3, float f, float g)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        movss       xmm0, dword ptr [ebp+18h];
        mov         eax, dword ptr [ebp+0Ch];
        shufps      xmm0, xmm0, 0;
        mov         ecx, dword ptr [ebp+10h];
        movups      xmm2, xmmword ptr [eax];
        movss       xmm1, dword ptr [ebp+1Ch];
        movups      xmm3, xmmword ptr [ecx];
        shufps      xmm1, xmm1, 0;
        mov         eax, dword ptr [ebp+14h];
        subps       xmm3, xmm2;
        movups      xmm4, xmmword ptr [eax];
        mulps       xmm3, xmm0;
        subps       xmm4, xmm2;
        mov         eax, dword ptr [ebp+8];
        addps       xmm3, xmm2;
        mulps       xmm4, xmm1;
        addps       xmm3, xmm4;
        movups      xmmword ptr [eax], xmm3;
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         18h;
    }
}

__gs_naked vec4* __stdcall sse_vec4transform(vec4* o, const vec4* v, const matrix* m)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         eax, dword ptr [ebp+0Ch];
        mov         ecx, dword ptr [ebp+10h];
        movups      xmm3, xmmword ptr [eax];
        mov         eax, dword ptr [ebp+8];
        test        cl, 0Fh;
        jne         label1;
        movaps      xmm2, xmm3;
        shufps      xmm2, xmm3, 0FFh;
        mulps       xmm2, xmmword ptr [ecx+30h];
        movaps      xmm1, xmm3;
        shufps      xmm1, xmm3, 0AAh;
        mulps       xmm1, xmmword ptr [ecx+20h];
        addps       xmm2, xmm1;
        movaps      xmm1, xmm3;
        shufps      xmm1, xmm3, 55h;
        mulps       xmm1, xmmword ptr [ecx+10h];
        shufps      xmm3, xmm3, 0;
        mulps       xmm3, xmmword ptr [ecx];
        addps       xmm1, xmm3;
        jmp         label2;
    label1:
        movsd       xmm2, mmword ptr [ecx+30h];
        movhps      xmm2, qword ptr [ecx+38h];
        movsd       xmm1, mmword ptr [ecx+20h];
        movhps      xmm1, qword ptr [ecx+28h];
        movaps      xmm0, xmm3;
        shufps      xmm0, xmm3, 0FFh;
        mulps       xmm2, xmm0;
        movaps      xmm0, xmm3;
        shufps      xmm0, xmm3, 0AAh;
        mulps       xmm1, xmm0;
        addps       xmm2, xmm1;
        movsd       xmm1, mmword ptr [ecx+10h];
        movhps      xmm1, qword ptr [ecx+18h];
        movaps      xmm0, xmm3;
        shufps      xmm0, xmm3, 55h;
        mulps       xmm1, xmm0;
        movsd       xmm0, mmword ptr [ecx];
        movhps      xmm0, qword ptr [ecx+8];
        shufps      xmm3, xmm3, 0;
        mulps       xmm0, xmm3;
        addps       xmm1, xmm0;
    label2:
        addps       xmm2, xmm1;
        movups      xmmword ptr [eax], xmm2;
        pop         ebp;
        ret         0Ch;
    }
}

__gs_naked vec4* __stdcall sse2_vec4transformarray(vec4* o, uint ostride, const vec4* v, uint vstride, const matrix* m, uint n)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ecx;
        push        ecx;
        and         esp, 0FFFFFFF0h;
        add         esp, 4;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 158h;
        mov         eax, dword ptr [ebx+0Ch];
        mov         ecx, dword ptr [ebx+8];
        mov         edx, dword ptr [ebx+10h];
        mov         dword ptr [ebp-120h], eax;
        mov         eax, dword ptr [ebx+14h];
        push        esi;
        mov         esi, dword ptr [ebx+1Ch];
        push        edi;
        mov         dword ptr [ebp-11Ch], eax;
        mov         eax, dword ptr [ebx+18h];
        mov         dword ptr [ebp-124h], eax;
        push        0Ah;
        mov         eax, esi;
        pop         edi;
        and         eax, 0FFFFFFFCh;
        cmp         edi, esi;
        sbb         edi, edi;
        and         edi, eax;
        sub         esi, edi;
        shr         edi, 2;
        mov         dword ptr [ebp-114h], ecx;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebx+1Ch], esi;
        mov         dword ptr [ebp-12Ch], ecx;
        test        edi, edi;
        je          label1;
        push        4;
        push        4;
        push        dword ptr [ebp-124h];
        lea         ecx, [ebp-110h];
        call        aligned_matrix;
        mov         ecx, dword ptr [ebp-11Ch];
        mov         edx, dword ptr [ebp-120h];
        imul        edx, edx, 3;
        mov         eax, ecx;
        imul        eax, eax, 3;
        mov         dword ptr [ebp-130h], edx;
        mov         edx, dword ptr [ebp-118h];
        mov         dword ptr [ebp-128h], eax;
        test        edi, edi;
        je          label2;
        movaps      xmm3, xmmword ptr [ebp-40h];
        movaps      xmm4, xmmword ptr [ebp-50h];
        movaps      xmm2, xmmword ptr [ebp-110h];
        mov         esi, dword ptr [ebp-11Ch];
        shl         ecx, 2;
        mov         dword ptr [ebp-134h], ecx;
        mov         ecx, dword ptr [ebp-114h];
    label3:
        movsd       xmm0, mmword ptr [edx+esi*2];
        movsd       xmm7, mmword ptr [edx];
        movhps      xmm7, qword ptr [esi+edx];
        movsd       xmm6, mmword ptr [edx+8];
        movhps      xmm6, qword ptr [esi+edx+8];
        mov         eax, dword ptr [ebp-128h];
        movhps      xmm0, qword ptr [edx+eax];
        movaps      xmm5, xmm7;
        shufps      xmm5, xmm0, 88h;
        shufps      xmm7, xmm0, 0DDh;
        movsd       xmm0, mmword ptr [edx+esi*2+8];
        movhps      xmm0, qword ptr [eax+edx+8];
        mulps       xmm2, xmm5;
        movaps      xmm1, xmm6;
        shufps      xmm1, xmm0, 88h;
        shufps      xmm6, xmm0, 0DDh;
        movaps      xmm0, xmmword ptr [ebp-0D0h];
        mulps       xmm0, xmm7;
        addps       xmm2, xmm0;
        movaps      xmm0, xmmword ptr [ebp-90h];
        mulps       xmm0, xmm1;
        addps       xmm2, xmm0;
        movaps      xmm0, xmm4;
        movaps      xmm4, xmmword ptr [ebp-100h];
        mulps       xmm0, xmm6;
        addps       xmm2, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0C0h];
        mulps       xmm0, xmm7;
        mov         eax, dword ptr [ebp-120h];
        mulps       xmm4, xmm5;
        addps       xmm4, xmm0;
        movaps      xmm0, xmmword ptr [ebp-80h];
        mulps       xmm0, xmm1;
        addps       xmm4, xmm0;
        mov         esi, dword ptr [ebp-130h];
        movaps      xmm0, xmm3;
        movaps      xmm3, xmmword ptr [ebp-0F0h];
        mulps       xmm0, xmm6;
        addps       xmm4, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0B0h];
        mulps       xmm0, xmm7;
        mulps       xmm3, xmm5;
        addps       xmm3, xmm0;
        movaps      xmm0, xmmword ptr [ebp-70h];
        mulps       xmm0, xmm1;
        addps       xmm3, xmm0;
        movaps      xmm0, xmmword ptr [ebp-30h];
        mulps       xmm0, xmm6;
        add         edx, dword ptr [ebp-134h];
        addps       xmm3, xmm0;
        movaps      xmm0, xmmword ptr [ebp-0A0h];
        movaps      xmmword ptr [ebp-150h], xmm2;
        movaps      xmm2, xmmword ptr [ebp-0E0h];
        mulps       xmm0, xmm7;
        mulps       xmm2, xmm5;
        movaps      xmm5, xmmword ptr [ebp-150h];
        addps       xmm2, xmm0;
        movaps      xmm0, xmmword ptr [ebp-60h];
        mulps       xmm0, xmm1;
        addps       xmm2, xmm0;
        movaps      xmm0, xmmword ptr [ebp-20h];
        mulps       xmm0, xmm6;
        addps       xmm2, xmm0;
        movaps      xmm1, xmm5;
        unpcklps    xmm1, xmm4;
        movlps      qword ptr [ecx], xmm1;
        movaps      xmm0, xmm3;
        unpcklps    xmm0, xmm2;
        movlps      qword ptr [ecx+8], xmm0;
        movhps      qword ptr [eax+ecx], xmm1;
        movhps      qword ptr [eax+ecx+8], xmm0;
        unpckhps    xmm5, xmm4;
        movlps      qword ptr [ecx+eax*2], xmm5;
        unpckhps    xmm3, xmm2;
        movlps      qword ptr [ecx+eax*2+8], xmm3;
        movhps      qword ptr [ecx+esi], xmm5;
        movhps      qword ptr [esi+ecx+8], xmm3;
        mov         esi, eax;
        shl         esi, 2;
        add         ecx, esi;
        dec         edi;
        movaps      xmm3, xmmword ptr [ebp-40h];
        movaps      xmm4, xmmword ptr [ebp-50h];
        movaps      xmm2, xmmword ptr [ebp-110h];
        mov         esi, dword ptr [ebp-11Ch];
        jne         label3;
        mov         esi, dword ptr [ebx+1Ch];
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jmp         label1;
    label2:
        mov         ecx, dword ptr [ebp-114h];
    label1:
        test        esi, esi;
        je          label4;
        mov         edi, dword ptr [ebp-120h];
    label5:
        push        dword ptr [ebp-124h];
        push        edx;
        push        ecx;
        call        sse_vec4transform;
        mov         edx, dword ptr [ebp-118h];
        mov         ecx, dword ptr [ebp-114h];
        add         edx, dword ptr [ebp-11Ch];
        add         ecx, edi;
        dec         esi;
        mov         dword ptr [ebp-118h], edx;
        mov         dword ptr [ebp-114h], ecx;
        jne         label5;
    label4:
        mov         ecx, dword ptr [ebp-4];
        mov         eax, dword ptr [ebp-12Ch];
        pop         edi;
        xor         ecx, ebp;
        pop         esi;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         18h;
    }
}

__gs_naked float __stdcall sse2_matdeterminant(const matrix* m)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        sub         esp, 8;
        and         esp, 0FFFFFFF0h;
        add         esp, 8;
        push        ebp;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 30h;
        mov         eax, dword ptr [ebx+8];
        test        al, 0Fh;
        jne         label1;
        movaps      xmm7, xmmword ptr [eax];
        movaps      xmm1, xmmword ptr [eax+10h];
        movaps      xmm2, xmmword ptr [eax+20h];
        movaps      xmm0, xmmword ptr [eax+30h];
        jmp         label2;
    label1:
        movlps      xmm1, qword ptr [eax+10h];
        movlps      xmm7, qword ptr [eax];
        movhps      xmm1, qword ptr [eax+18h];
        movhps      xmm7, qword ptr [eax+8];
        movlps      xmm2, qword ptr [eax+20h];
        movlps      xmm0, qword ptr [eax+30h];
        movhps      xmm2, qword ptr [eax+28h];
        movhps      xmm0, qword ptr [eax+38h];
    label2:
        shufps      xmm2, xmm2, 39h;
        shufps      xmm1, xmm1, 39h;
        movaps      xmm3, xmm0;
        shufps      xmm0, xmm0, 4Eh;
        mulps       xmm3, xmm2;
        movaps      xmm4, xmm0;
        shufps      xmm0, xmm0, 39h;
        mulps       xmm4, xmm2;
        mulps       xmm2, xmm0;
        movaps      xmm0, xmm2;
        shufps      xmm2, xmm2, 4Eh;
        subps       xmm2, xmm0;
        movaps      xmm0, xmm1;
        shufps      xmm1, xmm1, 39h;
        mulps       xmm2, xmm1;
        shufps      xmm1, xmm1, 39h;
        movaps      xmm5, xmm4;
        shufps      xmm4, xmm4, 39h;
        movaps      xmm6, xmm3;
        shufps      xmm3, xmm3, 4Eh;
        shufps      xmm6, xmm6, 39h;
        subps       xmm4, xmm3;
        subps       xmm5, xmm6;
        mulps       xmm0, xmm4;
        mulps       xmm1, xmm5;
        addps       xmm0, xmm2;
        addps       xmm0, xmm1;
        mulps       xmm0, xmm7;
        movaps      xmm1, xmm0;
        movhlps     xmm0, xmm0;
        addps       xmm1, xmm0;
        movaps      xmm0, xmm1;
        shufps      xmm1, xmm1, 1;
        subss       xmm0, xmm1;
        movss       dword ptr [ebp-20h], xmm0;
        fld         dword ptr [ebp-20h];
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         4;
    }
}

__gs_naked matrix* __stdcall sse_matmultiply(matrix* o, const matrix* m1, const matrix* m2)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         eax, dword ptr [ebp+0Ch];
        mov         ecx, dword ptr [ebp+10h];
        mov         edx, dword ptr [ebp+8];
        movlps      xmm0, qword ptr [ecx];
        movhps      xmm0, qword ptr [ecx+8];
        movlps      xmm1, qword ptr [ecx+10h];
        movhps      xmm1, qword ptr [ecx+18h];
        movlps      xmm2, qword ptr [ecx+20h];
        movhps      xmm2, qword ptr [ecx+28h];
        movlps      xmm3, qword ptr [ecx+30h];
        movhps      xmm3, qword ptr [ecx+38h];
        movss       xmm4, dword ptr [eax];
        movss       xmm5, dword ptr [eax+4];
        movss       xmm6, dword ptr [eax+8];
        movss       xmm7, dword ptr [eax+0Ch];
        shufps      xmm4, xmm4, 0;
        shufps      xmm5, xmm5, 0;
        shufps      xmm6, xmm6, 0;
        shufps      xmm7, xmm7, 0;
        mulps       xmm4, xmm0;
        mulps       xmm5, xmm1;
        mulps       xmm6, xmm2;
        mulps       xmm7, xmm3;
        addps       xmm4, xmm5;
        addps       xmm6, xmm7;
        addps       xmm4, xmm6;
        movss       xmm5, dword ptr [eax+10h];
        movss       xmm6, dword ptr [eax+14h];
        movss       xmm7, dword ptr [eax+18h];
        shufps      xmm5, xmm5, 0;
        shufps      xmm6, xmm6, 0;
        shufps      xmm7, xmm7, 0;
        mulps       xmm5, xmm0;
        mulps       xmm6, xmm1;
        mulps       xmm7, xmm2;
        addps       xmm5, xmm6;
        addps       xmm5, xmm7;
        movss       xmm6, dword ptr [eax+1Ch];
        shufps      xmm6, xmm6, 0;
        mulps       xmm6, xmm3;
        addps       xmm5, xmm6;
        movss       xmm6, dword ptr [eax+20h];
        movss       xmm7, dword ptr [eax+24h];
        shufps      xmm6, xmm6, 0;
        shufps      xmm7, xmm7, 0;
        mulps       xmm6, xmm0;
        mulps       xmm7, xmm1;
        addps       xmm6, xmm7;
        movss       xmm7, dword ptr [eax+28h];
        shufps      xmm7, xmm7, 0;
        mulps       xmm7, xmm2;
        addps       xmm6, xmm7;
        movss       xmm7, dword ptr [eax+2Ch];
        shufps      xmm7, xmm7, 0;
        mulps       xmm7, xmm3;
        addps       xmm6, xmm7;
        movss       xmm7, dword ptr [eax+30h];
        shufps      xmm7, xmm7, 0;
        mulps       xmm0, xmm7;
        movss       xmm7, dword ptr [eax+34h];
        shufps      xmm7, xmm7, 0;
        mulps       xmm1, xmm7;
        movss       xmm7, dword ptr [eax+38h];
        shufps      xmm7, xmm7, 0;
        mulps       xmm2, xmm7;
        movss       xmm7, dword ptr [eax+3Ch];
        shufps      xmm7, xmm7, 0;
        mulps       xmm3, xmm7;
        movlps      qword ptr [edx], xmm4;
        movhps      qword ptr [edx+8], xmm4;
        addps       xmm0, xmm1;
        movlps      qword ptr [edx+10h], xmm5;
        movhps      qword ptr [edx+18h], xmm5;
        addps       xmm2, xmm3;
        movlps      qword ptr [edx+20h], xmm6;
        movhps      qword ptr [edx+28h], xmm6;
        addps       xmm0, xmm2;
        movlps      qword ptr [edx+30h], xmm0;
        movhps      qword ptr [edx+38h], xmm0;
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         0Ch;
    }
}

__gs_naked matrix* __stdcall sse2_matmultiplytranspose(matrix* o, const matrix* m1, const matrix* m2)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         eax, dword ptr [ebp+0Ch];
        mov         ecx, dword ptr [ebp+10h];
        mov         edx, dword ptr [ebp+8];
        movlps      xmm0, qword ptr [ecx];
        movhps      xmm0, qword ptr [ecx+8];
        movlps      xmm1, qword ptr [ecx+10h];
        movhps      xmm1, qword ptr [ecx+18h];
        movlps      xmm2, qword ptr [ecx+20h];
        movhps      xmm2, qword ptr [ecx+28h];
        movlps      xmm3, qword ptr [ecx+30h];
        movhps      xmm3, qword ptr [ecx+38h];
        movss       xmm4, dword ptr [eax];
        movss       xmm5, dword ptr [eax+4];
        movss       xmm6, dword ptr [eax+8];
        movss       xmm7, dword ptr [eax+0Ch];
        shufps      xmm4, xmm4, 0;
        shufps      xmm5, xmm5, 0;
        shufps      xmm6, xmm6, 0;
        shufps      xmm7, xmm7, 0;
        mulps       xmm4, xmm0;
        mulps       xmm5, xmm1;
        mulps       xmm6, xmm2;
        mulps       xmm7, xmm3;
        addps       xmm4, xmm5;
        addps       xmm6, xmm7;
        addps       xmm4, xmm6;
        movss       xmm5, dword ptr [eax+10h];
        movss       xmm6, dword ptr [eax+14h];
        movss       xmm7, dword ptr [eax+18h];
        shufps      xmm5, xmm5, 0;
        shufps      xmm6, xmm6, 0;
        shufps      xmm7, xmm7, 0;
        mulps       xmm5, xmm0;
        mulps       xmm6, xmm1;
        mulps       xmm7, xmm2;
        addps       xmm5, xmm6;
        addps       xmm5, xmm7;
        movss       xmm6, dword ptr [eax+1Ch];
        shufps      xmm6, xmm6, 0;
        mulps       xmm6, xmm3;
        addps       xmm5, xmm6;
        movss       xmm6, dword ptr [eax+20h];
        movss       xmm7, dword ptr [eax+24h];
        shufps      xmm6, xmm6, 0;
        shufps      xmm7, xmm7, 0;
        mulps       xmm6, xmm0;
        mulps       xmm7, xmm1;
        addps       xmm6, xmm7;
        movss       xmm7, dword ptr [eax+28h];
        shufps      xmm7, xmm7, 0;
        mulps       xmm7, xmm2;
        addps       xmm6, xmm7;
        movss       xmm7, dword ptr [eax+2Ch];
        shufps      xmm7, xmm7, 0;
        mulps       xmm7, xmm3;
        addps       xmm6, xmm7;
        movss       xmm7, dword ptr [eax+30h];
        shufps      xmm7, xmm7, 0;
        mulps       xmm0, xmm7;
        movss       xmm7, dword ptr [eax+34h];
        shufps      xmm7, xmm7, 0;
        mulps       xmm1, xmm7;
        movss       xmm7, dword ptr [eax+38h];
        shufps      xmm7, xmm7, 0;
        mulps       xmm2, xmm7;
        movss       xmm7, dword ptr [eax+3Ch];
        shufps      xmm7, xmm7, 0;
        mulps       xmm3, xmm7;
        addps       xmm0, xmm1;
        addps       xmm2, xmm3;
        addps       xmm0, xmm2;
        movaps      xmm1, xmm4;
        movaps      xmm2, xmm6;
        unpcklps    xmm4, xmm5;
        unpcklps    xmm6, xmm0;
        unpckhps    xmm1, xmm5;
        unpckhps    xmm2, xmm0;
        movlps      qword ptr [edx], xmm4;
        movlps      qword ptr [edx+8], xmm6;
        movhps      qword ptr [edx+10h], xmm4;
        movhps      qword ptr [edx+18h], xmm6;
        movlps      qword ptr [edx+20h], xmm1;
        movlps      qword ptr [edx+28h], xmm2;
        movhps      qword ptr [edx+30h], xmm1;
        movhps      qword ptr [edx+38h], xmm2;
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         0Ch;
    }
}

__gs_naked matrix* __stdcall sse2_matinverse(matrix* o, float* determinant, const matrix* m)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        sub         esp, 8;
        and         esp, 0FFFFFFF0h;
        add         esp, 8;
        push        ebp;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 110h;
        mov         eax, dword ptr [ebx+10h];
        xorps       xmm0, xmm0;
        movlps      xmm2, qword ptr [eax+8];
        movlps      xmm4, qword ptr [eax+28h];
        movhps      xmm2, qword ptr [eax+18h];
        movhps      xmm4, qword ptr [eax+38h];
        movlps      xmm3, qword ptr [eax+20h];
        movlps      xmm1, qword ptr [eax];
        movhps      xmm3, qword ptr [eax+30h];
        movhps      xmm1, qword ptr [eax+10h];
        movaps      xmm5, xmm2;
        shufps      xmm5, xmm4, 88h;
        shufps      xmm4, xmm2, 0DDh;
        movaps      xmm2, xmm4;
        mulps       xmm2, xmm5;
        shufps      xmm2, xmm2, 0B1h;
        movaps      xmm6, xmm2;
        shufps      xmm6, xmm6, 4Eh;
        movaps      xmm7, xmm3;
        shufps      xmm3, xmm1, 0DDh;
        shufps      xmm1, xmm7, 88h;
        movaps      xmm7, xmm3;
        mulps       xmm3, xmm6;
        mulps       xmm6, xmm1;
        movaps      xmmword ptr [ebp-0D0h], xmm6;
        movaps      xmm6, xmm7;
        mulps       xmm7, xmm2;
        mulps       xmm2, xmm1;
        subps       xmm3, xmm7;
        movaps      xmm7, xmm6;
        mulps       xmm7, xmm5;
        shufps      xmm5, xmm5, 4Eh;
        shufps      xmm7, xmm7, 0B1h;
        movaps      xmmword ptr [ebp-0C0h], xmm2;
        movaps      xmm2, xmm4;
        mulps       xmm2, xmm7;
        addps       xmm2, xmm3;
        movaps      xmm3, xmm7;
        shufps      xmm7, xmm7, 4Eh;
        mulps       xmm3, xmm1;
        movaps      xmmword ptr [ebp-0B0h], xmm3;
        movaps      xmm3, xmm4;
        mulps       xmm3, xmm7;
        mulps       xmm7, xmm1;
        subps       xmm2, xmm3;
        movaps      xmm3, xmm6;
        shufps      xmm3, xmm3, 4Eh;
        mulps       xmm3, xmm4;
        shufps      xmm3, xmm3, 0B1h;
        movaps      xmmword ptr [ebp-0A0h], xmm7;
        movaps      xmm7, xmm5;
        mulps       xmm5, xmm3;
        addps       xmm5, xmm2;
        movaps      xmm2, xmm3;
        shufps      xmm3, xmm3, 4Eh;
        mulps       xmm2, xmm1;
        movaps      xmmword ptr [ebp-90h], xmm4;
        movaps      xmm4, xmm7;
        mulps       xmm7, xmm3;
        mulps       xmm3, xmm1;
        subps       xmm5, xmm7;
        subps       xmm3, xmm2;
        movaps      xmm2, xmm1;
        mulps       xmm1, xmm5;
        shufps      xmm3, xmm3, 4Eh;
        movaps      xmm7, xmm1;
        shufps      xmm1, xmm1, 4Eh;
        movaps      xmmword ptr [ebp-0E0h], xmm5;
        addps       xmm1, xmm7;
        movaps      xmm5, xmm1;
        shufps      xmm1, xmm1, 0B1h;
        addss       xmm1, xmm5;
        movaps      xmmword ptr [ebp-0F0h], xmm1;
        movaps      xmm5, xmm6;
        mulps       xmm5, xmm2;
        shufps      xmm5, xmm5, 0B1h;
        movaps      xmm7, xmm5;
        shufps      xmm5, xmm5, 4Eh;
        movaps      xmmword ptr [ebp-80h], xmm4;
        movaps      xmm4, xmmword ptr [ebp-90h];
        movaps      xmmword ptr [ebp-70h], xmm6;
        movaps      xmm6, xmm4;
        mulps       xmm6, xmm7;
        addps       xmm6, xmm3;
        movaps      xmm3, xmm4;
        mulps       xmm3, xmm5;
        subps       xmm3, xmm6;
        movaps      xmm6, xmm4;
        mulps       xmm6, xmm2;
        shufps      xmm6, xmm6, 0B1h;
        movaps      xmmword ptr [ebp-60h], xmm5;
        movaps      xmm5, xmmword ptr [ebp-70h];
        movaps      xmmword ptr [ebp-50h], xmm7;
        movaps      xmm7, xmm6;
        mulps       xmm7, xmm5;
        addps       xmm7, xmm3;
        movaps      xmm3, xmm6;
        shufps      xmm3, xmm3, 4Eh;
        movaps      xmmword ptr [ebp-40h], xmm4;
        movaps      xmm4, xmm5;
        mulps       xmm5, xmm3;
        movaps      xmmword ptr [ebp-30h], xmm4;
        movaps      xmm4, xmm6;
        movaps      xmm6, xmm7;
        subps       xmm6, xmm5;
        movaps      xmm5, xmmword ptr [ebp-0D0h];
        movaps      xmm7, xmmword ptr [ebp-0C0h];
        subps       xmm5, xmm7;
        shufps      xmm5, xmm5, 4Eh;
        movaps      xmm7, xmmword ptr [ebp-80h];
        mulps       xmm4, xmm7;
        mulps       xmm3, xmm7;
        subps       xmm5, xmm4;
        mulps       xmm2, xmm7;
        addps       xmm3, xmm5;
        shufps      xmm2, xmm2, 0B1h;
        movaps      xmm4, xmm2;
        shufps      xmm4, xmm4, 4Eh;
        movaps      xmm5, xmmword ptr [ebp-40h];
        movaps      xmmword ptr [ebp-20h], xmm6;
        movaps      xmm6, xmm5;
        mulps       xmm5, xmm2;
        mulps       xmm6, xmm4;
        addps       xmm5, xmm3;
        movaps      xmm3, xmm4;
        movaps      xmm4, xmm5;
        subps       xmm4, xmm6;
        movaps      xmm5, xmmword ptr [ebp-0A0h];
        movaps      xmm6, xmmword ptr [ebp-0B0h];
        subps       xmm5, xmm6;
        shufps      xmm5, xmm5, 4Eh;
        movaps      xmm6, xmmword ptr [ebp-50h];
        mulps       xmm6, xmm7;
        subps       xmm6, xmm5;
        movaps      xmm5, xmmword ptr [ebp-60h];
        mulps       xmm7, xmm5;
        subps       xmm6, xmm7;
        movaps      xmm5, xmmword ptr [ebp-30h];
        mulps       xmm2, xmm5;
        mulps       xmm5, xmm3;
        subps       xmm6, xmm2;
        movaps      xmm2, xmm5;
        addps       xmm2, xmm6;
        comiss      xmm1, xmm0;
        movaps      xmm6, xmmword ptr [ebp-20h];
        je          label1;
        mov         eax, dword ptr [ebx+0Ch];
        test        eax, eax;
        je          label2;
        movaps      xmm0, xmm1;
        movss       dword ptr [eax], xmm0;
    label2:
        movaps      xmm0, xmm1;
        movaps      xmm1, xmmword ptr [ebp-0E0h];
        mov         eax, dword ptr [ebx+8];
        movaps      xmm3, xmm0;
        rcpss       xmm0, xmm0;
        movaps      xmm5, xmm0;
        mulss       xmm0, xmm0;
        mulss       xmm3, xmm0;
        addss       xmm5, xmm5;
        subss       xmm5, xmm3;
        shufps      xmm5, xmm5,0;
        mulps       xmm1, xmm5;
        movlps      qword ptr [eax], xmm1;
        movhps      qword ptr [eax+8], xmm1;
        mulps       xmm4, xmm5;
        movlps      qword ptr [eax+10h], xmm4;
        movhps      qword ptr [eax+18h], xmm4;
        mulps       xmm6, xmm5;
        movlps      qword ptr [eax+20h], xmm6;
        movhps      qword ptr [eax+28h], xmm6;
        mulps       xmm5, xmm2;
        movlps      qword ptr [eax+30h], xmm5;
        movhps      qword ptr [eax+38h], xmm5;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         0Ch;
    label1:
        xor         eax, eax;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         0Ch;
    }
}

__gs_naked matrix* __stdcall sse2_matshadow(matrix* out, const vec4* plight, const plane* pln)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        push        ebx;
        mov         eax, dword ptr [ebp+10h];
        movups      xmm1, xmmword ptr [eax];
        movaps      xmm2, xmm1;
        lea         ebx, srrr;
        andps       xmm2, xmmword ptr [ebx+40h];
        mulps       xmm2, xmm2;
        movhlps     xmm3, xmm2;
        addps       xmm3, xmm2;
        movaps      xmm2, xmm3;
        shufps      xmm2, xmm2, 1;
        addss       xmm2, xmm3;
        xorps       xmm0, xmm0;
        comiss      xmm2, xmm0;
        je          label1;
        movaps      xmm6, xmm2;
        rsqrtss     xmm2, xmm6;
        movaps      xmm3, xmm2;
        mulss       xmm6, xmm2;
        mulss       xmm6, xmm2;
        mulss       xmm2, dword ptr [s0d50];
        movss       xmm3, dword ptr [s3d00];
        subss       xmm3, xmm6;
        mulss       xmm2, xmm3;
        shufps      xmm2, xmm2, 0;
        mulps       xmm2, xmm1;
    label1:
        mov         ecx, dword ptr [ebp+0Ch];
        movups      xmm4, xmmword ptr [ecx];
        movaps      xmm5, xmm4;
        mulps       xmm5, xmm2;
        movhlps     xmm3, xmm5;
        addps       xmm5, xmm3;
        movaps      xmm3, xmm5;
        shufps      xmm3, xmm3, 1;
        addss       xmm5, xmm3;
        shufps      xmm5, xmm5, 0;
        xorps       xmm2, xmmword ptr [ebx];
        movaps      xmm0, xmm2;
        movaps      xmm1, xmm2;
        movaps      xmm3, xmm2;
        shufps      xmm0, xmm0, 0;
        shufps      xmm1, xmm1, 55h;
        shufps      xmm2, xmm2, 0AAh;
        shufps      xmm3, xmm3, 0FFh;
        mulps       xmm0, xmm4;
        mulps       xmm1, xmm4;
        mulps       xmm2, xmm4;
        mulps       xmm3, xmm4;
        addss       xmm0, xmm5;
        movaps      xmm6, xmm5;
        movaps      xmm7, xmm5;
        andps       xmm5, xmmword ptr [ebx+10h];
        andps       xmm6, xmmword ptr [ebx+20h];
        andps       xmm7, xmmword ptr [ebx+30h];
        addps       xmm1, xmm5;
        addps       xmm2, xmm6;
        addps       xmm3, xmm7;
        mov         edx, dword ptr [ebp+8];
        movlps      qword ptr [edx], xmm0;
        movhps      qword ptr [edx+8], xmm0;
        movlps      qword ptr [edx+10h], xmm1;
        movhps      qword ptr [edx+18h], xmm1;
        movlps      qword ptr [edx+20h], xmm2;
        movhps      qword ptr [edx+28h], xmm2;
        movlps      qword ptr [edx+30h], xmm3;
        movhps      qword ptr [edx+38h], xmm3;
        mov         eax, dword ptr [ebp+8];
        pop         ebx;
        pop         ebp;
        ret         0Ch;
    }
}

__gs_naked matrix* __stdcall sse2_matreflect(matrix* out, const plane* pln)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ebp;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 10h;
        mov         edx, dword ptr [ebx+0Ch];
        mov         eax, dword ptr [ebx+8];
        movhps      xmm0, qword ptr [edx+8];
        movlps      xmm0, qword ptr [edx];
        movaps      xmm1, xmmword ptr [arrr];
        andps       xmm1, xmm0;
        mulps       xmm1, xmm1;
        movss       xmm2, dword ptr [s3d00];
        movss       xmm3, dword ptr [s0d50];
        movaps      xmm4, xmm1;
        movhlps     xmm1, xmm1;
        addps       xmm4, xmm1;
        movaps      xmm1, xmm4;
        shufps      xmm4, xmm4, 1;
        addss       xmm1, xmm4;
        movaps      xmm4, xmmword ptr [drrr];
        movaps      xmm5, xmm1;
        rsqrtss     xmm1, xmm1;
        mulss       xmm5, xmm1;
        mulss       xmm5, xmm1;
        subss       xmm2, xmm5;
        mulss       xmm3, xmm1;
        mulss       xmm3, xmm2;
        shufps      xmm3, xmm3, 0;
        mulps       xmm0, xmm3;
        mulps       xmm4, xmm0;
        movaps      xmm1, xmm0;
        shufps      xmm0, xmm0, 0;
        mulps       xmm0, xmm4;
        addps       xmm0, xmmword ptr [sf1000];
        movaps      xmm2, xmm1;
        shufps      xmm1, xmm1, 0FFh;
        mulps       xmm1, xmm4;
        addps       xmm1, xmmword ptr [sf0001];
        movaps      xmm3, xmm2;
        shufps      xmm2, xmm2, 0AAh;
        mulps       xmm2, xmm4;
        addps       xmm2, xmmword ptr [sf0010];
        shufps      xmm3, xmm3, 55h;
        mulps       xmm4, xmm3;
        addps       xmm4, xmmword ptr [sf0100];
        movups      xmmword ptr [eax], xmm0;
        movups      xmmword ptr [eax+10h], xmm4;
        movups      xmmword ptr [eax+20h], xmm2;
        movups      xmmword ptr [eax+30h], xmm1;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         8;
    }
}

static const uint align16 sunk0[] =  { 0x3f22f983, 0x3f22f983, 0x3f22f983, 0x3f22f983 };
static const uint align16 sunk1[] =  { 0x3fc90000, 0x3fc90000, 0x3fc90000, 0x3fc90000 };
static const uint align16 sunk2[] =  { 0x39fda000, 0x39fda000, 0x39fda000, 0x39fda000 };
static const uint align16 sunk3[] =  { 0x33a22000, 0x33a22000, 0x33a22000, 0x33a22000 };
static const uint align16 sunk4[] =  { 0x2c34611a, 0x2c34611a, 0x2c34611a, 0x2c34611a };
static const uint align16 sunk5[] =  { 0x4b000000, 0x4b000000, 0x4b000000, 0x4b000000 };
static const uint align16 sunk6[] =  { 0xb94ca1f0, 0xb94ca1f0, 0xb94ca1f0, 0xb94ca1f0 };
static const uint align16 sunk7[] =  { 0x37ccf5ce, 0x37ccf5ce, 0x37ccf5ce, 0x37ccf5ce };
static const uint align16 sunk8[] =  { 0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff };
static const uint align16 sunk9[] =  { 0x3c08839d, 0x3c08839d, 0x3c08839d, 0x3c08839d };
static const uint align16 sunk10[] = { 0xbab6061a, 0xbab6061a, 0xbab6061a, 0xbab6061a };
static const uint align16 sunk11[] = { 0xbe2aaaa3, 0xbe2aaaa3, 0xbe2aaaa3, 0xbe2aaaa3 };
static const uint align16 sunk12[] = { 0x3d2aaaa5, 0x3d2aaaa5, 0x3d2aaaa5, 0x3d2aaaa5 };

__gs_naked void* __stdcall sse2_sincos(void*, void*, void*)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         eax, dword ptr [ebp+8];
        movaps      xmm0, xmmword ptr [eax];
        nop;
        movaps      xmm1, xmm0;
        movaps      xmm6, xmm0;
        pand        xmm0, xmmword ptr [saaaa];
        pand        xmm1, xmmword ptr [siiii];
        movaps      xmm2, xmm0;
        movaps      xmm7, xmm0;
        mulps       xmm0, xmmword ptr [sunk0];
        movaps      xmm4, xmmword ptr [sunk1];
        movaps      xmm5, xmmword ptr [sunk2];
        movaps      xmm6, xmmword ptr [sunk3];
        movaps      xmm7, xmmword ptr [sunk4];
        addps       xmm0, xmmword ptr [sunk5];
        movaps      xmm3, xmm0;
        subps       xmm0, xmmword ptr [sunk5];
        mulps       xmm4, xmm0;
        mulps       xmm5, xmm0;
        mulps       xmm6, xmm0;
        mulps       xmm7, xmm0;
        subps       xmm2, xmm4;
        subps       xmm2, xmm5;
        movaps      xmm5, xmmword ptr [sunk6];
        subps       xmm2, xmm6;
        movaps      xmm6, xmmword ptr [sunk7];
        subps       xmm2, xmm7;
        movaps      xmm4, xmm2;
        mulps       xmm4, xmm4;
        andps       xmm3, xmmword ptr [sunk8];
        movaps      xmm0, xmm3;
        movaps      xmm7, xmm3;
        andps       xmm0, xmmword ptr [s1111];
        psubd       xmm3, xmm0;
        paddd       xmm7, xmm0;
        pand        xmm3, xmmword ptr [s2222];
        pand        xmm7, xmmword ptr [s2222];
        pslld       xmm3, 1Eh;
        pslld       xmm7, 1Eh;
        pxor        xmm3, xmm1;
        pslld       xmm0, 1Eh;
        pxor        xmm1, xmm1;
        cmpneqps    xmm0, xmm1;
        mulps       xmm5, xmm4;
        mulps       xmm6, xmm4;
        addps       xmm5, xmmword ptr [sunk9];
        addps       xmm6, xmmword ptr [sunk10];
        mulps       xmm5, xmm4;
        mulps       xmm6, xmm4;
        addps       xmm5, xmmword ptr [sunk11];
        addps       xmm6, xmmword ptr [sunk12];
        mulps       xmm5, xmm4;
        mulps       xmm6, xmm4;
        addps       xmm5, xmmword ptr [sf1111];
        addps       xmm6, xmmword ptr [sfqrh];
        mulps       xmm5, xmm2;
        mulps       xmm6, xmm4;
        addps       xmm6, xmmword ptr [sf1111];
        movaps      xmm1, xmm0;
        movaps      xmm2, xmm0;
        andnps      xmm1, xmm5;
        andps       xmm2, xmm6;
        orps        xmm1, xmm2;
        movaps      xmm2, xmm0;
        andps       xmm0, xmm5;
        andnps      xmm2, xmm6;
        orps        xmm0, xmm2;
        xorps       xmm0, xmm7;
        xorps       xmm1, xmm3;
        mov         eax, dword ptr [ebp+0Ch];
        mov         edx, dword ptr [ebp+10h];
        movaps      xmmword ptr [eax], xmm1;
        movaps      xmmword ptr [edx], xmm0;
        pop         ebp;
        ret         0Ch;
    }
}

__gs_naked quat* __stdcall sse2_quatrotateeuler(quat* out, float yaw, float pitch, float roll)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        push        ecx;
        push        ecx;
        and         esp, 0FFFFFFF0h;
        add         esp, 4;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 20h;
        movss       xmm2, dword ptr [ebx+14h];
        movss       xmm0, dword ptr [ebx+10h];
        movss       xmm1, dword ptr [ebx+0Ch];
        lea         eax, [ebp-20h];
        push        eax;
        unpcklps    xmm2, xmm0;
        lea         eax, [ebp-10h];
        unpcklps    xmm1, xmm1;
        movlhps     xmm2, xmm1;
        movaps      xmmword ptr [ebp-10h], xmm2;
        movaps      xmm0, xmmword ptr [shhhh];
        push        eax;
        mulps       xmm0, xmm2;
        push        eax;
        movaps      xmmword ptr [ebp-10h], xmm0;
        call        sse2_sincos;
        movaps      xmm2, xmmword ptr [ebp-10h];
        mov         eax, dword ptr [ebx+8];
        movaps      xmm4, xmm2;
        shufps      xmm4, xmmword ptr [ebp-20h], 99h;
        shufps      xmm2, xmmword ptr [ebp-20h], 0;
        movaps      xmm3, xmm4;
        shufps      xmm3, xmm4, 0F7h;
        movaps      xmm1, xmm3;
        shufps      xmm2, xmm2, 8Ah;
        shufps      xmm3, xmm3, 51h;
        shufps      xmm4, xmm4, 0A8h;
        movaps      xmm0, xmm2;
        mulps       xmm1, xmm2;
        mulps       xmm1, xmm4;
        shufps      xmm0, xmm2, 8Ah;
        mulps       xmm3, xmm0;
        shufps      xmm4, xmm4, 1;
        mulps       xmm3, xmm4;
        xorps       xmm3, xmmword ptr [s0ii0];
        addps       xmm3, xmm1;
        movups      xmmword ptr [eax], xmm3;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         10h;
    }
}

__gs_naked quat* __stdcall sse_quatmultiply(quat* out, const quat* q1, const quat* q2)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        push        ebx;
        mov         eax, dword ptr [ebp+10h];
        mov         ebx, dword ptr [ebp+0Ch];
        mov         edx, dword ptr [ebp+8];
        movlps      xmm1, qword ptr [eax];
        movhps      xmm1, qword ptr [eax+8];
        movlps      xmm0, qword ptr [ebx];
        movhps      xmm0, qword ptr [ebx+8];
        movaps      xmm2, xmm1;
        shufps      xmm1, xmm1, 0FFh;
        mulps       xmm1, xmm0;
        movaps      xmm3, xmm2;
        shufps      xmm2, xmm2, 0;
        shufps      xmm0, xmm0, 1Bh;
        xorps       xmm2, xmmword ptr [s0i0i];
        mulps       xmm2, xmm0;
        addps       xmm1, xmm2;
        movaps      xmm2, xmm3;
        shufps      xmm3, xmm3, 55h;
        shufps      xmm0, xmm0, 0B1h;
        xorps       xmm3, xmmword ptr [s00ii];
        mulps       xmm3, xmm0;
        addps       xmm1, xmm3;
        shufps      xmm2, xmm2, 0AAh;
        shufps      xmm0, xmm0, 1Bh;
        xorps       xmm2, xmmword ptr [si00i];
        mulps       xmm2, xmm0;
        addps       xmm1, xmm2;
        movlps      qword ptr [edx], xmm1;
        movhps      qword ptr [edx+8], xmm1;
        mov         eax, dword ptr [ebp+8];
        pop         ebx;
        pop         ebp;
        ret         0Ch;
    }
}

__gs_naked quat* __stdcall sse_quatnormalize(quat* out, const quat* q)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         eax, dword ptr [ebp+0Ch];
        movss       xmm0, dword ptr [eax];
        movss       xmm1, dword ptr [eax+4];
        movss       xmm2, dword ptr [eax+8];
        movss       xmm3, dword ptr [eax+0Ch];
        movss       xmm4, xmm0;
        movss       xmm5, xmm1;
        movss       xmm6, xmm2;
        movss       xmm7, xmm3;
        mulss       xmm4, xmm0;
        mulss       xmm5, xmm1;
        mulss       xmm6, xmm2;
        addss       xmm4, xmm5;
        mulss       xmm7, xmm3;
        addss       xmm6, xmm7;
        addss       xmm4, xmm6;
        rsqrtss     xmm5, xmm4;
        movss       xmm6, xmm5;
        mulss       xmm5, xmm5;
        mulss       xmm4, xmm5;
        movss       xmm5, dword ptr [s3d00];
        subss       xmm5, xmm4;
        mulss       xmm5, dword ptr [s0d50];
        mulss       xmm5, xmm6;
        mov         eax, dword ptr [ebp+8];
        mulss       xmm0, xmm5;
        mulss       xmm1, xmm5;
        movss       dword ptr [eax], xmm0;
        mulss       xmm2, xmm5;
        movss       dword ptr [eax+4], xmm1;
        mulss       xmm3, xmm5;
        movss       dword ptr [eax+8], xmm2;
        movss       dword ptr [eax+0Ch], xmm3;
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         8;
    }
}

__gs_naked quat* __stdcall sse2_quatinverse(quat* out, const quat* q)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         eax, dword ptr [ebp+0Ch];
        movsd       xmm3, mmword ptr [eax];
        movhps      xmm3, qword ptr [eax+8];
        mov         eax, dword ptr [ebp+8];
        movaps      xmm0, xmm3;
        mulps       xmm0, xmm3;
        movaps      xmm1, xmm0;
        movhlps     xmm1, xmm0;
        addps       xmm1, xmm0;
        movaps      xmm0, xmm1;
        shufps      xmm0, xmm1, 55h;
        addss       xmm1, xmm0;
        movaps      xmm2, xmm1;
        rcpss       xmm2, xmm2;
        movaps      xmm0, xmm2;
        mulss       xmm0, xmm1;
        mulss       xmm0, xmm2;
        addss       xmm2, xmm2;
        subss       xmm2, xmm0;
        movaps      xmm0, xmmword ptr [rrrp];
        shufps      xmm2, xmm2, 0;
        mulps       xmm0, xmm3;
        mulps       xmm2, xmm0;
        movups      xmmword ptr [eax], xmm2;
        pop         ebp;
        ret         8;
    }
}

__gs_naked plane* __stdcall sse2_planenormalize(plane* out, const plane* p)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         eax, dword ptr [ebp+0Ch];
        movss       xmm0, dword ptr [eax];
        movss       xmm1, dword ptr [eax+4];
        movss       xmm2, dword ptr [eax+8];
        movaps      xmm3, xmm0;
        movaps      xmm4, xmm1;
        movaps      xmm5, xmm2;
        mulss       xmm0, xmm0;
        mulss       xmm1, xmm1;
        mulss       xmm2, xmm2;
        addss       xmm0, xmm1;
        addss       xmm0, xmm2;
        mov         ecx, dword ptr [ebp+8];
        comiss      xmm0, dword ptr [s1d42];
        jae         label1;
        xor         eax, eax;
        mov         dword ptr [ecx], eax;
        mov         dword ptr [ecx+4], eax;
        mov         dword ptr [ecx+8], eax;
        mov         dword ptr [ecx+0Ch], eax;
        jmp         label2;
        mov         edi, edi;
    label1:
        rsqrtss     xmm6, xmm0;
        movaps      xmm7, xmm6;
        mulss       xmm6, xmm0;
        mulss       xmm6, xmm7;
        movss       xmm1, dword ptr [s3d00];
        mulss       xmm7, dword ptr [s0d50];
        subss       xmm1, xmm6;
        mulss       xmm1, xmm7;
        mulss       xmm3, xmm1;
        mulss       xmm4, xmm1;
        mulss       xmm5, xmm1;
        mulss       xmm1, dword ptr [eax+0Ch];
        movss       dword ptr [ecx], xmm3;
        movss       dword ptr [ecx+4], xmm4;
        movss       dword ptr [ecx+8], xmm5;
        movss       dword ptr [ecx+0Ch], xmm1;
    label2:
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         8;
    }
}

__gs_naked vec3* __stdcall sse_planeintersectline(vec3* out, const plane* p, const vec3* v1, const vec3* v2)
{
    __asm
    {
        push        ebp;
        mov         ebp, esp;
        mov         eax, dword ptr [ebp+8];
        mov         dword ptr [ebp+8], eax;
        mov         eax, dword ptr [ebp+0Ch];
        movups      xmm0, xmmword ptr [eax];
        mov         ecx, dword ptr [ebp+10h];
        movss       xmm1, dword ptr [ecx+8];
        mov         edx, dword ptr [ebp+14h];
        movhps      xmm1, qword ptr [ecx];
        movss       xmm2, dword ptr [edx+8];
        movaps      xmm4, xmm0;
        shufps      xmm0, xmm0, 4Eh;
        movaps      xmm3, xmm1;
        shufps      xmm4, xmm4, 0FFh;
        movhps      xmm2, qword ptr [edx];
        mulps       xmm3, xmm0;
        mulps       xmm0, xmm2;
        movhlps     xmm5, xmm3;
        movhlps     xmm6, xmm0;
        addps       xmm3, xmm5;
        addps       xmm0, xmm6;
        shufps      xmm5, xmm3, 55h;
        shufps      xmm6, xmm0, 55h;
        addss       xmm3, xmm5;
        addss       xmm0, xmm6;
        movss       xmm5, xmm3;
        subss       xmm5, xmm0;
        addss       xmm3, xmm4;
        comiss      xmm5, dword ptr [spr0];
        je          label1;
        rcpss       xmm0, xmm5;
        movss       xmm6, xmm0;
        addss       xmm6, xmm6;
        mulss       xmm5, xmm0;
        mulss       xmm5, xmm0;
        subss       xmm6, xmm5;
        mulss       xmm3, xmm6;
        shufps      xmm3, xmm3, 0;
        subps       xmm2, xmm1;
        mulps       xmm3, xmm2;
        mov         eax, dword ptr [ebp+8];
        addps       xmm1, xmm3;
        movhps      qword ptr [eax], xmm1;
        movss       dword ptr [eax+8], xmm1;
        jmp         label2;
    label1:
        mov         dword ptr [ebp+8], 0;
    label2:
        mov         eax, dword ptr [ebp+8];
        pop         ebp;
        ret         10h;
    }
}

__gs_naked plane* __stdcall sse2_planefrompoints(plane* out, const vec3* v1, const vec3* v2, const vec3* v3)
{
    __asm
    {
        push        ebx;
        mov         ebx, esp;
        sub         esp, 8;
        and         esp, 0FFFFFFF0h;
        add         esp, 8;
        push        ebp;
        push        ebp;
        mov         ebp, dword ptr [ebx+4];
        mov         dword ptr [esp+4], ebp;
        mov         ebp, esp;
        sub         esp, 10h;
        mov         edx, dword ptr [ebx+0Ch];
        mov         ecx, dword ptr [ebx+14h];
        movss       xmm0, dword ptr [edx+8];
        movss       xmm1, dword ptr [ecx+8];
        movhps      xmm0, qword ptr [edx];
        movhps      xmm1, qword ptr [ecx];
        mov         edx, dword ptr [ebx+10h];
        mov         eax, dword ptr [ebx+8];
        movss       xmm2, dword ptr [edx+8];
        movhps      xmm2, qword ptr [edx];
        movaps      xmm3, xmm0;
        subps       xmm0, xmm1;
        movaps      xmm1, xmm0;
        shufps      xmm0, xmm0, 78h;
        shufps      xmm1, xmm1, 63h;
        movaps      xmm4, xmm3;
        subps       xmm3, xmm2;
        movaps      xmm2, xmmword ptr [stttt];
        movaps      xmm5, xmm3;
        shufps      xmm3, xmm3, 63h;
        shufps      xmm5, xmm5, 78h;
        mulps       xmm0, xmm3;
        mulps       xmm5, xmm1;
        movaps      xmm1, xmmword ptr [shhhh];
        subps       xmm0, xmm5;
        shufps      xmm4, xmm4, 0Eh;
        movaps      xmm3, xmm0;
        mulps       xmm0, xmm0;
        movaps      xmm5, xmm0;
        movhlps     xmm0, xmm0;
        addps       xmm5, xmm0;
        movaps      xmm0, xmm5;
        shufps      xmm5, xmm5, 1;
        addss       xmm0, xmm5;
        movaps      xmm5, xmm0;
        rsqrtss     xmm0, xmm0;
        mulss       xmm1, xmm0;
        movaps      xmm6, xmm5;
        mulss       xmm5, xmm0;
        mulss       xmm5, xmm0;
        subss       xmm2, xmm5;
        mulss       xmm1, xmm2;
        shufps      xmm1, xmm1, 0;
        mulps       xmm3, xmm1;
        mulps       xmm4, xmm3;
        movups      xmmword ptr [eax], xmm3;
        movhlps     xmm6, xmm4;
        movaps      xmm0, xmm4;
        shufps      xmm4, xmm4, 55h;
        addss       xmm6, xmm4;
        addss       xmm0, xmm6;
        xorps       xmm0, xmmword ptr [siiii];
        movss       dword ptr [eax+0Ch], xmm0;
        mov         esp, ebp;
        pop         ebp;
        mov         esp, ebx;
        pop         ebx;
        ret         10h;
    }
}

static struct mathfn_initializer
{
    mathfn_initializer()
    {
        vec2normalize = sse2_vec2normalize;
        vec2hermite = sse_vec2hermite;
        vec2catmullrom = sse_vec2catmullrom;
        vec2barycentric = sse_vec2barycentric;
        vec2transform = sse_vec2transform;
        //vec2transformcoord = sse_vec2transformcoord;
        //vec2transformnormal = sse_vec2transformnormal;
        vec2transformarray = sse2_vec2transformarray;
        vec2transformcoordarray = sse2_vec2transformcoordarray;
        vec2transformnormalarray = sse2_vec2transformnormalarray;
        vec3normalize = sse2_vec3normalize;
        vec3hermite = sse_vec3hermite;
        vec3catmullrom = sse_vec3catmullrom;
        vec3barycentric = sse_vec3barycentric;
        vec3transform = sse2_vec3transform;
        vec3transformcoord = sse_vec3transformcoord;
        vec3transformnormal = sse2_vec3transformnormal;
        vec3transformarray = sse2_vec3transformarray;
        vec3transformcoordarray = sse2_vec3transformcoordarray;
        vec3transformnormalarray = sse2_vec3transformnormalarray;
        vec4cross = sse2_vec4cross;
        vec4normalize = sse_vec4normalize;
        vec4hermite = sse_vec4hermite;
        vec4catmullrom = sse_vec4catmullrom;
        vec4barycentric = sse_vec4barycentric;
        vec4transform = sse_vec4transform;
        vec4transformarray = sse2_vec4transformarray;
        matdeterminant = sse2_matdeterminant;
        matmultiply = sse_matmultiply;
        matmultiplytranspose = sse2_matmultiplytranspose;
        matinverse = sse2_matinverse;
        matshadow = sse2_matshadow;
        matreflect = sse2_matreflect;
        quatrotateeuler = sse2_quatrotateeuler;
        quatmultiply = sse_quatmultiply;
        quatnormalize = sse_quatnormalize;
        quatinverse = sse2_quatinverse;
        planenormalize = sse2_planenormalize;
        planeintersectline = sse_planeintersectline;
        planefrompoints = sse2_planefrompoints;
        planetransform = (fnplanetransform)sse_vec4transform;
        planetransformarray = (fnplanetransformarray)sse2_vec4transformarray;
    }
} __mfi_inst;

fnvec2normalize vec2normalize;
fnvec2hermite vec2hermite;
fnvec2catmullrom vec2catmullrom;
fnvec2barycentric vec2barycentric;
fnvec2transform vec2transform;
//fnvec2transformcoord vec2transformcoord;
//fnvec2transformnormal vec2transformnormal;
fnvec2transformarray vec2transformarray;
fnvec2transformcoordarray vec2transformcoordarray;
fnvec2transformnormalarray vec2transformnormalarray;
fnvec3normalize vec3normalize;
fnvec3hermite vec3hermite;
fnvec3catmullrom vec3catmullrom;
fnvec3barycentric vec3barycentric;
fnvec3transform vec3transform;
fnvec3transformcoord vec3transformcoord;
fnvec3transformnormal vec3transformnormal;
fnvec3transformarray vec3transformarray;
fnvec3transformcoordarray vec3transformcoordarray;
fnvec3transformnormalarray vec3transformnormalarray;
fnvec4cross vec4cross;
fnvec4normalize vec4normalize;
fnvec4hermite vec4hermite;
fnvec4catmullrom vec4catmullrom;
fnvec4barycentric vec4barycentric;
fnvec4transform vec4transform;
fnvec4transformarray vec4transformarray;
//fnmat3determinant mat3determinant;
//fnmat3multiply mat3multiply;
fnmatdeterminant matdeterminant;
fnmatmultiply matmultiply;
fnmatmultiplytranspose matmultiplytranspose;
fnmatinverse matinverse;
fnmatshadow matshadow;
fnmatreflect matreflect;
fnquatrotateeuler quatrotateeuler;
fnquatmultiply quatmultiply;
fnquatnormalize quatnormalize;
fnquatinverse quatinverse;
fnplanenormalize planenormalize;
fnplaneintersectline planeintersectline;
fnplanefrompoints planefrompoints;
fnplanetransform planetransform;
fnplanetransformarray planetransformarray;

__gslib_end__
