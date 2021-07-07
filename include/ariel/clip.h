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

typedef _treenode_wrapper<painter_path> clip_result_wrapper;
typedef tree<painter_path, clip_result_wrapper> clip_result;
typedef typename clip_result::iterator clip_result_iter;
typedef typename clip_result::const_iterator clip_result_const_iter;

extern void clip_simplify(painter_linestrips& lss, const painter_linestrip& input);
extern void clip_simplify(painter_linestrips& lss, const painter_linestrips& input);
extern void clip_simplify(clip_result& output, const painter_path& input);
extern void clip_union(clip_result& output, const painter_path& subjects, const painter_path& clips);
extern void clip_intersect(clip_result& output, const painter_path& subjects, const painter_path& clips);
extern void clip_substract(clip_result& output, const painter_path& subjects, const painter_path& clips);
extern void clip_exclude(clip_result& output, const painter_path& subjects, const painter_path& clips);
extern void clip_convert(painter_path& path, const clip_result& result);
//extern void clip_convert(clip_result2& result, const painter_path& path);

__ariel_end__

#endif
