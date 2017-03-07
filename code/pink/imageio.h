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

#ifndef imageio_be8e9f82_90f9_431f_a329_98041a02fbb1_h
#define imageio_be8e9f82_90f9_431f_a329_98041a02fbb1_h

#include <gslib/type.h>
#include <gslib/string.h>
#include <pink/config.h>

__pink_begin__

class image;

enum image_format_type
{
    image_format_unknown,
    image_format_bmp,
    image_format_png,
    image_format_jpg,
};

class imageio
{
public:
    static bool read_image(image& img, const string& path);
    static bool read_bmp_image(image& img, const void* ptr, int size);
    static bool save_bmp_image(image& img, const string& path);
    static bool read_png_image(image& img, const void* ptr, int size);
};

__pink_end__

#endif
