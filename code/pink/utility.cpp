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

#include <float.h>
#include <complex>
#include <gslib/std.h>
#include <gslib/error.h>
#include <pink/utility.h>

__pink_begin__

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

float quad_reparameterize(const vec3 para[2], const vec2& p)
{
    vec3 tryreparax = para[0];
    tryreparax.z -= p.x;
    float tx[2];
    int c1 = solve_univariate_quadratic(tx, tryreparax);
    for(int i = 0; i < c1; i ++) {
        float t = tx[i];
        float y = para[1].dot(vec3(t * t, t, 1.f));
        if(fuzz_cmp(y, p.y, 0.1f) == 0)
            return t;
    }
    vec3 tryreparay = para[1];
    tryreparay.z -= p.y;
    float ty[2];
    int c2 = solve_univariate_quadratic(ty, tryreparay);
    for(int i = 0; i < c2; i ++) {
        float t = ty[i];
        float x = para[0].dot(vec3(t * t, t, 1.f));
        if(fuzz_cmp(x, p.x, 0.1f) == 0)
            return t;
    }
    return -1.f;
}

float cubic_reparameterize(const vec4 para[2], const vec2& p)
{
    vec4 tryreparax = para[0];
    tryreparax.w -= p.x;
    float tx[3];
    int c1 = solve_univariate_cubic(tx, tryreparax);
    for(int i = 0; i < c1; i ++) {
        float t = tx[i];
        float y = para[1].dot(vec4(t * t * t, t * t, t, 1.f));
        if(fuzz_cmp(y, p.y, 0.1f) == 0)
            return t;
    }
    vec4 tryreparay = para[1];
    tryreparay.w -= p.y;
    float ty[3];
    int c2 = solve_univariate_cubic(ty, tryreparay);
    for(int i = 0; i < c2; i ++) {
        float t = ty[i];
        float x = para[0].dot(vec4(t * t * t, t * t, t, 1.f));
        if(fuzz_cmp(x, p.x, 0.1f) == 0)
            return t;
    }
    return -1.f;
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

bool is_approx_line(const vec2& p1, const vec2& p2, const vec2& p3, float epsilon)
{
    vec2 c1, c2;
    c1.sub(p2, p1);
    c2.sub(p3, p2);
    float costheta = c1.dot(c2) / (c1.length() * c2.length());
    float theta = acosf(costheta);
    return theta < epsilon;
}

void get_linear_coefficient(vec3& coef, const vec2& p, const vec2& d)
{
    assert(d.x != 0 || d.y != 0);
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
    assert(coef.x != 0);
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

static void get_cubic_pd(vec2& pd, const vec4& coef, const vec3& der, float t)
{
    float tsq = t * t, tcu = tsq * t;
    pd.x = vec4dot(&coef, &vec4(tcu, tsq, t, 1.f));
    pd.y = vec3dot(&der, &vec3(tsq, t, 1.f));
}

static float newton_iteration(const vec4& coef, const vec3& der, float t, float tolerance)
{
    vec2 pd;
    get_cubic_pd(pd, coef, der, t);
    float m = vec2ccw(&pd, &vec2(t, 1.f));
    float x = -m / pd.y;
    if(abs(x - t) < tolerance)
        return x;
    return newton_iteration(coef, der, x, tolerance);
}

static bool calc_univariate_cubic(float& t, const vec4& coef, const vec3& der, float start, float end, float tolerance)
{
    vec2 pdstart, pdend;
    get_cubic_pd(pdstart, coef, der, start);
    get_cubic_pd(pdend, coef, der, end);
    if(pdstart.x > pdend.x && pdstart.y < 0) {
        if(pdstart.x < 0)
            return false;
    }
    else if(pdstart.x < pdend.x && pdstart.y > 0) {
        if(pdstart.x > 0)
            return false;
    }
    t = newton_iteration(coef, der, 0.5f * (start + end), tolerance);
    return true;
}

int solve_univariate_cubic(float t[3], const vec4& coef, float tolerance)
{
    vec3 der;
    get_first_derivate_coef(der, coef);
    float dt[2];
    int n = solve_univariate_quadratic(dt, der);
    if(n == 2 && (dt[0] > dt[1]))
        gs_swap(dt[0], dt[1]);
    float dtasc[4] = { 0.f, 1.f, 1.f, 1.f };
    int c = 1;
    for(int i = 0; i < n; i ++) {
        if(dt[i] > 0 && dt[i] < 1.f)
            dtasc[c++] = dt[i];
    }
    int cc = 0;
    for(int i = 0; i < c; i ++) {
        if(calc_univariate_cubic(t[cc], coef, der, dtasc[i], dtasc[i+1], tolerance))
            cc ++;
    }
    return cc;
}

static float cbrt(float d)
{
    return d >= 0.f ? powf(d, 1.f / 3) :
        -powf(-d, 1.f / 3);
}

typedef std::complex<float> fcomplex;

static float cmodulesq(const fcomplex& c) { return c.real() * c.real() + c.imag() * c.imag(); }
static float cmodule(const fcomplex& c) { return sqrtf(cmodulesq(c)); }
static float ctheta(const fcomplex& c) { return atan2f(c.imag(), c.real()); }

static void csqrt(fcomplex out[2], const fcomplex& c)
{
    float m = sqrtf(cmodule(c));
    float t = ctheta(c) * 0.5f;
    out[0]._Val[_RE] = m * cosf(t);
    out[0]._Val[_IM] = m * sinf(t);
    out[1]._Val[_RE] = m * cosf(t + PI);
    out[1]._Val[_IM] = m * sinf(t + PI);
}

static void ccbrt(fcomplex out[3], const fcomplex& c)
{
    float m = cbrt(cmodule(c));
    float t = ctheta(c) * 0.333333f;
    out[0]._Val[_RE] = m * cosf(t);
    out[0]._Val[_IM] = m * sinf(t);
    t += PI * 0.666667f;
    out[1]._Val[_RE] = m * cosf(t);
    out[1]._Val[_IM] = m * sinf(t);
    t += PI * 0.666667f;
    out[2]._Val[_RE] = m * cosf(t);
    out[2]._Val[_IM] = m * sinf(t);
}

int solve_univariate_cubic(float t[3], const vec4& coef)
{
    assert(!(fuzzy_zero(coef.x) && fuzzy_zero(coef.y) && fuzzy_zero(coef.z)));
    if(fuzzy_zero(coef.x) && fuzzy_zero(coef.y)) {
        t[0] = -coef.w /coef.z;
        return 1;
    }
    else if(fuzzy_zero(coef.x)) {
        float delta = coef.z * coef.z - 4.f * coef.y * coef.w;
        if(delta < 0)
            return 0;
        delta = sqrtf(delta);
        t[0] = (-coef.z + delta) / coef.y * 0.5f;
        t[1] = (-coef.z - delta) / coef.y * 0.5f;
        return fuzzy_zero(delta) ? 1 : 2;
    }
    else {
        float ydx = coef.y / coef.x;
        float a = -ydx * 0.333333f;
        float b = (ydx * coef.z * 0.166667f - ydx * ydx * coef.y * 0.037037f - coef.w * 0.5f) / coef.x;
        float c = (coef.z * 0.333333f - ydx * coef.y * 0.111111f) / coef.x;
        float delta = b * b + c * c * c;
        if(fuzzy_zero(delta)) {
            if(fuzzy_zero(b)) { t[0] = a; return 1; }
            else {
                float cb = cbrt(b);
                t[0] = a + cb + cb;
                t[1] = a - cb;
                return 2;
            }
        }
        else if(delta > 0.f) {
            delta = sqrtf(delta);
            t[0] = a + cbrt(b - delta) + cbrt(b + delta);
            return 1;
        }
        else {
            fcomplex ca(a, 0), cb(b, 0), cc(c, 0);
            fcomplex cdelta = cb * cb + cc * cc * cc;
            fcomplex sqd[2], cbrt1[3], cbrt2[3];
            csqrt(sqd, cdelta);
            ccbrt(cbrt1, cb + sqd[0]);
            ccbrt(cbrt2, cb + sqd[1]);
            fcomplex r1 = ca + cbrt1[0] + cbrt2[0],
                r2 = ca + cbrt1[1] + cbrt2[2],
                r3 = ca + cbrt1[2] + cbrt2[1];
            t[0] = r1.real();
            t[1] = r2.real();
            t[2] = r3.real();
            return 3;
        }
    }
}

static float det(float a, float b, float c, float d) { return (a * d - b * c) / (a * a); }

int solve_univariate_quartic(float t[4], const float coef[5])
{
    int cnt = 0;
    float k0, l0, m0, n0, p0, q0, r0, discr, x1, x2, x3, u, v;
    float a = coef[0];
    float b = coef[1] * 0.25f;
    float c = coef[2] * 0.1666667f;
    float d = coef[3] * 0.25f;
    float e = coef[4];
    k0 = -b / a;
    l0 = det(a, b, b, c);
    m0 = det(a, b, c, d);
    n0 = det(a, b, d, e);
    p0 = 6.f * l0;
    q0 = 8.f * k0 * l0 + 4.f * m0;
    r0 = 3.f * k0 * k0 * l0 + 3.f * k0 * m0 + n0;
    if(fuzzy_zero(q0)) {
        p0 *= 0.5f;
        discr = p0 * p0 - r0;
        if(fuzzy_zero(discr)) {
            if(fuzzy_zero(p0)) {
                t[0] = k0;
                cnt = 1;
            }
            else if(p0 > 1e-10) {
                t[0] = k0;
                cnt = 1;
            }
            else {
                u = sqrtf(-p0);
                t[0] = k0 - u;
                t[1] = k0 + u;
                cnt = 2;
            }
        }
        else if(discr > 1e-10) {
            float sq = sqrtf(discr);
            x1 = -p0 - sq;
            x2 = -p0 + sq;
            if(fuzzy_zero(x2)) {
                t[0] = k0;
                cnt = 1;
            }
            else if(fuzzy_zero(x1)) {
                u = sqrtf(x2);
                t[0] = k0 - u;
                t[1] = k0 + u;
                t[2] = k0;
                cnt = 3;
            }
            else if(x2 < -1e-10) {
                t[0] = k0;
                cnt = 1;
            }
            else if(x1 > 1e-10) {
                u = sqrtf(x1);
                v = sqrtf(x2);
                t[0] = k0 - v;
                t[1] = k0 - u;
                t[2] = k0 + u;
                t[3] = k0 + v;
                cnt = 4;
            }
            else {
                v = sqrtf(x2);
                t[0] = k0 - v;
                t[1] = k0 + v;
                t[2] = k0;
                cnt = 3;
            }
        }
        else {
            float pwcf = powf(r0, 0.25f) * cosf(atan2f(sqrtf(-discr), -p0) * 0.5f);
            t[0] = k0 - pwcf;
            t[1] = k0 + pwcf;
            cnt = 2;
        }
    }
    else {
        float p1, q1, r1, i, j, k, m, n, p, q, t1, t2;
        p1 = (p0 * p0 - 4.f * r0) / (6.f * q0);
        q1 = -p0 * 0.1666667f;
        r1 = q0 * 0.125f;
        k = -p1;
        i = det(1,p1,p1,q1);
        j = det(1,p1,q1,r1);
        p = i;
        q = k * i + j * 0.5f;
        discr = q * q + p * p * p;
        if(fuzzy_zero(discr)) {
            if(fuzzy_zero(q)) {
                t[0] = k0 + 3.f * k;
                t[1] = k0 - k;
                cnt = 2;
            }
            else {
                float cb = cbrt(-q);
                x1 = k + cb + cb;
                x2 = k - cb;
                if(x1 > 1e-10 && x2 > 1e-10 || x1 < -1e-10 && x2 < -1e-10) {
                    float dsq = 2.f * sqrtf(x1 * x2);
                    t[0] = k0 - dsq + x2;
                    t[1] = k0 + dsq + x2;
                    t[2] = k0 - x2;
                    cnt = 3;
                }
                else if(x1 < -1e-10 && x2 > 1e-10 || x1 > 1e-10 && x2 < -1e-10) {
                    t[0] = k0 - x2;
                    t[1] = k0 + x2;
                    cnt = 2;
                }
            }
        }
        else if(discr > 1e-10) {
            discr = sqrtf(discr);
            t1 = -q - discr;
            t2 = -q + discr;
            t1 = cbrt(t1);
            t2 = cbrt(t2);
            x1 = k + t1 + t2;
            m = k - (t1 + t2) * 0.5f;
            n = 0.8660254f * (t2 - t1);
            float r = sqrtf(m * m + n * n);
            float x = atan2f(n, m);
            if(x1 > 0) {
                float dsqcf = 2.f * sqrtf(x1 * r) * cosf(x * 0.5f);
                t[0] = k0 + r - dsqcf;
                t[1] = k0 + r + dsqcf;
                t[2] = k0 - r;
                cnt = 3;
            }
            else {
                float dsqsf = 2.f * sqrtf(-x1 * r) * sinf(x * 0.5f);
                t[0] = k0 - r - dsqsf;
                t[1] = k0 - r + dsqsf;
                t[2] = k0 + r;
                cnt = 3;
            }
        }
        else {
            if(fuzzy_zero(q)) {
                float sq = sqrtf(-3.f * p);
                x1 = k - sq;
                x2 = k;
                x3 = k + sq;
            }
            else {
                discr = sqrtf(-discr);
                float x = atan2f(discr, -q);
                t1 = sqrtf(-p) * cosf(x * 0.3333333f);
                t2 = sqrtf(-3 * p) * sinf(x * 0.3333333f);
                x1 = k - t1 - t2;
                x2 = k - t1 + t2;
                x3 = k + t1 + t1;
            }
            if(x1 > 1e-10) {
                float sq12 = sqrtf(x1 * x2);
                float sq13 = sqrtf(x1 * x3);
                float sq23 = sqrtf(x2 * x3);
                t[0] = k0 + sq12 - sq13 - sq23;
                t[1] = k0 - sq12 + sq13 - sq23;
                t[2] = k0 - sq12 - sq13 + sq23;
                t[3] = k0 + sq12 + sq13 + sq23;
                cnt = 4;
            }
            else if(x3 < -1e-10) {
                float sq12 = sqrtf(x1 * x2);
                float sq13 = sqrtf(x1 * x3);
                float sq23 = sqrtf(x2 * x3);
                t[0] = k0 - sq12 - sq13 - sq23;
                t[1] = k0 - sq12 + sq13 + sq23;
                t[2] = k0 + sq12 - sq13 + sq23;
                t[3] = k0 + sq12 + sq13 - sq23;
                cnt = 4;
            }
            else {
                if(x2 > 0) {
                    float sq = sqrtf(x2 * x3);
                    t[0] = k0 - sq;
                    t[1] = k0 + sq;
                    cnt = 2;
                }
                else {
                    float sq = sqrtf(x1 * x2);
                    t[0] = k0 - sq;
                    t[1] = k0 + sq;
                    cnt = 2;
                }
            }
        }
    }
    return cnt;
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
    int cap = solve_univariate_quadratic(t, coef);
    int cnt = 0;
    /* filter the result within scope 0 - 1 */
    for(int i = 0; i < cap; i ++) {
        float v = t[i];
        if(v >= 0.f && v <= 1.f)
            t[cnt ++] = v;
    }
    return cnt;
}

void intersectp_linear_linear(vec2& ip, const vec2& p1, const vec2& p2, const vec2& d1, const vec2& d2)
{
    assert((d1.x != 0 || d1.y != 0) && "won't be parallel");
    float dxy = d1.x * d2.y, dyx = d2.x * d1.y;
    ip.y = (d1.y * d2.y * (p2.x - p1.x) + dxy * p1.y - dyx * p2.y) / (dxy - dyx);
    ip.x = (d1.y != 0) ? ((ip.y - p1.y) * d1.x / d1.y + p1.x) :
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
    vec3 substitu(
        quad2[0].x * quad1[1].x - quad2[1].x * quad1[0].x,
        quad2[0].y * quad1[1].x - quad2[1].y * quad1[0].x,
        quad2[0].z * quad1[1].x - quad2[1].z * quad1[0].x + quad1[0].x * quad1[1].z - quad1[0].z * quad1[1].x
        );
    substitu.scale(1.f / (quad1[0].y * quad1[1].x - quad1[0].x * quad1[1].y));
    float al = quad1[0].x * substitu.x;
    float am = quad1[0].x * substitu.y;
    float coef[5] = {
        al * substitu.x,
        2.f * al * substitu.y,
        2.f * al * substitu.z + am * substitu.y + quad1[0].y * substitu.x - quad2[0].x,
        2.f * am * substitu.z + quad1[0].y * substitu.y - quad2[0].y,
        quad1[0].x * substitu.z * substitu.z + quad1[0].y * substitu.z + quad1[0].z - quad2[0].z
    };
    float s[4];
    int cap = solve_univariate_quartic(s, coef), cnt = 0;
    for(int i = 0; i < cap; i ++) {
        if(fuzzy_between(0.f, s[i], 1.f, 0.001f)) {
            float t = vec3dot(&substitu, &vec3(s[i] * s[i], s[i], 1.f));
            if(fuzzy_between(0.f, t, 1.f, 0.001f)) {
                ts[cnt][0] = t;
                ts[cnt][1] = s[i];
                cnt ++;
            }
        }
    }
    return cnt;
}

int intersection_cubic_linear(float t[3], const vec4 cubic[2], const vec3& linear)
{
    int cnt = 0, cap = 0;
    float tt[3];
    vec4 cf, cf1, cf2;
    vec4scale(&cf1, &cubic[0], linear.x);
    vec4scale(&cf2, &cubic[1], linear.y);
    cf2.w += linear.z;
    vec4add(&cf, &cf1, &cf2);
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

static const vec2* decide_split_cubic(vec2 sp[10], const vec2 p[4], float t[2], int c)
{
    switch(c)
    {
    case 0:
        return p;
    case 1:
        split_cubic_bezier(sp, p, t[0]);
        return sp;
    case 2:
        t[0] < t[1] ? split_cubic_bezier(sp, p, t[0], t[1]) :
            split_cubic_bezier(sp, p, t[1], t[0]);
        return sp;
    }
    assert(!"unexpected.");
    return 0;
}

static int newton_raphson_iteration(vec2 ip[4], float ts[4][2], int c, const vec3 para1[2], const vec4 para2[2], float tolerance)
{
    vec2 fact10(para1[0].x * 3, para1[0].y * 2);
    vec2 fact20(para2[0].x * 3, para2[0].y * 2);
    vec2 fact11(para1[1].x * 3, para1[1].y * 2);
    vec2 fact21(para2[1].x * 3, para2[1].y * 2);
    int cnt = 0;
    for(int i = 0; i < c; i ++) {
        float s = ts[i][0], t = ts[i][1];
        do {
            vec2 d, a, b;
            eval_quad(a, para1, s);
            eval_cubic(b, para2, t);
            vec2sub(&d, &a, &b);
            if(vec2length(&d) < tolerance) {
                ip[cnt ++] = a;
                break;
            }
            float duds = fact10.x * s + fact10.y;
            float dudt = (fact20.x * t + fact20.y) * t + para2[0].z;
            float dvds = fact11.x * s + fact11.y;
            float dvdt = (fact21.x * t + fact21.y) * t + para2[1].z;
            float div = 1.f / (duds * dvdt - dvds * dudt);
            s += (dudt * d.y - dvdt * d.x) * div;
            t -= (dvds * d.x - duds * d.y) * div;
        }
        while(!(s < -0.001f || s > 1.001f || t < -0.001f || t > 1.001f));
    }
    return cnt;
}

static int initialize_quad_intersection(float ts[4][2], const vec2& p1, const vec2& p2, const vec2& center, const vec3 para1[2], const vec4 para2[2])
{
    vec3 coef;
    get_linear_coefficient(coef, p1, vec2().sub(p2, p1));
    vec3 ff[2];
    get_first_derivate_factor(ff, para2);
    float t[3];
    int c = intersection_cubic_linear(t, para2, coef);
    int cnt = 0;
    for(int i = 0; i < c; i ++) {
        vec2 p, d;
        eval_cubic(p, para2, t[i]);
        float lt = linear_reparameterize(p1, p2, p);
        if(lt < -0.0001f || lt > 1.0001f)
            continue;
        float s[2];
        eval_quad(d, ff, t[i]);
        get_linear_coefficient(coef, p, d);
        int c1 = intersection_quad_linear(s, para1, coef);
        if(c1 == 1) {
            vec2 v;
            eval_quad(v, para1, s[0]);
            lt = linear_reparameterize(p, vec2().add(p, d), v);
            if(lt > 0.f) {
                ts[cnt][0] = s[0];
                ts[cnt][1] = t[i];
                cnt ++;
                continue;
            }
        }
        else if(c1 == 2) {
            vec2 v1, v2, ep;
            eval_quad(v1, para1, s[0]);
            eval_quad(v2, para1, s[1]);
            ep.add(p, d);
            float lt1 = linear_reparameterize(p, ep, v1);
            float lt2 = linear_reparameterize(p, ep, v2);
            if(lt1 > 0.f && lt2 > 0.f) {
                ts[cnt][0] = lt1 < lt2 ? s[0] : s[1];
                ts[cnt][1] = t[i];
                cnt ++;
                continue;
            }
        }
        get_linear_coefficient(coef, p, vec2().sub(center, p));
        int c2 = intersection_quad_linear(s, para1, coef);
        assert(c2 == 1);
        ts[cnt][0] = s[0];
        ts[cnt][1] = t[i];
        cnt ++;
    }
    return cnt;
}

static int get_noninflection_quad_intersections(vec2 ip[4], const vec2 cp1[4], const vec2 cp2[3], const vec4 para1[2], const vec3 para2[2], float tolerance)
{
    float ts[4][2];
    int c;
    vec2 center;
    vec2lerp(&center, &cp2[0], &cp2[2], 0.5f);
    c = initialize_quad_intersection(ts, cp2[0], cp2[1], center, para2, para1);
    c += initialize_quad_intersection(ts + c, cp2[1], cp2[2], center, para2, para1);
    return c ? newton_raphson_iteration(ip, ts, c, para2, para1, tolerance) : 0;
}

int intersectp_cubic_quad(vec2 ip[6], const vec2 cp1[4], const vec2 cp2[3], float tolerance)
{
    float inf[2];
    int infc = get_cubic_inflection(inf, cp1[0], cp1[1], cp1[2], cp1[3]);
    vec2 group[10];
    const vec2* fg = decide_split_cubic(group, cp1, inf, infc);
    assert(fg);
    int c1 = infc + 1;
    vec4 para1[3][2];
    vec3 para2[2];
    int i = 0;
    const vec2* cp = fg;
    for(i = 0; i < c1; i ++, cp += 3)
        get_cubic_parameter_equation(para1[i], cp[0], cp[1], cp[2], cp[3]);
    get_quad_parameter_equation(para2, cp2[0], cp2[1], cp2[2]);
    int c = 0;
    for(i = 0; i < c1; i ++, fg += 3) {
        int extc = get_noninflection_quad_intersections(ip, fg, cp2, para1[i], para2, tolerance);
        ip += extc;
        c += extc;
    }
    assert(c <= 6);
    return c;
}

static int newton_raphson_iteration(vec2 ip[4], float ts[4][2], int c, const vec4 para1[2], const vec4 para2[2], float tolerance)
{
    vec2 fact10(para1[0].x * 3, para1[0].y * 2);
    vec2 fact20(para2[0].x * 3, para2[0].y * 2);
    vec2 fact11(para1[1].x * 3, para1[1].y * 2);
    vec2 fact21(para2[1].x * 3, para2[1].y * 2);
    int cnt = 0;
    for(int i = 0; i < c; i ++) {
        float s = ts[i][0], t = ts[i][1];
        do {
            vec2 d, a, b;
            eval_cubic(a, para1, s);
            eval_cubic(b, para2, t);
            vec2sub(&d, &a, &b);
            if(vec2length(&d) < tolerance) {
                ip[cnt ++] = a;
                break;
            }
            float duds = (fact10.x * s + fact10.y) * s + para1[0].z;
            float dudt = (fact20.x * t + fact20.y) * t + para2[0].z;
            float dvds = (fact11.x * s + fact11.y) * s + para1[1].z;
            float dvdt = (fact21.x * t + fact21.y) * t + para2[1].z;
            float div = 1.f / (duds * dvdt - dvds * dudt);
            s += (dudt * d.y - dvdt * d.x) * div;
            t -= (dvds * d.x - duds * d.y) * div;
        }
        while(!(s < -0.001f || s > 1.001f || t < -0.001f || t > 1.001f));
    }
    return cnt;
}

static int initialize_cubic_intersection(float ts[4][2], const vec2& p1, const vec2& p2, const vec2& center, const vec4 para1[2], const vec4 para2[2])
{
    vec3 coef;
    get_linear_coefficient(coef, p1, vec2().sub(p2, p1));
    vec3 ff[2];
    get_first_derivate_factor(ff, para2);
    float t[3];
    int c = intersection_cubic_linear(t, para2, coef);
    int cnt = 0;
    for(int i = 0; i < c; i ++) {
        vec2 p, d;
        eval_cubic(p, para2, t[i]);
        float lt = linear_reparameterize(p1, p2, p);
        if(lt < -0.0001f || lt > 1.0001f)
            continue;
        float s[3];
        eval_quad(d, ff, t[i]);
        get_linear_coefficient(coef, p, d);
        int c1 = intersection_cubic_linear(s, para1, coef);
        if(c1 == 1) {
            vec2 v;
            eval_cubic(v, para1, s[0]);
            lt = linear_reparameterize(p, vec2().add(p, d), v);
            if(lt > 0.f) {
                ts[cnt][0] = s[0];
                ts[cnt][1] = t[i];
                cnt ++;
                continue;
            }
        }
        else if(c1 == 2) {
            vec2 v1, v2, ep;
            eval_cubic(v1, para1, s[0]);
            eval_cubic(v2, para1, s[1]);
            ep.add(p, d);
            float lt1 = linear_reparameterize(p, ep, v1);
            float lt2 = linear_reparameterize(p, ep, v2);
            if(lt1 > 0.f && lt2 > 0.f) {
                ts[cnt][0] = lt1 < lt2 ? s[0] : s[1];
                ts[cnt][1] = t[i];
                cnt ++;
                continue;
            }
        }
        get_linear_coefficient(coef, p, vec2().sub(center, p));
        int c2 = intersection_cubic_linear(s, para1, coef);
        assert(c2 == 1);
        ts[cnt][0] = s[0];
        ts[cnt][1] = t[i];
        cnt ++;
    }
    return cnt;
}

static int get_non_inflection_cubic_intersections(vec2 ip[4], const vec2 cp1[4], const vec2 cp2[4], const vec4 para1[2], const vec4 para2[2], float tolerance)
{
    float ts[4][2];
    int c;
    vec2 center;
    vec2lerp(&center, &cp1[0], &cp1[3], 0.5f);
    c = initialize_cubic_intersection(ts, cp1[0], cp1[1], center, para1, para2);
    if(c < 4)
        c += initialize_cubic_intersection(ts + c, cp1[1], cp1[2], center, para1, para2);
    if(c < 4)
        c += initialize_cubic_intersection(ts + c, cp1[2], cp1[3], center, para1, para2);
    return c ? newton_raphson_iteration(ip, ts, c, para1, para2, tolerance) : 0;
}

/*
 * To solve the cubic - cubic intersection, we use the following strategy:
 * 1.split the cubics into 2 groups by inflection, so that each of the intersection count would be no more than 4.
 * 2.assume that b0, b1, b2, b3 was the 4 control points of the cubic bezier, solve b0 - b1, b1 - b2, b2 - b3 line intersections with the other curve.
 * 3.reparametric the intersection points and get the t-value, treat them as the initial value for the newton-ralph iteration
 * 4.get the t-values of each iterate process, filter the duplicated roots
 * 5.get the point value of each t-value, if you need the original t-value, reparametrize them by the original cubic.
 */
int intersectp_cubic_cubic(vec2 ip[9], const vec2 cp1[4], const vec2 cp2[4], float tolerance)
{
    float inf1[2], inf2[2];
    int infc1 = get_cubic_inflection(inf1, cp1[0], cp1[1], cp1[2], cp1[3]);
    int infc2 = get_cubic_inflection(inf2, cp2[0], cp2[1], cp2[2], cp2[3]);
    vec2 group1[10], group2[10];
    const vec2* fg1 = decide_split_cubic(group1, cp1, inf1, infc1);
    const vec2* fg2 = decide_split_cubic(group2, cp2, inf2, infc2);
    assert(fg1 && fg2);
    int c1 = infc1 + 1, c2 = infc2 + 1;
    vec4 para1[3][2], para2[3][2];
    int i = 0;
    const vec2* cp = fg1;
    for(i = 0; i < c1; i ++, cp += 3)
        get_cubic_parameter_equation(para1[i], cp[0], cp[1], cp[2], cp[3]);
    for(i = 0, cp = fg2; i < c2; i ++, cp += 3)
        get_cubic_parameter_equation(para2[i], cp[0], cp[1], cp[2], cp[3]);
    int c = 0;
    for(i = 0; i < c1; i ++, fg1 += 3) {
        vec4* fp1 = para1[i];
        const vec2* yafg2 = fg2;
        for(int j = 0; j < c2; j ++, yafg2 += 3) {
            vec4* fp2 = para2[j];
            int extc = get_non_inflection_cubic_intersections(ip, fg1, yafg2, fp1, fp2, tolerance);
            ip += extc;
            c += extc;
        }
    }
    assert(c <= 9);
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
    struct build_equation {
        /* coefficient for [1, u, v] */
        build_equation(vec3& v, float a, float b, float c, float d)
        {
            v.x = 3.f * (b - a);
            v.y = 3.f * (a - 2.f * b + c);
            v.z = -a + 3.f * (b - c) + d;
        }
    };
    vec3 equ1, equ2;
    build_equation build1(equ1, a.x, b.x, c.x, d.x), build2(equ2, a.y, b.y, c.y, d.y);
    vec3scale(&equ1, &equ1, equ2.y);
    vec3scale(&equ2, &equ2, equ1.y);
    float v = (equ2.x - equ1.x) / (equ1.z - equ2.z);
    float u = -(v * equ1.z + equ1.x) / equ1.y;
    /*
     * now we solve the [u, v, s, t] puzzle
     * t = u - s
     * u^2 - us + s^2 = 0   => retrieve s
     */
    vec3 coef(1, -u, u * u);
    float tt[2];
    int count = solve_univariate_quadratic(tt, coef);
    if (count != 2)
        return false;
    else
    {
        if (tt[0] >= 0 && tt[0] <= 1.f && tt[1] >= 0 && tt[1] <= 1.f)
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
 * There wasn't a convenient way to transform the t arg while dividing.
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
    get_cubic_parameter_equation(para, p[0], p[1], p[2], p[3]);
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

int get_interpolate_step(const vec2& a, const vec2& b, const vec2& c)
{
    int step = round(quad_control_length(a, b, c));
    step >>= 3;
    return step < 2 ? 2 : step;
}

int get_interpolate_step(const vec2& a, const vec2& b, const vec2& c, const vec2& d)
{
    int step = round(cubic_control_length(a, b, c, d));
    step >>= 3;
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

__pink_end__
