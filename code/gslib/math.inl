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

#ifndef math_56c94d7e_d5fd_4744_9a4c_fe98b3b977d8_inl
#define math_56c94d7e_d5fd_4744_9a4c_fe98b3b977d8_inl

#include <assert.h>
#include <memory.h>
#include <math.h>

__gslib_begin__

/* additional */
inline int round(float f) { return (int)(f > 0.f ? (f + 0.5f) : (f - 0.5f)); }
inline int round(double f) { return (int)(f > 0.0 ? (f + 0.5) : (f - 0.5)); }
inline float frac(float f) { return f - floorf(f); }
inline double frac(double f) { return f - floor(f); }

template<class _ty>
inline _ty abs(_ty t) { return t >= 0 ? t : -t; }
template<class _ty>
inline int sgn(_ty n) { return n == 0 ? 0 : (n > 0 ? 1 : -1); }
template<class _ty>
inline _ty fuzz_cmp(_ty t1, _ty t2, _ty tol)
{
    _ty c = t2 - t1;
    if(abs(c) < tol)
        return 0;
    return c;
}
template<class _ty>
inline _ty fuzz_cmp(_ty t1, _ty t2) { return fuzz_cmp(t1, t2, abs(t1 - t2) / 10000); }
inline float fuzz_cmp(const vec2& a, const vec2& b) { return vec2length(&(a - b)); }

/* math types */
inline vec2::vec2(const float* pf)
{
    assert(pf);
    x = pf[0];
    y = pf[1];
}

inline vec2::vec2(float fx, float fy)
{
    x = fx;
    y = fy;
}

inline vec2::operator float* ()
{
    return (float*)&x;
}

inline vec2::operator const float* () const
{
    return (const float*)&x;
}

inline vec2& vec2::operator += (const vec2& v)
{
    x += v.x;
    y += v.y;
    return *this;
}

inline vec2& vec2::operator -= (const vec2& v)
{
    x -= v.x;
    y -= v.y;
    return *this;
}

inline vec2& vec2::operator *= (float f)
{
    x *= f;
    y *= f;
    return *this;
}

inline vec2& vec2::operator /= (float f)
{
    float finv = 1.f / f;
    x *= finv;
    y *= finv;
    return *this;
}

inline vec2 vec2::operator + () const
{
    return *this;
}

inline vec2 vec2::operator - () const
{
    return vec2(-x, -y);
}

inline vec2 vec2::operator + (const vec2& v) const
{
    return vec2(x + v.x, y + v.y);
}

inline vec2 vec2::operator - (const vec2& v) const
{
    return vec2(x - v.x, y - v.y);
}

inline vec2 vec2::operator * (float f) const
{
    return vec2(x * f, y * f);
}

inline vec2 vec2::operator / (float f) const
{
    float finv = 1.f / f;
    return vec2(x * finv, y * finv);
}

inline vec2 operator * (float f, const vec2& v)
{
    return vec2(f * v.x, f* v.y);
}

inline bool vec2::operator == (const vec2& v) const
{
    return x == v.x && y == v.y;
}

inline bool vec2::operator != (const vec2& v) const
{
    return x != v.x || y != v.y;
}

inline vec3::vec3(const float* pf)
{
    assert(pf);
    x = pf[0];
    y = pf[1];
    z = pf[2];
}

inline vec3::vec3(float fx, float fy, float fz)
{
    x = fx;
    y = fy;
    z = fz;
}

inline vec3::operator float* ()
{
    return (float*)&x;
}

inline vec3::operator const float* () const
{
    return (const float*)&x;
}

inline vec3& vec3::operator += (const vec3& v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

inline vec3& vec3::operator -= (const vec3& v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

inline vec3& vec3::operator *= (float f)
{
    x *= f;
    y *= f;
    z *= f;
    return *this;
}

inline vec3& vec3::operator /= (float f)
{
    float finv = 1.f / f;
    x *= finv;
    y *= finv;
    z *= finv;
    return *this;
}

inline vec3 vec3::operator + () const
{
    return *this;
}

inline vec3 vec3::operator - () const
{
    return vec3(-x, -y, -z);
}

inline vec3 vec3::operator + (const vec3& v) const
{
    return vec3(x + v.x, y + v.y, z + v.z);
}

inline vec3 vec3::operator - (const vec3& v) const
{
    return vec3(x - v.x, y - v.y, z - v.z);
}

inline vec3 vec3::operator * (float f) const
{
    return vec3(x * f, y * f, z * f);
}

inline vec3 vec3::operator / (float f) const
{
    float finv = 1.f / f;
    return vec3(x * finv, y * finv, z * finv);
}

inline vec3 operator * (float f, const vec3& v)
{
    return vec3(f * v.x, f * v.y, f * v.z);
}

inline bool vec3::operator == (const vec3& v) const
{
    return x == v.x && y == v.y && z == v.z;
}

inline bool vec3::operator != (const vec3& v) const
{
    return x != v.x || y != v.y || z != v.z;
}

inline vec4::vec4(const float* pf)
{
    assert(pf);
    x = pf[0];
    y = pf[1];
    z = pf[2];
    w = pf[3];
}

inline vec4::vec4(const _vec3& xyz, float fw)
{
    x = xyz.x;
    y = xyz.y;
    z = xyz.z;
    w = fw;
}

inline vec4::vec4(float fx, float fy, float fz, float fw)
{
    x = fx;
    y = fy;
    z = fz;
    w = fw;
}

inline vec4::operator float* ()
{
    return (float*)&x;
}

inline vec4::operator const float* () const
{
    return (const float*)&x;
}

inline vec4& vec4::operator += (const vec4& v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
}

inline vec4& vec4::operator -= (const vec4& v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
}

inline vec4& vec4::operator *= (float f)
{
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return *this;
}

inline vec4& vec4::operator /= (float f)
{
    float finv = 1.f / f;
    x *= finv;
    y *= finv;
    z *= finv;
    w *= finv;
    return *this;
}

inline vec4 vec4::operator + () const
{
    return *this;
}

inline vec4 vec4::operator - () const
{
    return vec4(-x, -y, -z, -w);
}

inline vec4 vec4::operator + (const vec4& v) const
{
    return vec4(x + v.x, y + v.y, z + v.z, w + v.w);
}

inline vec4 vec4::operator - (const vec4& v) const
{
    return vec4(x - v.x, y - v.y, z - v.z, w - v.w);
}

inline vec4 vec4::operator * (float f) const
{
    return vec4(x * f, y * f, z * f, w * f);
}

inline vec4 vec4::operator / (float f) const
{
    float finv = 1.f / f;
    return vec4(x * finv, y * finv, z * finv, w * finv);
}

inline vec4 operator * (float f, const vec4& v)
{
    return vec4(f * v.x, f * v.y, f * v.z, f * v.w);
}

inline bool vec4::operator == (const vec4& v) const
{
    return x == v.x && y == v.y && z == v.z && w == v.w;
}

inline bool vec4::operator != (const vec4& v) const
{
    return x != v.x || y != v.y || z != v.z || w != v.w;
}

inline mat3::mat3(const float* pf)
{
    assert(pf);
    memcpy_s(&_11, sizeof(mat3), pf, sizeof(mat3));
}

inline mat3::mat3(const mat3& mat)
{
    memcpy_s(&_11, sizeof(mat3), &mat, sizeof(mat3));
}

inline mat3::mat3(float f11, float f12, float f13, 
                  float f21, float f22, float f23, 
                  float f31, float f32, float f33)
{
    _11 = f11, _12 = f12, _13 = f13;
    _21 = f21, _22 = f22, _23 = f23;
    _31 = f31, _32 = f32, _33 = f33;
}

inline float& mat3::operator () (uint row, uint col)
{
    return m[row][col];
}

inline float mat3::operator () (uint row, uint col) const
{
    return m[row][col];
}

inline mat3::operator float* ()
{
    return (float*)&_11;
}

inline mat3::operator const float* () const
{
    return (const float*)&_11;
}

inline mat3& mat3::operator *= (const mat3& mat)
{
    mat3multiply(this, this, &mat);
    return *this;
}

inline mat3& mat3::operator += (const mat3& mat)
{
    _11 += mat._11, _12 += mat._12, _13 += mat._13;
    _21 += mat._21, _22 += mat._22, _23 += mat._23;
    _31 += mat._31, _32 += mat._32, _33 += mat._33;
    return *this;
}

inline mat3& mat3::operator -= (const mat3& mat)
{
    _11 -= mat._11, _12 -= mat._12, _13 -= mat._13;
    _21 -= mat._21, _22 -= mat._22, _23 -= mat._23;
    _31 -= mat._31, _32 -= mat._32, _33 -= mat._33;
    return *this;
}

inline mat3& mat3::operator *= (float f)
{
    _11 *= f, _12 *= f, _13 *= f;
    _21 *= f, _22 *= f, _23 *= f;
    _31 *= f, _32 *= f, _33 *= f;
    return *this;
}

inline mat3& mat3::operator /= (float f)
{
    float finv = 1.f / f;
    _11 *= finv, _12 *= finv, _13 *= finv;
    _21 *= finv, _22 *= finv, _23 *= finv;
    _31 *= finv, _32 *= finv, _33 *= finv;
    return *this;
}

inline mat3 mat3::operator + () const
{
    return *this;
}

inline mat3 mat3::operator - () const
{
    return mat3(
        -_11, -_12, -_13,
        -_21, -_22, -_23,
        -_31, -_32, -_33
        );
}

inline mat3 mat3::operator * (const mat3& mat) const
{
    mat3 t;
    mat3multiply(&t, this, &mat);
    return t;
}

inline mat3 mat3::operator + (const mat3& mat) const
{
    return mat3(
        _11 + mat._11, _12 + mat._12, _13 + mat._13,
        _21 + mat._21, _22 + mat._22, _23 + mat._23,
        _31 + mat._31, _32 + mat._32, _33 + mat._33
        );
}

inline mat3 mat3::operator - (const mat3& mat) const
{
    return mat3(
        _11 - mat._11, _12 - mat._12, _13 - mat._13,
        _21 - mat._21, _22 - mat._22, _23 - mat._23,
        _31 - mat._31, _32 - mat._32, _33 - mat._33
        );
}

inline mat3 mat3::operator * (float f) const
{
    return mat3(
        _11 * f, _12 * f, _13 * f,
        _21 * f, _22 * f, _23 * f,
        _31 * f, _32 * f, _33 * f
        );
}

inline mat3 mat3::operator / (float f) const
{
    float finv = 1.f / f;
    return mat3(
        _11 * finv, _12 * finv, _13 * finv,
        _21 * finv, _22 * finv, _23 * finv,
        _31 * finv, _32 * finv, _33 * finv
        );
}

inline mat3 operator * (float f, const mat3& mat)
{
    return mat3(
        f * mat._11, f * mat._12, f * mat._13,
        f * mat._21, f * mat._22, f * mat._23,
        f * mat._31, f * mat._32, f * mat._33
        );
}

inline bool mat3::operator == (const mat3& mat) const
{
    return memcmp(this, &mat, sizeof(mat3)) == 0;
}

inline bool mat3::operator != (const mat3& mat) const
{
    return memcmp(this, &mat, sizeof(mat3)) != 0;
}

inline mat4::mat4(const float* pf)
{
    assert(pf);
    memcpy_s(&_11, sizeof(mat4), pf, sizeof(mat4));
}

inline mat4::mat4(const mat4& mat)
{
    memcpy_s(&_11, sizeof(mat4), &mat, sizeof(mat4));
}

inline mat4::mat4(float f11, float f12, float f13, float f14,
                  float f21, float f22, float f23, float f24,
                  float f31, float f32, float f33, float f34,
                  float f41, float f42, float f43, float f44)
{
    _11 = f11, _12 = f12, _13 = f13, _14 = f14;
    _21 = f21, _22 = f22, _23 = f23, _24 = f24;
    _31 = f31, _32 = f32, _33 = f33, _34 = f34;
    _41 = f41, _42 = f42, _43 = f43, _44 = f44;
}

inline float& mat4::operator () (uint row, uint col)
{
    return m[row][col];
}

inline float mat4::operator () (uint row, uint col) const
{
    return m[row][col];
}

inline mat4::operator float* ()
{
    return (float*)&_11;
}

inline mat4::operator const float* () const
{
    return (const float*)&_11;
}

inline mat4& mat4::operator *= (const mat4& mat)
{
    matmultiply(this, this, &mat);
    return *this;
}

inline mat4& mat4::operator += (const mat4& mat)
{
    _11 += mat._11, _12 += mat._12, _13 += mat._13, _14 += mat._14;
    _21 += mat._21, _22 += mat._22, _23 += mat._23, _24 += mat._24;
    _31 += mat._31, _32 += mat._32, _33 += mat._33, _34 += mat._34;
    _41 += mat._41, _42 += mat._42, _43 += mat._43, _44 += mat._44;
    return *this;
}

inline mat4& mat4::operator -= (const mat4& mat)
{
    _11 -= mat._11, _12 -= mat._12, _13 -= mat._13, _14 -= mat._14;
    _21 -= mat._21, _22 -= mat._22, _23 -= mat._23, _24 -= mat._24;
    _31 -= mat._31, _32 -= mat._32, _33 -= mat._33, _34 -= mat._34;
    _41 -= mat._41, _42 -= mat._42, _43 -= mat._43, _44 -= mat._44;
    return *this;
}

inline mat4& mat4::operator *= (float f)
{
    _11 *= f, _12 *= f, _13 *= f, _14 *= f;
    _21 *= f, _22 *= f, _23 *= f, _24 *= f;
    _31 *= f, _32 *= f, _33 *= f, _34 *= f;
    _41 *= f, _42 *= f, _43 *= f, _44 *= f;
    return *this;
}

inline mat4& mat4::operator /= (float f)
{
    float finv = 1.f / f;
    _11 *= finv, _12 *= finv, _13 *= finv, _14 *= finv;
    _21 *= finv, _22 *= finv, _23 *= finv, _24 *= finv;
    _31 *= finv, _32 *= finv, _33 *= finv, _34 *= finv;
    _41 *= finv, _42 *= finv, _43 *= finv, _44 *= finv;
    return *this;
}

inline mat4 mat4::operator + () const
{
    return *this;
}

inline mat4 mat4::operator - () const
{
    return mat4(
        -_11, -_12, -_13, -_14,
        -_21, -_22, -_23, -_24,
        -_31, -_32, -_33, -_34,
        -_41, -_42, -_43, -_44
        );
}

inline mat4 mat4::operator * (const mat4& mat) const
{
    mat4 t;
    matmultiply(&t, this, &mat);
    return t;
}

inline mat4 mat4::operator + (const mat4& mat) const
{
    return mat4(
        _11 + mat._11, _12 + mat._12, _13 + mat._13, _14 + mat._14,
        _21 + mat._21, _22 + mat._22, _23 + mat._23, _24 + mat._24,
        _31 + mat._31, _32 + mat._32, _33 + mat._33, _34 + mat._34,
        _41 + mat._41, _42 + mat._42, _43 + mat._43, _44 + mat._44
        );
}

inline mat4 mat4::operator - (const mat4& mat) const
{
    return mat4(
        _11 - mat._11, _12 - mat._12, _13 - mat._13, _14 - mat._14,
        _21 - mat._21, _22 - mat._22, _23 - mat._23, _24 - mat._24,
        _31 - mat._31, _32 - mat._32, _33 - mat._33, _34 - mat._34,
        _41 - mat._41, _42 - mat._42, _43 - mat._43, _44 - mat._44
        );
}

inline mat4 mat4::operator * (float f) const
{
    return mat4(
        _11 * f, _12 * f, _13 * f, _14 * f,
        _21 * f, _22 * f, _23 * f, _24 * f,
        _31 * f, _32 * f, _33 * f, _34 * f,
        _41 * f, _42 * f, _43 * f, _44 * f
        );
}

inline mat4 mat4::operator / (float f) const
{
    float finv = 1.f / f;
    return mat4(
        _11 * finv, _12 * finv, _13 * finv, _14 * finv,
        _21 * finv, _22 * finv, _23 * finv, _24 * finv,
        _31 * finv, _32 * finv, _33 * finv, _34 * finv,
        _41 * finv, _42 * finv, _43 * finv, _44 * finv
        );
}

inline mat4 operator * (float f, const mat4& mat)
{
    return mat4(
        f * mat._11, f * mat._12, f * mat._13, f * mat._14,
        f * mat._21, f * mat._22, f * mat._23, f * mat._24,
        f * mat._31, f * mat._32, f * mat._33, f * mat._34,
        f * mat._41, f * mat._42, f * mat._43, f * mat._44
        );
}

inline bool mat4::operator == (const mat4& mat) const
{
    return memcmp(this, &mat, sizeof(mat4)) == 0;
}

inline bool mat4::operator != (const mat4& mat) const
{
    return memcmp(this, &mat, sizeof(mat4)) != 0;
}

inline quat::quat(const float* pf)
{
    assert(pf);
    x = pf[0];
    y = pf[1];
    z = pf[2];
    w = pf[3];
}

inline quat::quat(float fx, float fy, float fz, float fw)
{
    x = fx;
    y = fy;
    z = fz;
    w = fw;
}

inline quat::operator float* ()
{
    return (float*)&x;
}

inline quat::operator const float* () const
{
    return (const float*)&x;
}

inline quat& quat::operator += (const quat& q)
{
    x += q.x;
    y += q.y;
    z += q.z;
    w += q.w;
    return *this;
}

inline quat& quat::operator -= (const quat& q)
{
    x -= q.x;
    y -= q.y;
    z -= q.z;
    w -= q.w;
    return *this;
}

inline quat& quat::operator *= (const quat& q)
{
    quatmultiply(this, this, &q);
    return *this;
}

inline quat& quat::operator *= (float f)
{
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return *this;
}

inline quat& quat::operator /= (float f)
{
    float finv = 1.f / f;
    x *= finv;
    y *= finv;
    z *= finv;
    w *= finv;
    return *this;
}

inline quat quat::operator + () const
{
    return *this;
}

inline quat quat::operator - () const
{
    return quat(-x, -y, -z, -w);
}

inline quat quat::operator + (const quat& q) const
{
    return quat(x + q.x, y + q.y, z + q.z, w + q.w);
}

inline quat quat::operator - (const quat& q) const
{
    return quat(x - q.x, y - q.y, z - q.z, w - q.w);
}

inline quat quat::operator * (const quat& q) const
{
    quat t;
    quatmultiply(&t, this, &q);
    return t;
}

inline quat quat::operator * (float f) const
{
    return quat(x * f, y * f, z * f, w * f);
}

inline quat quat::operator / (float f) const
{
    float finv = 1.f / f;
    return quat(x * finv, y * finv, z * finv, w * finv);
}

inline quat operator * (float f, const quat& q)
{
    return quat(f * q.x, f * q.y, f * q.z, f * q.w);
}

inline bool quat::operator == (const quat& q) const
{
    return x == q.x && y == q.y && z == q.z && w == q.w;
}

inline bool quat::operator != (const quat& q) const
{
    return x != q.x || y != q.y || z != q.z || w != q.w;
}

inline plane::plane(const float* pf)
{
    assert(pf);
    a = pf[0];
    b = pf[1];
    c = pf[2];
    d = pf[3];
}

inline plane::plane(float fa, float fb, float fc, float fd)
{
    a = fa;
    b = fb;
    c = fc;
    d = fd;
}

inline plane::operator float* ()
{
    return (float*)&a;
}

inline plane::operator const float* () const
{
    return (const float*)&a;
}

inline plane& plane::operator *= (float f)
{
    a *= f;
    b *= f;
    c *= f;
    d *= f;
    return *this;
}

inline plane& plane::operator /= (float f)
{
    float finv = 1.f / f;
    a *= finv;
    b *= finv;
    c *= finv;
    d *= finv;
    return *this;
}

inline plane plane::operator + () const
{
    return *this;
}

inline plane plane::operator - () const
{
    return plane(-a, -b, -c, -d);
}

inline plane plane::operator * (float f) const
{
    return plane(a * f, b * f, c * f, d * f);
}

inline plane plane::operator / (float f) const
{
    float finv = 1.f / f;
    return plane(a * finv, b * finv, c * finv, d * finv);
}

inline plane operator * (float f, const plane& p)
{
    return plane(f * p.a, f * p.b, f * p.c, f * p.d);
}

inline bool plane::operator == (const plane& p) const
{
    return a == p.a && b == p.b && c == p.c && d == p.d;
}

inline bool plane::operator != (const plane& p) const
{
    return a != p.a || b != p.b || c != p.c || d != p.d;
}

inline float vec2length(const vec2* p)
{
    assert(p);
    return sqrtf(p->x * p->x + p->y * p->y);
}

inline float vec2lengthsq(const vec2* p)
{
    assert(p);
    return p->x * p->x + p->y * p->y;
}

inline float vec2dot(const vec2* p1, const vec2* p2)
{
    assert(p1 && p2);
    return p1->x * p2->x + p1->y * p2->y;
}

inline float vec2ccw(const vec2* p1, const vec2* p2)
{
    assert(p1 && p2);
    return p1->x * p2->y - p1->y * p2->x;
}

inline vec2* vec2add(vec2* out, const vec2* p1, const vec2* p2)
{
    assert(out && p1 && p2);
    out->x = p1->x + p2->x;
    out->y = p1->y + p2->y;
    return out;
}

inline vec2* vec2sub(vec2* out, const vec2* p1, const vec2* p2)
{
    assert(out && p1 && p2);
    out->x = p1->x - p2->x;
    out->y = p1->y - p2->y;
    return out;
}

inline vec2* vec2min(vec2* out, const vec2* p1, const vec2* p2)
{
    assert(out && p1 && p2);
    out->x = p1->x < p2->x ? p1->x : p2->x;
    out->y = p1->y < p2->y ? p1->y : p2->y;
    return out;
}

inline vec2* vec2max(vec2* out, const vec2* p1, const vec2* p2)
{
    assert(out && p1 && p2);
    out->x = p1->x > p2->x ? p1->x : p2->x;
    out->y = p1->y > p2->y ? p1->y : p2->y;
    return out;
}

inline vec2* vec2scale(vec2* out, const vec2* p, float s)
{
    assert(out && p);
    out->x = p->x * s;
    out->y = p->y * s;
    return out;
}

inline vec2* vec2multiply(vec2* out, const vec2* p, const matrix2* m)
{
    assert(out && p && m);
    vec2 v;
    if(out == p) { v = *p; p = &v; }
    out->x = p->x * m->_11 + p->y * m->_21;
    out->y = p->x * m->_12 + p->y * m->_22;
    return out;
}

inline vec2* vec2transformcoord(vec2* out, const vec2* p, const mat3* m)
{
    assert(out && p && m);
    vec3 tmp;
    vec3multiply(&tmp, &vec3(p->x, p->y, 1.f), m);
    out->x = tmp.x / tmp.z;
    out->y = tmp.y / tmp.z;
    return out;
}

inline vec2* vec2transformnormal(vec2* out, const vec2* p, const mat3* m)
{
    assert(out && p && m);
    vec3 tmp;
    vec3multiply(&tmp, &vec3(p->x, p->y, 0.f), m);
    out->x = tmp.x;
    out->y = tmp.y;
    return out;
}

inline vec2* vec2lerp(vec2* out, const vec2* p1, const vec2* p2, float s)
{
    assert(out && p1 && p2);
    out->x = p1->x + s * (p2->x - p1->x);
    out->y = p1->y + s * (p2->y - p1->y);
    return out;
}

inline float vec2lerp_x(const vec2* p1, const vec2* p2, float y)
{
    assert(p1 && p2);
    return (p2->x - p1->x) * (y - p1->y) / (p2->y - p1->y) + p1->x;
}

inline float vec2lerp_y(const vec2* p1, const vec2* p2, float x)
{
    assert(p1 && p2);
    return (p2->y - p1->y) * (x - p1->x) / (p2->x - p1->x) + p1->y;
}

inline float vec3length(const vec3* p)
{
    assert(p);
    return sqrtf(p->x * p->x + p->y * p->y + p->z * p->z);
}

inline float vec3lengthsq(const vec3* p)
{
    assert(p);
    return p->x * p->x + p->y * p->y + p->z * p->z;
}

inline float vec3dot(const vec3* p1, const vec3* p2)
{
    assert(p1 && p2);
    return p1->x * p2->x + p1->y * p2->y + p1->z * p2->z;
}

inline vec3* vec3cross(vec3* out, const vec3* v1, const vec3* v2)
{
    assert(out && v1 && v2);
    out->x = v1->y * v2->z - v1->z * v2->y;
    out->y = v1->z * v2->x - v1->x * v2->z;
    out->z = v1->x * v2->y - v1->y * v2->x;
    return out;
}

inline vec3* vec3add(vec3* out, const vec3* v1, const vec3* v2)
{
    assert(out && v1 && v2);
    out->x = v1->x + v2->x;
    out->y = v1->y + v2->y;
    out->z = v1->z + v2->z;
    return out;
}

inline vec3* vec3sub(vec3* out, const vec3* v1, const vec3* v2)
{
    assert(out && v1 && v2);
    out->x = v1->x - v2->x;
    out->y = v1->y - v2->y;
    out->z = v1->z - v2->z;
    return out;
}

inline vec3* vec3min(vec3* out, const vec3* p1, const vec3* p2)
{
    assert(out && p1 && p2);
    out->x = p1->x < p2->x ? p1->x : p2->x;
    out->y = p1->y < p2->y ? p1->y : p2->y;
    out->z = p1->z < p2->z ? p1->z : p2->z;
    return out;
}

inline vec3* vec3max(vec3* out, const vec3* p1, const vec3* p2)
{
    assert(out && p1 && p2);
    out->x = p1->x > p2->x ? p1->x : p2->x;
    out->y = p1->y > p2->y ? p1->y : p2->y;
    out->z = p1->z > p2->z ? p1->z : p2->z;
    return out;
}

inline vec3* vec3scale(vec3* out, const vec3* v, float s)
{
    assert(out && v);
    out->x = v->x * s;
    out->y = v->y * s;
    out->z = v->z * s;
    return out;
}

inline vec3* vec3lerp(vec3* out, const vec3* v1, const vec3* v2, float s)
{
    assert(out && v1 && v2);
    out->x = v1->x + s * (v2->x - v1->x);
    out->y = v1->y + s * (v2->y - v1->y);
    out->z = v1->z + s * (v2->z - v1->z);
    return out;
}

inline vec3* vec3multiply(vec3* out, const vec3* v, const mat3* m)
{
    assert(out && v && m);
    out->x = v->x * m->_11 + v->y * m->_21 + v->z * m->_31;
    out->y = v->x * m->_12 + v->y * m->_22 + v->z * m->_32;
    out->z = v->x * m->_13 + v->y * m->_23 + v->z * m->_33;
    return out;
}

inline vec3* vec3project(vec3* out, const vec3* v, const viewport* vp, const matrix* prj, const matrix* view, const matrix* world)
{
    assert(out && v);
    matrix m;
    const matrix* mt = &m;
    if(!prj && !view && !world)     { matidentity(&m); }
    else if(prj && view && world)   { matmultiply(&m, prj, view); matmultiply(&m, &m, world); }
    else if(prj && view)            { matmultiply(&m, prj, view); }
    else if(prj && world)           { matmultiply(&m, prj, world); }
    else if(view && world)          { matmultiply(&m, view, world); }
    else if(prj)                    { mt = prj; }
    else                            { mt = view ? view : world; }
    assert(mt);
    vec3transformcoord(out, v, mt);
    if(vp) {
        out->x = (out->x + 1.f) * 0.5f * vp->width + vp->left;
        out->y = (1.f - out->y) * 0.5f * vp->height + vp->top;
        out->z = (vp->max_depth - vp->min_depth) * out->z + vp->min_depth;
    }
    return out;
}

inline vec3* vec3unproject(vec3* out, const vec3* v, const viewport* vp, const matrix* prj, const matrix* view, const matrix* world)
{
    assert(out && v);
    matrix m;
    const matrix* mt = &m;
    if(!prj && !view && !world)     { matidentity(&m); }
    else if(prj && view && world)   { matmultiply(&m, prj, view); matmultiply(&m, &m, world); }
    else if(prj && view)            { matmultiply(&m, prj, view); }
    else if(prj && world)           { matmultiply(&m, prj, world); }
    else if(view && world)          { matmultiply(&m, view, world); }
    else if(prj)                    { mt = prj; }
    else                            { mt = view ? view : world; }
    assert(mt);
    matinverse(&m, 0, mt);
    if(!vp) {
        vec3transformcoord(out, v, &m);
        return out;
    }
    out->x = 2.f * (v->x - vp->left) / vp->width - 1.f;
    out->y = 1.f - 2.f * (v->y - vp->top) / vp->height;
    out->z = (v->z - vp->min_depth) / (vp->max_depth - vp->min_depth);
    vec3transformcoord(out, out, &m);
    return out;
}

inline vec3* vec3projectarray(vec3* out, uint ostride, const vec3* v, uint vstride, const viewport* vp, const matrix* prj, const matrix* view, const matrix* world, uint n)
{
    assert(out && v);
    matrix m;
    const matrix* mt = &m;
    if(!prj && !view && !world)     { matidentity(&m); }
    else if(prj && view && world)   { matmultiply(&m, prj, view); matmultiply(&m, &m, world); }
    else if(prj && view)            { matmultiply(&m, prj, view); }
    else if(prj && world)           { matmultiply(&m, prj, world); }
    else if(view && world)          { matmultiply(&m, view, world); }
    else if(prj)                    { mt = prj; }
    else                            { mt = view ? view : world; }
    assert(mt);
    if(vp) {
        matrix mvp;
        matviewportproject(&mvp, vp);
        matmultiply(&m, mt, &mvp);
        mt = &m;
    }
    assert(mt);
    return vec3transformcoordarray(out, ostride, v, vstride, mt, n);
}

inline vec3* vec3unprojectarray(vec3* out, uint ostride, const vec3* v, uint vstride, const viewport* vp, const matrix* prj, const matrix* view, const matrix* world, uint n)
{
    assert(out && v);
    matrix m;
    const matrix* mt = &m;
    if(!prj && !view && !world)     { matidentity(&m); }
    else if(prj && view && world)   { matmultiply(&m, prj, view); matmultiply(&m, &m, world); }
    else if(prj && view)            { matmultiply(&m, prj, view); }
    else if(prj && world)           { matmultiply(&m, prj, world); }
    else if(view && world)          { matmultiply(&m, view, world); }
    else if(prj)                    { mt = prj; }
    else                            { mt = view ? view : world; }
    assert(mt);
    matinverse(&m, 0, mt);
    if(vp) {
        matrix mvp;
        matviewportunproject(&mvp, vp);
        matmultiply(&m, &mvp, &m);
    }
    return vec3transformcoordarray(out, ostride, v, vstride, &m, n);
}

inline float vec4length(const vec4* v)
{
    assert(v);
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z + v->w * v->w);
}

inline float vec4lengthsq(const vec4* v)
{
    assert(v);
    return v->x * v->x + v->y * v->y + v->z * v->z + v->w * v->w;
}

inline float vec4dot(const vec4* v1, const vec4* v2)
{
    assert(v1 && v2);
    return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z + v1->w * v2->w;
}

inline vec4* vec4add(vec4* out, const vec4* v1, const vec4* v2)
{
    assert(out && v1 && v2);
    out->x = v1->x + v2->x;
    out->y = v1->y + v2->y;
    out->z = v1->z + v2->z;
    out->w = v1->w + v2->w;
    return out;
}

inline vec4* vec4sub(vec4* out, const vec4* v1, const vec4* v2)
{
    assert(out && v1 && v2);
    out->x = v1->x - v2->x;
    out->y = v1->y - v2->y;
    out->z = v1->z - v2->z;
    out->w = v1->w - v2->w;
    return out;
}

inline vec4* vec4min(vec4* out, const vec4* v1, const vec4* v2)
{
    assert(out && v1 && v2);
    out->x = v1->x < v2->x ? v1->x : v2->x;
    out->y = v1->y < v2->y ? v1->y : v2->y;
    out->z = v1->z < v2->z ? v1->z : v2->z;
    out->w = v1->w < v2->w ? v1->w : v2->w;
    return out;
}

inline vec4* vec4max(vec4* out, const vec4* v1, const vec4* v2)
{
    assert(out && v1 && v2);
    out->x = v1->x > v2->x ? v1->x : v2->x;
    out->y = v1->y > v2->y ? v1->y : v2->y;
    out->z = v1->z > v2->z ? v1->z : v2->z;
    out->w = v1->w > v2->w ? v1->w : v2->w;
    return out;
}

inline vec4* vec4scale(vec4* out, const vec4* v, float s)
{
    assert(out && v);
    out->x = v->x * s;
    out->y = v->y * s;
    out->z = v->z * s;
    out->w = v->w * s;
    return out;
}

inline vec4* vec4lerp(vec4* out, const vec4* v1, const vec4* v2, float s)
{
    assert(out && v1 && v2);
    out->x = v1->x + s * (v2->x - v1->x);
    out->y = v1->y + s * (v2->y - v1->y);
    out->z = v1->z + s * (v2->z - v1->z);
    out->w = v1->w + s * (v2->w - v1->w);
    return out;
}

inline vec4* vec4multiply(vec4* out, const vec4* v, const matrix* m)
{
    assert(out && v && m);
    vec4 t;
    if(out == v) { t = *v; v = &t; }
    out->x = v->x * m->_11 + v->y * m->_21 + v->z * m->_31 + v->w * m->_41;
    out->y = v->x * m->_12 + v->y * m->_22 + v->z * m->_32 + v->w * m->_42;
    out->z = v->x * m->_13 + v->y * m->_23 + v->z * m->_33 + v->w * m->_43;
    out->w = v->x * m->_14 + v->y * m->_24 + v->z * m->_34 + v->w * m->_44;
    return out;
}

inline matrix3* mat3identity(matrix3* out)
{
    assert(out);
    out->m[0][1] = out->m[0][2] =
    out->m[1][0] = out->m[1][2] =
    out->m[2][0] = out->m[2][1] = 0.f;
    out->m[0][0] = out->m[1][1] = out->m[2][2] = 1.f;
    return out;
}

inline bool mat3isidentity(const matrix3* m)
{
    assert(m);
    return m->m[0][0] == 1.f && m->m[0][1] == 0.f && m->m[0][2] == 0.f &&
           m->m[1][0] == 0.f && m->m[1][1] == 1.f && m->m[1][2] == 0.f &&
           m->m[2][0] == 0.f && m->m[2][1] == 0.f && m->m[2][2] == 1.f;
}

inline float mat3determinant(const matrix3* m)
{
    assert(m);
    return m->_11 * (m->_22 * m->_33 - m->_23 * m->_32) +
        m->_12 * (m->_23 * m->_31 - m->_21 * m->_33) +
        m->_13 * (m->_21 * m->_32 - m->_22 * m->_31);
}

inline matrix3* mat3multiply(matrix3* out, const matrix3* m1, const matrix3* m2)
{
    assert(out && m1 && m2);
    vec3 c1(m2->_11, m2->_21, m2->_31),
        c2(m2->_12, m2->_22, m2->_32),
        c3(m2->_13, m2->_23, m2->_33);
    const vec3* r1 = (const vec3*)m1->m[0];
    const vec3* r2 = (const vec3*)m1->m[1];
    const vec3* r3 = (const vec3*)m1->m[2];
    out->_11 = r1->dot(c1), out->_12 = r1->dot(c2), out->_13 = r1->dot(c3);
    out->_21 = r2->dot(c1), out->_22 = r2->dot(c2), out->_23 = r2->dot(c3);
    out->_31 = r3->dot(c1), out->_32 = r3->dot(c2), out->_33 = r3->dot(c3);
    return out;
}

inline matrix3* mat3transpose(matrix3* m)
{
    assert(m);
    gs_swap(m->_12, m->_21);
    gs_swap(m->_13, m->_31);
    gs_swap(m->_23, m->_32);
    return m;
}

inline matrix3* mat3transpose(matrix3* out, const matrix3* m)
{
    assert(out && m);
    if(out != m)
        memcpy_s(out, sizeof(matrix3), m, sizeof(matrix3));
    return mat3transpose(out);
}

inline matrix3* mat3scaling(matrix3* out, float sx, float sy)
{
    assert(out);
    out->_12 = out->_13 = out->_21 = out->_23 = out->_31 = out->_32 = 0.f;
    out->_33 = 1.f;
    out->_11 = sx;
    out->_22 = sy;
    return out;
}

inline matrix3* mat3translation(matrix3* out, float x, float y)
{
    assert(out);
    out->_11 = out->_22 = out->_33 = 1.f;
    out->_12 = out->_21 = out->_13 = out->_23 = 0.f;
    out->_31 = x;
    out->_32 = y;
    return out;
}

inline matrix3* mat3rotation(matrix3* out, float angle)
{
    assert(out);
    out->_11 = out->_22 = cosf(angle);
    out->_21 = sinf(angle);
    out->_12 = -out->_21;
    out->_13 = out->_23 = out->_31 = out->_32 = 0.f;
    out->_33 = 1.f;
    return out;
}

inline matrix3* mat3rotationpos(matrix3* out, const vec2* p, float angle)
{
    assert(out && p);
    float sf = sinf(angle), cf = cosf(angle);
    out->_11 = out->_22 = cf;
    out->_12 = -sf;
    out->_21 = sf;
    out->_31 = out->_32 = 0.f;
    out->_13 = p->x * (1 - cf) + p->y * sf;
    out->_23 = p->y * (1 - cf) - p->x * sf;
    out->_33 = 1.f;
    return out;
}

inline matrix* matidentity(matrix* out)
{
    assert(out);
    out->m[0][1] = out->m[0][2] = out->m[0][3] =
    out->m[1][0] = out->m[1][2] = out->m[1][3] =
    out->m[2][0] = out->m[2][1] = out->m[2][3] =
    out->m[3][0] = out->m[3][1] = out->m[3][2] = 0.0f;
    out->m[0][0] = out->m[1][1] = out->m[2][2] = out->m[3][3] = 1.0f;
    return out;
}

inline bool matisidentity(const matrix* m)
{
    assert(m);
    return m->m[0][0] == 1.0f && m->m[0][1] == 0.0f && m->m[0][2] == 0.0f && m->m[0][3] == 0.0f &&
           m->m[1][0] == 0.0f && m->m[1][1] == 1.0f && m->m[1][2] == 0.0f && m->m[1][3] == 0.0f &&
           m->m[2][0] == 0.0f && m->m[2][1] == 0.0f && m->m[2][2] == 1.0f && m->m[2][3] == 0.0f &&
           m->m[3][0] == 0.0f && m->m[3][1] == 0.0f && m->m[3][2] == 0.0f && m->m[3][3] == 1.0f;
}

inline bool matdecompose(vec3* scale, quat* rot, vec3* trans, const matrix* m)
{
    assert(m);
    matrix mt;
    trans->x = m->_41;
    trans->y = m->_42;
    trans->z = m->_43;
    mt._11 = m->_11;
    mt._12 = m->_12;
    mt._13 = m->_13;
    mt._21 = m->_21;
    mt._22 = m->_22;
    mt._23 = m->_23;
    mt._31 = m->_31;
    mt._32 = m->_32;
    mt._33 = m->_33;
    mt._14 = mt._24 = mt._34 = mt._41 = mt._42 = mt._43 = 0;
    mt._44 = 1.f;
    scale->x = sqrtf(m->_11 * m->_11 + m->_12 * m->_12 + m->_13 * m->_13);
    scale->y = sqrtf(m->_21 * m->_21 + m->_22 * m->_22 + m->_23 * m->_23);
    scale->z = sqrtf(m->_31 * m->_31 + m->_32 * m->_32 + m->_33 * m->_33);
    vec3* v[] = { (vec3*)&mt._11, (vec3*)&mt._21, (vec3*)&mt._31 };
    int n = 0, i, j, k;
    if(scale->y <= scale->x) {
        if(scale->x >= scale->z) {
            i = 0;
            if(scale->y >= scale->z) { j = 1; k = 2; }
            else { j = 2; k = 1; }
        }
        else { i = 2; j = 0; k = 1; }
    }
    else if(scale->y < scale->z) { i = 2; j = 1; k = 0; }
    else if(scale->x < scale->z) { i = 1; j = 2; k = 0; }
    else { i = 1; j = 0; k = 2; }
    float* scf = &scale->x;
    vec3 univ[3] = { vec3(1.f, 0, 0), vec3(0, 1.f, 0), vec3(0, 0, 1.f) };
    static const float fcmp = 9.9999997473787516e-005f;
    vec3* v1 = v[i];
    if(scf[i] < fcmp) { *v1 = univ[i]; }
    vec3normalize(v1, v1);
    vec3* v2 = v[j];
    if(scf[j] < fcmp) {
        float a = abs(v1->x);
        float b = abs(v1->y);
        float c = abs(v1->z);
        if(b <= a) {
            if(c > a || c > b)
                n = 1;
        }
        else {
            if(c <= b && c <= a)
                n = 2;
        }
        v2->x = v1->y * univ[n].z - v1->z * univ[n].y;
        v2->y = v1->z * univ[n].x - v1->x * univ[n].z;
        v2->z = v1->x * univ[n].y - v1->y * univ[n].x;
    }
    vec3normalize(v2, v2);
    vec3* v3 = v[k];
    if(scf[k] < fcmp) {
        v3->x = v1->y * v2->z - v1->z * v2->y;
        v3->y = v1->z * v2->x - v1->x * v2->z;
        v3->z = v1->x * v2->y - v1->y * v2->x;
    }
    vec3normalize(v3, v3);
    float md = matdeterminant(&mt);
    if(md < 0) {
        md = -md;
        scf[i] = -scf[i];
        v1->x = -v1->x;
        v1->y = -v1->y;
        v1->z = -v1->z;
    }
    md -= 1.f;
    if(md * md <= fcmp) {
        quatrotatematrix(rot, &mt);
        return true;
    }
    return false;
}

inline matrix* mattranspose(matrix* m)
{
    assert(m);
    gs_swap(m->_12, m->_21);
    gs_swap(m->_13, m->_31);
    gs_swap(m->_14, m->_41);
    gs_swap(m->_23, m->_32);
    gs_swap(m->_24, m->_42);
    gs_swap(m->_34, m->_43);
    return m;
}

inline matrix* mattranspose(matrix* out, const matrix* m)
{
    assert(out && m);
    if(out != m)
        memcpy_s(out, sizeof(matrix), m, sizeof(matrix));
    return mattranspose(out);
}

inline matrix* matscaling(matrix* out, float sx, float sy, float sz)
{
    assert(out);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 =
        out->_31 = out->_32 = out->_34 = out->_41 = out->_42 = out->_43 = 0;
    out->_11 = sx;
    out->_22 = sy;
    out->_33 = sz;
    out->_44 = 1.f;
    return out;
}

inline matrix* mattranslation(matrix* out, float x, float y, float z)
{
    assert(out);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 =
        out-> _31 = out->_32 = out->_34 = 0;
    out->_11 = out->_22 = out->_33 = out->_44 = 1.f;
    out->_41 = x;
    out->_42 = y;
    out->_43 = z;
    return out;
}

inline matrix* matrotation_x(matrix* out, float angle)
{
    assert(out);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_24 = 
        out->_31 = out->_34 = out->_41 = out->_42 = out->_43 = 0;
    out->_11 = out->_44 = 1.f;
    out->_22 = out->_33 = cosf(angle);
    out->_23 = sinf(angle);
    out->_32 = -out->_23;
    return out;
}

inline matrix* matrotation_y(matrix* out, float angle)
{
    assert(out);
    out->_12 = out->_14 = out->_21 = out->_23 = out->_24 = out->_32 = out->_34 =
        out->_41 = out->_42 = out->_43 = 0;
    out->_22 = out->_44 = 1.f;
    out->_11 = out->_33 = cosf(angle);
    out->_31 = sinf(angle);
    out->_13 = -out->_31;
    return out;
}

inline matrix* matrotation_z(matrix* out, float angle)
{
    assert(out);
    out->_13 = out->_14 = out->_23 = out->_24 = out->_31 = out->_32 = out->_34 =
        out->_41 = out->_42 = out->_43 = 0;
    out->_33 = out->_44 = 1.f;
    out->_11 = out->_22 = cosf(angle);
    out->_12 = sinf(angle);
    out->_21 = -out->_12;
    return out;
}

inline matrix* matrotationaxis(matrix* out, const vec3* v, float angle)
{
    assert(out && v);
    vec3 nv;
    vec3normalize(&nv, v);
    out->_14 = out->_24 = out->_34 = out->_41 = out->_42 = out->_43 = 0;
    out->_44 = 1.f;
    float cct = cosf(angle), sct = sinf(angle), icct = 1.f - cct;
    float x = nv.x, y = nv.y, z = nv.z;
    float xy = x * y, xz = x * z, yz = y * z;
    out->_11 = x * x * icct + cct;
    out->_12 = xy * icct - z * sct;
    out->_13 = xz * icct + y * sct;
    out->_21 = xy * icct + z * sct;
    out->_22 = y * y * icct + cct;
    out->_23 = yz * icct - x * sct;
    out->_31 = xz * icct - y * sct;
    out->_32 = yz * icct + x * sct;
    out->_33 = z * z * icct + cct;
    return out;
}

inline matrix* matrotationquartern(matrix* out, const quat* q)
{
    assert(out && q);
    float x = q->x, y = q->y, z = q->z, w = q->w;
    float x2 = x * x, y2 = y * y, z2 = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;
    out->_14 = out->_24 = out->_34 = out->_41 = out->_42 = out->_43 = 0.f;
    out->_44 = 1.f;
    out->_11 = 1.f - 2.f * (y2 + z2);
    out->_21 = 2.f * (xy - wz);
    out->_31 = 2.f * (xz + wy);
    out->_12 = 2.f * (xy + wz);
    out->_22 = 1.f - 2.f * (x2 + z2);
    out->_32 = 2.f * (yz - wx);
    out->_13 = 2.f * (xz - wy);
    out->_23 = 2.f * (yz + wx);
    out->_33 = 1.f - 2.f * (x2 + y2);
    return out;
}

inline matrix* matrotationeuler(matrix* out, float yaw, float pitch, float roll)
{
    assert(out);
    float sy = sinf(yaw), cy = cosf(yaw);
    float sp = sinf(pitch), cp = cosf(pitch);
    float sr = sinf(roll), cr = cosf(roll);
    out->_14 = out->_24 = out->_34 = out->_41 = out->_42 = out->_43 = 0.f;
    out->_44 = 1.f;
    float crcy = cr * cy;
    float spsr = sp * sr;
    float crsy = cr * sy;
    out->_11 = crcy + spsr * sy;
    out->_12 = sr * cp;
    out->_13 = -crsy + spsr * cy;
    out->_21 = -sr * cy + sp * crsy;
    out->_22 = cr * cp;
    out->_23 = sy * sr + sp * crcy;
    out->_31 = sy * cp;
    out->_32 = -sp;
    out->_33 = cp * cy;
    return out;
}

inline matrix* matlookatlh(matrix* out, const vec3* eye, const vec3* at, const vec3* up)
{
    assert(out && eye && at && up);
    vec3 v1, v2, v3;
    vec3sub(&v3, at, eye);
    vec3normalize(&v3, &v3);
    vec3cross(&v1, up, &v3);
    vec3normalize(&v1, &v1);
    vec3cross(&v2, &v3, &v1);
    out->_14 = out->_24 = out->_34 = 0;
    out->_44 = 1.f;
    out->_11 = v1.x;
    out->_21 = v1.y;
    out->_31 = v1.z;
    out->_41 = -vec3dot(&v1, eye);
    out->_12 = v2.x;
    out->_22 = v2.y;
    out->_32 = v2.z;
    out->_42 = -vec3dot(&v2, eye);
    out->_13 = v3.x;
    out->_23 = v3.y;
    out->_33 = v3.z;
    out->_43 = -vec3dot(&v3, eye);
    return out;
}

inline matrix* matlookatrh(matrix* out, const vec3* eye, const vec3* at, const vec3* up)
{
    assert(out && eye && at && up);
    vec3 v1, v2, v3;
    vec3sub(&v3, eye, at);
    vec3normalize(&v3, &v3);
    vec3cross(&v1, up, &v3);
    vec3normalize(&v1, &v1);
    vec3cross(&v2, &v3, &v1);
    out->_14 = out->_24 = out->_34 = 0;
    out->_44 = 1.f;
    out->_11 = v1.x;
    out->_21 = v1.y;
    out->_31 = v1.z;
    out->_41 = -vec3dot(&v1, eye);
    out->_12 = v2.x;
    out->_22 = v2.y;
    out->_32 = v2.z;
    out->_42 = -vec3dot(&v2, eye);
    out->_13 = v3.x;
    out->_23 = v3.y;
    out->_33 = v3.z;
    out->_43 = -vec3dot(&v3, eye);
    return out;
}

inline matrix* matperspectivelh(matrix* out, float w, float h, float zn, float zf)
{
    assert(out);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 = out->_31 = out->_32 =
        out->_41 = out->_42 = out->_44 = 0;
    out->_11 = (zn + zn) / w;
    out->_22 = (zn + zn) / h;
    out->_33 = out->_43 = zf / (zf - zn);
    out->_34 = 1.f;
    out->_43 *= -zn;
    return out;
}

inline matrix* matperspectiverh(matrix* out, float w, float h, float zn, float zf)
{
    assert(out);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 = out->_31 = out->_32 =
        out->_41 = out->_42 = out->_44 = 0;
    out->_11 = (zn + zn) / w;
    out->_22 = (zn + zn) / h;
    out->_33 = out->_43 = zf / (zn - zf);
    out->_34 = -1.f;
    out->_43 *= zn;
    return out;
}

inline matrix* matperspectivefovlh(matrix* out, float fovy, float aspect, float zn, float zf)
{
    assert(out);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 = out->_31 = out->_32 =
        out->_41 = out->_42 = out->_44 = 0;
    float hfov = fovy * 0.5f;
    float cfov = cosf(hfov), sfov = sinf(hfov);
    out->_11 = out->_22 = cfov / sfov;
    out->_11 /= aspect;
    out->_33 = out->_43 = zf / (zf - zn);
    out->_43 *= -zn;
    out->_34 = 1.f;
    return out;
}

inline matrix* matperspectivefovrh(matrix* out, float fovy, float aspect, float zn, float zf)
{
    assert(out);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 = out->_31 = out->_32 =
        out->_41 = out->_42 = out->_44 = 0;
    float hfov = fovy * 0.5f;
    float cfov = cosf(hfov), sfov = sinf(hfov);
    out->_11 = out->_22 = cfov / sfov;
    out->_11 /= aspect;
    out->_33 = out->_43 = zf / (zn - zf);
    out->_43 *= zn;
    out->_34 = -1.f;
    return out;
}

inline matrix* matperspectiveoffcenterlh(matrix* out, float l, float r, float b, float t, float zn, float zf)
{
    assert(out);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 = out->_41 = out->_42 = out->_44 = 0;
    out->_34 = 1.f;
    out->_11 = 2.f * zn / (r - 1.f);
    out->_22 = 2.f * zn / (t - b);
    out->_31 = (1.f + r) / (1.f - r);
    out->_32 = (t + b) / (b - t);
    out->_33 = zf / (zf - zn);
    out->_43 = zn * zf / (zn - zf);
    return out;
}

inline matrix* matperspectiveoffcenterrh(matrix* out, float l, float r, float b, float t, float zn, float zf)
{
    assert(out);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 = out->_41 = out->_42 = out->_44 = 0;
    out->_34 = -1.f;
    out->_11 = 2.f * zn / (r - 1.f);
    out->_22 = 2.f * zn / (t - b);
    out->_31 = (1.f + r) / (r - 1.f);
    out->_32 = (t + b) / (t - b);
    out->_33 = zf / (zn - zf);
    out->_43 = zn * zf / (zn - zf);
    return out;
}

inline matrix* matortholh(matrix* out, float w, float h, float zn, float zf)
{
    assert(out);
    float szz = zf - zn;
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 =
        out->_31 = out->_32 = out->_34 = out->_41 = out->_42 = 0;
    out->_44 = 1.f;
    out->_11 = 2.f / w;
    out->_22 = 2.f / h;
    out->_33 = 1 / szz;
    out->_43 = -zn / szz;
    return out;
}

inline matrix* matorthorh(matrix* out, float w, float h, float zn, float zf)
{
    assert(out);
    float szz = zn - zf;
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 =
        out->_31 = out->_32 = out->_34 = out->_41 = out->_42 = 0;
    out->_44 = 1.f;
    out->_11 = 2.f / w;
    out->_22 = 2.f / h;
    out->_33 = 1 / szz;
    out->_43 = zn / szz;
    return out;
}

inline matrix* matorthooffcenterlh(matrix* out, float l, float r, float b, float t, float zn, float zf)
{
    assert(out);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 = out->_31 = out->_32 = out->_34 = 0;
    out->_44 = 1.f;
    out->_11 = 2.f / (r - 1.f);
    out->_22 = 2.f / (t - b);
    out->_33 = 1.f / (zf - zn);
    out->_41 = (1 + r) / (1 - r);
    out->_42 = (t + b) / (b - t);
    out->_43 = zn / (zn - zf);
    return out;
}

inline matrix* matorthooffcenterrh(matrix* out, float l, float r, float b, float t, float zn, float zf)
{
    assert(out);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 = out->_31 = out->_32 = out->_34 = 0;
    out->_44 = 1.f;
    out->_11 = 2.f / (r - 1.f);
    out->_22 = 2.f / (t - b);
    out->_33 = 1.f / (zn - zf);
    out->_41 = (1 + r) / (1 - r);
    out->_42 = (t + b) / (b - t);
    out->_43 = zn / (zn - zf);
    return out;
}

inline matrix* matviewportproject(matrix* out, const viewport* vp)
{
    assert(out && vp);
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 = out->_31 = out->_32 = out->_34 = 0;
    out->_44 = 1.f;
    out->_11 = out->_41 = (float)(vp->width >> 1);
    out->_22 = -(out->_42 = (float)(vp->height >> 1));
    out->_41 += vp->left;
    out->_42 += vp->top;
    out->_33 = vp->max_depth - vp->min_depth;
    out->_43 = vp->min_depth;
    return out;
}

inline matrix* matviewportunproject(matrix* out, const viewport* vp)
{
    assert(out && vp);
    float ddepth = vp->max_depth - vp->min_depth;
    out->_12 = out->_13 = out->_14 = out->_21 = out->_23 = out->_24 = out->_31 = out->_32 = out->_34 = 0;
    out->_44 = 1.f;
    out->_11 = 2.f / vp->width;
    out->_22 = -2.f / vp->height;
    out->_33 = 1.f / ddepth;
    out->_41 = out->_11 * (-vp->left) - 1.f;
    out->_42 = out->_22 * (-vp->top) + 1.f;
    out->_43 = -vp->min_depth / ddepth;
    return out;
}

inline matrix* mattransform(matrix* out, const vec3* scalecenter, const quat* scalerot, const vec3* scale, const vec3* rotcenter, const quat* rot, const vec3* trans)
{
    assert(out);
    return out;
}

inline matrix* mattransform2d(matrix* out, const vec2* scalecenter, float scalerot, const vec2* scale, const vec2* rotcenter, float rot, const vec2* trans);
inline matrix* mataffinetransform(matrix* out, float scale, const vec3* rotcenter, const quat* rot, const vec3* trans);
inline matrix* mataffinetransform2d(matrix* out, float scale, const vec3* rotcenter, const quat* rot, const vec3* trans);

inline float quatlength(const quat* q)
{
    assert(q);
    return sqrtf(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
}

inline float quatlengthsq(const quat* q)
{
    assert(q);
    return q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w;
}

inline float quatdot(const quat* q1, const quat* q2)
{
    assert(q1 && q2);
    return q1->x * q2->x + q1->y * q2->y + q1->z * q2->z + q1->w * q2->w;
}

inline quat* quatidentity(quat* out)
{
    assert(out);
    out->x = out->y = out->z = 0.0f;
    out->w = 1.0f;
    return out;
}

inline bool quatisidenetity(const quat* q)
{
    assert(q);
    return q->x == 0.0f && q->y == 0.0f && q->z == 0.0f && q->w == 1.0f;
}

inline quat* quatconjugate(quat* out, const quat* q)
{
    assert(out && q);
    out->x = -q->x;
    out->y = -q->y;
    out->z = -q->z;
    out->w =  q->w;
    return out;
}

inline void quattoaxisangle(const quat* q, vec3* axis, float* angle)
{
    assert(q);
    if(axis) {
        axis->x = q->x;
        axis->y = q->y;
        axis->z = q->z;
    }
    if(angle) {
        float ac = acosf(q->w);
        *angle = ac + ac;
    }
}

inline quat* quatrotatematrix(quat* out, const matrix* m)
{
    assert(out && m);
    float d = m->_11 + m->_22 + m->_33;
    if(d <= 0) {
        int i = 0, j, k;
        float v[3];
        v[0] = m->_11 - m->_22 - m->_33;
        v[1] = m->_22 - m->_11 - m->_33;
        v[2] = m->_33 - m->_11 - m->_22;
        int idx[3] = { 1, 2, 0 };
        if(v[1] > v[0]) i = 1;
        if(v[2] > v[i]) i = 2;
        j = idx[i];
        k = idx[j];
        out->q[i] = sqrtf(v[i] + 1.f) * 0.5f;
        out->q[j] = out->q[k] = out->q[3] = 0.25f / out->q[i];
        out->q[j] *= (m->m[j][i] + m->m[i][j]);
        out->q[k] *= (m->m[k][i] + m->m[i][k]);
        out->q[3] *= (m->m[j][k] - m->m[k][j]);
    }
    else {
        out->w = sqrtf(d + 1.f) * 0.5f;
        out->x = out->y = out->z = 0.25f / out->w;
        out->x *= (m->_23 - m->_32);
        out->y *= (m->_31 - m->_13);
        out->z *= (m->_12 - m->_21);
    }
    return out;
}

inline quat* quatrotateaxis(quat* out, const vec3* v, float angle)
{
    assert(out && v);
    vec3 nv;
    vec3normalize(&nv, v);
    angle *= 0.5f;
    out->w = cosf(angle);
    out->x = out->y = out->z = sinf(angle);
    out->x *= nv.x;
    out->y *= nv.y;
    out->z *= nv.z;
    return out;
}

inline bool cpt_boundingbox(float a, float b)
{
    double c = a - b;
    static const double bound = 1.1920928955078125e-007;
    return c >= -bound && c <= bound;
}

inline quat* quatln(quat* out, const quat* q)
{
    assert(out && q);
    out->x = q->x;
    out->y = q->y;
    out->z = q->z;
    out->w = 0;
    if(q->w >= 1.f)
        return out;
    float acqw = acosf(q->w);
    float sac = sinf(acqw);
    if(cpt_boundingbox(sac, 0))
        return out;
    acqw /= sac;
    out->x *= acqw;
    out->y *= acqw;
    out->z *= acqw;
    return out;
}

inline quat* quatexp(quat* out, const quat* q)
{
    assert(out && q);
    out->x = q->x;
    out->y = q->y;
    out->z = q->z;
    float chord = (float)sqrt(q->x * q->x + q->y * q->y + q->z * q->z);
    out->w = cosf(chord);
    float schd = sinf(chord);
    if(!cpt_boundingbox(schd, 0)) {
        schd /= chord;
        out->x *= schd;
        out->y *= schd;
        out->z *= schd;
    }
    return out;
}

inline quat* quatslerp(quat* out, const quat* q1, const quat* q2, float t)
{
    assert(out && q1 && q2);
    assert(t >= 0 && t <= 1.f);
    float comt = 1.f - t;
    float crsm = q1->x * q2->x + q1->y * q2->y + q1->z * q2->z + q1->w * q2->w;
    float sig = crsm < 0 ? -1.f : 1.f;
    crsm *= sig;
    if(1.0 - crsm > 1.1920928955078125e-007) {
        float c = sqrtf(1.f - crsm * crsm);
        float a = atan2f(c, crsm);
        c = 1.f / c;
        t = sinf(a * t) * c;
        comt = sinf(a * comt) * c;
    }
    t *= sig;
    out->x = q1->x * comt + q2->x * t;
    out->y = q1->y * comt + q2->y * t;
    out->z = q1->z * comt + q2->z * t;
    out->w = q1->w * comt + q2->w * t;
    return out;
}

inline quat* quatsquad(quat* out, const quat* q, const quat* a, const quat* b, const quat* c, float t)
{
    assert(out && q && a && b && c);
    quat tq1, tq2;
    quatslerp(&tq1, q, c, t);
    quatslerp(&tq2, a, b, t);
    return quatslerp(out, &tq1, &tq2, (1.f - t)*(t + t));
}

inline quat* quatbarycentric(quat* out, const quat* q1, const quat* q2, const quat* q3, float f, float g)
{
    assert(out && q1 && q2 && q3);
    float t = f + g;
    if(cpt_boundingbox(t, 0)) {
        if(out != q1)
            *out = *q1;
        return out;
    }
    else {
        quat tq1, tq2;
        quatslerp(&tq1, q1, q2, t);
        quatslerp(&tq2, q1, q3, t);
        return quatslerp(out, &tq1, &tq2, g/t);
    }
}

inline void quatsquadsetup(quat* a, quat* b, quat* c, const quat* q1, const quat* q2, const quat* q3, const quat* q4)
{
}

inline float planedot(const plane* p, const vec4* v)
{
    assert(p && v);
    return p->a * v->x + p->b * v->y + p->c * v->z + p->d * v->w;
}

inline float planedotcoord(const plane* p, const vec3* v)
{
    assert(p && v);
    return p->a * v->x + p->b * v->y + p->c * v->z + p->d;
}

inline float planedotnormal(const plane* p, const vec3* v)
{
    assert(p && v);
    return p->a * v->x + p->b * v->y + p->c * v->z;
}

inline plane* planescale(plane* out, const plane* p, float s)
{
    assert(out && p);
    out->a = p->a * s;
    out->b = p->b * s;
    out->c = p->c * s;
    out->d = p->d * s;
    return out;
}

inline plane* planefrompointnormal(plane* out, const vec3* p, const vec3* normal)
{
    assert(out && p && normal);
    out->a = normal->x;
    out->b = normal->y;
    out->c = normal->z;
    out->d = -p->x * normal->x - p->y * normal->y - p->z * normal->z;
    return out;
}

__gslib_end__

#endif
