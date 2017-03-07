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

#ifndef polygon_9cb4a5c6_8903_42fd_9232_eb81bfcd30a5_h
#define polygon_9cb4a5c6_8903_42fd_9232_eb81bfcd30a5_h

#include <gslib/std.h>
#include <gslib/math.h>
#include <ariel/config.h>

__ariel_begin__

class polygon
{
public:
    typedef gs::vector<point3> vtstream;
    typedef gs::vector<int> idstream;
    typedef vtstream::iterator vtiter;
    typedef vtstream::const_iterator const_vtiter;
    typedef idstream::iterator iditer;
    typedef idstream::const_iterator const_iditer;

public:
    polygon();
    int get_vertex_count() const { return (int)_vtstream.size(); }
    int get_index_count() const { return (int)_idstream.size(); }
    int get_face_count() const { return _faces; }
    int query_face_count();
    void write_vtstream(int start, int cnt, const point3* src);
    void append_vtstream(int cnt, const point3* src) { write_vtstream(get_vertex_count(), cnt, src); }
    void append_vtstream(const point3& src) { _vtstream.push_back(src); }
    void write_idstream(int start, int cnt, const int* src);
    void append_idstream(int cnt, const int* src) { write_idstream(get_index_count(), cnt, src); }
    void append_idstream(int idx) { _idstream.push_back(idx); }
    point3& get_vertex(int i) { return _vtstream.at(i); }
    const point3& get_vertex(int i) const { return _vtstream.at(i); }
    int& get_index(int i) { return _idstream.at(i); }
    const int& get_index(int i) const { return _idstream.at(i); }

protected:
    vtstream    _vtstream;
    idstream    _idstream;
    int         _faces;
};

__ariel_end__

#endif
