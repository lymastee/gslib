/*
 * Copyright (c) 2016-2018 lymastee, All rights reserved.
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

#include <ariel/imagefx.h>

__ariel_begin__

static void adjust_brightness(color& original, float scale)
{
    assert(scale >= 0 && scale < 2.0f);
    if(scale == 1.0f);
    else if(scale > 1.0f) {
        scale -= 1.0f;
        int r = 0xff - original.red;
        float rf = (float)r * scale;
        original.red += (int)rf;
        r = 0xff - original.green;
        rf = (float)r * scale;
        original.green += (int)rf;
        r = 0xff - original.blue;
        rf = (float)r * scale;
        original.blue += (int)rf;
    }
    else {
        float r = (float)original.red * scale;
        original.red = (int)r;
        r = (float)original.green * scale;
        original.green = (int)r;
        r = (float)original.blue * scale;
        original.blue = (int)r;
    }
}

void imagefx::set_brightness(image& img, const image& isrc, float s)
{
    if(img.get_format() != image::fmt_rgba)
        return;
    int w = img.get_width(), h = img.get_height();
    if(w < isrc.get_width() || h < isrc.get_height())
        return;
    for(int j = 0; j < h; j ++) {
        for(int i = 0; i < w; i ++) {
            color* dest = reinterpret_cast<color*>(img.get_data(i, j));
            const color* src = reinterpret_cast<const color*>(isrc.get_data(i, j));
            assert(dest && src);
            *dest = *src;
            adjust_brightness(*dest, s);
        }
    }
}

void imagefx::set_gray(image& img, const image& isrc)
{
    if(img.get_format() != image::fmt_rgba)
        return;
    int w = img.get_width(), h = img.get_height();
    if(w < isrc.get_width() || h < isrc.get_height())
        return;
    for(int j = 0; j < h; j ++) {
        for(int i = 0; i < w; i ++) {
            color* dest = reinterpret_cast<color*>(img.get_data(i, j));
            const color* src = reinterpret_cast<const color*>(isrc.get_data(i, j));
            assert(dest && src);
            int av = src->red + src->green + src->blue;
            av /= 3;
            dest->red = dest->green = dest->blue = av;
            dest->alpha = src->alpha;
        }
    }
}

__ariel_end__
