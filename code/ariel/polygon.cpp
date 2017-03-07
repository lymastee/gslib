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

#include <ariel\polygon.h>

__ariel_begin__

polygon::polygon()
{
    _faces = 0;
}

int polygon::query_face_count()
{
    // to do:
    return _faces;
}

void polygon::write_vtstream(int start, int cnt, const point3* src)
{
    _vtstream.resize(start+cnt);
    point3* des = &(_vtstream.at(start));
    memcpy_s(des, sizeof(point3)*cnt, src, sizeof(point3)*cnt);
}

void polygon::write_idstream(int start, int cnt, const int* src)
{
    _idstream.resize(start+cnt);
    int* des = &(_idstream.at(start));
    memcpy_s(des, sizeof(int)*cnt, src, sizeof(int)*cnt);
    query_face_count();
}

__ariel_end__
