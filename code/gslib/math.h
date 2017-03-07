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

#ifndef math_870b373b_c3fb_4c7a_abc7_bc4bd4095aef_h
#define math_870b373b_c3fb_4c7a_abc7_bc4bd4095aef_h

#include <gslib/config.h>
#include <gslib/type.h>

#ifndef PI
#define PI  (3.14159265358979323846f)
#endif

__gslib_begin__

struct _vec2 { float x, y; };
struct _vec3 { float x, y, z; };
struct _vec4 { float x, y, z, w; };
struct _mat2
{
    union
    {
        struct
        {
            float _11, _12, 
                _21, _22;
        };
        float  m[2][2];
    };
};
struct _mat3
{
    union
    {
        struct
        {
            float _11, _12, _13, 
                _21, _22, _23, 
                _31, _32, _33;
        };
        float   m[3][3];
    };
};
struct _mat4
{
    union
    {
        struct 
        {
            float _11, _12, _13, _14, 
                _21, _22, _23, _24, 
                _31, _32, _33, _34, 
                _41, _42, _43, _44;
        };
        float   m[4][4];
    };
};
struct _quat
{
    union
    {
        struct  { float x, y, z, w; };
        float   q[4];
    };
};
struct _plane { float a, b, c, d; };

struct viewport
{
    int         left;
    int         top;
    uint        width;
    uint        height;
    float       min_depth;
    float       max_depth;
};

class vec2;
class vec3;
class vec4;
class mat3;
class mat4;
class quat;
class plane;

typedef vec2 point2;
typedef vec3 point3;
typedef mat4 matrix;
typedef mat3 matrix3;
typedef _mat2 matrix2;

/* inline */
float vec2length(const vec2* p);
float vec2lengthsq(const vec2* p);
float vec2dot(const vec2* p1, const vec2* p2);
float vec2ccw(const vec2* p1, const vec2* p2);
vec2* vec2add(vec2* out, const vec2* p1, const vec2* p2);
vec2* vec2sub(vec2* out, const vec2* p1, const vec2* p2);
vec2* vec2min(vec2* out, const vec2* p1, const vec2* p2);
vec2* vec2max(vec2* out, const vec2* p1, const vec2* p2);
vec2* vec2scale(vec2* out, const vec2* p, float s);
vec2* vec2lerp(vec2* out, const vec2* p1, const vec2* p2, float s);
float vec2lerp_x(const vec2* p1, const vec2* p2, float y);
float vec2lerp_y(const vec2* p1, const vec2* p2, float x);
vec2* vec2multiply(vec2* out, const vec2* p, const matrix2* m);
vec2* vec2transformcoord(vec2* out, const vec2* p, const mat3* m);
vec2* vec2transformnormal(vec2* out, const vec2* p, const mat3* m);
float vec3length(const vec3* v);
float vec3lengthsq(const vec3* v);
float vec3dot(const vec3* v1, const vec3* v2);
vec3* vec3cross(vec3* out, const vec3* v1, const vec3* v2);
vec3* vec3add(vec3* out, const vec3* v1, const vec3* v2);
vec3* vec3sub(vec3* out, const vec3* v1, const vec3* v2);
vec3* vec3min(const vec3* v1, const vec3* v2);
vec3* vec3max(const vec3* v1, const vec3* v2);
vec3* vec3scale(vec3* out, const vec3* v, float s);
vec3* vec3lerp(vec3* out, const vec3* v1, const vec3* v2, float s);
vec3* vec3multiply(vec3* out, const vec3* v, const mat3* m);
vec3* vec3project(vec3* out, const vec3* v, const viewport* vp, const matrix* prj, const matrix* view, const matrix* world);
vec3* vec3unproject(vec3* out, const vec3* v, const viewport* vp, const matrix* prj, const matrix* view, const matrix* world);
vec3* vec3projectarray(vec3* out, uint ostride, const vec3* v, uint vstride, const viewport* vp, const matrix* prj, const matrix* view, const matrix* world, uint n);
vec3* vec3unprojectarray(vec3* out, uint ostride, const vec3* v, uint vstride, const viewport* vp, const matrix* prj, const matrix* view, const matrix* world, uint n);
float vec4length(const vec4* v);
float vec4lengthsq(const vec4* v);
float vec4dot(const vec4* v1, const vec4* v2);
vec4* vec4add(vec4* out, const vec4* v1, const vec4* v2);
vec4* vec4sub(vec4* out, const vec4* v1, const vec4* v2);
vec4* vec4min(const vec4* v1, const vec4* v2);
vec4* vec4max(const vec4* v1, const vec4* v2);
vec4* vec4scale(vec4* out, const vec4* v, float s);
vec4* vec4lerp(vec4* out, const vec4* v1, const vec4* v2, float s);
vec4* vec4multiply(vec4* out, const vec4* v, const matrix* m);
matrix3* mat3identity(matrix3* out);
bool mat3isidentity(const matrix3* m);
float mat3determinant(const matrix3* m);
matrix3* mat3multiply(matrix3* out, const matrix3* m1, const matrix3* m2);
matrix3* mat3transpose(matrix3* out, const matrix3* m);
matrix3* mat3scaling(matrix3* out, float sx, float sy);
matrix3* mat3translation(matrix3* out, float x, float y);
matrix3* mat3rotation(matrix3* out, float angle);
matrix3* mat3rotationpos(matrix3* out, const vec2* p, float angle);
matrix* matidentity(matrix* out);
bool matisidentity(const matrix* m);
bool matdecompose(vec3* scale, quat* rot, vec3* trans, const matrix* m);
matrix* mattranspose(matrix* out, const matrix* m);
matrix* matscaling(matrix* out, float sx, float sy, float sz);
matrix* mattranslation(matrix* out, float x, float y, float z);
matrix* matrotation_x(matrix* out, float angle);
matrix* matrotation_y(matrix* out, float angle); 
matrix* matrotation_z(matrix* out, float angle);
matrix* matrotationaxis(matrix* out, const vec3* v, float angle);
matrix* matrotationquartern(matrix* out, const quat* q);
matrix* matrotationeuler(matrix* out, float yaw, float pitch, float roll);
matrix* matlookatlh(matrix* out, const vec3* eye, const vec3* at, const vec3* up);
matrix* matlookatrh(matrix* out, const vec3* eye, const vec3* at, const vec3* up);
matrix* matperspectivelh(matrix* out, float w, float h, float zn, float zf);
matrix* matperspectiverh(matrix* out, float w, float h, float zn, float zf);
matrix* matperspectivefovlh(matrix* out, float fovy, float aspect, float zn, float zf);
matrix* matperspectivefovrh(matrix* out, float fovy, float aspect, float zn, float zf);
matrix* matperspectiveoffcenterlh(matrix* out, float l, float r, float b, float t, float zn, float zf);
matrix* matperspectiveoffcenterrh(matrix* out, float l, float r, float b, float t, float zn, float zf);
matrix* matortholh(matrix* out, float w, float h, float zn, float zf);
matrix* matorthorh(matrix* out, float w, float h, float zn, float zf);
matrix* matorthooffcenterlh(matrix* out, float l, float r, float b, float t, float zn, float zf);
matrix* matorthooffcenterrh(matrix* out, float l, float r, float b, float t, float zn, float zf);
matrix* matviewportproject(matrix* out, const viewport* vp);
matrix* matviewportunproject(matrix* out, const viewport* vp);
matrix* mattransform(matrix* out, const vec3* scalecenter, const quat* scalerot, const vec3* scale, const vec3* rotcenter, const quat* rot, const vec3* trans);
matrix* mattransform2d(matrix* out, const vec2* scalecenter, float scalerot, const vec2* scale, const vec2* rotcenter, float rot, const vec2* trans);
matrix* mataffinetransform(matrix* out, float scale, const vec3* rotcenter, const quat* rot, const vec3* trans);
matrix* mataffinetransform2d(matrix* out, float scale, const vec3* rotcenter, const quat* rot, const vec3* trans);
float quatlength(const quat* q);
float quatlengthsq(const quat* q);
float quatdot(const quat* q1, const quat* q2);
quat* quatidentity(quat* out);
bool quatisidenetity(const quat* q);
quat* quatconjugate(quat* out, const quat* q);
void quattoaxisangle(const quat* q, vec3* axis, float* angle);
quat* quatrotatematrix(quat* out, const matrix* m);
quat* quatrotateaxis(quat* out, const vec3* v, float angle);
quat* quatln(quat* out, const quat* q);
quat* quatexp(quat* out, const quat* q);
quat* quatslerp(quat* out, const quat* q1, const quat* q2, float t);
quat* quatsquad(quat* out, const quat* q, const quat* a, const quat* b, const quat* c, float t);
quat* quatbarycentric(quat* out, const quat* q1, const quat* q2, const quat* q3, float f, float g);
void quatsquadsetup(quat* a, quat* b, quat* c, const quat* q1, const quat* q2, const quat* q3, const quat* q4);
float planedot(const plane* p, const vec4* v);
float planedotcoord(const plane* p, const vec3* v);
float planedotnormal(const plane* p, const vec3* v);
plane* planescale(plane* out, const plane* p, float s);
plane* planefrompointnormal(plane* out, const vec3* p, const vec3* normal);

/* non-inline */
typedef vec2* (__stdcall *fnvec2normalize)(vec2* out, const vec2* v);
typedef vec2* (__stdcall *fnvec2hermite)(vec2* out, const vec2* p1, const vec2* t1, const vec2* p2, const vec2* t2, float s);
typedef vec2* (__stdcall *fnvec2catmullrom)(vec2* out, const vec2* p1, const vec2* p2, const vec2* p3, const vec2* p4, float s);
typedef vec2* (__stdcall *fnvec2barycentric)(vec2* out, const vec2* p1, const vec2* p2, const vec2* p3, float f, float g);
typedef vec4* (__stdcall *fnvec2transform)(vec4* out, const vec2* p, const matrix* m);
typedef vec2* (__stdcall *fnvec2transformcoord)(vec2* out, const vec2* p, const matrix* m);
typedef vec2* (__stdcall *fnvec2transformnormal)(vec2* out, const vec2* p, const matrix* m);
typedef vec4* (__stdcall *fnvec2transformarray)(vec4* out, uint ostride, const vec2* v, uint vstride, const matrix* m, uint n);
typedef vec2* (__stdcall *fnvec2transformcoordarray)(vec2* out, uint ostride, const vec2* v, uint vstride, const matrix* m, uint n);
typedef vec2* (__stdcall *fnvec2transformnormalarray)(vec2* out, uint ostride, const vec2* v, uint vstride, const matrix* m, uint n);
typedef vec3* (__stdcall *fnvec3normalize)(vec3* out, const vec3* v);
typedef vec3* (__stdcall *fnvec3hermite)(vec3* out, const vec3* v1, const vec3* t1, const vec3* v2, const vec3* t2, float s);
typedef vec3* (__stdcall *fnvec3catmullrom)(vec3* out, const vec3* v1, const vec3* v2, const vec3* v3, const vec3* v4, float s);
typedef vec3* (__stdcall *fnvec3barycentric)(vec3* out, const vec3* v1, const vec3* v2, const vec3* v3, float f, float g);
typedef vec4* (__stdcall *fnvec3transform)(vec4* out, const vec3* v, const matrix* m);
typedef vec3* (__stdcall *fnvec3transformcoord)(vec3* out, const vec3* v, const matrix* m);
typedef vec3* (__stdcall *fnvec3transformnormal)(vec3* out, const vec3* v, const matrix* m);
typedef vec4* (__stdcall *fnvec3transformarray)(vec4* out, uint ostride, const vec3* v, uint vstride, const matrix* m, uint n);
typedef vec3* (__stdcall *fnvec3transformcoordarray)(vec3* out, uint ostride, const vec3* v, uint vstride, const matrix* m, uint n);
typedef vec3* (__stdcall *fnvec3transformnormalarray)(vec3* out, uint ostride, const vec3* v, uint vstride, const matrix* m, uint n);
typedef vec4* (__stdcall *fnvec4cross)(vec4* out, const vec4* v1, const vec4* v2, const vec4* v3);
typedef vec4* (__stdcall *fnvec4normalize)(vec4* out, const vec4* v);
typedef vec4* (__stdcall *fnvec4hermite)(vec4* out, const vec4* v1, const vec4* t1, const vec4* v2, const vec4* t2, float s);
typedef vec4* (__stdcall *fnvec4catmullrom)(vec4* out, const vec4* v1, const vec4* v2, const vec4* v3, const vec4* v4, float s);
typedef vec4* (__stdcall *fnvec4barycentric)(vec4* out, const vec4* v1, const vec4* v2, const vec4* v3, float f, float g);
typedef vec4* (__stdcall *fnvec4transform)(vec4* out, const vec4* v, const matrix* m);
typedef vec4* (__stdcall *fnvec4transformarray)(vec4* out, uint ostride, const vec4* v, uint vstride, const matrix* m, uint n);
typedef float (__stdcall *fnmatdeterminant)(const matrix* m);
typedef matrix* (__stdcall *fnmatmultiply)(matrix* out, const matrix* m1, const matrix* m2);
typedef matrix* (__stdcall *fnmatmultiplytranspose)(matrix* out, const matrix* m1, const matrix* m2);
typedef matrix* (__stdcall *fnmatinverse)(matrix* out, float* determinant, const matrix* m);
typedef matrix* (__stdcall *fnmatshadow)(matrix* out, const vec4* plight, const plane* pln);
typedef matrix* (__stdcall *fnmatreflect)(matrix* out ,const plane* pln);
typedef quat* (__stdcall *fnquatrotateeuler)(quat* out, float yaw, float pitch, float roll);
typedef quat* (__stdcall *fnquatmultiply)(quat* out, const quat* q1, const quat* q2);
typedef quat* (__stdcall *fnquatnormalize)(quat* out, const quat* q);
typedef quat* (__stdcall *fnquatinverse)(quat* out, const quat* q);
typedef plane* (__stdcall *fnplanenormalize)(plane* out, const plane* p);
typedef vec3* (__stdcall *fnplaneintersectline)(vec3* out, const plane* p, const vec3* v1, const vec3* v2);
typedef plane* (__stdcall *fnplanefrompoints)(plane* out, const vec3* v1, const vec3* v2, const vec3* v3);
typedef plane* (__stdcall *fnplanetransform)(plane* out, const plane* p, const matrix* m);
typedef plane* (__stdcall *fnplanetransformarray)(plane* out, uint ostride, const plane* p, uint pstride, const matrix* m, uint n);

extern fnvec2normalize vec2normalize;
extern fnvec2hermite vec2hermite;
extern fnvec2catmullrom vec2catmullrom;
extern fnvec2barycentric vec2barycentric;
extern fnvec2transform vec2transform;
extern fnvec2transformarray vec2transformarray;
extern fnvec2transformcoordarray vec2transformcoordarray;
extern fnvec2transformnormalarray vec2transformnormalarray;
extern fnvec3normalize vec3normalize;
extern fnvec3hermite vec3hermite;
extern fnvec3catmullrom vec3catmullrom;
extern fnvec3barycentric vec3barycentric;
extern fnvec3transform vec3transform;
extern fnvec3transformcoord vec3transformcoord;
extern fnvec3transformnormal vec3transformnormal;
extern fnvec3transformarray vec3transformarray;
extern fnvec3transformcoordarray vec3transformcoordarray;
extern fnvec3transformnormalarray vec3transformnormalarray;
extern fnvec4cross vec4cross;
extern fnvec4normalize vec4normalize;
extern fnvec4hermite vec4hermite;
extern fnvec4catmullrom vec4catmullrom;
extern fnvec4barycentric vec4barycentric;
extern fnvec4transform vec4transform;
extern fnvec4transformarray vec4transformarray;
extern fnmatdeterminant matdeterminant;
extern fnmatmultiply matmultiply;
extern fnmatmultiplytranspose matmultiplytranspose;
extern fnmatinverse matinverse;
extern fnmatshadow matshadow;
extern fnmatreflect matreflect;
extern fnquatrotateeuler quatrotateeuler;
extern fnquatmultiply quatmultiply;
extern fnquatnormalize quatnormalize;
extern fnquatinverse quatinverse;
extern fnplanenormalize planenormalize;
extern fnplaneintersectline planeintersectline;
extern fnplanefrompoints planefrompoints;
extern fnplanetransform planetransform;
extern fnplanetransformarray planetransformarray;

class vec2:
    public _vec2
{
public:
    vec2() {}
    vec2(const float*);
    vec2(float x, float y);

public:
    operator float* ();
    operator const float* () const;

public:
    vec2& operator += (const vec2&);
    vec2& operator -= (const vec2&);
    vec2& operator *= (float);
    vec2& operator /= (float);

public:
    vec2 operator + () const;
    vec2 operator - () const;

public:
    vec2 operator + (const vec2&) const;
    vec2 operator - (const vec2&) const;
    vec2 operator * (float) const;
    vec2 operator / (float) const;
    friend vec2 operator * (float, const vec2&);

public:
    bool operator == (const vec2&) const;
    bool operator != (const vec2&) const;

public:
    float length() const { return vec2length(this); }
    float lengthsq() const { return vec2lengthsq(this); }
    float dot(const vec2& v) const { return vec2dot(this, &v); }
    float ccw(const vec2& v) const { return vec2ccw(this, &v); }
    vec2& add(const vec2& p1, const vec2& p2) { return *vec2add(this, &p1, &p2); }
    vec2& sub(const vec2& p1, const vec2& p2) { return *vec2sub(this, &p1, &p2); }
    vec2& scale(float s) { return *vec2scale(this, this, s); }
    vec2& scale(const vec2& v, float s) { return *vec2scale(this, &v, s); }
    vec2& lerp(const vec2& p1, const vec2& p2, float s) { return *vec2lerp(this, &p1, &p2, s); }
    vec2& multiply(const matrix2& m) { return *vec2multiply(this, this, &m); }
    vec2& multiply(const vec2& p, const matrix2& m) { return *vec2multiply(this, &p, &m); }
    vec2& normalize() { return *vec2normalize(this, this); }
    vec2& normalize(const vec2& v) { return *vec2normalize(this, &v); }
    vec2& hermite(const vec2& p1, const vec2& t1, const vec2& p2, const vec2& t2, float s) { return *vec2hermite(this, &p1, &t1, &p2, &t2, s); }
    vec2& catmullrom(const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4, float s) { return *vec2catmullrom(this, &p1, &p2, &p3, &p4, s); }
    vec2& barycentric(const vec2& p1, const vec2& p2, const vec2& p3, float f, float g) { return *vec2barycentric(this, &p1, &p2, &p3, f, g); }
    vec4& transform(vec4& v, const matrix& m) const { return *vec2transform(&v, this, &m); }
    vec2& transformcoord(const mat3& m) { return *vec2transformcoord(this, this, &m); }
    vec2& transformcoord(const vec2& p, const mat3& m) { return *vec2transformcoord(this, &p, &m); }
    vec2& transformnormal(const mat3& m) { return *vec2transformnormal(this, this, &m); }
    vec2& transformnormal(const vec2& p, const mat3& m) { return *vec2transformnormal(this, &p, &m); }
};

class vec3:
    public _vec3
{
public:
    vec3() {}
    vec3(const float*);
    vec3(float x, float y, float z);

public:
    operator float* ();
    operator const float* () const;

public:
    vec3& operator += (const vec3&);
    vec3& operator -= (const vec3&);
    vec3& operator *= (float);
    vec3& operator /= (float);
    
public:
    vec3 operator + () const;
    vec3 operator - () const;

public:
    vec3 operator + (const vec3&) const;
    vec3 operator - (const vec3&) const;
    vec3 operator * (float) const;
    vec3 operator / (float) const;
    friend vec3 operator * (float, const vec3&);

public:
    bool operator == (const vec3&) const;
    bool operator != (const vec3&) const;

public:
    float length() const { return vec3length(this); }
    float lengthsq() const { return vec3lengthsq(this); }
    float dot(const vec3& v) const { return vec3dot(this, &v); }
    vec3& add(const vec3& v1, const vec3& v2) { return *vec3add(this, &v1, &v2); }
    vec3& sub(const vec3& v1, const vec3& v2) { return *vec3sub(this, &v1, &v2); }
    vec3& cross(const vec3& v) { return *vec3cross(this, &vec3(*this), &v); }
    vec3& cross(const vec3& v1, const vec3& v2) { return *vec3cross(this, &v1, &v2); }
    vec3& scale(float s) { return *vec3scale(this, this, s); }
    vec3& scale(const vec3& v, float s) { return *vec3scale(this, &v, s); }
    vec3& lerp(const vec3& p1, const vec3& p2, float s) { return *vec3lerp(this, &p1, &p2, s); }
    vec3& multiply(const vec3& p, const mat3& m) { return *vec3multiply(this, &p, &m); }
    vec3& project(const viewport& vp, const matrix& prj, const matrix& view, const matrix& world) { return *vec3project(this, this, &vp, &prj, &view, &world); }
    vec3& unproject(const viewport& vp, const matrix& prj, const matrix& view, const matrix& world) { return *vec3unproject(this, this, &vp, &prj, &view, &world); }
    vec3& normalize() { return *vec3normalize(this, this); }
    vec3& normalize(const vec3& v) { return *vec3normalize(this, &v); }
    vec3& hermite(const vec3& v1, const vec3& t1, const vec3& v2, const vec3& t2, float s) { return *vec3hermite(this, &v1, &t1, &v2, &t2, s); }
    vec3& catmullrom(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& v4, float s) { return *vec3catmullrom(this, &v1, &v2, &v3, &v4, s); }
    vec3& barycentric(const vec3& v1, const vec3& v2, const vec3& v3, float f, float g) { return *vec3barycentric(this, &v1, &v2, &v3, f, g); }
    vec4& transform(vec4& v, const matrix& m) const { return *vec3transform(&v, this, &m); }
    vec3& transformcoord(const matrix& m) { return *vec3transformcoord(this, this, &m); }
    vec3& transformcoord(const vec3& v, const matrix& m) { return *vec3transformcoord(this, &v, &m); }
    vec3& transformnormal(const matrix& m) { return *vec3transformnormal(this, this, &m); }
    vec3& transformnormal(const vec3& v, const matrix& m) { return *vec3transformnormal(this, &v, &m); }
};

class vec4:
    public _vec4
{
public:
    vec4() {}
    vec4(const float*);
    vec4(const _vec3& xyz, float w);
    vec4(float x, float y, float z, float w);

public:
    operator float* ();
    operator const float* () const;

public:
    vec4& operator += (const vec4&);
    vec4& operator -= (const vec4&);
    vec4& operator *= (float);
    vec4& operator /= (float);
    
public:
    vec4 operator + () const;
    vec4 operator - () const;

public:
    vec4 operator + (const vec4&) const;
    vec4 operator - (const vec4&) const;
    vec4 operator * (float) const;
    vec4 operator / (float) const;
    friend vec4 operator * (float, const vec4&);

public:
    bool operator == (const vec4&) const;
    bool operator != (const vec4&) const;

public:
    float length() const { return vec4length(this); }
    float lengthsq() const { return vec4lengthsq(this); }
    float dot(const vec4& v) const { return vec4dot(this, &v); }
    vec4& add(const vec4& v1, const vec4& v2) { return *vec4add(this, &v1, &v2); }
    vec4& sub(const vec4& v1, const vec4& v2) { return *vec4sub(this, &v1, &v2); }
    vec4& scale(float s) { return *vec4scale(this, this, s); }
    vec4& scale(const vec4& v, float s) { return *vec4scale(this, &v, s); }
    vec4& lerp(const vec4& v1, const vec4& v2, float s) { return *vec4lerp(this, &v1, &v2, s); }
    vec4& multiply(const matrix& m) { return *vec4multiply(this, this, &m); }
    vec4& multiply(const vec4& v, const matrix& m) { return *vec4multiply(this, &v, &m); }
    vec4& cross(const vec4& v1, const vec4& v2, const vec4& v3) { return *vec4cross(this, &v1, &v2, &v3); }
    vec4& hermite(const vec4& v1, const vec4& t1, const vec4& v2, const vec4& t2, float s) { return *vec4hermite(this, &v1, &t1, &v2, &t2, s); }
    vec4& catmullrom(const vec4& v1, const vec4& v2, const vec4& v3, const vec4& v4, float s) { return *vec4catmullrom(this, &v1, &v2, &v3, &v4, s); }
    vec4& barycentric(const vec4& v1, const vec4& v2, const vec4& v3, float f, float g) { return *vec4barycentric(this, &v1, &v2, &v3, f, g); }
    vec4& transform(const matrix& m) { return *vec4transform(this, this, &m); }
    vec4& transform(const vec4& v, const matrix& m) { return *vec4transform(this, &v, &m); }
};

class mat3:
    public _mat3
{
public:
    mat3() {}
    mat3(const float*);
    mat3(const mat3&);
    mat3(float f11, float f12, float f13,
         float f21, float f22, float f23,
         float f31, float f32, float f33
         );

public:
    float& operator () (uint row, uint col);
    float operator () (uint row, uint col) const;
    operator float* ();
    operator const float* () const;

public:
    mat3& operator *= (const mat3&);
    mat3& operator += (const mat3&);
    mat3& operator -= (const mat3&);
    mat3& operator *= (float);
    mat3& operator /= (float);

public:
    mat3 operator + () const;
    mat3 operator - () const;

public:
    mat3 operator * (const mat3&) const;
    mat3 operator + (const mat3&) const;
    mat3 operator - (const mat3&) const;
    mat3 operator * (float) const;
    mat3 operator / (float) const;
    friend mat3 operator * (float, const mat3&);

public:
    bool operator == (const mat3&) const;
    bool operator != (const mat3&) const;

public:
    mat3& identity() { return *mat3identity(this); }
    bool isidentity() const { return mat3isidentity(this); }
    mat3& transpose() { return *mat3transpose(this, this); }
    mat3& transpose(const mat3& m) { return *mat3transpose(this, &m); }
    mat3& scaling(float sx, float sy) { return *mat3scaling(this, sx, sy); }
    mat3& translation(float x, float y) { return *mat3translation(this, x, y); }
    mat3& rotation(float angle) { return *mat3rotation(this, angle); }
    mat3& rotationpos(const vec2& p, float angle) { return *mat3rotationpos(this, &p, angle); }
    mat3& multiply(const mat3& m) { return *mat3multiply(this, this, &m); }
    mat3& multiply(const mat3& m1, const mat3& m2) { return *mat3multiply(this, &m1, &m2); }
    float determinant() const { return mat3determinant(this); }
};

class mat4:
    public _mat4
{
public:
    mat4() {}
    mat4(const float*);
    mat4(const mat4&);
    mat4(float f11, float f12, float f13, float f14,
         float f21, float f22, float f23, float f24,
         float f31, float f32, float f33, float f34,
         float f41, float f42, float f43, float f44);

public:
    float& operator () (uint row, uint col);
    float operator () (uint row, uint col) const;
    operator float* ();
    operator const float* () const;

public:
    mat4& operator *= (const mat4&);
    mat4& operator += (const mat4&);
    mat4& operator -= (const mat4&);
    mat4& operator *= (float);
    mat4& operator /= (float);
    
public:
    mat4 operator + () const;
    mat4 operator - () const;
    
public:
    mat4 operator * (const mat4&) const;
    mat4 operator + (const mat4&) const;
    mat4 operator - (const mat4&) const;
    mat4 operator * (float) const;
    mat4 operator / (float) const;
    friend mat4 operator * (float, const mat4&);

public:
    bool operator == (const mat4&) const;
    bool operator != (const mat4&) const;

public:
    mat4& identity() { return *matidentity(this); }
    bool isidentity() const { return matisidentity(this); }
    bool decompose(vec3& scale, quat& rot, vec3& trans) { return matdecompose(&scale, &rot, &trans, this); }
    mat4& transpose() { return *mattranspose(this, this); }
    mat4& transpose(const mat4& m) { return *mattranspose(this, &m); }
    mat4& scaling(float sx, float sy, float sz) { return *matscaling(this, sx, sy, sz); }
    mat4& translation(float x, float y, float z) { return *mattranslation(this, x, y, z); }
    mat4& rotation_x(float angle) { return *matrotation_x(this, angle); }
    mat4& rotation_y(float angle) { return *matrotation_y(this, angle); }
    mat4& rotation_z(float angle) { return *matrotation_z(this, angle); }
    mat4& rotation(const vec3& v, float angle) { return *matrotationaxis(this, &v, angle); }
    mat4& rotation(const quat& q) { return *matrotationquartern(this, &q); }
    mat4& rotation(float yaw, float pitch, float roll) { return *matrotationeuler(this, yaw, pitch, roll); }
    mat4& lookatlh(const vec3& eye, const vec3& at, const vec3& up) { return *matlookatlh(this, &eye, &at, &up); }
    mat4& lookatrh(const vec3& eye, const vec3& at, const vec3& up) { return *matlookatrh(this, &eye, &at, &up); }
    mat4& perspectivelh(float w, float h, float zn, float zf) { return *matperspectivelh(this, w, h, zn, zf); }
    mat4& perspectiverh(float w, float h, float zn, float zf) { return *matperspectiverh(this, w, h, zn, zf); }
    mat4& perspectivefovlh(float fovy, float aspect, float zn, float zf) { return *matperspectivefovlh(this, fovy, aspect, zn, zf); }
    mat4& perspectivefovrh(float fovy, float aspect, float zn, float zf) { return *matperspectivefovrh(this, fovy, aspect, zn, zf); }
    mat4& perspectiveoffcenterlh(float l, float r, float b, float t, float zn, float zf) { return *matperspectiveoffcenterlh(this, l ,r, b, t, zn, zf); }
    mat4& perspectiveoffcenterrh(float l, float r, float b, float t, float zn, float zf) { return *matperspectiveoffcenterlh(this, l ,r, b, t, zn, zf); }
    mat4& ortholh(float w, float h, float zn, float zf) { return *matortholh(this, w, h, zn, zf); }
    mat4& orthorh(float w, float h, float zn, float zf) { return *matorthorh(this, w, h, zn, zf); }
    mat4& orthooffcenterlh(float l, float r, float b, float t, float zn, float zf) { return *matorthooffcenterlh(this, l, r, b, t, zn, zf); }
    mat4& orthooffcenterrh(float l, float r, float b, float t, float zn, float zf) { return *matorthooffcenterrh(this, l, r, b, t, zn, zf); }
    mat4& viewportproject(const viewport& vp) { return *matviewportproject(this, &vp); }
    mat4& viewportunproject(const viewport& vp) { return *matviewportunproject(this, &vp); }
    mat4& transform(const vec3& scalecenter, const quat& scalerot, const vec3& scale, const vec3& rotcenter, const quat& rot, const vec3& trans) { return *mattransform(this, &scalecenter, &scalerot, &scale, &rotcenter, &rot, &trans); }
    mat4& transform2d(const vec2& scalecenter, float scalerot, const vec2& scale, const vec2& rotcenter, float rot, const vec2& trans) { return *mattransform2d(this, &scalecenter, scalerot, &scale, &rotcenter, rot, &trans); }
    mat4& affinetransform(float scale, const vec3& rotcenter, const quat& rot, const vec3& trans) { return *mataffinetransform(this, scale, &rotcenter, &rot, &trans); }
    mat4& affinetransform2d(float scale, const vec3& rotcenter, const quat& rot, const vec3& trans);    // todo:
    float determinant() const { return matdeterminant(this); }
    mat4& multiply(const mat4& m) { return *matmultiply(this, this, &m); }
    mat4& multiply(const mat4& m1, const mat4& m2) { return *matmultiply(this, &m1, &m2); }
    mat4& multiplytranspose(const mat4& m) { return *matmultiplytranspose(this, this, &m); }
    mat4& multiplytranspose(const mat4& m1, const mat4& m2) { return *matmultiplytranspose(this, &m1, &m2); }
    mat4& inverse(float* determinant, const mat4& m) { return *matinverse(this, determinant, &m); }
    mat4& shadow(const vec4& plight, const plane& pln) { return *matshadow(this, &plight, &pln); }
    mat4& reflect(const plane& pln) { return *matreflect(this, &pln); }
};

class quat:
    public _quat
{
public:
    quat() {}
    quat(const float*);
    quat(float x, float y, float z, float w);

public:
    operator float* ();
    operator const float* () const;

public:
    quat& operator += (const quat&);
    quat& operator -= (const quat&);
    quat& operator *= (const quat&);
    quat& operator *= (float);
    quat& operator /= (float);
    
public:
    quat operator + () const;
    quat operator - () const;

public:
    quat operator + (const quat&) const;
    quat operator - (const quat&) const;
    quat operator * (const quat&) const;
    quat operator * (float) const;
    quat operator / (float) const;
    friend quat operator * (float, const quat&);

public:
    bool operator == (const quat&) const;
    bool operator != (const quat&) const;

public:
    float length() const { return quatlength(this); }
    float lengthsq() const { return quatlengthsq(this); }
    float dot(const quat& q) const { return quatdot(this, &q); }
    quat& identity() { return *quatidentity(this); }
    bool isidentity() const { return quatisidenetity(this); }
    quat& conjugate() { return *quatconjugate(this, this); }
    quat& conjugate(const quat& q) { return *quatconjugate(this, &q); }
    float toaxisangle(vec3& axis) const
    {
        float f;
        quattoaxisangle(this, &axis, &f);
        return f;
    }
    quat& rotate(const matrix& m) { return *quatrotatematrix(this, &m); }
    quat& rotate(const vec3& v, float angle) { return *quatrotateaxis(this, &v, angle); }
    quat& ln() { return *quatln(this, this); }
    quat& ln(const quat& q) { return *quatln(this, &q); }
    quat& exp() { return *quatexp(this, this); }
    quat& exp(const quat& q) { return *quatexp(this, &q); }
    quat& slerp(const quat& q1, const quat& q2, float t) { return *quatslerp(this, &q1, &q2, t); }
    quat& squad(const quat& q, const quat& a, const quat& b, const quat& c, float f) { return *quatsquad(this, &q, &a, &b, &c, f); }
    quat& squad(const quat& a, const quat& b, const quat& c, float f) { return *quatsquad(this, this, &a, &b, &c, f); }
    quat& barycentric(const quat& q1, const quat& q2, const quat& q3, float f, float g) { return *quatbarycentric(this, &q1, &q2, &q3, f, g); }
    quat& rotate(float yaw, float pitch, float roll) { return *quatrotateeuler(this, yaw, pitch, roll); }
    quat& multiply(const quat& q1, const quat& q2) { return *quatmultiply(this, &q1, &q2); }
    quat& normalize() { return *quatnormalize(this, this); }
    quat& normalize(const quat& q) { return *quatnormalize(this, &q); }
    quat& inverse() { return *quatinverse(this, this); }
    quat& inverse(const quat& q) { return *quatinverse(this, &q); }
};

class plane:
    public _plane
{
public:
    plane() {}
    plane(const float*);
    plane(float a, float b, float c, float d);

public:
    operator float* ();
    operator const float* () const;

public:
    plane& operator *= (float);
    plane& operator /= (float);
    plane operator + () const;
    plane operator - () const;
    plane operator * (float) const;
    plane operator / (float) const;
    friend plane operator * (float, const plane&);
    
public:
    bool operator == (const plane&) const;
    bool operator != (const plane&) const;

public:
    float dot(const vec4& v) const { return planedot(this, &v); }
    float dotcoord(const vec3& v) const { return planedotcoord(this, &v); }
    float dotnormal(const vec3& v) const { return planedotnormal(this, &v); }
    plane& scale(float s) { return *planescale(this, this, s); }
    plane& scale(const plane& p, float s) { return *planescale(this, &p, s); }
    plane& frompointnormal(const vec3& p, const vec3& normal) { return *planefrompointnormal(this, &p, &normal); }
    plane& normalize() { return *planenormalize(this, this); }
    plane& normalize(const plane& p) { *planenormalize(this, &p); }
    vec3& intersectline(vec3& v, const vec3& v1, const vec3& v2) const { return *planeintersectline(&v, this, &v1, &v2); }
    plane& frompoints(const vec3& v1, const vec3& v2, const vec3& v3) { return *planefrompoints(this, &v1, &v2, &v3); }
    plane& transform(const matrix& m) { return *planetransform(this, this, &m); }
    plane& transform(const plane& p, const matrix& m) { return *planetransform(this, &p, &m); }
};

__gslib_end__

#include <gslib/math.inl>

#endif
