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

#include <float.h>
#include <complex>
#include <gslib/std.h>
#include <gslib/error.h>
#include <gslib/utility.h>

__gslib_begin__

static const float __plot_tol = 1.5f;
static const float __plot_step_len = 7.f;

void linear_interpolate(vec2 c[], const vec2& p1, const vec2& p2, int step)
{
    float chord, t;
    chord = t = 1.f / (-- step);
    vec2 para[2];
    get_linear_parameter_equation(para, p1, p2);
    c[0] = p1;
    for(int i = 1; i < step; i ++, t += chord)
        eval_linear(c[i], para, t);
    c[step] = p2;
}

void quadratic_interpolate(vec2 c[], const vec2& p1, const vec2& p2, const vec2& p3, int step)
{
    float chord, t;
    chord = t = 1.f / (-- step);
    vec3 para[2];
    get_quad_parameter_equation(para, p1, p2, p3);
    c[0] = p1;
    for(int i = 1; i < step; i ++, t += chord)
        eval_quad(c[i], para, t);
    c[step] = p3;
}

void cubic_interpolate(vec2 c[], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4, int step)
{
    float chord, t;
    chord = t = 1.f / (-- step);
    vec4 para[2];
    get_cubic_parameter_equation(para, p1, p2, p3, p4);
    c[0] = p1;
    for(int i = 1; i < step; i ++, t += chord)
        eval_cubic(c[i], para, t);
    c[step] = p4;
}

void hermite_interpolate(vec2 c[], const vec2& p1, const vec2& p2, const vec2& t1, const vec2& t2, int step)
{
    float chord, t;
    chord = t = 1.f / (-- step);
    vec2* cv = c;
    for(int i = 1; i < step; i ++, cv ++, t += chord)
        cv->hermite(p1, t1, p2, t2, t);
}

float linear_reparameterize(const vec2& p1, const vec2& p2, const vec2& p)
{
    float xdis = p2.x - p1.x, ydis = p2.y - p1.y;
    float xspan = abs(xdis), yspan = abs(ydis);
    return xspan > yspan ? (p.x - p1.x) / xdis : (p.y - p1.y) / ydis;
}

float quad_reparameterize(const vec3 para[2], const vec2& p, float tol)
{
    vec3 tryreparax = para[0];
    tryreparax.z -= p.x;
    float tx[2];
    int c1 = solve_univariate_quadratic(tx, tryreparax);
    for(int i = 0; i < c1; i ++) {
        float t = tx[i];
        float y = para[1].dot(vec3(t * t, t, 1.f));
        if(fuzz_cmp(y, p.y, tol) == 0.f)
            return t;
    }
    vec3 tryreparay = para[1];
    tryreparay.z -= p.y;
    float ty[2];
    int c2 = solve_univariate_quadratic(ty, tryreparay);
    for(int i = 0; i < c2; i ++) {
        float t = ty[i];
        float x = para[0].dot(vec3(t * t, t, 1.f));
        if(fuzz_cmp(x, p.x, tol) == 0.f)
            return t;
    }
    return -1.f;
}

float cubic_reparameterize(const vec4 para[2], const vec2& p, float tol)
{
    vec4 tryreparax = para[0];
    tryreparax.w -= p.x;
    float tx[3];
    int c1 = solve_univariate_cubic(tx, tryreparax);
    for(int i = 0; i < c1; i ++) {
        float t = tx[i];
        float y = para[1].dot(vec4(t * t * t, t * t, t, 1.f));
        if(fuzz_cmp(y, p.y, tol) == 0.f)
            return t;
    }
    vec4 tryreparay = para[1];
    tryreparay.w -= p.y;
    float ty[3];
    int c2 = solve_univariate_cubic(ty, tryreparay);
    for(int i = 0; i < c2; i ++) {
        float t = ty[i];
        float x = para[0].dot(vec4(t * t * t, t * t, t, 1.f));
        if(fuzz_cmp(x, p.x, tol) == 0.f)
            return t;
    }
    return -1.f;
}

float best_quad_reparameterize(const vec3 para[2], const vec2& p)
{
    float s = -1.f;
    float min_dis = FLT_MAX;
    vec3 tryreparax = para[0];
    tryreparax.z -= p.x;
    float tx[2];
    int c1 = solve_univariate_quadratic(tx, tryreparax);
    for(int i = 0; i < c1; i ++) {
        float t = tx[i];
        float y = para[1].dot(vec3(t * t, t, 1.f));
        float dis = abs(y - p.y);
        if(dis < min_dis) {
            s = t;
            min_dis = dis;
        }
    }
    vec3 tryreparay = para[1];
    tryreparay.z -= p.y;
    float ty[2];
    int c2 = solve_univariate_quadratic(ty, tryreparay);
    for(int i = 0; i < c2; i ++) {
        float t = ty[i];
        float x = para[0].dot(vec3(t * t, t, 1.f));
        float dis = abs(x - p.x);
        if(dis < min_dis) {
            s = t;
            min_dis = dis;
        }
    }
    return s;
}

float best_cubic_reparameterize(const vec4 para[2], const vec2& p)
{
    float s = -1.f;
    float min_dis = FLT_MAX;
    vec4 tryreparax = para[0];
    tryreparax.w -= p.x;
    float tx[3];
    int c1 = solve_univariate_cubic(tx, tryreparax);
    for(int i = 0; i < c1; i ++) {
        float t = tx[i];
        float y = para[1].dot(vec4(t * t * t, t * t, t, 1.f));
        float dis = abs(y - p.y);
        if(dis < min_dis) {
            s = t;
            min_dis = dis;
        }
    }
    vec4 tryreparay = para[1];
    tryreparay.w -= p.y;
    float ty[3];
    int c2 = solve_univariate_cubic(ty, tryreparay);
    for(int i = 0; i < c2; i ++) {
        float t = ty[i];
        float x = para[0].dot(vec4(t * t * t, t * t, t, 1.f));
        float dis = abs(x - p.x);
        if(dis < min_dis) {
            s = t;
            min_dis = dis;
        }
    }
    return s;
}

bool is_concave_angle(const vec2& p1, const vec2& p2, const vec2& p3)
{
    vec2 v1, v2;
    v1.sub(p2, p1);
    v2.sub(p3, p2);
    return v1.ccw(v2) > 0;
}

bool is_concave_angle(const vec2& p1, const vec2& p2, const vec2& p3, bool cw)
{
    vec2 v1, v2;
    v1.sub(p2, p1);
    v2.sub(p3, p2);
    return cw ? v1.ccw(v2) > 0 : 
        v1.ccw(v2) < 0;
}

bool is_approx_line(const vec2& p1, const vec2& p2, const vec2& p3, float tolerance)
{
    float d = point_line_distance(p2, p1, p3);
    return abs(d) < tolerance;
}

void get_linear_coefficient(vec3& coef, const vec2& p, const vec2& d)
{
    assert(d.x != 0.f || d.y != 0.f);
    coef.x = d.y;
    coef.y = -d.x;
    coef.z = d.ccw(p);
}

static void get_linear_parameter_equation(vec2& para, float a, float b)
{
    para.x = b - a;
    para.y = a;
}

void get_linear_parameter_equation(vec2 para[2], const vec2& a, const vec2& b)
{
    get_linear_parameter_equation(para[0], a.x, b.x);
    get_linear_parameter_equation(para[1], a.y, b.y);
}

static void get_quad_parameter_equation(vec3& para, float a, float b, float c)
{
    para.x = a - 2.f * b + c;
    para.y = 2.f * (b - a);
    para.z = a;
}

void get_quad_parameter_equation(vec3 para[2], const vec2& a, const vec2& b, const vec2& c)
{
    get_quad_parameter_equation(para[0], a.x, b.x, c.x);
    get_quad_parameter_equation(para[1], a.y, b.y, c.y);
}

static void get_cubic_parameter_equation(vec4& para, float a, float b, float c, float d)
{
    para.x = d - 3.f * (c - b) - a;
    para.y = 3.f * (c - 2.f * b + a);
    para.z = 3.f * (b - a);
    para.w = a;
}

void get_cubic_parameter_equation(vec4 para[2], const vec2& a, const vec2& b, const vec2& c, const vec2& d)
{
    get_cubic_parameter_equation(para[0], a.x, b.x, c.x, d.x);
    get_cubic_parameter_equation(para[1], a.y, b.y, c.y, d.y);
}

static void get_first_derivate_coef(vec2& d, const vec3& coef)
{
    d.x = coef.x * 2.f;
    d.y = coef.y;
}

void get_first_derivate_factor(vec2 df1[2], const vec3 para[2])
{
    get_first_derivate_coef(df1[0], para[0]);
    get_first_derivate_coef(df1[1], para[1]);
}

static void get_first_derivate_coef(vec3& d, const vec4& coef)
{
    d.x = coef.x * 3.f;
    d.y = coef.y * 2.f;
    d.z = coef.z;
}

static void get_second_derivate_coef(vec2& d, const vec4& coef)
{
    d.x = 6.f * coef.x;
    d.y = 2.f * coef.y;
}

void get_first_derivate_factor(vec3 df1[2], const vec4 para[2])
{
    get_first_derivate_coef(df1[0], para[0]);
    get_first_derivate_coef(df1[1], para[1]);
}

void get_second_derivate_factor(vec2 df2[2], const vec4 para[2])
{
    get_second_derivate_coef(df2[0], para[0]);
    get_second_derivate_coef(df2[1], para[1]);
}

void get_first_derivate_factor(vec3 df1[2], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4)
{
    vec4 para[2];
    get_cubic_parameter_equation(para, p1, p2, p3, p4);
    get_first_derivate_factor(df1, para);
}

void get_second_derivate_factor(vec2 df2[2], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4)
{
    vec4 para[2];
    get_cubic_parameter_equation(para, p1, p2, p3, p4);
    get_second_derivate_factor(df2, para);
}

void get_first_derivate(vec2& d, const vec3 df1[2], float t)
{
    vec3 tv(t * t, t, 1.f);
    d.x = vec3dot(&tv, &df1[0]);
    d.y = vec3dot(&tv, &df1[1]);
}

void get_second_derivate(vec2& d, const vec2 df2[2], float t)
{
    vec2 tv(t, 1.f);
    d.x = vec2dot(&tv, &df2[0]);
    d.y = vec2dot(&tv, &df2[1]);
}

float get_cubic_curvature(const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4, float t)
{
    vec4 para[2];
    get_cubic_parameter_equation(para, p1, p2, p3, p4);
    vec3 fact1[2];
    vec2 fact2[2];
    get_first_derivate_factor(fact1, para);
    get_second_derivate_factor(fact2, para);
    vec2 v1, v2;
    get_first_derivate(v1, fact1, t);
    get_second_derivate(v2, fact2, t);
    float cross = vec2ccw(&v1, &v2);
    float dsq = vec2dot(&v1, &v1);
    float rc = cross / powf(dsq, 1.5f);
    return rc;
}

int solve_univariate_quadratic(float t[2], const vec3& coef)
{
    assert(!(fuzzy_zero(coef.x) && fuzzy_zero(coef.y)));
    if(fuzzy_zero(coef.x)) {
        t[0] = -coef.z / coef.y;
        return 1;
    }
    float delta = coef.y * coef.y - 4.f * coef.x * coef.z;
    if(delta < 0)
        return 0;
    else if(delta == 0) {
        t[0] = -coef.y / (2.f * coef.x);
        return 1;
    }
    else {
        float asq = sqrtf(delta / (4.f * coef.x * coef.x));
        float sb = coef.y / (2.f * coef.x);
        t[0] = -sb + asq;
        t[1] = -sb - asq;
        return 2;
    }
}

typedef std::complex<float> complexf;

static float cbrt(float d) { return powf(abs(d), 1.f / 3); }
static float cmod(const complexf& c) { return _hypotf(c.real(), c.imag()); }
static float ctheta(const complexf& c) { return atan2f(c.imag(), c.real()); }

static complexf csqrt(const complexf& c)
{
    float m = sqrtf(cmod(c));
    float t = ctheta(c) * 0.5f;
    return complexf(m * cosf(t), m * sinf(t));
}

static complexf ccbrt(const complexf& c)
{
    float m = cbrt(cmod(c));
    float t = ctheta(c) / 3.f;
    return complexf(m * cosf(t), m * sinf(t));
}

static int calc_real_roots(float t[], const complexf fc[], int c)
{
    int cnt = 0;
    for(int i = 0; i < c; i ++) {
        if(!isfinite(fc[i].real()) || !isfinite(fc[i].imag()))
            continue;
        if(fuzzy_zero(fc[i].imag()))
            t[cnt ++] = fc[i].real();
    }
    return cnt;
}

int solve_univariate_cubic(float t[3], const vec4& coef)
{
    assert(!(fuzzy_zero(coef.x) && fuzzy_zero(coef.y) && fuzzy_zero(coef.z)));
    if(fuzzy_zero(coef.x))
        return solve_univariate_quadratic(t, vec3(coef.y, coef.z, coef.w));
    complexf a(coef.x), b(coef.y), c(coef.z),d (coef.w);
    complexf P = 4.f * (b * b - 3.f * a * c);
    complexf Q = 4.f * (9.f * a * b * c - 27.f * a * a * d - 2.f * b * b * b);
    complexf D = csqrt(Q * Q - P * P * P);
    complexf u = ccbrt(Q + D);
    complexf v = ccbrt(Q - D);
    if(v.real() * v.real() + v.imag() * v.imag() > u.real() * u.real() + u.imag() * u.imag())
        u = v;
    if(fabsf(u.real()) > FLT_MIN || fabsf(u.imag()) > FLT_MIN)
        v = P / u;
    else
        u = v = 0.f;
    complexf o1(-0.5f, 0.8660254f);
    complexf o2(-0.5f, -0.8660254f);
    D = 1.f / 6.f / a;
    complexf cr[3];
    cr[0] = (u + v - 2.f * b) * D;
    cr[1] = (o1 * u + o2 * v - 2.f * b) * D;
    cr[2] = (o2 * u + o1 * v - 2.f * b) * D;
    return calc_real_roots(t, cr, 3);
}

int solve_univariate_quartic(float t[4], const float coef[5])
{
    if(fuzzy_zero(coef[0]))
        return solve_univariate_cubic(t, vec4(coef[1], coef[2], coef[3], coef[4]));
    complexf a(coef[0]), b(coef[1]), c(coef[2]), d(coef[3]), e(coef[4]);
    complexf P = (c * c + 12.f * a * e - 3.f * b * d) / 9.f;
    complexf Q = (27.f * a * d * d + 2.f * c * c * c + 27.f * b * b * e - 72.f * a * c * e - 9.f * b * c * d) / 54.f;
    complexf D = csqrt(Q * Q - P * P * P);
    complexf u = ccbrt(Q + D);
    complexf v = ccbrt(Q - D);
    if(v.real() * v.real() + v.imag() * v.imag() > u.real() * u.real() + u.imag() * u.imag())
        u = v;
    if(fabsf(u.real()) > FLT_MIN || fabsf(u.imag()) > FLT_MIN)
        v = P / u;
    else
        u = v = 0.f;
    complexf m;
    complexf S = b * b - (8.f / 3.f) * a * c;
    complexf T = 4.f * a;
    complexf o1(-0.5f, 0.8660254f);
    complexf o2(-0.5f, -0.8660254f);
    u *= T, v *= T;
    complexf r[3] = {
        u + v,
        o1 * u + o2 * v,
        o2 * u + o1 * v
    };
    float msq = 0.f, maxmsq = -1.f;
    int pos = -1;
    for(int i = 0; i < 3; ++ i) {
        T = S + r[i];
        msq = T.real() * T.real() + T.imag() * T.imag();
        if(maxmsq < msq) {
            maxmsq = msq;
            pos = i;
        }
    }
    if(maxmsq > FLT_MIN) {
        m = csqrt(S + r[pos]);
        S = 2.f * b * b - (16.f / 3.f) * a * c - r[pos];
        T = (8.f * a * b * c - 16.f * a * a * d - 2.f * b * b * b) / m;
    }
    else
        m = T = 0.f;
    complexf cr[4];
    v = 1.f / 4.f / a;
    u = csqrt(S - T);
    cr[0] = (-b - m + u) * v;
    cr[1] = (-b - m - u) * v;
    u = csqrt(S + T);
    cr[2] = (-b + m + u) * v;
    cr[3] = (-b + m - u) * v;
    return calc_real_roots(t, cr, 4);
}

int get_cubic_inflection(float t[2], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4)
{
    vec4 para[2];
    get_cubic_parameter_equation(para, p1, p2, p3, p4);
    vec3 ff[2];
    vec2 sf[2];
    get_first_derivate_factor(ff, para);
    get_second_derivate_factor(sf, para);
    return get_cubic_inflection(t, ff, sf);
}

/*
 * Assume that:
 * the first derivate of the cubic curve was,
 *   x:f(t)     { a, b, c }
 *   y:g(t)     { d, e, f }
 * the second derivate of the cubic curve was,
 *   x:u(t)     { g, h }
 *   y:v(t)     { i, j }
 * The formula to determine the inflection was:
 *   f(t)v(t) - g(t)u(t) = 0
 * Then we get:
 *   (ai - dg)t^3 + (bi + aj - eg - dh)t^2 + (ci + bj - fg -eh)t + (cj - fh) = 0
 * Notice that, the second derivate of cubic curve was always
     { 3a, b }
     { 3d, e }
 * so that the (ai - dg) was always 0.
 * eventually, we are going to solve:
 *   (bi + aj - eg - dh)t^2 + (ci + bj - fg -eh)t + (cj - fh) = 0
 */
int get_cubic_inflection(float t[2], const vec3 ff[2], const vec2 sf[2])
{
    const float &a = ff[0].x, &b = ff[0].y, &c = ff[0].z;
    const float &d = ff[1].x, &e = ff[1].y, &f = ff[1].z;
    const float &g = sf[0].x, &h = sf[0].y;
    const float &i = sf[1].x, &j = sf[1].y;
    vec3 coef(
        b * i + a * j - e * g - d * h,
        c * i + b * j - f * g - e * h,
        c * j - f * h
        );
    if(fuzzy_zero(coef.x))
        return 0;
    int cap = solve_univariate_quadratic(t, coef);
    int cnt = 0;
    /* filter the result within scope 0 - 1 */
    const float tol = 1e-4f;
    for(int i = 0; i < cap; i ++) {
        float v = t[i];
        if(v >= tol && v <= 1.f - tol)
            t[cnt ++] = v;
    }
    return cnt;
}

int get_quad_extrema(float t[], const vec2& ff)
{
    if(fuzzy_zero(ff.x, 1e-5f))
        return 0;
    assert(t);
    *t = -ff.y / ff.x;
    return (*t >= 0.f && *t <= 1.f) ? 1 : 0;
}

int get_cubic_extrema(float t[2], const vec3& ff)
{
    if(fuzzy_zero(ff.x, 1e-5f))
        return get_quad_extrema(t, vec2(ff.y, ff.z));
    float s[2];
    int c = solve_univariate_quadratic(s, ff);
    int r = 0;
    for(int i = 0; i < c; i ++) {
        if(s[i] >= 0.f && s[i] <= 1.f)
            t[r ++] = s[i];
    }
    return r;
}

static void get_quad_bound_box(rectf& rc, const vec2& p1, const vec2& p2, const vec2& p3, const vec3 para[2])
{
    rc.set_by_pts(p1, p3);
    vec2 ff[2];
    get_first_derivate_factor(ff, para);
    float tx, ty;
    bool has_x_extrema = (get_quad_extrema(&tx, ff[0]) != 0);
    bool has_y_extrema = (get_quad_extrema(&ty, ff[1]) != 0);
    if(has_x_extrema) {
        float x = para[0].dot(vec3(tx * tx, tx, 1.f));
        rc.left = gs_min(rc.left, x);
        rc.right = gs_max(rc.right, x);
    }
    if(has_y_extrema) {
        float y = para[1].dot(vec3(ty * ty, ty, 1.f));
        rc.top = gs_min(rc.top, y);
        rc.bottom = gs_max(rc.bottom, y);
    }
}

void get_quad_bound_box(rectf& rc, const vec2& p1, const vec2& p2, const vec2& p3)
{
    vec3 para[2];
    get_quad_parameter_equation(para, p1, p2, p3);
    get_quad_bound_box(rc, p1, p2, p3, para);
}

static void get_cubic_bound_box(rectf& rc, const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4, const vec4 para[2])
{
    rc.set_by_pts(p1, p4);
    vec3 ff[2];
    get_first_derivate_factor(ff, para);
    float tx[2], ty[2];
    int cx = get_cubic_extrema(tx, ff[0]);
    int cy = get_cubic_extrema(ty, ff[1]);
    for(int i = 0; i < cx; i ++) {
        float x = para[0].dot(vec4(tx[i] * tx[i] * tx[i], tx[i] * tx[i], tx[i], 1.f));
        rc.left = gs_min(rc.left, x);
        rc.right = gs_max(rc.right, x);
    }
    for(int i = 0; i < cy; i ++) {
        float y = para[1].dot(vec4(ty[i] * ty[i] * ty[i], ty[i] * ty[i], ty[i], 1.f));
        rc.top = gs_min(rc.top, y);
        rc.bottom = gs_max(rc.bottom, y);
    }
}

void get_cubic_bound_box(rectf& rc, const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4)
{
    vec4 para[2];
    get_cubic_parameter_equation(para, p1, p2, p3, p4);
    get_cubic_bound_box(rc, p1, p2, p3, p4, para);
}

void intersectp_linear_linear(vec2& ip, const vec2& p1, const vec2& p2, const vec2& d1, const vec2& d2)
{
    assert(!is_parallel(d1, d2) && "won't be parallel");
    float dxy = d1.x * d2.y, dyx = d2.x * d1.y;
    ip.y = (d1.y * d2.y * (p2.x - p1.x) + dxy * p1.y - dyx * p2.y) / (dxy - dyx);
    ip.x = (abs(d1.y) > abs(d2.y)) ? ((ip.y - p1.y) * d1.x / d1.y + p1.x) :
        ((ip.y - p2.y) * d2.x / d2.y + p2.x);
}

int intersection_quad_linear(float t[2], const vec3 quad[2], const vec3& linear)
{
    int cnt = 0, cap = 0;
    vec3 cf, cf1, cf2;
    vec3scale(&cf1, &quad[0], linear.x);
    vec3scale(&cf2, &quad[1], linear.y);
    vec3add(&cf, &cf1, &cf2);
    cf.z += linear.z;
    cap = solve_univariate_quadratic(t, cf);
    for(int i = 0; i < cap; i ++) {
        if(t[i] > -0.001f && t[i] < 1.001f)
            t[cnt ++] = t[i];
    }
    return cnt;
}

int intersection_quad_quad(float ts[4][2], const vec3 quad1[2], const vec3 quad2[2])
{
    const float a = quad1[0].x, b = quad1[0].y, c = quad1[0].z,
        d = quad1[1].x, e = quad1[1].y, f = quad1[1].z,
        g = quad2[0].x, h = quad2[0].y, i = quad2[0].z,
        j = quad2[1].x, k = quad2[1].y, l = quad2[1].z;
    /* try to choose a non-zero coef */
    const vec3* tcoef = quad1;
    const vec3* scoef = quad2;
    if(fuzzy_zero(tcoef->x))
        ++ tcoef, ++ scoef;
    float tc = d * b - a * e;
    if(fuzzy_zero(tc)) {
        float s[2];
        vec3 coef;
        coef.x = d * g - a * j;
        coef.y = d * h - a * k;
        coef.z = d * i - a * l + a * f - d * c;
        int n = solve_univariate_quadratic(s, coef);
        int cnt = 0;
        for(int m = 0; m < n; m ++) {
            if(!fuzzy_between(0.f, s[m], 1.f, 1e-4f))
                continue;
            float spart = vec3(s[m] * s[m], s[m], 1.f).dot(*scoef);
            vec3 solvet(tcoef->x, tcoef->y, tcoef->z - spart);
            float t[2];
            int ct = solve_univariate_quadratic(t, solvet);
            for(int o = 0; o < ct; o ++) {
                if(fuzzy_between(0.f, t[o], 1.f, 1e-5f)) {
                    ts[cnt][0] = t[o];
                    ts[cnt][1] = s[m];
                    ++ cnt;
                    break;
                }
            }
        }
        return cnt;
    }
    /* here comes the non-zero part */
    /* t = us^2 + vs + w */
    const float m = tcoef->x, n = tcoef->y, o = tcoef->z,
        p = scoef->x, q = scoef->y, r = scoef->z;
    float u = (d * g - a * j) / tc, v = (d * h - a * k) / tc, w = (d * i - a * l + a * f - d * c) / tc;
    /* mt^2 + nt + o = ps^2 + qs + r */
    float quartic[5] = { u * u * m, 2.f * u * v * m, (2.f * u * w + v * v) * m, 2.f * v * w * m, w * w * m };
    quartic[2] += n * u - p;
    quartic[3] += n * v - q;
    quartic[4] += n * w + o - r;
    float s[4];
    int cnt = 0;
    int qc = solve_univariate_quartic(s, quartic);
    for(int id = 0; id < qc; id ++) {
        if(!fuzzy_between(0.f, s[id], 1.f, 1e-4f))
            continue;
        float t = vec3(u, v, w).dot(vec3(s[id] * s[id], s[id], 1.f));
        if(fuzzy_between(0.f, t, 1.f, 1e-5f)) {
            ts[cnt][0] = t;
            ts[cnt][1] = s[id];
            ++ cnt;
        }
    }
    return cnt;
}

int intersection_cubic_linear(float t[3], const vec4 cubic[2], const vec3& linear)
{
    int cnt = 0, cap = 0;
    float tt[3];
    vec4 cf, cf1, cf2;
    cf1.scale(cubic[0], linear.x);
    cf2.scale(cubic[1], linear.y);
    cf2.w += linear.z;
    cf.add(cf1, cf2);
    cap = solve_univariate_cubic(tt, cf);
    for(int i = 0; i < cap; i ++) {
        if(tt[i] < 0.f && tt[i] > -0.001f) tt[i] = 0.f;
        else if(tt[i] > 1.f && tt[i] < 1.001f) tt[i] = 1.f;
        if(tt[i] >= 0.f && tt[i] <= 1.f)
            t[cnt ++] = tt[i];
    }
    return cnt;
}

static void trace_cubic_chain(const vec2 p[10], int c)
{
    if(c >= 4) {
        trace(_t("@moveTo %f, %f;\n"), p[0].x, p[0].y);
        trace(_t("@cubicTo %f, %f, %f, %f, %f, %f;\n"), p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);
        if(c >= 7) {
            trace(_t("@moveTo %f, %f;\n"), p[3].x, p[3].y);
            trace(_t("@cubicTo %f, %f, %f, %f, %f, %f;\n"), p[4].x, p[4].y, p[5].x, p[5].y, p[6].x, p[6].y);
            if(c >= 10) {
                trace(_t("@moveTo %f, %f;\n"), p[6].x, p[6].y);
                trace(_t("@cubicTo %f, %f, %f, %f, %f, %f;\n"), p[7].x, p[7].y, p[8].x, p[8].y, p[9].x, p[9].y);
            }
        }
    }
}

int intersection_cubic_quad(float t[6], const vec2 cp1[4], const vec2 cp2[3], float tolerance)
{
    vector<vec2> quads;
    int qc = cubic_to_quad_bezier(quads, cp1, tolerance);
    vec3 para2[2];
    get_quad_parameter_equation(para2, cp2[0], cp2[1], cp2[2]);
    rectf rc2;
    get_quad_bound_box(rc2, cp2[0], cp2[1], cp2[2], para2);
    vector<float> vt;
    for(int i = 2; i < qc; i += 2) {
        vec3 para1[2];
        get_quad_parameter_equation(para1, quads.at(i - 2), quads.at(i - 1), quads.at(i));
        rectf rc1;
        get_quad_bound_box(rc1, quads.at(i - 2), quads.at(i - 1), quads.at(i), para1);
        if(is_rect_intersected(rc1, rc2)) {
            float ts[4][2];
            int c = intersection_quad_quad(ts, para1, para2);
            for(int j = 0; j < c; j ++)
                vt.push_back(ts[j][1]);
        }
    }
    if(vt.empty())
        return 0;
    std::sort(vt.begin(), vt.end(), [](float t1, float t2)-> bool { return t1 < t2; });
    int c = 1;
    t[0] = vt.front();
    for(size_t i = 1; i < vt.size() && c < 6; i ++) {
        if(!fuzzy_zero(t[c - 1] - vt.at(i)))
            t[c ++] = vt.at(i);
    }
    return c;
}

int intersectp_cubic_cubic(vec2 ip[9], const vec2 cp1[4], const vec2 cp2[4], float tolerance)
{
    vector<vec2> quad1, quad2;
    int qc1 = cubic_to_quad_bezier(quad1, cp1, tolerance);
    int qc2 = cubic_to_quad_bezier(quad2, cp2, tolerance);
    vector<vec2> pts;
    for(int i = 2; i < qc1; i += 2) {
        vec3 para1[2];
        get_quad_parameter_equation(para1, quad1.at(i - 2), quad1.at(i - 1), quad1.at(i));
        rectf rc1;
        get_quad_bound_box(rc1, quad1.at(i - 2), quad1.at(i - 1), quad1.at(i), para1);
        for(int j = 2; j < qc2; j += 2) {
            vec3 para2[2];
            get_quad_parameter_equation(para2, quad2.at(j - 2), quad2.at(j - 1), quad2.at(j));
            rectf rc2;
            get_quad_bound_box(rc2, quad2.at(j - 2), quad2.at(j - 1), quad2.at(j), para2);
            if(is_rect_intersected(rc1, rc2)) {
                float ts[4][2];
                int c = intersection_quad_quad(ts, para1, para2);
                for(int k = 0; k < c; k ++) {
                    vec2 p;
                    eval_quad(p, para1, ts[k][0]);
                    pts.push_back(p);
                }
            }
        }
    }
    int c = 0;
    for(const vec2& p : pts) {
        if(c >= 9)
            break;
        bool has_same_pt = false;
        for(int i = 0; i < c; i ++) {
            if(fuzz_cmp(ip[i], p) < tolerance) {
                has_same_pt = true;
                break;
            }
        }
        if(!has_same_pt)
            ip[c ++] = p;
    }
    return c;
}

bool get_self_intersection(float ts[2], const vec2& a, const vec2& b, const vec2& c, const vec2& d)
{
    /*
     * u = s + t
     * v = s^2 + st + t^2
     * solve u, v first, IE.
     * 3(b - a) + 3(a - 2b + c)u + (-a + 3(b - c) + d)v = 0
     */
    auto build_equation = [](float a, float b, float c, float d)-> vec3 {
        return vec3(
            3.f * (a - 2.f * b + c),
            -(a - 3.f * b + 3.f * c - d),
            3.f * (a - b)
            );
    };
    vec3 eq1 = build_equation(a.x, b.x, c.x, d.x);
    vec3 eq2 = build_equation(a.y, b.y, c.y, d.y);
    float detc = eq1.x * eq2.y - eq1.y * eq2.x;
    if(fabsf(detc) < FLT_EPSILON)
        return false;
    float detx = eq1.z * eq2.y - eq1.y * eq2.z;
    float dety = eq1.x * eq2.z - eq1.z * eq2.x;
    float u = detx / detc;
    float v = dety / detc;
    /*
     * now we solve the [u, v, s, t] puzzle
     * t = u - s
     * u^2 - us + s^2 = 0   => retrieve s
     */
    vec3 coef(1, -u, u * u - v);
    float tt[2];
    int count = solve_univariate_quadratic(tt, coef);
    if (count != 2)
        return false;
    else
    {
        if (tt[0] >= 0.f && tt[0] <= 1.f && tt[1] >= 0.f && tt[1] <= 1.f)
        {
            ts[0] = gs_min(tt[0], tt[1]);
            ts[1] = gs_max(tt[0], tt[1]);
            return true;
        }
        return false;
    }
}

void split_quad_bezier(vec2 c[5], const vec2 p[3], float t)
{
    assert(t > 0 && t < 1.f);
    c[0] = p[0];
    c[4] = p[2];
    vec2lerp(&c[1], &p[0], &p[1], t);
    vec2lerp(&c[3], &p[1], &p[2], t);
    vec2lerp(&c[2], &c[1], &c[3], t);
}

void split_quad_bezier(vec2 c[7], const vec2 p[3], float t1, float t2)
{
    assert(t1 < t2);
    vec3 para[2];
    get_quad_parameter_equation(para, p[0], p[1], p[2]);
    vec2 p2;
    eval_quad(p2, para, t2);
    split_quad_bezier(c, p, t1);
    get_quad_parameter_equation(para, c[2], c[3], c[4]);
    float s = quad_reparameterize(para, p2);
    vec2 cp[3];
    memmove_s(cp, sizeof(cp), c + 2, sizeof(cp));
    split_quad_bezier(c + 2, cp, s);
}

void split_cubic_bezier(vec2 c[7], const vec2 p[4], float t)
{
    assert(t > 0 && t < 1.f);
    c[0] = p[0];
    c[6] = p[3];
    vec2 lv2t;
    vec2lerp(&c[1], &p[0], &p[1], t);
    vec2lerp(&lv2t, &p[1], &p[2], t);
    vec2lerp(&c[5], &p[2], &p[3], t);
    vec2lerp(&c[2], &c[1], &lv2t, t);
    vec2lerp(&c[4], &lv2t, &c[5], t);
    vec2lerp(&c[3], &c[2], &c[4], t);
}

/*
 * Tri-division of a cubic bezier
 * be caution that you can't make it like this:
 * divide by t1, then divide by t2 * (1 - t1), this was so wrong.
 * There wasn't a convenient way to transform the t-value while dividing.
 * So we decided to:
 * 1.get p2 of t2
 * 2.divide by t1 and got curve1, curve2
 * 3.get s of p2 in curve2
 * 4.divide curve2 by s
 */
void split_cubic_bezier(vec2 c[10], const vec2 p[4], float t1, float t2)
{
    assert(t1 < t2);
    /* get p2 */
    vec2 p2, d1, d2;
    vec2sub(&d1, &p[1], &p[0]);
    vec2sub(&d2, &p[3], &p[2]);
    vec2scale(&d1, &d1, 3.f);
    vec2scale(&d2, &d2, 3.f);
    vec2hermite(&p2, &p[0], &d1, &p[3], &d2, t2);
    /* divide t1 */
    split_cubic_bezier(c, p, t1);
    /* get s of p2 */
    vec4 para[2];
    get_cubic_parameter_equation(para, c[3], c[4], c[5], c[6]);
    float s = cubic_reparameterize(para, p2);
    /* divide s */
    vec2 cp[4];
    memmove_s(cp, sizeof(cp), c + 3, sizeof(cp));
    split_cubic_bezier(c + 3, cp, s);
}

void offset_cubic_bezier(vec2 c[4], const vec2 p[4], float d)
{
    vec2 d1, d2, d3;
    vec2sub(&d1, &p[1], &p[0]);
    vec2sub(&d2, &p[2], &p[1]);
    vec2sub(&d3, &p[3], &p[2]);
    vec2normalize(&d1, &d1);
    vec2normalize(&d2, &d2);
    vec2normalize(&d3, &d3);
    vec2 v1, v2;
    vec2sub(&v1, &d1, &d2);
    vec2sub(&v2, &d3, &d2);
    vec2 t1(d1.y, -d1.x), t2(d3.y, -d3.x);
    if(fuzz_cmp(v1, vec2(0, 0)) < 0.1f) { v1 = t1; }
    if(fuzz_cmp(v2, vec2(0, 0)) < 0.1f) { v2 = t2; }
    vec2scale(&t1, &t1, d);
    vec2scale(&t2, &t2, d);
    vec2add(&c[0], &p[0], &t1);
    vec2add(&c[3], &p[3], &t2);
    intersectp_linear_linear(c[1], c[0], p[1], d1, v1);
    intersectp_linear_linear(c[2], c[3], p[2], d3, v2);
}

float quad_bezier_length(const vec2 cp[3])
{
    if(abs(calc_triangle_area(cp[0], cp[1], cp[2])) < 1e-6f) {
        vec2 d;
        d.sub(cp[2], cp[0]);
        return d.length();
    }
    vec2 t1, t2;
    t1.add(cp[0], cp[2]).sub(t1, vec2().scale(cp[1], 2.f));
    t2.sub(cp[1], cp[0]).scale(2.f);
    float a = t1.lengthsq() * 4.f;
    if(fuzzy_zero(a))
        return quad_control_length(cp[0], cp[1], cp[2]);
    float b = t1.dot(t2) * 4.f;
    float c = t2.lengthsq();
    float ssabc = sqrtf(a + b + c);
    float r = (2.f * sqrtf(a) * (2.f * a * ssabc + b * (ssabc - sqrtf(c))) + (b * b - 4.f * a * c) * (logf(b + 2.f * sqrtf(a * c)) -  logf(b + 2.f * a + 2.f * sqrtf(a) * ssabc))) / (8.f * powf(a, 1.5f));
    if(isnan(r) || isinf(r))
        return quad_control_length(cp[0], cp[1], cp[2]);
    return r;
}

float cubic_bezier_length(const vec2 cp[4], float tolerance)
{
    vector<vec2> quadctls;
    cubic_to_quad_bezier(quadctls, cp, tolerance);
    int size = (int)quadctls.size();
    if(size <= 0 || (size - 1) % 2 != 0) {
        assert(!"convert cubic to quad bezier failed.");
        return 0.f;
    }
    float len = 0.f;
    for(int i = 0; (i + 2) < size; i += 2)
        len += quad_bezier_length(&quadctls.at(i));
    return len;
}

static bool cubic_to_quad_bezier_dcvt(vector<vec2>& quadctls, const vec2 cp[4], float tolerance)
{
    float ctlen = cubic_control_length(cp[0], cp[1], cp[2], cp[3]);
    if(ctlen < tolerance) {
        vec2 center;
        center.add(cp[0], cp[3]).scale(0.5f);
        quadctls.push_back(center);
        quadctls.push_back(cp[3]);
        return true;
    }
    return false;
}

int cubic_to_quad_bezier_non_inflection(vector<vec2>& quadctls, const vec2 cp[4], float tolerance);

static int cubic_to_quad_bezier_rcvt(vector<vec2>& quadctls, const vec2 cp[4], float tolerance)
{
    vec2 ip;
    get_reduce_point(ip, cp[0], cp[1], cp[2], cp[3]);
    vec4 cpara[2];
    get_cubic_parameter_equation(cpara, cp[0], cp[1], cp[2], cp[3]);
    if(fuzzy_zero(cpara[0].x) && fuzzy_zero(cpara[0].y) && fuzzy_zero(cpara[0].z) ||
        fuzzy_zero(cpara[1].x) && fuzzy_zero(cpara[1].y) && fuzzy_zero(cpara[1].z)) {
        quadctls.push_back(ip);
        quadctls.push_back(cp[3]);
        return (int)quadctls.size();
    }
    vec3 qpara[2];
    get_quad_parameter_equation(qpara, cp[0], ip, cp[3]);
    auto close_test = [](const vec4 cpara[2], const vec3 qpara[2], float t, float tol)-> bool {
        vec2 v1, v2;
        eval_quad(v1, qpara, t);
        float s = best_cubic_reparameterize(cpara, v1);
        if(s < 0.f || s > 1.f)
            return false;
        eval_cubic(v2, cpara, s);
        float d = vec2().sub(v1, v2).length();
        return d < tol;
    };
    if(close_test(cpara, qpara, 0.25f, tolerance) && close_test(cpara, qpara, 0.75f, tolerance)) {
        quadctls.push_back(ip);
        quadctls.push_back(cp[3]);
        return (int)quadctls.size();
    }
    vec2 dcp[7];
    split_cubic_bezier(dcp, cp, 0.5f);
    cubic_to_quad_bezier_non_inflection(quadctls, dcp, tolerance);
    return cubic_to_quad_bezier_non_inflection(quadctls, dcp + 3, tolerance);
}

static int cubic_to_quad_bezier_non_inflection(vector<vec2>& quadctls, const vec2 cp[4], float tolerance)
{
    if(cubic_to_quad_bezier_dcvt(quadctls, cp, tolerance))
        return (int)quadctls.size();
    return cubic_to_quad_bezier_rcvt(quadctls, cp, tolerance);
}

static int cubic_to_quad_bezier_iteration(vector<vec2>& quadctls, const vec2 cp[4], float tolerance)
{
    if(cubic_to_quad_bezier_dcvt(quadctls, cp, tolerance))
        return (int)quadctls.size();
    float t[2];
    int i = get_cubic_inflection(t, cp[0], cp[1], cp[2], cp[3]);
    assert(i <= 2);
    switch(i)
    {
    case 1:
        {
            vec2 dcp[7];
            split_cubic_bezier(dcp, cp, t[0]);
            cubic_to_quad_bezier_non_inflection(quadctls, dcp, tolerance);
            return cubic_to_quad_bezier_non_inflection(quadctls, dcp + 3, tolerance);
        }
    case 2:
        {
            vec2 dcp[10];
            if(t[0] > t[1])
                gs_swap(t[0], t[1]);
            split_cubic_bezier(dcp, cp, t[0], t[1]);
            cubic_to_quad_bezier_non_inflection(quadctls, dcp, tolerance);
            cubic_to_quad_bezier_non_inflection(quadctls, dcp + 3, tolerance);
            return cubic_to_quad_bezier_non_inflection(quadctls, dcp + 6, tolerance);
        }
    }
    return cubic_to_quad_bezier_rcvt(quadctls, cp, tolerance);
}

int cubic_to_quad_bezier(vector<vec2>& quadctl, const vec2 cp[4], float tolerance)
{
    assert(quadctl.empty());
    quadctl.push_back(cp[0]);
    return cubic_to_quad_bezier_iteration(quadctl, cp, tolerance);
}

float quad_control_length(const vec2& a, const vec2& b, const vec2& c)
{
    vec2 d1, d2;
    d1.sub(b, a);
    d2.sub(c, b);
    return d1.length() + d2.length();
}

float cubic_control_length(const vec2& a, const vec2& b, const vec2& c, const vec2& d)
{
    vec2 d1, d2, d3;
    d1.sub(b, a);
    d2.sub(c, b);
    d3.sub(d, c);
    return d1.length() + d2.length() + d3.length();
}

float point_line_distance(const vec2& p, const vec2& p1, const vec2& p2)
{
    vec3 coef;
    get_linear_coefficient(coef, p1, vec2().sub(p2, p1));
    float d = coef.dot(vec3(p.x, p.y, 1.f)) / vec2(coef.x, coef.y).length();
    return d;
}

int get_interpolate_step(const vec2& a, const vec2& b, const vec2& c, float step_len)
{
    if(step_len <= 0.f)
        step_len = __plot_step_len;
    vec2 pt[] = { a, b, c };
    float len = quad_bezier_length(pt);
    int step = (int)ceilf(len / step_len) + 1;
    return step < 2 ? 2 : step;
}

int get_interpolate_step(const vec2& a, const vec2& b, const vec2& c, const vec2& d, float step_len)
{
    if(step_len <= 0.f)
        step_len = __plot_step_len;
    vec2 pt[] = { a, b, c, d };
    float len = cubic_bezier_length(pt, __plot_tol);
    int step = (int)ceilf(len / step_len) + 1;
    return step < 2 ? 2 : step;
}

int get_rough_interpolate_step(const vec2& a, const vec2& b, const vec2& c)
{
    int step = round(quad_control_length(a, b, c));
    step /= 29;
    if(step >= 3)
        return step;
    float angle = get_include_angle(a, b, c);
    return angle > (PI / 3.f) ? 2 : 3;
}

int get_rough_interpolate_step(const vec2& a, const vec2& b, const vec2& c, const vec2& d)
{
    int step = round(cubic_control_length(a, b, c, d));
    step >>= 5;
    if(step >= 3)
        return step;
    vec2 p, d1, d2;
    d1.sub(d, a);
    d2.sub(c, b);
    if(!is_parallel(d1, d2)) {
        intersectp_linear_linear(p, a, b, d1, d2);
        float t = linear_reparameterize(a, d, p);
        float s = linear_reparameterize(b, c, p);
        if(t >= 0.f && t <= 1.f && s >= 0.f && s <= 1.f)
            return 2;
    }
    get_reduce_point(p, a, b, c, d);
    float angle = get_include_angle(a, p, d);
    return angle > (PI / 3.f) ? 2 : 3;
}

float get_include_angle(const vec2& d1, const vec2& d2)
{
    float dp = d1.dot(d2);
    float ll = d1.length() * d2.length();
    assert(ll != 0.f);
    return acosf(dp / ll);
}

float get_include_angle(const vec2& a, const vec2& b, const vec2& c)
{
    vec2 v1, v2;
    v1.sub(a, b);
    v2.sub(c, b);
    return get_include_angle(v1, v2);
}

float get_rotate_angle(const vec2& d1, const vec2& d2)
{
    const float epsilon = 1.0e-6f;
    vec2 v1, v2;
    v1.normalize(d1);
    v2.normalize(d2);
    float angle, dot;
    dot = v1.dot(v2);
    if(abs(dot - 1.f) <= epsilon)
        angle = 0.f;
    else if(abs(dot + 1.f) <= epsilon)
        angle = PI;
    else {
        angle = acosf(dot);
        float cross = v1.ccw(v2);
        if(cross < 0.f)
            angle = PI * 2 - angle;
    }
    return angle;
}

float get_rotate_angle(const vec2& a, const vec2& b, const vec2& c)
{
    vec2 v1, v2;
    v1.sub(a, b);
    v2.sub(c, b);
    return get_rotate_angle(v1, v2);
}

static float reduce_cubic_to_quadratic(float a, float b, float c, float d)
{
    float t = 3.f * (b + c) - a - d;
    return t * 0.25f;
}

void get_reduce_point(vec2& p, const vec2& a, const vec2& b, const vec2& c, const vec2& d)
{
    p.x = reduce_cubic_to_quadratic(a.x, b.x, c.x, d.x);
    p.y = reduce_cubic_to_quadratic(a.y, b.y, c.y, d.y);
}

/*
 *     |-1   0   0 |
 * O = | 0  -1   0 |
 *     | 0   0   1 |
 * M = IM * O
 */
static void reverse_cubic_klm_coords(vec3 m[4], const vec3 im[4])
{
    assert(m && im);
    m[0].x = -im[0].x;
    m[1].x = -im[1].x;
    m[2].x = -im[2].x;
    m[3].x = -im[3].x;
    m[0].y = -im[0].y;
    m[1].y = -im[1].y;
    m[2].y = -im[2].y;
    m[3].y = -im[3].y;
    if(m != im) {
        m[0].z = im[0].z;
        m[1].z = im[1].z;
        m[2].z = im[2].z;
        m[3].z = im[3].z;
    }
}

/*
 * According to the papers of Blinn2003, there comes out the way of figuring out the implicit equation of
 * any form of bezier curves.
 * There was a mistake in the book GPU Gems 3, as well in the nVidia website too:
 * http://http.developer.nvidia.com/GPUGems3/gpugems3_ch25.html
 *  a3 = b2 ¡¤ (b1 x b1).
 * It's actually to be a3 = b2 ¡¤ (b1 x b0).
 * But the paper of loopblinn05 seems to be correct.
 * http://research.microsoft.com/en-us/um/people/cloop/loopblinn05.pdf
 * Here was an implementation about this algorithm.
 */
enum cubic_klm_type
{
    ckt_serpentine,
    ckt_cusp,
    ckt_loop,
    ckt_quadratic,
    ckt_line,
};

struct cubic_klm_classifier
{
    cubic_klm_type type;
    vec2    cp[4];
    float   d1, d2, d3;
    float   sp;             /* -1.f means no need to split */

public:
    cubic_klm_classifier(const vec2& c0, const vec2& c1, const vec2& c2, const vec2& c3,
        const vec2& ac1, const vec2& ac2, const vec2& ac3, const vec2& ac4  /* for error control */
        )
    {
        sp = -1.f;
        cp[0] = ac1, cp[1] = ac2, cp[2] = ac3, cp[3] = ac4;
        vec3 b0(c0.x, c0.y, 1.f),
            b1(c1.x, c1.y, 1.f),
            b2(c2.x, c2.y, 1.f),
            b3(c3.x, c3.y, 1.f);
        float a1 = b0.dot(vec3().cross(b3, b2));
        float a2 = b1.dot(vec3().cross(b0, b3));
        float a3 = b2.dot(vec3().cross(b1, b0));
        float m1 = a1 - 2.f * a2 + 3.f * a3;
        float m2 = -a2 + 3.f * a3;
        float m3 = 3.f * a3;
        vec3 v(m1, m2, m3);
        v.normalize();
        m1 = v.x;
        m2 = v.y;
        m3 = v.z;
        float sbst = 3.f * m2 * m2 - 4.f * m1 * m3;
        float discr = m1 * m1 * sbst;
        d1 = fuzzy_zero(m1) ? 0.f : m1;
        d2 = fuzzy_zero(m2) ? 0.f : m2;
        d3 = fuzzy_zero(m3) ? 0.f : m3;
        discr = fuzzy_zero(discr) ? 0.f : discr;
        if(discr == 0.f) {
            if(d1 == 0.f && d2 == 0.f)
                type = (d3 == 0.f) ? ckt_line : ckt_quadratic;
            else if(d1 == 0.f)
                type = ckt_cusp;
            else if(sbst < 0.f)
                type = ckt_loop;
            else
                type = ckt_serpentine;
        }
        else if(discr > 0.f)
            type = ckt_serpentine;
        else
            type = ckt_loop;
    }
    void get_coords(vec3 m[4])
    {
        switch(type)
        {
        case ckt_serpentine:
            return get_coords_serpentine(m);
        case ckt_cusp:
            return get_coords_cusp(m);
        case ckt_loop:
            return get_coords_loop(m);
        case ckt_quadratic:
            return get_coords_quadratic(m);
        case ckt_line:
            return get_coords_line(m);
        }
    }
    void get_coords_serpentine(vec3 m[4])
    {
        float sq = sqrtf(9.f * d2 * d2 - 12.f * d1 * d3);
        float ls = 3.f * d2 - sq;
        float lt = 6.f * d1;
        float ms = 3.f * d2 + sq;
        float mt = lt;
        float ltmls = lt - ls;
        float mtmms = mt - ms;
        m[0].x = ls * ms;
        m[0].y = ls * ls * ls;
        m[0].z = ms * ms * ms;
        m[1].x = 0.33333333f * (3.f * ls * ms - ls * mt - lt * ms);
        m[1].y = -ls * ls * ltmls;
        m[1].z = -ms * ms * mtmms;
        m[2].x = 0.33333333f * (lt * (mt - 2.f * ms) + ls * (3.f * ms - 2.f * mt));
        m[2].y = ltmls * ltmls * ls;
        m[2].z = mtmms * mtmms * ms;
        m[3].x = ltmls * mtmms;
        m[3].y = -ltmls * ltmls * ltmls;
        m[3].z = -mtmms * mtmms * mtmms;
        if(d1 < 0.f)
            reverse_cubic_klm_coords(m, m);
    }
    void get_coords_cusp(vec3 m[4])
    {
        float ls = d3;
        float lt = 3.f * d2;
        float lsmlt = ls - lt;
        m[0].x = ls;
        m[0].y = ls * ls * ls;
        m[0].z = 1.f;
        m[1].x = ls - 0.33333333f * lt;
        m[1].y = ls * ls * lsmlt;
        m[1].z = 1.f;
        m[2].x = ls - 0.66666667f * lt;
        m[2].y = lsmlt * lsmlt * ls;
        m[2].z = 1.f;
        m[3].x = lsmlt;
        m[3].y = lsmlt * lsmlt * lsmlt;
        m[3].z = 1.f;
    }
    void get_coords_loop(vec3 m[4])
    {
        float sq = sqrtf(4.f * d1 * d3 - 3.f * d2 * d2);
        float ls = d2 - sq;
        float lt = 2.f * d1;
        float ms = d2 + sq;
        float mt = lt;
        float ql = ls / lt;
        float qm = ms / mt;
        float sp1 = -1.f;
        const float tol = 0.00015f;
        if(ql > tol && ql < (1.f - tol))
            sp1 = ql;
        else if(qm > tol && qm < (1.f - tol))
            sp1 = qm;
        if(sp1 != -1.f) {
            vec2 ip, t1, t2;
            t1.sub(cp[1], cp[0]).scale(3.f);
            t2.sub(cp[3], cp[2]).scale(3.f);
            ip.hermite(cp[0], t1, cp[3], t2, sp1);
            float d1 = vec2().sub(ip, cp[0]).length();
            float d2 = vec2().sub(ip, cp[3]).length();
            if(d1 > 3.f && d2 > 3.f) {
                sp = sp1;
                return;
            }
        }
        float ltmls = lt - ls;
        float mtmms = mt - ms;
        m[0].x = ls * ms;
        m[0].y = ls * ls * ms;
        m[0].z = ls * ms * ms;
        m[1].x = 0.33333333f * (-ls * mt - lt * ms + 3.f * ls * ms);
        m[1].y = -0.33333333f * ls * (ls * (mt - 3.f * ms) + 2.f * lt * ms);
        m[1].z = -0.33333333f * ms * (ls * (2.f * mt - 3.f * ms) + lt * ms);
        m[2].x = 0.33333333f * (lt * (mt - 2.f * ms) + ls * (3.f * ms - 2.f * mt));
        m[2].y = 0.33333333f * ltmls * (ls * (2.f * mt - 3.f * ms) + lt * ms);
        m[2].z = 0.33333333f * mtmms * (ls * (mt - 3.f * ms) + 2.f * lt * ms);
        m[3].x = ltmls * mtmms;
        m[3].y = -ltmls * ltmls * mtmms;
        m[3].z = -ltmls * mtmms * mtmms;
        if((d1 > 0.f && m[0].x < 0.f) || (d1 < 0.f && m[0].x > 0.f))
            reverse_cubic_klm_coords(m, m);
    }
    void get_coords_quadratic(vec3 m[4])
    {
        m[0].x = 0.f;
        m[0].y = 0.f;
        m[0].z = 0.f;
        m[1].x = 0.33333333f;
        m[1].y = 0.f;
        m[1].z = 0.33333333f;
        m[2].x = 0.66666667f;
        m[2].y = 0.33333333f;
        m[2].z = 0.66666667f;
        m[3].x = 1.f;
        m[3].y = 1.f;
        m[3].z = 1.f;
        if(d3 < 0.f)
            reverse_cubic_klm_coords(m, m);
    }
    void get_coords_line(vec3 m[4])
    {
        assert(!"process me outside.");
    }
};

float get_cubic_klmcoords(vec3 m[4], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4, const vec2& ac1, const vec2& ac2, const vec2& ac3, const vec2& ac4)
{
    cubic_klm_classifier classifier(p1, p2, p3, p4, ac1, ac2, ac3, ac4);
    classifier.get_coords(m);
    return classifier.sp;
}

bool point_in_triangle(const vec2& p, const vec2& p1, const vec2& p2, const vec2& p3)
{
    vec2 v0, v1, v2;
    v0.sub(p3, p1);
    v1.sub(p2, p1);
    v2.sub(p, p1);
    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d02 = v0.dot(v2);
    float d11 = v1.dot(v1);
    float d12 = v1.dot(v2);
    float invd = 1.f / (d00 * d11 - d01 * d01);
    float u = invd * (d11 * d02 - d01 * d12);
    if(u <= 0.f)
        return false;
    float v = invd * (d00 * d12 - d01 * d02);
    if(v <= 0.f)
        return false;
    return u + v < 1.f;
}

int point_in_polygon(const vec2& p, const vector<vec2>& poly)
{
    if(poly.size() < 3)
        return 0;
    int result = 0;
#define in_poly_judge(id0, id1) { \
        const vec2& p0 = poly.at(id0); \
        const vec2& p1 = poly.at(id1); \
        if(p1.y == p.y) { \
            if((p1.x == p.x) || (p0.y == p.y && ((p1.x > p.x) == (p0.x < p.x)))) \
                 return -1; \
        } \
        if((p0.y < p.y) != (p1.y < p.y)) { \
            if(p0.x >= p.x) { \
                if(p1.x > p.x) \
                    result = 1 - result; \
                else { \
                    float d = (p0.x - p.x) * (p1.y - p.y) - (p1.x - p.x) * (p0.y - p.y); \
                    if(!d) return -1; \
                    if((d > 0.f) == (p1.y > p0.y)) \
                        result = 1 - result; \
                } \
            } \
            else { \
                if(p1.x > p.x) { \
                    float d = (p0.x - p.x) * (p1.y - p.y) - (p1.x - p.x) * (p0.y - p.y); \
                    if(!d) return -1; \
                    if((d > 0.f) == (p1.y > p0.y)) \
                        result = 1 - result; \
                } \
            } \
        } \
    }
    for(int last = 0, i = 1; i < (int)poly.size(); last = i ++)
        in_poly_judge(last, i);
    in_poly_judge(poly.size() - 1, 0);
#undef in_poly_judge
    return result;
}

float calc_triangle_area(const vec2& p1, const vec2& p2, const vec2& p3)
{
    return 0.5f * (p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p1.x * p3.y - p2.x * p1.y - p3.x * p2.y);
}

double hp_calc_triangle_area(const vec2& p1, const vec2& p2, const vec2& p3)
{
    double d = (double)p1.x * p2.y + (double)p2.x * p3.y + (double)p3.x * p1.y - (double)p1.x * p3.y - (double)p2.x * p1.y - (double)p3.x * p2.y;
    return d * 0.5;
}

vec3 get_barycentric_coords(const vec2& p, const vec2& p1, const vec2& p2, const vec2& p3)
{
    float s = calc_triangle_area(p1, p2, p3);
    float s1 = calc_triangle_area(p, p2, p3);
    float s2 = calc_triangle_area(p, p3, p1);
    float s3 = calc_triangle_area(p, p1, p2);
    return vec3(s1 / s, s2 / s, s3 / s);
}

double calc_polygon_area(const vec2 p[], int size)
{
    if(!p || size < 3)
        return 0.0;
    double area = 0.0;
    for(int i = 2; i < size; i ++) {
        double d = hp_calc_triangle_area(p[0], p[i - 1], p[i]);
        area += d;
    }
    return area;
}

bool clip_line_rect(float t[2], const vec2& p1, const vec2& p2, const rect& clipbox)
{
    rectf rc;
    rc.set_ltrb((float)clipbox.left, (float)clipbox.top, (float)clipbox.right, (float)clipbox.bottom);
    return clip_line_rectf(t, p1, p2, rc);
}

static float linear_intersect_x(const vec3& coef, float y)
{
    return fuzzy_zero(coef.x) ? NAN : -(coef.y * y + coef.z) / coef.x;
}

static float linear_intersect_y(const vec3& coef, float x)
{
    return fuzzy_zero(coef.y) ? NAN : -(coef.x * x + coef.z) / coef.y;
}

bool clip_line_rectf(float t[2], const vec2& p1, const vec2& p2, const rectf& clipbox)
{
    assert((clipbox.left <= clipbox.right) && (clipbox.top <= clipbox.bottom));
    if(gs_max(p1.x, p2.x) <= clipbox.left || gs_min(p1.x, p2.x) >= clipbox.right ||
        gs_max(p1.y, p2.y) <= clipbox.top || gs_min(p1.y, p2.y) >= clipbox.bottom
        )
        return false;
    auto validate_check_x = [&](const vec2& p)-> float {
        if(p.x < clipbox.left || p.y >= clipbox.right)
            return NAN;
        float t = linear_reparameterize(p1, p2, p);
        if(t >= 0.f && t <= 1.f)
            return t;
        return NAN;
    };
    auto validate_check_y = [&](const vec2& p)-> float {
        if(p.y < clipbox.top || p.y >= clipbox.bottom)
            return NAN;
        float t = linear_reparameterize(p1, p2, p);
        if(t >= 0.f && t <= 1.f)
            return t;
        return NAN;
    };
    if(clipbox.in_rect((const pointf&)p1)) {
        t[0] = 0.f;
        if(clipbox.in_rect((const pointf&)p2)) {
            t[1] = 1.f;
            return true;
        }
        vec3 coef;
        get_linear_coefficient(coef, p1, vec2().sub(p2, p1));
        float x = linear_intersect_x(coef, clipbox.top);
        if(!isnan(x)) {
            t[1] = validate_check_x(vec2(x, clipbox.top));
            if(!isnan(t[1]))
                return true;
        }
        x = linear_intersect_x(coef, clipbox.bottom);
        if(!isnan(x)) {
            t[1] = validate_check_x(vec2(x, clipbox.bottom));
            if(!isnan(t[1]))
                return true;
        }
        float y = linear_intersect_y(coef, clipbox.left);
        if(!isnan(y)) {
            t[1] = validate_check_y(vec2(clipbox.left, y));
            if(!isnan(t[1]))
                return true;
        }
        y = linear_intersect_y(coef, clipbox.right);
        if(!isnan(y)) {
            t[1] = validate_check_y(vec2(clipbox.right, y));
            if(!isnan(t[1]))
                return true;
        }
    }
    else if(clipbox.in_rect((const pointf&)p2)) {
        t[1] = 1.f;
        vec3 coef;
        get_linear_coefficient(coef, p1, vec2().sub(p2, p1));
        float x = linear_intersect_x(coef, clipbox.top);
        if(!isnan(x)) {
            t[0] = validate_check_x(vec2(x, clipbox.top));
            if(!isnan(t[0]))
                return true;
        }
        x = linear_intersect_x(coef, clipbox.bottom);
        if(!isnan(x)) {
            t[0] = validate_check_x(vec2(x, clipbox.bottom));
            if(!isnan(t[0]))
                return true;
        }
        float y = linear_intersect_y(coef, clipbox.left);
        if(!isnan(y)) {
            t[0] = validate_check_y(vec2(clipbox.left, y));
            if(!isnan(t[0]))
                return true;
        }
        y = linear_intersect_y(coef, clipbox.right);
        if(!isnan(y)) {
            t[0] = validate_check_y(vec2(clipbox.right, y));
            if(!isnan(t[0]))
                return true;
        }
    }
    else {
        int c = 0;
        vec3 coef;
        get_linear_coefficient(coef, p1, vec2().sub(p2, p1));
        for(;;) {
            float x = linear_intersect_x(coef, clipbox.top);
            if(!isnan(x)) {
                t[c] = validate_check_x(vec2(x, clipbox.top));
                if(!isnan(t[c]))
                    ++ c;
            }
            x = linear_intersect_x(coef, clipbox.bottom);
            if(!isnan(x)) {
                t[c] = validate_check_x(vec2(x, clipbox.bottom));
                if(!isnan(t[c])) {
                    if(++ c >= 2)
                        break;
                }
            }
            float y = linear_intersect_y(coef, clipbox.left);
            if(!isnan(y)) {
                t[c] = validate_check_y(vec2(clipbox.left, y));
                if(!isnan(t[c])) {
                    if(++ c >= 2)
                        break;
                }
            }
            y = linear_intersect_y(coef, clipbox.right);
            if(!isnan(y)) {
                t[c] = validate_check_y(vec2(clipbox.right, y));
                if(!isnan(t[c]))
                    ++ c;
            }
            break;
        }
        if(c == 2) {
            if(t[0] > t[1])
                gs_swap(t[0], t[1]);
            return true;
        }
    }
    return false;
}

void trace_quad_strip(const vec2 cp[], int size)
{
    assert(cp && size > 0);
    assert((size - 1) % 2 == 0);
    trace(_t("@!\n"));
    trace(_t("@moveTo %f, %f;\n"), cp[0].x, cp[0].y);
    trace(_t("@dot %f, %f;\n"), cp[0].x, cp[0].y);
    for(int i = 1; i < size; i += 2) {
        trace(_t("@quadTo %f, %f, %f, %f;\n"), cp[i].x, cp[i].y, cp[i + 1].x, cp[i + 1].y);
        trace(_t("@dot %f, %f;\n"), cp[i + 1].x, cp[i + 1].y);
    }
    trace(_t("@@\n"));
}

void trace_cubic_strip(const vec2 cp[], int size)
{
    assert(cp && size > 0);
    assert((size - 1) % 3 == 0);
    trace(_t("@!\n"));
    trace(_t("@moveTo %f, %f;\n"), cp[0].x, cp[0].y);
    trace(_t("@dot %f, %f;\n"), cp[0].x, cp[0].y);
    for(int i = 1; i < size; i += 3) {
        trace(_t("@cubicTo %f, %f, %f, %f, %f, %f;\n"), cp[i].x, cp[i].y, cp[i + 1].x, cp[i + 1].y, cp[i + 2].x, cp[i + 2].y);
        trace(_t("@dot %f, %f;\n"), cp[i + 2].x, cp[i + 2].y);
    }
    trace(_t("@@\n"));
}

__gslib_end__
