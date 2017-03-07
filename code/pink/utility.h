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

#ifndef utility_77d83a8d_bf54_4fbe_ab3c_b06d0be9e40e_h
#define utility_77d83a8d_bf54_4fbe_ab3c_b06d0be9e40e_h

#include <pink/config.h>
#include <pink/type.h>

/*
 * This file include a series of intersection operations.
 * Here are all the details:
 *                      point       ratio
 * linear - linear:     available   available
 * linear - quad:       none        available
 * linear - cubic:      none        available
 * quad - quad:         none        available
 * quad - cubic:        available   none
 * cubic - cubic:       available   none
 * To convert from ratio to point was quite simple, this was why I suggest ratio first.
 * other cases limited by the algorithm where the point was the first result, I hope you
 * notice this and use certain methods to get your result, so I didn't offer an altered
 * way, but you can simply achieve that by calling the reparameterize functions.
 * Caution that the precision of intersection may cause loss of roots,
 * set tolerance around 0.2 - 0.3 might be good.
 */

__pink_begin__

extern void linear_interpolate(vec2 c[], const vec2& p1, const vec2& p2, int step);
extern void quadratic_interpolate(vec2 c[], const vec2& p1, const vec2& p2, const vec2& p3, int step);
extern void cubic_interpolate(vec2 c[], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4, int step);
extern void hermite_interpolate(vec2 c[], const vec2& p1, const vec2& p2, const vec2& t1, const vec2& t2, int step);
extern float linear_reparameterize(const vec2& p1, const vec2& p2, const vec2& p);
extern float quad_reparameterize(const vec3 para[2], const vec2& p);
extern float cubic_reparameterize(const vec4 para[2], const vec2& p);
extern bool is_concave_angle(const vec2& p1, const vec2& p2, const vec2& p3);
extern bool is_concave_angle(const vec2& p1, const vec2& p2, const vec2& p3, bool cw);
extern bool is_approx_line(const vec2& p1, const vec2& p2, const vec2& p3, float epsilon);
extern void get_linear_coefficient(vec3& coef, const vec2& p, const vec2& d);
extern void get_linear_parameter_equation(vec2 para[2], const vec2& a, const vec2& b);
extern void get_quad_parameter_equation(vec3 para[2], const vec2& a, const vec2& b, const vec2& c);
extern void get_cubic_parameter_equation(vec4 para[2], const vec2& a, const vec2& b, const vec2& c, const vec2& d);
extern void get_first_derivate_factor(vec3 df1[2], const vec4 para[2]);
extern void get_second_derivate_factor(vec2 df2[2], const vec4 para[2]);
extern void get_first_derivate_factor(vec3 df1[2], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4);
extern void get_second_derivate_factor(vec2 df2[2], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4);
extern void get_first_derivate(vec2& d, const vec3 df1[2], float t);
extern void get_second_derivate(vec2& d, const vec2 df2[2], float t);
extern float get_cubic_curvature(const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4, float t);
extern int solve_univariate_quadratic(float t[2], const vec3& coef);
extern int solve_univariate_cubic(float t[3], const vec4& coef);
extern int solve_univariate_cubic(float t[3], const vec4& coef, float tolerance);
extern int solve_univariate_quartic(float t[4], const float coef[5]);
extern int get_cubic_inflection(float t[2], const vec2& a, const vec2& b, const vec2& c, const vec2& d);
extern int get_cubic_inflection(float t[2], const vec3 ff[2], const vec2 sf[2]);
extern int intersection_quad_linear(float t[2], const vec3 quad[2], const vec3& linear);
extern int intersection_cubic_linear(float t[3], const vec4 cubic[2], const vec3& linear);
extern int intersection_quad_quad(float ts[4][2], const vec3 quad1[2], const vec3 quad2[2]);
extern void intersectp_linear_linear(vec2& ip, const vec2& p1, const vec2& p2, const vec2& d1, const vec2& d2);
extern int intersectp_cubic_quad(vec2 ip[6], const vec2 cp1[4], const vec2 cp2[3], float tolerance);
extern int intersectp_cubic_cubic(vec2 ip[9], const vec2 cp1[4], const vec2 cp2[4], float tolerance);
extern bool get_self_intersection(float ts[2], const vec2& a, const vec2& b, const vec2& c, const vec2& d);
extern void split_quad_bezier(vec2 c[5], const vec2 p[3], float t);
extern void split_cubic_bezier(vec2 c[7], const vec2 p[4], float t);
extern void split_cubic_bezier(vec2 c[10], const vec2 p[4], float t1, float t2);
extern void offset_cubic_bezier(vec2 c[4], const vec2 p[4], float d);
extern float quad_control_length(const vec2& a, const vec2& b, const vec2& c);
extern float cubic_control_length(const vec2& a, const vec2& b, const vec2& c, const vec2& d);
extern float point_line_distance(const vec2& p, const vec2& p1, const vec2& p2);
extern int get_interpolate_step(const vec2& a, const vec2& b, const vec2& c);
extern int get_interpolate_step(const vec2& a, const vec2& b, const vec2& c, const vec2& d);
extern int get_rough_interpolate_step(const vec2& a, const vec2& b, const vec2& c);
extern int get_rough_interpolate_step(const vec2& a, const vec2& b, const vec2& c, const vec2& d);
extern float get_cubic_klmcoords(vec3 m[4], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4,
    const vec2& ac1, const vec2& ac2, const vec2& ac3, const vec2& ac4  /* absolute coord for error control */
    );
extern float get_include_angle(const vec2& d1, const vec2& d2);
extern float get_include_angle(const vec2& a, const vec2& b, const vec2& c);
extern float get_rotate_angle(const vec2& d1, const vec2& d2);
extern float get_rotate_angle(const vec2& a, const vec2& b, const vec2& c);
extern void get_reduce_point(vec2& p, const vec2& a, const vec2& b, const vec2& c, const vec2& d);
extern bool point_in_triangle(const vec2& p, const vec2& p1, const vec2& p2, const vec2& p3);

inline bool is_isosigned(float a, float b) { return (int)((*(uint*)&a ^ *(uint*)&b)) >= 0; }
inline bool is_isosigned(double a, double b) { return (long long)((*(unsigned long long*)&a ^ *(unsigned long long*)&b)) >= 0; }
inline bool fuzzy_zero(float f) { return (f <= 1e-10f && f >= -1e-10f); }
inline bool fuzzy_zero(float f, float tol) { return f <= tol && f >= -tol; }
//inline bool fuzzy_greater(float m, float n, float tol) { return m > (n + tol); }
//inline bool fuzzy_less(float m, float n, float tol) { return m < (n - tol); }
inline bool fuzzy_between(float a, float b, float c, float tol) { return b > a - tol && b < c + tol; }
inline bool fuzzy_greater_inclusive(float m, float n, float tol) { return m > (n - tol); }
inline bool fuzzy_less_inclusive(float m, float n, float tol) { return m < (n + tol); }
inline bool fuzzy_greater_exclusive(float m, float n, float tol) { return m > (n + tol); }
inline bool fuzzy_less_exclusive(float m, float n, float tol) { return m < (n - tol); }
inline bool is_parallel(const vec2& a, const vec2& b) { return fuzzy_zero(a.ccw(b)); }

inline vec2& eval_linear(vec2& v, const vec2 para[2], float t)
{
    vec2 sv(t, 1.f);
    v.x = para[0].dot(sv);
    v.y = para[1].dot(sv);
    return v;
}

inline vec2& eval_quad(vec2& v, const vec3 para[2], float t)
{
    vec3 sv(t * t, t , 1.f);
    v.x = para[0].dot(sv);
    v.y = para[1].dot(sv);
    return v;
}

inline vec2& eval_cubic(vec2& v, const vec4 para[2], float t)
{
    vec4 sv(t * t * t, t * t, t, 1.f);
    v.x = para[0].dot(sv);
    v.y = para[1].dot(sv);
    return v;
}

__pink_end__

#endif
