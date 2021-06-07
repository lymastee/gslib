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

#include <ariel/smaa.h>

#if use_rendersys_d3d_11
#include <ariel/smaad3d11.cpp>
#endif

__ariel_begin__

smaa::smaa(rendersys* rsys, int w, int h)
{
    assert(rsys);
    _width = w;
    _height = h;
    setup(rsys);
    prepare_render_targets(rsys, w, h);
}

smaa::~smaa()
{
    destroy_miscs();
    /* destroy render targets */
    delete _edges_rt;
    delete _blend_rt;
    _edges_rt = _blend_rt = nullptr;
}

void smaa::setup(rendersys* rsys)
{
    assert(rsys);
    initialize(rsys);
}

__ariel_end__
