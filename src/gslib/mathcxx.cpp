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

#include <gslib/type.h>
#include <gslib/math.h>

__gslib_begin__

#define gs_isnan(x)  ((*(uint*)&(x) & 0x7f800000) == 0x7f800000 && (*(uint*)&(x) & 0x7fffff) != 0)
#define gs_isinf(x)  ((*(uint*)&(x) & 0x7fffffff) == 0x7f800000)

#define GS_PERMUTE_0X       0x00010203
#define GS_PERMUTE_0Y       0x04050607
#define GS_PERMUTE_0Z       0x08090a0b
#define GS_PERMUTE_0W       0x0c0d0e0f
#define GS_PERMUTE_1X       0x10111213
#define GS_PERMUTE_1Y       0x14151617
#define GS_PERMUTE_1Z       0x18191a1b
#define GS_PERMUTE_1W       0x1c1d1e1f
#define GS_SELECT_0         0x00000000
#define GS_SELECT_1         0xffffffff

union vec4_u32
{
    uint u[4];
    vec4 v;
};

union vec4_i32
{
    int u[4];
    vec4 v;
};

inline vec3 vector_near_equal(const vec3& v1, const vec3& v2, const vec3& epsilon)
{
    float dx = v1.x - v2.x;
    float dy = v1.y - v2.y;
    float dz = v1.z - v2.z;
    dx = fabsf(dx);
    dy = fabsf(dy);
    dz = fabsf(dz);
    vec4_u32 control = { 0 };
    control.u[0] = (dx <= epsilon.x) ? 0xffffffffu : 0;
    control.u[1] = (dy <= epsilon.y) ? 0xffffffffu : 0;
    control.u[2] = (dz <= epsilon.z) ? 0xffffffffu : 0;
    return vec3(control.v.x, control.v.y, control.v.z);
}

inline vec4 vector_less(const vec4& v1, const vec4& v2)
{
    vec4_u32 control = { 0 };
    control.u[0] = (v1.x < v2.x) ? 0xffffffff : 0;
    control.u[1] = (v1.y < v2.y) ? 0xffffffff : 0;
    control.u[2] = (v1.z < v2.z) ? 0xffffffff : 0;
    control.u[3] = (v1.w < v2.w) ? 0xffffffff : 0;
    return control.v;
}

inline vec4 vector_less_or_equal(const vec4& v1, const vec4& v2)
{
    vec4_u32 control = { 0 };
    control.u[0] = (v1.x <= v2.x) ? 0xffffffff : 0;
    control.u[1] = (v1.y <= v2.y) ? 0xffffffff : 0;
    control.u[2] = (v1.z <= v2.z) ? 0xffffffff : 0;
    control.u[3] = (v1.z <= v2.w) ? 0xffffffff : 0;
    return control.v;
}

inline bool vector_is_infinite(const vec4& v)
{
    return (gs_isinf(v.x) ||
        gs_isinf(v.y) ||
        gs_isinf(v.z) ||
        gs_isinf(v.w));
}

inline vec3 vector_select(const vec3& v1, const vec3& v2, const vec3& control)
{
    vec4_u32 u1 = { 0 }, u2 = { 0 }, uc = { 0 };
    vec4_u32 result = { 0 };
    memcpy(&u1.v, &v1, sizeof(vec3));
    memcpy(&u2.v, &v2, sizeof(vec3));
    memcpy(&uc.v, &control, sizeof(vec3));
    result.u[0] = (u1.u[0] & ~uc.u[0]) | (u2.u[0] & uc.u[0]);
    result.u[1] = (u1.u[1] & ~uc.u[1]) | (u2.u[1] & uc.u[1]);
    result.u[2] = (u1.u[2] & ~uc.u[2]) | (u2.u[2] & uc.u[2]);
    return vec3(result.v.x, result.v.y, result.v.z);
}

inline vec4 vector_select(const vec4& v1, const vec4& v2, const vec4& control)
{
    vec4_u32 u1 = { 0 }, u2 = { 0 }, uc = { 0 };
    vec4_u32 result = { 0 };
    u1.v = v1; u2.v = v2; uc.v = control;
    result.u[0] = (u1.u[0] & ~uc.u[0]) | (u2.u[0] & uc.u[0]);
    result.u[1] = (u1.u[1] & ~uc.u[1]) | (u2.u[1] & uc.u[1]);
    result.u[2] = (u1.u[2] & ~uc.u[2]) | (u2.u[2] & uc.u[2]);
    result.u[3] = (u1.u[3] & ~uc.u[3]) | (u2.u[3] & uc.u[3]);
    return result.v;
}

inline vec4 vector_permute(const vec4& v1, const vec4& v2, const vec4& control)
{
    // Indices must be in range from 0 to 31
    assert((uint(control.x) & 0xe0e0e0e0) == 0);
    assert((uint(control.y) & 0xe0e0e0e0) == 0);
    assert((uint(control.z) & 0xe0e0e0e0) == 0);
    assert((uint(control.w) & 0xe0e0e0e0) == 0);
    const byte *abyte[2];
    // 0-15 = V1, 16-31 = V2
    abyte[0] = (const byte*)(&v1);
    abyte[1] = (const byte*)(&v2);
    uint i = 16;
    uint ui, vi;
    const byte *pcontrol = (const byte *)(&control);
    vec4 result;
    byte* pwork = (byte*)(&result);
    do {
        // Get the byte to map from
        ui = pcontrol[0];
        ++ pcontrol;
        vi = (ui >> 4) & 1;
        ui &= 0x0F;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        ui ^= 3; // Swap byte ordering on little endian machines
#endif
        pwork[0] = abyte[vi][ui];
        ++ pwork;
    } while(-- i);
    return result;
}

inline vec3 vector_splat_x(const vec3& v)
{
    return vec3(v.x, v.x, v.x);
}

inline vec3 vector_splat_y(const vec3& v)
{
    return vec3(v.y, v.y, v.y);
}

inline vec3 vector_splat_z(const vec3& v)
{
    return vec3(v.z, v.z, v.z);
}

inline vec4 vector_splat_x(const vec4& v)
{
    return vec4(v.x, v.x, v.x, v.x);
}

inline vec4 vector_splat_y(const vec4& v)
{
    return vec4(v.y, v.y, v.y, v.y);
}

inline vec4 vector_splat_z(const vec4& v)
{
    return vec4(v.z, v.z, v.z, v.z);
}

inline vec4 vector_splat_w(const vec4& v)
{
    return vec4(v.w, v.w, v.w, v.w);
}

inline vec2 vector2_replicate(float v)
{
    return vec2(v, v);
}

inline vec3 vector3_replicate(float v)
{
    return vec3(v, v, v);
}

inline vec4 vector4_replicate(float v)
{
    return vec4(v, v, v, v);
}

inline vec2 vector_multiply(const vec2 &v1, const vec2 &v2)
{
    return vec2(v1.x * v2.x, v1.y * v2.y);
}

inline vec3 vector_multiply(const vec3& v1, const vec3& v2)
{
    return vec3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

inline vec4 vector_multiply(const vec4& v1, const vec4& v2)
{
    return vec4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

inline vec2 vector_multiply_add(const vec2& v1, const vec2& v2, const vec2& v3)
{
    return vec2(v1.x * v2.x + v3.x, v1.y * v2.y + v3.y);
}

inline vec3 vector_multiply_add(const vec3& v1, const vec3& v2, const vec3& v3)
{
    return vec3(
        v1.x * v2.x + v3.x,
        v1.y * v2.y + v3.y,
        v1.z * v2.z + v3.z
    );
}

inline vec4 vector_multiply_add(const vec4& v1, const vec4& v2, const vec4& v3)
{
    return vec4(
        v1.x * v2.x + v3.x,
        v1.y * v2.y + v3.y,
        v1.z * v2.z + v3.z,
        v1.w * v2.w + v3.w
    );
}

inline vec4 vector_negative_multiply_subtract(const vec4& v1, const vec4& v2, const vec4& v3)
{
    return vec4(
        v3.x - (v1.x * v2.x),
        v3.y - (v1.y * v2.y),
        v3.z - (v1.z * v2.z),
        v3.w - (v1.w * v2.w)
    );
}

inline vec3 vector_reciprocal(const vec3& v)
{
    union union3
    {
        float f[3];
        uint u[3];
        _vec3 v;
    };
    union3 v3;
    v3.v = v;
    union3 result;
    result.f[0] = 0.f;
    for(int i = 0; i < 3; ++ i) {
        if(gs_isnan(v3.f[i]))
            result.u[i] = 0x7fc00000;
        else if(v3.f[i] == 0.0f || v3.f[i] == -0.0f)
            result.u[i] = 0x7f800000 | (v3.u[i] & 0x80000000);
        else
            result.f[i] = 1.f / v3.f[i];
    }
    return vec3(result.v.x, result.v.y, result.v.z);
}

inline vec4 vector_reciprocal(const vec4& v)
{
    union union4
    {
        float f[4];
        uint u[4];
        _vec4 v;
    };
    union4 v4;
    v4.v = v;
    union4 result;
    result.f[0] = 0.f;
    for(int i = 0; i < 4; ++ i) {
        if(gs_isnan(v4.f[i]))
            result.u[i] = 0x7fc00000;
        else if(v4.f[i] == 0.f || v4.f[i] == -0.f)
            result.u[i] = 0x7f800000 | (v4.u[i] & 0x80000000);
        else
            result.f[i] = 1.f / v4.f[i];
    }
    return vec4(result.v.x, result.v.y, result.v.z, result.v.w);
}

inline vec4 vector_truncate(const vec4& v)
{
    union union4
    {
        float f[4];
        uint u[4];
        _vec4 v;
    };
    union4 v4;
    v4.v = v;
    union4 result;
    result.f[0] = 0.f;
    for(int i = 0; i < 4; ++ i) {
        if(gs_isnan(v4.f[i]))
            result.u[i] = 0x7fc00000;
        else if(fabsf(v4.f[i]) < 8388608.0f)
            result.f[i] = (float)((int)v4.f[i]);
        else
            result.f[i] = v4.f[i];
    }
    return vec4(result.v.x, result.v.y, result.v.z, result.v.w);
}

inline vec4 vector_round(const vec4& v)
{
    const vec4 Zero(0.0f, 0.0f, 0.0f, 0.0f);
    const vec4 BiasPos = vector4_replicate(0.5f);
    const vec4 BiasNeg = vector4_replicate(-0.5f);
    vec4 bias = vector_less(v, Zero);
    bias = vector_select(BiasPos, BiasNeg, bias);
    vec4 result = v + bias;
    result = vector_truncate(result);
    return result;
}

inline vec4 vector_mod_angles(const vec4& angles)
{
    const float _1DIV2PI = 0.159154943f;
    const float _2PI = 6.283185307f;
    static const vec4 ReciprocalTwoPi = { _1DIV2PI, _1DIV2PI, _1DIV2PI, _1DIV2PI };
    static const vec4 TwoPi = { _2PI, _2PI, _2PI, _2PI };
    // Modulo the range of the given angles such that -K3D_PI <= Angles < K3D_PI
    vec4 v = vector_multiply(angles, ReciprocalTwoPi);
    v = vector_round(v);
    vec4 result = vector_negative_multiply_subtract(TwoPi, v, angles);
    return result;
}

/* sin(V) ~= V - V^3 / 3! + V^5 / 5! - V^7 / 7! + V^9 / 9! - V^11 / 11! + V^13 / 13! -
 *           V^15 / 15! + V^17 / 17! - V^19 / 19! + V^21 / 21! - V^23 / 23! (for -PI <= V < PI)
 * cos(V) ~= 1 - V^2 / 2! + V^4 / 4! - V^6 / 6! + V^8 / 8! - V^10 / 10! + V^12 / 12! -
 *           V^14 / 14! + V^16 / 16! - V^18 / 18! + V^20 / 20! - V^22 / 22! (for -PI <= V < PI)
 */
inline void vector_sin_cos(vec4* psin, vec4* pcos, const vec4& v)
{
    vec4 V1, V2, V3, V4, V5, V6, V7, V8, V9, V10, V11, V12, V13;
    vec4 V14, V15, V16, V17, V18, V19, V20, V21, V22, V23;
    vec4 S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11;
    vec4 C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11;
    vec4 Sin, Cos;
    assert(psin);
    assert(pcos);
    V1 = vector_mod_angles(v);
    static const vec4 XMSinCoefficients0 = { 1.0f, -0.166666667f, 8.333333333e-3f, -1.984126984e-4f };
    static const vec4 XMSinCoefficients1 = { 2.755731922e-6f, -2.505210839e-8f, 1.605904384e-10f, -7.647163732e-13f };
    static const vec4 XMSinCoefficients2 = { 2.811457254e-15f, -8.220635247e-18f, 1.957294106e-20f, -3.868170171e-23f };
    static const vec4 XMCosCoefficients0 = { 1.0f, -0.5f, 4.166666667e-2f, -1.388888889e-3f };
    static const vec4 XMCosCoefficients1 = { 2.480158730e-5f, -2.755731922e-7f, 2.087675699e-9f, -1.147074560e-11f };
    static const vec4 XMCosCoefficients2 = { 4.779477332e-14f, -1.561920697e-16f, 4.110317623e-19f, -8.896791392e-22f };
    static const vec4 XMOne = { 1.0f, 1.0f, 1.0f, 1.0f };
    V2 = vector_multiply(V1, V1);
    V3 = vector_multiply(V2, V1);
    V4 = vector_multiply(V2, V2);
    V5 = vector_multiply(V3, V2);
    V6 = vector_multiply(V3, V3);
    V7 = vector_multiply(V4, V3);
    V8 = vector_multiply(V4, V4);
    V9 = vector_multiply(V5, V4);
    V10 = vector_multiply(V5, V5);
    V11 = vector_multiply(V6, V5);
    V12 = vector_multiply(V6, V6);
    V13 = vector_multiply(V7, V6);
    V14 = vector_multiply(V7, V7);
    V15 = vector_multiply(V8, V7);
    V16 = vector_multiply(V8, V8);
    V17 = vector_multiply(V9, V8);
    V18 = vector_multiply(V9, V9);
    V19 = vector_multiply(V10, V9);
    V20 = vector_multiply(V10, V10);
    V21 = vector_multiply(V11, V10);
    V22 = vector_multiply(V11, V11);
    V23 = vector_multiply(V12, V11);
    S1 = vector_splat_y(XMSinCoefficients0);
    S2 = vector_splat_z(XMSinCoefficients0);
    S3 = vector_splat_w(XMSinCoefficients0);
    S4 = vector_splat_x(XMSinCoefficients1);
    S5 = vector_splat_y(XMSinCoefficients1);
    S6 = vector_splat_z(XMSinCoefficients1);
    S7 = vector_splat_w(XMSinCoefficients1);
    S8 = vector_splat_x(XMSinCoefficients2);
    S9 = vector_splat_y(XMSinCoefficients2);
    S10 = vector_splat_z(XMSinCoefficients2);
    S11 = vector_splat_w(XMSinCoefficients2);
    C1 = vector_splat_y(XMCosCoefficients0);
    C2 = vector_splat_z(XMCosCoefficients0);
    C3 = vector_splat_w(XMCosCoefficients0);
    C4 = vector_splat_x(XMCosCoefficients1);
    C5 = vector_splat_y(XMCosCoefficients1);
    C6 = vector_splat_z(XMCosCoefficients1);
    C7 = vector_splat_w(XMCosCoefficients1);
    C8 = vector_splat_x(XMCosCoefficients2);
    C9 = vector_splat_y(XMCosCoefficients2);
    C10 = vector_splat_z(XMCosCoefficients2);
    C11 = vector_splat_w(XMCosCoefficients2);
    Sin = vector_multiply_add(S1, V3, V1);
    Sin = vector_multiply_add(S2, V5, Sin);
    Sin = vector_multiply_add(S3, V7, Sin);
    Sin = vector_multiply_add(S4, V9, Sin);
    Sin = vector_multiply_add(S5, V11, Sin);
    Sin = vector_multiply_add(S6, V13, Sin);
    Sin = vector_multiply_add(S7, V15, Sin);
    Sin = vector_multiply_add(S8, V17, Sin);
    Sin = vector_multiply_add(S9, V19, Sin);
    Sin = vector_multiply_add(S10, V21, Sin);
    Sin = vector_multiply_add(S11, V23, Sin);
    Cos = vector_multiply_add(C1, V2, XMOne);
    Cos = vector_multiply_add(C2, V4, Cos);
    Cos = vector_multiply_add(C3, V6, Cos);
    Cos = vector_multiply_add(C4, V8, Cos);
    Cos = vector_multiply_add(C5, V10, Cos);
    Cos = vector_multiply_add(C6, V12, Cos);
    Cos = vector_multiply_add(C7, V14, Cos);
    Cos = vector_multiply_add(C8, V16, Cos);
    Cos = vector_multiply_add(C9, V18, Cos);
    Cos = vector_multiply_add(C10, V20, Cos);
    Cos = vector_multiply_add(C11, V22, Cos);
    *psin = Sin;
    *pcos = Cos;
}

inline vec4 vector_rotate_left(const vec4& v, uint elements)
{
    assert(elements < 4);
    union union4
    {
        float f[4];
        _vec4 v;
    };
    union4 v4; v4.v = v;
    return vec4(v4.f[elements & 3],
        v4.f[(elements + 1) & 3],
        v4.f[(elements + 2) & 3],
        v4.f[(elements + 3) & 3]
    );
}

inline void matrix_from_vector4(matrix* o, const vec4& v1, const vec4& v2, const vec4& v3, const vec4& v4)
{
    assert(o);
    memcpy(&o->m[0][0], &v1, sizeof(vec4));
    memcpy(&o->m[1][0], &v2, sizeof(vec4));
    memcpy(&o->m[2][0], &v3, sizeof(vec4));
    memcpy(&o->m[3][0], &v4, sizeof(vec4));
}

static vec2* __stdcall c_vec2normalize(vec2* o, const vec2* v)
{
    assert(o && v);
    float len = vec2length(v);
    if(abs(len) > 1e-4f) {
        vec2 result = (*v / len);
        o->x = result.x;
        o->y = result.y;
    }
    return o;
}

/* result = (2 * s^3 - 3 * s^2 + 1) * p1 +
 *          (s^3 - 2 * s^2 + s) * t1 +
 *          (-2 * s^3 + 3 * s^2) * p2 +
 *          (s^3 - s^2) * t2
 */
static vec2* __stdcall c_vec2hermite(vec2* o, const vec2* p1, const vec2* t1, const vec2* p2, const vec2* t2, float s)
{
    assert(o && p1 && t1 && p2 && t2);
    float ft2 = s * s;
    float ft3 = s * ft2;
    vec2 P0 = vector2_replicate(2.0f * ft3 - 3.0f * ft2 + 1.0f);
    vec2 T0 = vector2_replicate(ft3 - 2.0f * ft2 + s);
    vec2 P1 = vector2_replicate(-2.0f * ft3 + 3.0f * ft2);
    vec2 T1 = vector2_replicate(ft3 - ft2);
    vec2 result = vector_multiply(P0, *p1);
    result = vector_multiply_add(T0, *t1, result);
    result = vector_multiply_add(P1, *p2, result);
    result = vector_multiply_add(T1, *t2, result);
    o->x = result.x;
    o->y = result.y;
    return o;
}

/* result = ((-s^3 + 2 * s^2 - s) * p1 +
 *           (3 * s^3 - 5 * s^2 + 2) * p2 +
 *           (-3 * s^3 + 4 * s^2 + s) * p3 +
 *           (s^3 - s^2) * v4) * 0.5
 */
static vec2* __stdcall c_vec2catmullrom(vec2* o, const vec2* p1, const vec2* p2, const vec2* p3, const vec2* p4, float s)
{
    assert(o && p1 && p2 && p3 && p4);
    float t2 = s * s;
    float t3 = s * t2;
    vec2 P0 = vector2_replicate((-t3 + 2.0f * t2 - s) * 0.5f);
    vec2 P1 = vector2_replicate((3.0f * t3 - 5.0f * t2 + 2.0f) * 0.5f);
    vec2 P2 = vector2_replicate((-3.0f * t3 + 4.0f * t2 + s) * 0.5f);
    vec2 P3 = vector2_replicate((t3 - t2) * 0.5f);
    vec2 result = vector_multiply(P0, *p1);
    result = vector_multiply_add(P1, *p2, result);
    result = vector_multiply_add(P2, *p3, result);
    result = vector_multiply_add(P3, *p4, result);
    o->x = result.x;
    o->y = result.y;
    return o;
}

static vec2* __stdcall c_vec2barycentric(vec2* o, const vec2* p1, const vec2* p2, const vec2* p3, float f, float g)
{
    assert(o && p1 && p2 && p3);
    vec2 P10 = *p2 - *p1;
    vec2 ScaleF = vector2_replicate(f);
    vec2 P20 = *p3 - *p1;
    vec2 ScaleG = vector2_replicate(g);
    vec2 result = vector_multiply_add(P10, ScaleF, *p1);
    result = vector_multiply_add(P20, ScaleG, result);
    o->x = result.x;
    o->y = result.y;
    return o;
}

static vec4* __stdcall c_vec2transform(vec4* o, const vec2* p, const matrix* m)
{
    assert(o && p && m);
    vec4 V(p->x, p->y, 0.0f, 0.0f);
    vec4 Y = vector_splat_y(V);
    vec4 X = vector_splat_x(V);
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 r3(m->_41, m->_42, m->_43, m->_44);
    vec4 result = vector_multiply_add(Y, r1, r3);
    result = vector_multiply_add(X, r0, result);
    memcpy(o, &result, sizeof(vec4));
    return o;
}

static vec2* __stdcall c_vec2transformcoord(vec2* o, const vec2* p, const matrix* m)
{
    assert(o && p && m);
    vec4 V(p->x, p->y, 0.f, 0.f);
    vec4 Y = vector_splat_y(V);
    vec4 X = vector_splat_x(V);
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 r3(m->_41, m->_42, m->_43, m->_44);
    vec4 result = vector_multiply_add(Y, r1, r3);
    result = vector_multiply_add(X, r0, result);
    vec4 InverseW = vector_splat_w(result);
    InverseW = vector_reciprocal(InverseW);
    result = vector_multiply(result, InverseW);
    o->x = result.x;
    o->y = result.y;
    return o;
}

static vec2* __stdcall c_vec2transformnormal(vec2* o, const vec2* p, const matrix* m)
{
    assert(o && p && m);
    vec4 V(p->x, p->y, 0.0f, 0.0f);
    vec4 Y = vector_splat_y(V);
    vec4 X = vector_splat_x(V);
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 result = vector_multiply(Y, r1);
    result = vector_multiply_add(X, r0, result);
    o->x = result.x;
    o->y = result.y;
    return o;
}

static vec4* __stdcall c_vec2transformarray(vec4* o, uint ostride, const vec2* v, uint vstride, const matrix* m, uint n)
{
    assert(o);
    assert(v);
    byte *pInputVector = (byte*)v;
    byte *pOutputVector = (byte*)o;
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 r3(m->_41, m->_42, m->_43, m->_44);
    for(uint i = 0; i < n; ++ i) {
        vec4 V;
        vec2 *input = (vec2*)pInputVector;
        ((uint*)(&V.x))[0] = ((const uint*)(&input->x))[0];
        ((uint*)(&V.y))[0] = ((const uint*)(&input->y))[0];
        vec4 Y = vector_splat_y(V);
        vec4 X = vector_splat_x(V);
        vec4 result = vector_multiply_add(Y, r1, r3);
        result = vector_multiply_add(X, r0, result);
        vec4 *out = (vec4*)pOutputVector;
        out->x = result.x;
        out->y = result.y;
        out->z = result.z;
        out->w = result.w;
        pInputVector += vstride;
        pOutputVector += ostride;
    }
    return o;
}

static vec2* __stdcall c_vec2transformcoordarray(vec2* o, uint ostride, const vec2* v, uint vstride, const matrix* m, uint n)
{
    assert(o);
    assert(v);
    byte *pInputVector = (byte*)v;
    byte *pOutputVector = (byte*)o;
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 r3(m->_41, m->_42, m->_43, m->_44);
    for(uint i = 0; i < n; ++ i) {
        vec4 V;
        vec2 *input = (vec2*)pInputVector;
        ((uint*)(&V.x))[0] = ((const uint*)(&input->x))[0];
        ((uint*)(&V.y))[0] = ((const uint*)(&input->y))[0];
        vec4 Y = vector_splat_y(V);
        vec4 X = vector_splat_x(V);
        vec4 result = vector_multiply_add(Y, r1, r3);
        result = vector_multiply_add(X, r0, result);
        vec4 InverseW = vector_splat_w(result);
        InverseW = vector_reciprocal(InverseW);
        result = vector_multiply(result, InverseW);
        vec2 *out = (vec2*)pOutputVector;
        out->x = result.x;
        out->y = result.y;
        pInputVector += vstride;
        pOutputVector += ostride;
    }
    return o;
}

static vec2* __stdcall c_vec2transformnormalarray(vec2* o, uint ostride, const vec2* v, uint vstride, const matrix* m, uint n)
{
    assert(o);
    assert(v);
    byte* pInputVector = (byte*)v;
    byte* pOutputVector = (byte*)o;
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    for(uint i = 0; i < n; ++ i) {
        vec4 V;
        vec2 *input = (vec2*)pInputVector;
        ((uint*)(&V.x))[0] = ((const uint*)(&input->x))[0];
        ((uint*)(&V.y))[0] = ((const uint*)(&input->y))[0];
        vec4 Y = vector_splat_y(V);
        vec4 X = vector_splat_x(V);
        vec4 result = vector_multiply(Y, r1);
        result = vector_multiply_add(X, r0, result);
        vec2 *out = (vec2*)pOutputVector;
        out->x = result.x;
        out->y = result.y;
        pInputVector += vstride;
        pOutputVector += ostride;
    }
    return o;
}

static vec3* __stdcall c_vec3normalize(vec3* o, const vec3* v)
{
    assert(o && v);
    float len = vec3length(v);
    if(abs(len) > 1e-4f) {
        vec3 result = (*v / len);
        o->x = result.x;
        o->y = result.y;
        o->z = result.z;
    }
    return o;
}

static vec3* __stdcall c_vec3hermite(vec3* o, const vec3* v1, const vec3* t1, const vec3* v2, const vec3* t2, float s)
{
    assert(o && v1 && t1 && v2 && t2);
    float ft2 = s * s;
    float ft3 = s * ft2;
    vec3 P0 = vector3_replicate(2.0f * ft3 - 3.0f * ft2 + 1.0f);
    vec3 T0 = vector3_replicate(ft3 - 2.0f * ft2 + s);
    vec3 P1 = vector3_replicate(-2.0f * ft3 + 3.0f * ft2);
    vec3 T1 = vector3_replicate(ft3 - ft2);
    vec3 result = vector_multiply(P0, *v1);
    result = vector_multiply_add(T0, *t1, result);
    result = vector_multiply_add(P1, *v2, result);
    result = vector_multiply_add(T1, *t2, result);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    return o;
}

/* result = ((-s^3 + 2 * s^2 - s) * v1 +
 *          (3 * s^3 - 5 * s^2 + 2) * v2 +
 *          (-3 * s^3 + 4 * s^2 + s) * v3 +
 *          (s^3 - s^2) * v4) * 0.5
 */
static vec3* __stdcall c_vec3catmullrom(vec3* o, const vec3* v1, const vec3* v2, const vec3* v3, const vec3* v4, float s)
{
    assert(o && v1 && v2 && v3 && v4);
    float t2 = s * s;
    float t3 = s * t2;
    vec3 P0 = vector3_replicate((-t3 + 2.0f * t2 - s) * 0.5f);
    vec3 P1 = vector3_replicate((3.0f * t3 - 5.0f * t2 + 2.0f) * 0.5f);
    vec3 P2 = vector3_replicate((-3.0f * t3 + 4.0f * t2 + s) * 0.5f);
    vec3 P3 = vector3_replicate((t3 - t2) * 0.5f);
    vec3 result = vector_multiply(P0, *v1);
    result = vector_multiply_add(P1, *v2, result);
    result = vector_multiply_add(P2, *v3, result);
    result = vector_multiply_add(P3, *v4, result);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    return o;
}

static vec3* __stdcall c_vec3barycentric(vec3* o, const vec3* v1, const vec3* v2, const vec3* v3, float f, float g)
{
    assert(o && v1 && v2 && v3);
    vec3 P10 = *v2 - *v1;
    vec3 ScaleF = vector3_replicate(f);
    vec3 P20 = *v3 - *v1;
    vec3 ScaleG = vector3_replicate(g);
    vec3 result = vector_multiply_add(P10, ScaleF, *v1);
    result = vector_multiply_add(P20, ScaleG, result);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    return o;
}

static vec4* __stdcall c_vec3transform(vec4* o, const vec3* v, const matrix* m)
{
    assert(o && v && m);
    vec4 V(v->x, v->y, v->z, 0.0f);
    vec4 z = vector_splat_z(V);
    vec4 x = vector_splat_x(V);
    vec4 y = vector_splat_y(V);
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 r2(m->_31, m->_32, m->_33, m->_34);
    vec4 r3(m->_41, m->_42, m->_43, m->_44);
    vec4 result = vector_multiply_add(z, r2, r3);
    result = vector_multiply_add(y, r1, result);
    result = vector_multiply_add(x, r0, result);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    o->w = result.w;
    return o;
}

static vec3* __stdcall c_vec3transformcoord(vec3* o, const vec3* v, const matrix* m)
{
    assert(o && v && m);
    vec4 V(v->x, v->y, v->z, 0.f);
    vec4 Z = vector_splat_z(V);
    vec4 Y = vector_splat_y(V);
    vec4 X = vector_splat_x(V);
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 r2(m->_31, m->_32, m->_33, m->_34);
    vec4 r3(m->_41, m->_42, m->_43, m->_44);
    vec4 result = vector_multiply_add(Z, r2, r3);
    result = vector_multiply_add(Y, r1, result);
    result = vector_multiply_add(X, r0, result);
    vec4 InverseW = vector_splat_w(result);
    InverseW = vector_reciprocal(InverseW);
    result = vector_multiply(result, InverseW);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    return o;
}

static vec3* __stdcall c_vec3transformnormal(vec3* o, const vec3* v, const matrix* m)
{
    assert(o && v && m);
    vec3 Z = vector_splat_z(*v);
    vec3 Y = vector_splat_y(*v);
    vec3 X = vector_splat_x(*v);
    vec3 r0(m->_11, m->_12, m->_13);
    vec3 r1(m->_21, m->_22, m->_23);
    vec3 r2(m->_31, m->_32, m->_33);
    vec3 result = vector_multiply(Z, r2);
    result = vector_multiply_add(Y, r1, result);
    result = vector_multiply_add(X, r0, result);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    return o;
}

static vec4* __stdcall c_vec3transformarray(vec4* o, uint ostride, const vec3* v, uint vstride, const matrix* m, uint n)
{
    assert(o);
    assert(v);
    byte *pInputVector = (byte*)v;
    byte *pOutputVector = (byte*)o;
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 r2(m->_31, m->_32, m->_33, m->_34);
    vec4 r3(m->_41, m->_42, m->_43, m->_44);
    for(uint i = 0; i < n; ++ i) {
        vec4 V;
        vec3 *input = (vec3*)pInputVector;
        ((uint*)(&V.x))[0] = ((const uint*)(&input->x))[0];
        ((uint*)(&V.y))[0] = ((const uint*)(&input->y))[0];
        ((uint*)(&V.z))[0] = ((const uint*)(&input->z))[0];
        vec4 Z = vector_splat_z(V);
        vec4 Y = vector_splat_y(V);
        vec4 X = vector_splat_x(V);
        vec4 result = vector_multiply_add(Z, r2, r3);
        result = vector_multiply_add(Y, r1, result);
        result = vector_multiply_add(X, r0, result);
        vec4 *out = (vec4*)pOutputVector;
        out->x = result.x;
        out->y = result.y;
        out->z = result.z;
        out->w = result.w;
        pInputVector += vstride;
        pOutputVector += ostride;
    }
    return o;
}

static vec3* __stdcall c_vec3transformcoordarray(vec3* os, uint ostride, const vec3* v, uint vstride, const matrix* m, uint n)
{
    assert(os);
    assert(v);
    byte* pInputVector = (byte*)v;
    byte* pOutputVector = (byte*)os;
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 r2(m->_31, m->_32, m->_33, m->_34);
    vec4 r3(m->_41, m->_42, m->_43, m->_44);
    for(uint i = 0; i < n; ++ i) {
        vec4 V;
        vec3 *input = (vec3*)pInputVector;
        ((uint*)(&V.x))[0] = ((const uint*)(&input->x))[0];
        ((uint*)(&V.y))[0] = ((const uint*)(&input->y))[0];
        ((uint*)(&V.z))[0] = ((const uint*)(&input->z))[0];
        vec4 Z = vector_splat_z(V);
        vec4 Y = vector_splat_y(V);
        vec4 X = vector_splat_x(V);
        vec4 result = vector_multiply_add(Z, r2, r3);
        result = vector_multiply_add(Y, r1, result);
        result = vector_multiply_add(X, r0, result);
        vec4 InverseW = vector_splat_w(result);
        InverseW = vector_reciprocal(InverseW);
        result = vector_multiply(result, InverseW);
        vec3 *out = (vec3*)pOutputVector;
        out->x = result.x;
        out->y = result.y;
        out->z = result.z;
        pInputVector += vstride;
        pOutputVector += ostride;
    }
    return os;
}

static vec3* __stdcall c_vec3transformnormalarray(vec3* o, uint ostride, const vec3* v, uint vstride, const matrix* m, uint n)
{
    assert(o);
    assert(v);
    byte *pInputVector = (byte*)v;
    byte *pOutputVector = (byte*)o;
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 r2(m->_31, m->_32, m->_33, m->_34);
    for(uint i = 0; i < n; ++ i) {
        vec4 V;
        vec3* input = (vec3*)pInputVector;
        ((uint*)(&V.x))[0] = ((const uint*)(&input->x))[0];
        ((uint*)(&V.y))[0] = ((const uint*)(&input->y))[0];
        ((uint*)(&V.z))[0] = ((const uint*)(&input->z))[0];
        vec4 Z = vector_splat_z(V);
        vec4 Y = vector_splat_y(V);
        vec4 X = vector_splat_x(V);
        vec4 result = vector_multiply(Z, r2);
        result = vector_multiply_add(Y, r1, result);
        result = vector_multiply_add(X, r0, result);
        vec3 *out = (vec3*)pOutputVector;
        out->x = result.x;
        out->y = result.y;
        out->z = result.z;
        pInputVector += vstride;
        pOutputVector += ostride;
    }
    return o;
}

static vec4* __stdcall c_vec4cross(vec4* o, const vec4* v1, const vec4* v2, const vec4* v3)
{
    assert(o && v1 && v2 && v3);
    o->x = (((v2->z*v3->w) - (v2->w*v3->z))*v1->y) - (((v2->y*v3->w) - (v2->w*v3->y))*v1->z) + (((v2->y*v3->z) - (v2->z*v3->y))*v1->w);
    o->y = (((v2->w*v3->z) - (v2->z*v3->w))*v1->x) - (((v2->w*v3->x) - (v2->x*v3->w))*v1->z) + (((v2->z*v3->x) - (v2->x*v3->z))*v1->w);
    o->z = (((v2->y*v3->w) - (v2->w*v3->y))*v1->x) - (((v2->x*v3->w) - (v2->w*v3->x))*v1->y) + (((v2->x*v3->y) - (v2->y*v3->x))*v1->w);
    o->w = (((v2->z*v3->y) - (v2->y*v3->z))*v1->x) - (((v2->z*v3->x) - (v2->x*v3->z))*v1->y) + (((v2->y*v3->x) - (v2->x*v3->y))*v1->z);
    return o;
}

static vec4* __stdcall c_vec4normalize(vec4* o, const vec4* v)
{
    assert(o && v);
    float len = vec4length(v);
    if(abs(len) > 1e-4f) {
        vec4 result = (*v / len);
        o->x = result.x;
        o->y = result.y;
        o->z = result.z;
        o->w = result.w;
    }
    return o;
}

static vec4* __stdcall c_vec4hermite(vec4* o, const vec4* v1, const vec4* t1, const vec4* v2, const vec4* t2, float s)
{
    assert(o && v1 && t1 && v2 && t2);
    float ft2 = s * s;
    float ft3 = s * ft2;
    vec4 P0 = vector4_replicate(2.0f * ft3 - 3.0f * ft2 + 1.0f);
    vec4 T0 = vector4_replicate(ft3 - 2.0f * ft2 + s);
    vec4 P1 = vector4_replicate(-2.0f * ft3 + 3.0f * ft2);
    vec4 T1 = vector4_replicate(ft3 - ft2);
    vec4 result = vector_multiply(P0, *v1);
    result = vector_multiply_add(T0, *t1, result);
    result = vector_multiply_add(P1, *v2, result);
    result = vector_multiply_add(T1, *t2, result);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    o->w = result.w;
    return o;
}

/* result = ((-s^3 + 2 * s^2 - s) * v1 +
 *          (3 * s^3 - 5 * s^2 + 2) * v2 +
 *          (-3 * s^3 + 4 * s^2 + s) * v3 +
 *          (s^3 - s^2) * v4) * 0.5
 */
static vec4* __stdcall c_vec4catmullrom(vec4* o, const vec4* v1, const vec4* v2, const vec4* v3, const vec4* v4, float s)
{
    assert(o && v1 && v2 && v3 && v4);
    float t2 = s * s;
    float t3 = s * t2;
    vec4 P0 = vector4_replicate((-t3 + 2.0f * t2 - s) * 0.5f);
    vec4 P1 = vector4_replicate((3.0f * t3 - 5.0f * t2 + 2.0f) * 0.5f);
    vec4 P2 = vector4_replicate((-3.0f * t3 + 4.0f * t2 + s) * 0.5f);
    vec4 P3 = vector4_replicate((t3 - t2) * 0.5f);
    vec4 result = vector_multiply(P0, *v1);
    result = vector_multiply_add(P1, *v2, result);
    result = vector_multiply_add(P2, *v3, result);
    result = vector_multiply_add(P3, *v4, result);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    o->w = result.w;
    return o;
}

// result = v1 + f * (v2 - v1) + g * (v3 - v1)
static vec4* __stdcall c_vec4barycentric(vec4* o, const vec4* v1, const vec4* v2, const vec4* v3, float f, float g)
{
    assert(o && v1 && v2 && v3);
    vec4 P10 = *v2 - *v1;
    vec4 ScaleF = vector4_replicate(f);
    vec4 P20 = *v3 - *v1;
    vec4 ScaleG = vector4_replicate(g);
    vec4 result = vector_multiply_add(P10, ScaleF, *v1);
    result = vector_multiply_add(P20, ScaleG, result);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    o->w = result.w;
    return o;
}

static vec4* __stdcall c_vec4transform(vec4* o, const vec4* v, const matrix* m)
{
    assert(o && v && m);
    o->x = (m->m[0][0] * v->x) + (m->m[1][0] * v->y) + (m->m[2][0] * v->z) + (m->m[3][0] * v->w);
    o->y = (m->m[0][1] * v->x) + (m->m[1][1] * v->y) + (m->m[2][1] * v->z) + (m->m[3][1] * v->w);
    o->z = (m->m[0][2] * v->x) + (m->m[1][2] * v->y) + (m->m[2][2] * v->z) + (m->m[3][2] * v->w);
    o->w = (m->m[0][3] * v->x) + (m->m[1][3] * v->y) + (m->m[2][3] * v->z) + (m->m[3][3] * v->w);
    return o;
}

static vec4* __stdcall c_vec4transformarray(vec4* o, uint ostride, const vec4* v, uint vstride, const matrix* m, uint n)
{
    assert(o);
    assert(v);
    assert(m);
    byte* pInputVector = (byte*)v;
    byte* pOutputVector = (byte*)o;
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 r2(m->_31, m->_32, m->_33, m->_34);
    vec4 r3(m->_41, m->_42, m->_43, m->_44);
    for(uint i = 0; i < n; ++ i) {
        vec4 V;
        vec4 *input = (vec4*)pInputVector;
        ((uint*)(&V.x))[0] = ((const uint*)(&input->x))[0];
        ((uint*)(&V.y))[0] = ((const uint*)(&input->y))[0];
        ((uint*)(&V.z))[0] = ((const uint*)(&input->z))[0];
        ((uint*)(&V.w))[0] = ((const uint*)(&input->w))[0];
        vec4 W = vector_splat_w(V);
        vec4 Z = vector_splat_z(V);
        vec4 Y = vector_splat_y(V);
        vec4 X = vector_splat_x(V);
        vec4 result = vector_multiply(W, r3);
        result = vector_multiply_add(Z, r2, result);
        result = vector_multiply_add(Y, r1, result);
        result = vector_multiply_add(X, r0, result);
        vec4 *out = (vec4*)pOutputVector;
        out->x = result.x;
        out->y = result.y;
        out->z = result.z;
        out->w = result.w;
        pInputVector += vstride;
        pOutputVector += ostride;
    }
    return o;
}

static float __stdcall c_matdeterminant(const matrix* m)
{
    assert(m);
    static const vec4_u32 SwizzleYXXX = { GS_PERMUTE_0Y, GS_PERMUTE_0X, GS_PERMUTE_0X, GS_PERMUTE_0X };
    static const vec4_u32 SwizzleZZYY = { GS_PERMUTE_0Z, GS_PERMUTE_0Z, GS_PERMUTE_0Y, GS_PERMUTE_0Y };
    static const vec4_u32 SwizzleWWWZ = { GS_PERMUTE_0W, GS_PERMUTE_0W, GS_PERMUTE_0W, GS_PERMUTE_0Z };
    static const vec4 Sign = { 1.0f, -1.0f, 1.0f, -1.0f };
    vec4 r0(m->_11, m->_12, m->_13, m->_14);
    vec4 r1(m->_21, m->_22, m->_23, m->_24);
    vec4 r2(m->_31, m->_32, m->_33, m->_34);
    vec4 r3(m->_41, m->_42, m->_43, m->_44);
    vec4 V0 = vector_permute(r2, r2, SwizzleYXXX.v);
    vec4 V1 = vector_permute(r3, r3, SwizzleZZYY.v);
    vec4 V2 = vector_permute(r2, r2, SwizzleYXXX.v);
    vec4 V3 = vector_permute(r3, r3, SwizzleWWWZ.v);
    vec4 V4 = vector_permute(r2, r2, SwizzleZZYY.v);
    vec4 V5 = vector_permute(r3, r3, SwizzleWWWZ.v);
    vec4 P0 = vector_multiply(V0, V1);
    vec4 P1 = vector_multiply(V2, V3);
    vec4 P2 = vector_multiply(V4, V5);
    V0 = vector_permute(r2, r2, SwizzleZZYY.v);
    V1 = vector_permute(r3, r3, SwizzleYXXX.v);
    V2 = vector_permute(r2, r2, SwizzleWWWZ.v);
    V3 = vector_permute(r3, r3, SwizzleYXXX.v);
    V4 = vector_permute(r2, r2, SwizzleWWWZ.v);
    V5 = vector_permute(r3, r3, SwizzleZZYY.v);
    P0 = vector_negative_multiply_subtract(V0, V1, P0);
    P1 = vector_negative_multiply_subtract(V2, V3, P1);
    P2 = vector_negative_multiply_subtract(V4, V5, P2);
    V0 = vector_permute(r1, r1, SwizzleWWWZ.v);
    V1 = vector_permute(r1, r1, SwizzleZZYY.v);
    V2 = vector_permute(r1, r1, SwizzleYXXX.v);
    vec4 S = vector_multiply(r0, Sign);
    vec4 R = vector_multiply(V0, P0);
    R = vector_negative_multiply_subtract(V1, P1, R);
    R = vector_multiply_add(V2, P2, R);
    float result = vec4dot(&S, &R);
    return result;
}

static matrix* __stdcall c_matmultiply(matrix* o, const matrix* m1, const matrix* m2)
{
    assert(o && m1 && m2);
    // Cache the invariants in registers
    float x = m1->m[0][0];
    float y = m1->m[0][1];
    float z = m1->m[0][2];
    float w = m1->m[0][3];
    // Perform the operation on the first row
    o->m[0][0] = (m2->m[0][0] * x) + (m2->m[1][0] * y) + (m2->m[2][0] * z) + (m2->m[3][0] * w);
    o->m[0][1] = (m2->m[0][1] * x) + (m2->m[1][1] * y) + (m2->m[2][1] * z) + (m2->m[3][1] * w);
    o->m[0][2] = (m2->m[0][2] * x) + (m2->m[1][2] * y) + (m2->m[2][2] * z) + (m2->m[3][2] * w);
    o->m[0][3] = (m2->m[0][3] * x) + (m2->m[1][3] * y) + (m2->m[2][3] * z) + (m2->m[3][3] * w);
    // Repeat for all the other rows
    x = m1->m[1][0];
    y = m1->m[1][1];
    z = m1->m[1][2];
    w = m1->m[1][3];
    o->m[1][0] = (m2->m[0][0] * x) + (m2->m[1][0] * y) + (m2->m[2][0] * z) + (m2->m[3][0] * w);
    o->m[1][1] = (m2->m[0][1] * x) + (m2->m[1][1] * y) + (m2->m[2][1] * z) + (m2->m[3][1] * w);
    o->m[1][2] = (m2->m[0][2] * x) + (m2->m[1][2] * y) + (m2->m[2][2] * z) + (m2->m[3][2] * w);
    o->m[1][3] = (m2->m[0][3] * x) + (m2->m[1][3] * y) + (m2->m[2][3] * z) + (m2->m[3][3] * w);
    x = m1->m[2][0];
    y = m1->m[2][1];
    z = m1->m[2][2];
    w = m1->m[2][3];
    o->m[2][0] = (m2->m[0][0] * x) + (m2->m[1][0] * y) + (m2->m[2][0] * z) + (m2->m[3][0] * w);
    o->m[2][1] = (m2->m[0][1] * x) + (m2->m[1][1] * y) + (m2->m[2][1] * z) + (m2->m[3][1] * w);
    o->m[2][2] = (m2->m[0][2] * x) + (m2->m[1][2] * y) + (m2->m[2][2] * z) + (m2->m[3][2] * w);
    o->m[2][3] = (m2->m[0][3] * x) + (m2->m[1][3] * y) + (m2->m[2][3] * z) + (m2->m[3][3] * w);
    x = m1->m[3][0];
    y = m1->m[3][1];
    z = m1->m[3][2];
    w = m1->m[3][3];
    o->m[3][0] = (m2->m[0][0] * x) + (m2->m[1][0] * y) + (m2->m[2][0] * z) + (m2->m[3][0] * w);
    o->m[3][1] = (m2->m[0][1] * x) + (m2->m[1][1] * y) + (m2->m[2][1] * z) + (m2->m[3][1] * w);
    o->m[3][2] = (m2->m[0][2] * x) + (m2->m[1][2] * y) + (m2->m[2][2] * z) + (m2->m[3][2] * w);
    o->m[3][3] = (m2->m[0][3] * x) + (m2->m[1][3] * y) + (m2->m[2][3] * z) + (m2->m[3][3] * w);
    return o;
}

static matrix* __stdcall c_matmultiplytranspose(matrix* o, const matrix* m1, const matrix* m2)
{
    assert(o && m1 && m2);
    // Cache the invariants in registers
    float x = m2->m[0][0];
    float y = m2->m[1][0];
    float z = m2->m[2][0];
    float w = m2->m[3][0];
    // Perform the operation on the first row
    o->m[0][0] = (m1->m[0][0] * x) + (m1->m[0][1] * y) + (m1->m[0][2] * z) + (m1->m[0][3] * w);
    o->m[0][1] = (m1->m[1][0] * x) + (m1->m[1][1] * y) + (m1->m[1][2] * z) + (m1->m[1][3] * w);
    o->m[0][2] = (m1->m[2][0] * x) + (m1->m[2][1] * y) + (m1->m[2][2] * z) + (m1->m[2][3] * w);
    o->m[0][3] = (m1->m[3][0] * x) + (m1->m[3][1] * y) + (m1->m[3][2] * z) + (m1->m[3][3] * w);
    // Repeat for all the other rows
    x = m2->m[0][1];
    y = m2->m[1][1];
    z = m2->m[2][1];
    w = m2->m[3][1];
    o->m[1][0] = (m1->m[0][0] * x) + (m1->m[0][1] * y) + (m1->m[0][2] * z) + (m1->m[0][3] * w);
    o->m[1][1] = (m1->m[1][0] * x) + (m1->m[1][1] * y) + (m1->m[1][2] * z) + (m1->m[1][3] * w);
    o->m[1][2] = (m1->m[2][0] * x) + (m1->m[2][1] * y) + (m1->m[2][2] * z) + (m1->m[2][3] * w);
    o->m[1][3] = (m1->m[3][0] * x) + (m1->m[3][1] * y) + (m1->m[3][2] * z) + (m1->m[3][3] * w);
    x = m2->m[0][2];
    y = m2->m[1][2];
    z = m2->m[2][2];
    w = m2->m[3][2];
    o->m[2][0] = (m1->m[0][0] * x) + (m1->m[0][1] * y) + (m1->m[0][2] * z) + (m1->m[0][3] * w);
    o->m[2][1] = (m1->m[1][0] * x) + (m1->m[1][1] * y) + (m1->m[1][2] * z) + (m1->m[1][3] * w);
    o->m[2][2] = (m1->m[2][0] * x) + (m1->m[2][1] * y) + (m1->m[2][2] * z) + (m1->m[2][3] * w);
    o->m[2][3] = (m1->m[3][0] * x) + (m1->m[3][1] * y) + (m1->m[3][2] * z) + (m1->m[3][3] * w);
    x = m2->m[0][3];
    y = m2->m[1][3];
    z = m2->m[2][3];
    w = m2->m[3][3];
    o->m[3][0] = (m1->m[0][0] * x) + (m1->m[0][1] * y) + (m1->m[0][2] * z) + (m1->m[0][3] * w);
    o->m[3][1] = (m1->m[1][0] * x) + (m1->m[1][1] * y) + (m1->m[1][2] * z) + (m1->m[1][3] * w);
    o->m[3][2] = (m1->m[2][0] * x) + (m1->m[2][1] * y) + (m1->m[2][2] * z) + (m1->m[2][3] * w);
    o->m[3][3] = (m1->m[3][0] * x) + (m1->m[3][1] * y) + (m1->m[3][2] * z) + (m1->m[3][3] * w);
    return o;
}

static matrix* __stdcall c_matinverse(matrix* o, float* determinant, const matrix* m)
{
    assert(o && m);
    static const vec4_u32 SwizzleXXYY = { GS_PERMUTE_0X, GS_PERMUTE_0X, GS_PERMUTE_0Y, GS_PERMUTE_0Y };
    static const vec4_u32 SwizzleZWZW = { GS_PERMUTE_0Z, GS_PERMUTE_0W, GS_PERMUTE_0Z, GS_PERMUTE_0W };
    static const vec4_u32 SwizzleYZXY = { GS_PERMUTE_0Y, GS_PERMUTE_0Z, GS_PERMUTE_0X, GS_PERMUTE_0Y };
    static const vec4_u32 SwizzleZWYZ = { GS_PERMUTE_0Z, GS_PERMUTE_0W, GS_PERMUTE_0Y, GS_PERMUTE_0Z };
    static const vec4_u32 SwizzleWXWX = { GS_PERMUTE_0W, GS_PERMUTE_0X, GS_PERMUTE_0W, GS_PERMUTE_0X };
    static const vec4_u32 SwizzleZXYX = { GS_PERMUTE_0Z, GS_PERMUTE_0X, GS_PERMUTE_0Y, GS_PERMUTE_0X };
    static const vec4_u32 SwizzleYWXZ = { GS_PERMUTE_0Y, GS_PERMUTE_0W, GS_PERMUTE_0X, GS_PERMUTE_0Z };
    static const vec4_u32 SwizzleWZWY = { GS_PERMUTE_0W, GS_PERMUTE_0Z, GS_PERMUTE_0W, GS_PERMUTE_0Y };
    static const vec4_u32 Permute0X0Z1X1Z = { GS_PERMUTE_0X, GS_PERMUTE_0Z, GS_PERMUTE_1X, GS_PERMUTE_1Z };
    static const vec4_u32 Permute0Y0W1Y1W = { GS_PERMUTE_0Y, GS_PERMUTE_0W, GS_PERMUTE_1Y, GS_PERMUTE_1W };
    static const vec4_u32 Permute1Y0Y0W0X = { GS_PERMUTE_1Y, GS_PERMUTE_0Y, GS_PERMUTE_0W, GS_PERMUTE_0X };
    static const vec4_u32 Permute0W0X0Y1X = { GS_PERMUTE_0W, GS_PERMUTE_0X, GS_PERMUTE_0Y, GS_PERMUTE_1X };
    static const vec4_u32 Permute0Z1Y1X0Z = { GS_PERMUTE_0Z, GS_PERMUTE_1Y, GS_PERMUTE_1X, GS_PERMUTE_0Z };
    static const vec4_u32 Permute0W1Y0Y0Z = { GS_PERMUTE_0W, GS_PERMUTE_1Y, GS_PERMUTE_0Y, GS_PERMUTE_0Z };
    static const vec4_u32 Permute0Z0Y1X0X = { GS_PERMUTE_0Z, GS_PERMUTE_0Y, GS_PERMUTE_1X, GS_PERMUTE_0X };
    static const vec4_u32 Permute1Y0X0W1X = { GS_PERMUTE_1Y, GS_PERMUTE_0X, GS_PERMUTE_0W, GS_PERMUTE_1X };
    static const vec4_u32 Permute1W0Y0W0X = { GS_PERMUTE_1W, GS_PERMUTE_0Y, GS_PERMUTE_0W, GS_PERMUTE_0X };
    static const vec4_u32 Permute0W0X0Y1Z = { GS_PERMUTE_0W, GS_PERMUTE_0X, GS_PERMUTE_0Y, GS_PERMUTE_1Z };
    static const vec4_u32 Permute0Z1W1Z0Z = { GS_PERMUTE_0Z, GS_PERMUTE_1W, GS_PERMUTE_1Z, GS_PERMUTE_0Z };
    static const vec4_u32 Permute0W1W0Y0Z = { GS_PERMUTE_0W, GS_PERMUTE_1W, GS_PERMUTE_0Y, GS_PERMUTE_0Z };
    static const vec4_u32 Permute0Z0Y1Z0X = { GS_PERMUTE_0Z, GS_PERMUTE_0Y, GS_PERMUTE_1Z, GS_PERMUTE_0X };
    static const vec4_u32 Permute1W0X0W1Z = { GS_PERMUTE_1W, GS_PERMUTE_0X, GS_PERMUTE_0W, GS_PERMUTE_1Z };
    static const vec4_u32 Select0101 = { GS_SELECT_0, GS_SELECT_1, GS_SELECT_0, GS_SELECT_1 };
    matrix MT;
    mattranspose(&MT, m);
    vec4 V0[4], V1[4];
    vec4 r0(MT._11, MT._12, MT._13, MT._14);
    vec4 r1(MT._21, MT._22, MT._23, MT._24);
    vec4 r2(MT._31, MT._32, MT._33, MT._34);
    vec4 r3(MT._41, MT._42, MT._43, MT._44);
    V0[0] = vector_permute(r2, r2, SwizzleXXYY.v);
    V1[0] = vector_permute(r3, r3, SwizzleZWZW.v);
    V0[1] = vector_permute(r0, r0, SwizzleXXYY.v);
    V1[1] = vector_permute(r1, r1, SwizzleZWZW.v);
    V0[2] = vector_permute(r2, r0, Permute0X0Z1X1Z.v);
    V1[2] = vector_permute(r3, r1, Permute0Y0W1Y1W.v);
    vec4 D0 = vector_multiply(V0[0], V1[0]);
    vec4 D1 = vector_multiply(V0[1], V1[1]);
    vec4 D2 = vector_multiply(V0[2], V1[2]);
    V0[0] = vector_permute(r2, r2, SwizzleZWZW.v);
    V1[0] = vector_permute(r3, r3, SwizzleXXYY.v);
    V0[1] = vector_permute(r0, r0, SwizzleZWZW.v);
    V1[1] = vector_permute(r1, r1, SwizzleXXYY.v);
    V0[2] = vector_permute(r2, r0, Permute0Y0W1Y1W.v);
    V1[2] = vector_permute(r3, r1, Permute0X0Z1X1Z.v);
    D0 = vector_negative_multiply_subtract(V0[0], V1[0], D0);
    D1 = vector_negative_multiply_subtract(V0[1], V1[1], D1);
    D2 = vector_negative_multiply_subtract(V0[2], V1[2], D2);
    V0[0] = vector_permute(r1, r1, SwizzleYZXY.v);
    V1[0] = vector_permute(D0, D2, Permute1Y0Y0W0X.v);
    V0[1] = vector_permute(r0, r0, SwizzleZXYX.v);
    V1[1] = vector_permute(D0, D2, Permute0W1Y0Y0Z.v);
    V0[2] = vector_permute(r3, r3, SwizzleYZXY.v);
    V1[2] = vector_permute(D1, D2, Permute1W0Y0W0X.v);
    V0[3] = vector_permute(r2, r2, SwizzleZXYX.v);
    V1[3] = vector_permute(D1, D2, Permute0W1W0Y0Z.v);
    vec4 C0 = vector_multiply(V0[0], V1[0]);
    vec4 C2 = vector_multiply(V0[1], V1[1]);
    vec4 C4 = vector_multiply(V0[2], V1[2]);
    vec4 C6 = vector_multiply(V0[3], V1[3]);
    V0[0] = vector_permute(r1, r1, SwizzleZWYZ.v);
    V1[0] = vector_permute(D0, D2, Permute0W0X0Y1X.v);
    V0[1] = vector_permute(r0, r0, SwizzleWZWY.v);
    V1[1] = vector_permute(D0, D2, Permute0Z0Y1X0X.v);
    V0[2] = vector_permute(r3, r3, SwizzleZWYZ.v);
    V1[2] = vector_permute(D1, D2, Permute0W0X0Y1Z.v);
    V0[3] = vector_permute(r2, r2, SwizzleWZWY.v);
    V1[3] = vector_permute(D1, D2, Permute0Z0Y1Z0X.v);
    C0 = vector_negative_multiply_subtract(V0[0], V1[0], C0);
    C2 = vector_negative_multiply_subtract(V0[1], V1[1], C2);
    C4 = vector_negative_multiply_subtract(V0[2], V1[2], C4);
    C6 = vector_negative_multiply_subtract(V0[3], V1[3], C6);
    V0[0] = vector_permute(r1, r1, SwizzleWXWX.v);
    V1[0] = vector_permute(D0, D2, Permute0Z1Y1X0Z.v);
    V0[1] = vector_permute(r0, r0, SwizzleYWXZ.v);
    V1[1] = vector_permute(D0, D2, Permute1Y0X0W1X.v);
    V0[2] = vector_permute(r3, r3, SwizzleWXWX.v);
    V1[2] = vector_permute(D1, D2, Permute0Z1W1Z0Z.v);
    V0[3] = vector_permute(r2, r2, SwizzleYWXZ.v);
    V1[3] = vector_permute(D1, D2, Permute1W0X0W1Z.v);
    vec4 C1 = vector_negative_multiply_subtract(V0[0], V1[0], C0);
    C0 = vector_multiply_add(V0[0], V1[0], C0);
    vec4 C3 = vector_multiply_add(V0[1], V1[1], C2);
    C2 = vector_negative_multiply_subtract(V0[1], V1[1], C2);
    vec4 C5 = vector_negative_multiply_subtract(V0[2], V1[2], C4);
    C4 = vector_multiply_add(V0[2], V1[2], C4);
    vec4 C7 = vector_multiply_add(V0[3], V1[3], C6);
    C6 = vector_negative_multiply_subtract(V0[3], V1[3], C6);
    vec4 Rr0, Rr1, Rr2, Rr3;
    Rr0 = vector_select(C0, C1, Select0101.v);
    Rr1 = vector_select(C2, C3, Select0101.v);
    Rr2 = vector_select(C4, C5, Select0101.v);
    Rr3 = vector_select(C6, C7, Select0101.v);
    float Determinant = vec4dot(&Rr0, &r0);
    if(determinant)
        *determinant = Determinant;
    vec4 Reciprocal = vector_reciprocal(vector4_replicate(Determinant));
    vec4 Resultr0 = vector_multiply(Rr0, Reciprocal);
    vec4 Resultr1 = vector_multiply(Rr1, Reciprocal);
    vec4 Resultr2 = vector_multiply(Rr2, Reciprocal);
    vec4 Resultr3 = vector_multiply(Rr3, Reciprocal);
    matrix_from_vector4(o, Resultr0, Resultr1, Resultr2, Resultr3);
    return o;
}

static matrix* __stdcall c_matshadow(matrix* o, const vec4* plight, const plane* pln)
{
    assert(o && plight && pln);
    static const vec4_u32 Select0001 = { GS_SELECT_0, GS_SELECT_0, GS_SELECT_0, GS_SELECT_1 };
    vec3 V3(pln->a, pln->b, pln->c);
    vec3 Zero(0.0f, 0.0f, 0.0f);
    vec4 V4(pln->a, pln->b, pln->c, pln->d);
    assert(!(V3 == Zero));
    assert(!vector_is_infinite(V4));
    plane plane0;
    planenormalize(&plane0, pln);
    vec4 Dot = vector4_replicate(planedot(&plane0, plight));
    vec4 P(-plane0.a, -plane0.b, -plane0.c, -plane0.d);
    vec4 D = vector_splat_w(P);
    vec4 C = vector_splat_z(P);
    vec4 B = vector_splat_y(P);
    vec4 A = vector_splat_x(P);
    Dot = vector_select(Select0001.v, Dot, Select0001.v);
    vec4 r3 = vector_multiply_add(D, *plight, Dot);
    Dot = vector_rotate_left(Dot, 1);
    vec4 r2 = vector_multiply_add(C, *plight, Dot);
    Dot = vector_rotate_left(Dot, 1);
    vec4 r1 = vector_multiply_add(B, *plight, Dot);
    Dot = vector_rotate_left(Dot, 1);
    vec4 r0 = vector_multiply_add(A, *plight, Dot);
    matrix_from_vector4(o, r0, r1, r2, r3);
    return o;
}

static matrix* __stdcall c_matreflect(matrix* o, const plane* pln)
{
    assert(o && pln);
    static const vec4 NegativeTwo = { -2.0f, -2.0f, -2.0f, 0.0f };
    static const vec4 IdentityR0 = { 1.0f, 0.0f, 0.0f, 0.0f };
    static const vec4 IdentityR1 = { 0.0f, 1.0f, 0.0f, 0.0f };
    static const vec4 IdentityR2 = { 0.0f, 0.0f, 1.0f, 0.0f };
    static const vec4 IdentityR3 = { 0.0f, 0.0f, 0.0f, 1.0f };
    vec3 V3(pln->a, pln->b, pln->c);
    vec3 Zero(0.f, 0.f, 0.f);
    vec4 V4(pln->a, pln->b, pln->c, pln->d);
    assert(!(V3 == Zero));
    assert(!vector_is_infinite(V4));
    plane P;
    planenormalize(&P, pln);
    vec4 S = vector_multiply((const vec4&)P, NegativeTwo);
    vec4 A = vector_splat_x((const vec4&)P);
    vec4 B = vector_splat_y((const vec4&)P);
    vec4 C = vector_splat_z((const vec4&)P);
    vec4 D = vector_splat_w((const vec4&)P);
    vec4 r0 = vector_multiply_add(A, S, IdentityR0);
    vec4 r1 = vector_multiply_add(B, S, IdentityR1);
    vec4 r2 = vector_multiply_add(C, S, IdentityR2);
    vec4 r3 = vector_multiply_add(D, S, IdentityR3);
    matrix_from_vector4(o, r0, r1, r2, r3);
    return o;
}

static quat* __stdcall c_quatrotateeuler(quat* o, float yaw, float pitch, float roll)
{
    assert(o);
    static const vec4_u32 ControlPitch = { GS_PERMUTE_0X, GS_PERMUTE_1X, GS_PERMUTE_1X, GS_PERMUTE_1X };
    static const vec4_u32 ControlYaw = { GS_PERMUTE_1Y, GS_PERMUTE_0Y, GS_PERMUTE_1Y, GS_PERMUTE_1Y };
    static const vec4_u32 ControlRoll = { GS_PERMUTE_1Z, GS_PERMUTE_1Z, GS_PERMUTE_0Z, GS_PERMUTE_1Z };
    static const vec4 Sign = { 1.0f, -1.0f, -1.0f, 1.0f };
    vec4 Angles(pitch, yaw, roll, 0.0f);
    vec4 OneHalf(0.5f, 0.5f, 0.5f, 0.5f);
    vec4 HalfAngles = vector_multiply(Angles, OneHalf);
    vec4 SinAngles, CosAngles;
    vector_sin_cos(&SinAngles, &CosAngles, HalfAngles);
    vec4 P0 = vector_permute(SinAngles, CosAngles, ControlPitch.v);
    vec4 Y0 = vector_permute(SinAngles, CosAngles, ControlYaw.v);
    vec4 R0 = vector_permute(SinAngles, CosAngles, ControlRoll.v);
    vec4 P1 = vector_permute(CosAngles, SinAngles, ControlPitch.v);
    vec4 Y1 = vector_permute(CosAngles, SinAngles, ControlYaw.v);
    vec4 R1 = vector_permute(CosAngles, SinAngles, ControlRoll.v);
    vec4 Q1 = vector_multiply(P1, Sign);
    vec4 Q0 = vector_multiply(P0, Y0);
    Q1 = vector_multiply(Q1, Y1);
    Q0 = vector_multiply(Q0, R0);
    vec4 Q = vector_multiply_add(Q1, R1, Q0);
    o->x = Q.x;
    o->y = Q.y;
    o->z = Q.z;
    o->w = Q.w;
    return o;
}

static quat* __stdcall c_quatmultiply(quat* o, const quat* q1, const quat* q2)
{
    assert(o && q1 && q2);
    const vec4_u32 ControlWZYX = { GS_PERMUTE_0W, GS_PERMUTE_1Z, GS_PERMUTE_0Y, GS_PERMUTE_1X };
    const vec4_u32 ControlZWXY = { GS_PERMUTE_0Z, GS_PERMUTE_0W, GS_PERMUTE_1X, GS_PERMUTE_1Y };
    const vec4_u32 ControlYXWZ = { GS_PERMUTE_1Y, GS_PERMUTE_0X, GS_PERMUTE_0W, GS_PERMUTE_1Z };
    vec4 NegativeQ1(-q1->x, -q1->y, -q1->z, -q1->w);
    vec4 Q1(q1->x, q1->y, q1->z, q1->w);
    vec4 Q2(q2->x, q2->y, q2->z, q2->w);
    vec4 Q2W = vector_splat_w(Q2);
    vec4 Q2X = vector_splat_x(Q2);
    vec4 Q2Y = vector_splat_y(Q2);
    vec4 Q2Z = vector_splat_z(Q2);
    vec4 Q1WZYX = vector_permute(Q1, NegativeQ1, ControlWZYX.v);
    vec4 Q1ZWXY = vector_permute(Q1, NegativeQ1, ControlZWXY.v);
    vec4 Q1YXWZ = vector_permute(Q1, NegativeQ1, ControlYXWZ.v);
    vec4 result = vector_multiply(Q1, Q2W);
    result = vector_multiply_add(Q1WZYX, Q2X, result);
    result = vector_multiply_add(Q1ZWXY, Q2Y, result);
    result = vector_multiply_add(Q1YXWZ, Q2Z, result);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    o->w = result.w;
    return o;
}

static quat* __stdcall c_quatnormalize(quat* o, const quat* q)
{
    assert(o && q);
    vec4 Q(q->x, q->y, q->z, q->w);
    vec4 result;
    c_vec4normalize(&result, &Q);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    o->w = result.w;
    return o;
}

static quat* __stdcall c_quatinverse(quat* o, const quat* q)
{
    assert(o && q);
    vec4 Q(q->x, q->y, q->z, q->w);
    vec4 L(vector4_replicate(vec4lengthsq(&Q)));
    vec4 Conjugate(-Q.x, -Q.y, -Q.z, Q.w);
    vec4 Epsilon(1.192092896e-7f, 1.192092896e-7f, 1.192092896e-7f, 1.192092896e-7f);
    vec4 Control = vector_less_or_equal(L, Epsilon);
    L = vector_reciprocal(L);
    vec4 result = vector_multiply(Conjugate, L);
    vec4 Zero(0.0f, 0.0f, 0.0f, 0.0f);
    result = vector_select(result, Zero, Control);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    o->w = result.w;
    return o;
}

static plane* __stdcall c_planenormalize(plane* o, const plane* p)
{
    assert(o && p);
    float len = sqrtf((p->a * p->a) + (p->b * p->b) + (p->c * p->c));
    if(len)
        len = 1.0f / len;
    o->a = p->a * len,
        o->b = p->b * len,
        o->c = p->c * len,
        o->d = p->d * len;
    return o;
}

static vec3* __stdcall c_planeintersectline(vec3* o, const plane* p, const vec3* v1, const vec3* v2)
{
    assert(o && p && v1 && v2);
    vec3 P(p->a, p->b, p->c);
    vec3 V1 = vector3_replicate(vec3dot(&P, v1));
    vec3 V2 = vector3_replicate(vec3dot(&P, v2));
    vec3 D = V1 - V2;
    vec3 ReciprocalD = vector_reciprocal(D);
    vec3 VT = vector3_replicate(planedotcoord(p, v1));
    VT = vector_multiply(VT, ReciprocalD);
    vec3 Point = *v2 - *v1;
    Point = vector_multiply_add(Point, VT, *v1);
    vec3 Zero(0.0f, 0.0f, 0.0f);
    vec3 Epsilon(1.192092896e-7f, 1.192092896e-7f, 1.192092896e-7f);
    vec3 Control = vector_near_equal(D, Zero, Epsilon);
    vec4_i32 QNaN = { 0x7fc00000, 0x7fc00000, 0x7fc00000, 0x7fc00000 };
    vec3 result = vector_select(Point, vec3(QNaN.v.x, QNaN.v.y, QNaN.v.z), Control);
    o->x = result.x;
    o->y = result.y;
    o->z = result.z;
    return o;
}

static plane* __stdcall c_planefrompoints(plane* o, const vec3* v1, const vec3* v2, const vec3* v3)
{
    assert(o && v1 && v2 && v3);
    vec3 v21 = *v1 - *v2;
    vec3 v31 = *v1 - *v3;
    vec3 n;
    vec3cross(&n, &v21, &v31);
    vec3normalize(&n, &n);
    plane p4(n.x, n.y, n.z, 0);
    float d = planedotnormal(&p4, v1);
    d = -d;
    vec4 d4(d, d, d, d);
    const vec4_u32 select1110 = { GS_SELECT_1, GS_SELECT_1, GS_SELECT_1, GS_SELECT_0 };
    vec4 result = vector_select(d4, vec4(n.x, n.y, n.z, 0.f), select1110.v);
    o->a = result.x;
    o->b = result.y;
    o->c = result.z;
    o->d = result.w;
    return o;
}

static struct mathfn_initializer
{
    mathfn_initializer()
    {
        vec2normalize = c_vec2normalize;
        vec2hermite = c_vec2hermite;
        vec2catmullrom = c_vec2catmullrom;
        vec2barycentric = c_vec2barycentric;
        vec2transform = c_vec2transform;
        vec2transformarray = c_vec2transformarray;
        vec2transformcoordarray = c_vec2transformcoordarray;
        vec2transformnormalarray = c_vec2transformnormalarray;
        vec3normalize = c_vec3normalize;
        vec3hermite = c_vec3hermite;
        vec3catmullrom = c_vec3catmullrom;
        vec3barycentric = c_vec3barycentric;
        vec3transform = c_vec3transform;
        vec3transformcoord = c_vec3transformcoord;
        vec3transformnormal = c_vec3transformnormal;
        vec3transformarray = c_vec3transformarray;
        vec3transformcoordarray = c_vec3transformcoordarray;
        vec3transformnormalarray = c_vec3transformnormalarray;
        vec4cross = c_vec4cross;
        vec4normalize = c_vec4normalize;
        vec4hermite = c_vec4hermite;
        vec4catmullrom = c_vec4catmullrom;
        vec4barycentric = c_vec4barycentric;
        vec4transform = c_vec4transform;
        vec4transformarray = c_vec4transformarray;
        matdeterminant = c_matdeterminant;
        matmultiply = c_matmultiply;
        matmultiplytranspose = c_matmultiplytranspose;
        matinverse = c_matinverse;
        matshadow = c_matshadow;
        matreflect = c_matreflect;
        quatrotateeuler = c_quatrotateeuler;
        quatmultiply = c_quatmultiply;
        quatnormalize = c_quatnormalize;
        quatinverse = c_quatinverse;
        planenormalize = c_planenormalize;
        planeintersectline = c_planeintersectline;
        planefrompoints = c_planefrompoints;
        planetransform = (fnplanetransform)c_vec4transform;
        planetransformarray = (fnplanetransformarray)c_vec4transformarray;
    }
} __mfi_inst;

fnvec2normalize vec2normalize;
fnvec2hermite vec2hermite;
fnvec2catmullrom vec2catmullrom;
fnvec2barycentric vec2barycentric;
fnvec2transform vec2transform;
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
