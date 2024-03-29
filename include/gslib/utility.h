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

#pragma once

#ifndef utility_77d83a8d_bf54_4fbe_ab3c_b06d0be9e40e_h
#define utility_77d83a8d_bf54_4fbe_ab3c_b06d0be9e40e_h

#include <gslib/std.h>
#include <gslib/type.h>

/*
 * This file include a series of intersection operations.
 * Here are all the details:
 *                      point       ratio
 * linear - linear:     available   available
 * linear - quad:       none        available
 * linear - cubic:      none        available
 * quad - quad:         none        available
 * quad - cubic:        none        available
 * cubic - cubic:       available   none
 * To convert from ratio to point was quite simple, this was why I suggest ratio first.
 * other cases limited by the algorithm where the point was the first result, I hope you
 * notice this and use certain methods to get your result, so I didn't offer an altered
 * way, but you can simply achieve that by calling the reparameterize functions.
 * Caution that the precision of intersection may cause loss of roots,
 * set tolerance around 0.2 - 0.3 might be good.
 */

__gslib_begin__

gs_export extern void linear_interpolate(vec2 c[], const vec2& p1, const vec2& p2, int step);
gs_export extern void quadratic_interpolate(vec2 c[], const vec2& p1, const vec2& p2, const vec2& p3, int step);
gs_export extern void cubic_interpolate(vec2 c[], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4, int step);
gs_export extern void hermite_interpolate(vec2 c[], const vec2& p1, const vec2& p2, const vec2& t1, const vec2& t2, int step);
gs_export extern float linear_reparameterize(const vec2& p1, const vec2& p2, const vec2& p);
gs_export extern float quad_reparameterize(const vec3 para[2], const vec2& p, float tol = 0.1f);
gs_export extern float cubic_reparameterize(const vec4 para[2], const vec2& p, float tol = 0.1f);
gs_export extern float best_quad_reparameterize(const vec3 para[2], const vec2& p);
gs_export extern float best_cubic_reparameterize(const vec4 para[2], const vec2& p);
gs_export extern bool is_concave_angle(const vec2& p1, const vec2& p2, const vec2& p3);
gs_export extern bool is_concave_angle(const vec2& p1, const vec2& p2, const vec2& p3, bool cw);
gs_export extern bool is_approx_line(const vec2& p1, const vec2& p2, const vec2& p3, float tolerance);
gs_export extern void get_linear_coefficient(vec3& coef, const vec2& p, const vec2& d);
gs_export extern void get_linear_parameter_equation(vec2 para[2], const vec2& a, const vec2& b);
gs_export extern void get_quad_parameter_equation(vec3 para[2], const vec2& a, const vec2& b, const vec2& c);
gs_export extern void get_cubic_parameter_equation(vec4 para[2], const vec2& a, const vec2& b, const vec2& c, const vec2& d);
gs_export extern void get_first_derivate_factor(vec2 df1[2], const vec3 para[2]);
gs_export extern void get_first_derivate_factor(vec3 df1[2], const vec4 para[2]);
gs_export extern void get_second_derivate_factor(vec2 df2[2], const vec4 para[2]);
gs_export extern void get_first_derivate_factor(vec3 df1[2], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4);
gs_export extern void get_second_derivate_factor(vec2 df2[2], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4);
gs_export extern void get_first_derivate(vec2& d, const vec3 df1[2], float t);
gs_export extern void get_second_derivate(vec2& d, const vec2 df2[2], float t);
gs_export extern float get_cubic_curvature(const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4, float t);
gs_export extern int solve_univariate_quadratic(float t[2], const vec3& coef);
gs_export extern int solve_univariate_cubic(float t[3], const vec4& coef);
gs_export extern int solve_univariate_quartic(float t[4], const float coef[5]);
gs_export extern int get_cubic_inflection(float t[2], const vec2& a, const vec2& b, const vec2& c, const vec2& d);
gs_export extern int get_cubic_inflection(float t[2], const vec3 ff[2], const vec2 sf[2]);
gs_export extern int get_quad_extrema(float t[], const vec2& ff);
gs_export extern int get_cubic_extrema(float t[2], const vec3& ff);
gs_export extern void get_quad_bound_box(rectf& rc, const vec2& p1, const vec2& p2, const vec2& p3);
gs_export extern void get_cubic_bound_box(rectf& rc, const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4);
gs_export extern int intersection_quad_linear(float t[2], const vec3 quad[2], const vec3& linear);
gs_export extern int intersection_cubic_linear(float t[3], const vec4 cubic[2], const vec3& linear);
gs_export extern int intersection_quad_quad(float ts[4][2], const vec3 quad1[2], const vec3 quad2[2]);
gs_export extern int intersection_cubic_quad(float t[6], const vec2 cp1[4], const vec2 cp2[3], float tolerance);    /* t was for cp2 */
gs_export extern void intersectp_linear_linear(vec2& ip, const vec2& p1, const vec2& p2, const vec2& d1, const vec2& d2);
gs_export extern int intersectp_cubic_cubic(vec2 ip[9], const vec2 cp1[4], const vec2 cp2[4], float tolerance);
gs_export extern bool get_self_intersection(float ts[2], const vec2& a, const vec2& b, const vec2& c, const vec2& d);
gs_export extern void split_quad_bezier(vec2 c[5], const vec2 p[3], float t);
gs_export extern void split_quad_bezier(vec2 c[7], const vec2 p[3], float t1, float t2);
gs_export extern void split_cubic_bezier(vec2 c[7], const vec2 p[4], float t);
gs_export extern void split_cubic_bezier(vec2 c[10], const vec2 p[4], float t1, float t2);
gs_export extern void offset_cubic_bezier(vec2 c[4], const vec2 p[4], float d);
gs_export extern float quad_bezier_length(const vec2 cp[3]);
gs_export extern float cubic_bezier_length(const vec2 cp[4], float tolerance);
gs_export extern int cubic_to_quad_bezier(vector<vec2>& quadctl, const vec2 cp[4], float tolerance);
gs_export extern float quad_control_length(const vec2& a, const vec2& b, const vec2& c);
gs_export extern float cubic_control_length(const vec2& a, const vec2& b, const vec2& c, const vec2& d);
gs_export extern float point_line_distance(const vec2& p, const vec2& p1, const vec2& p2);
gs_export extern int get_interpolate_step(const vec2& a, const vec2& b, const vec2& c, float step_len = -1.f);
gs_export extern int get_interpolate_step(const vec2& a, const vec2& b, const vec2& c, const vec2& d, float step_len = -1.f);
gs_export extern int get_rough_interpolate_step(const vec2& a, const vec2& b, const vec2& c);
gs_export extern int get_rough_interpolate_step(const vec2& a, const vec2& b, const vec2& c, const vec2& d);
gs_export extern float get_cubic_klmcoords(vec3 m[4], const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4,
    const vec2& ac1, const vec2& ac2, const vec2& ac3, const vec2& ac4  /* absolute coord for error control */
    );
gs_export extern float get_include_angle(const vec2& d1, const vec2& d2);
gs_export extern float get_include_angle(const vec2& a, const vec2& b, const vec2& c);
gs_export extern float get_rotate_angle(const vec2& d1, const vec2& d2);
gs_export extern float get_rotate_angle(const vec2& a, const vec2& b, const vec2& c);
gs_export extern void get_reduce_point(vec2& p, const vec2& a, const vec2& b, const vec2& c, const vec2& d);
gs_export extern bool point_in_triangle(const vec2& p, const vec2& p1, const vec2& p2, const vec2& p3);
gs_export extern int point_in_polygon(const vec2& p, const vector<vec2>& poly);   /* 0: outside; 1 : inside; -1 : coincide */
gs_export extern float calc_triangle_area(const vec2& p1, const vec2& p2, const vec2& p3);
gs_export extern double hp_calc_triangle_area(const vec2& p1, const vec2& p2, const vec2& p3);
gs_export extern vec3 get_barycentric_coords(const vec2& p, const vec2& p1, const vec2& p2, const vec2& p3);
gs_export extern double calc_polygon_area(const vec2 p[], int size);
gs_export extern bool clip_line_rect(float t[2], const vec2& p1, const vec2& p2, const rect& clipbox);
gs_export extern bool clip_line_rectf(float t[2], const vec2& p1, const vec2& p2, const rectf& clipbox);
gs_export extern void trace_quad_strip(const vec2 cp[], int size);
gs_export extern void trace_cubic_strip(const vec2 cp[], int size);

inline bool is_isosigned(float a, float b) { return (int)((*(uint*)&a ^ *(uint*)&b)) >= 0; }
inline bool is_isosigned(double a, double b) { return (long long)((*(unsigned long long*)&a ^ *(unsigned long long*)&b)) >= 0; }
inline bool fuzzy_zero(float f) { return (f <= 1e-4f && f >= -1e-4f); }
inline bool fuzzy_zero(float f, float tol) { return f <= tol && f >= -tol; }
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

__gslib_end__

#endif
