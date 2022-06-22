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

#ifndef clip_c55b09bc_5fbd_463f_8d45_b12013b57f42_h
#define clip_c55b09bc_5fbd_463f_8d45_b12013b57f42_h

#include <gslib/tree.h>
#include <ariel/painterpath.h>

__ariel_begin__

enum clip_point_tag
{
    cpt_corner = 0x01,
    cpt_interpolate = 0x02,
};

enum clip_fill_type
{
    cft_even_odd = 0,
    cft_non_zero,
    cft_positive,
    cft_negative,
};

struct clip_vec2_hash
{
    size_t operator()(const vec2& pt) const { return hash_bytes((const byte*)&pt, sizeof(pt)); }
};

typedef _treenode_wrapper<painter_path> clip_result_wrapper;
typedef tree<painter_path, clip_result_wrapper> clip_result;
typedef typename clip_result::iterator clip_result_iter;
typedef typename clip_result::const_iterator clip_result_const_iter;
typedef unordered_map<vec2, uint, clip_vec2_hash> clip_point_attr;

ariel_export extern void clip_remap_points(painter_linestrips& output, clip_point_attr& attrmap, const painter_path& input, uint attr_selector, float step_len = -1.f);
ariel_export extern void clip_offset(painter_linestrips& lss, const painter_linestrips& input, float offset, float miter_limit = 2.f);
ariel_export extern void clip_simplify(painter_linestrips& lss, const painter_linestrip& input, clip_fill_type ft = cft_even_odd);
ariel_export extern void clip_simplify(painter_linestrips& lss, const painter_linestrips& input, clip_fill_type ft = cft_even_odd);
ariel_export extern void clip_simplify(clip_result& output, const painter_path& input);
ariel_export extern void clip_union(clip_result& output, const painter_path& subjects, const painter_path& clips);
ariel_export extern void clip_intersect(clip_result& output, const painter_path& subjects, const painter_path& clips);
ariel_export extern void clip_substract(clip_result& output, const painter_path& subjects, const painter_path& clips);
ariel_export extern void clip_exclude(clip_result& output, const painter_path& subjects, const painter_path& clips);
ariel_export extern void clip_convert(painter_path& path, const clip_result& result);

__ariel_end__

#endif
