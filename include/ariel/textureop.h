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

#ifndef textureop_1ee1bc81_29bb_4578_9534_56f175127289_h
#define textureop_1ee1bc81_29bb_4578_9534_56f175127289_h

#include <ariel/rendersys.h>

__ariel_begin__

class textureop
{
public:
    struct cb_configs
    {
        int             x, y, reserved1, reserved2;
        vec4            cr;
        float           arg;
    };

public:
    textureop(rendersys* rsys);
    void copy_texture_rect(render_texture2d* dest, render_texture2d* src, const rectf& rc);
    void initialize_texture_rect(unordered_access_view* dest, const color& cr, const rectf& rc);
    void transpose_texture_rect(unordered_access_view* dest, render_texture2d* src, const rectf& rc);
    void set_brightness(unordered_access_view* dest, render_texture2d* src, float s);
    void set_gray(unordered_access_view* dest, render_texture2d* src);
    void set_fade(unordered_access_view* dest, render_texture2d* src, float s);
    void set_inverse(unordered_access_view* dest, render_texture2d* src);
    void initialize_texture_rect(render_texture2d* dest, const color& cr, const rectf& rc);
    void transpose_texture_rect(render_texture2d* dest, render_texture2d* src, const rectf& rc);
    void set_brightness(render_texture2d* dest, render_texture2d* src, float s);
    void set_gray(render_texture2d* dest, render_texture2d* src);
    void set_fade(render_texture2d* dest, render_texture2d* src, float s);
    void set_inverse(render_texture2d* dest, render_texture2d* src);

public:
    static void get_texture_dimension(render_texture2d* p, int& w, int& h);
    static void get_assoc_device(render_texture2d* p, render_device** ppdev);
    static bool convert_to_image(image& img, render_texture2d* src);

protected:
    rendersys*          _rsys;

private:
    render_device* get_device() const;
    render_context* get_immediate_context() const;
    render_constant_buffer* get_constant_buffer() const;
    template<class _cls>
    bool check_valid_device(_cls* p) const;
};

__ariel_end__

#endif
