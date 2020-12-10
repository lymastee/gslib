/*
 * Copyright (c) 2016-2020 lymastee, All rights reserved.
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

#include <gslib/error.h>
#include <ariel/mesh.h>

__ariel_begin__

mesh::mesh()
{
    _transforms.identity();
}

void mesh::calc_normals()
{
    /* set all the normals to 0 */
    for(auto& v : _normals)
        v = vec3(0.f, 0.f, 0.f);
    /* calculate normals for each face, normalize it, add it up to the normals of each point. */
    assert(_faces.size() % 3 == 0);
    for(size_t m = 0; m < _faces.size(); m += 3) {
        auto i = _faces.at(m);
        auto j = _faces.at(m + 1);
        auto k = _faces.at(m + 2);
        auto& v1 = _points.at(i);
        auto& v2 = _points.at(j);
        auto& v3 = _points.at(k);
        vec3 normal;
        normal.cross(vec3().sub(v2, v1), vec3().sub(v3, v2)).normalize();
        _normals.at(i) += normal;
        _normals.at(j) += normal;
        _normals.at(k) += normal;
    }
    /* normalize each of the normals */
    for(auto& v : _normals)
        v.normalize();
}

__ariel_end__
