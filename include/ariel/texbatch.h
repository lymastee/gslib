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

#ifndef texbatch_dfe6f3b4_430b_4424_98b6_d6b23235dc22_h
#define texbatch_dfe6f3b4_430b_4424_98b6_d6b23235dc22_h

#include <gslib/std.h>
#include <ariel/type.h>
#include <ariel/image.h>
#include <ariel/rectpack.h>
#include <ariel/rendersys.h>

__ariel_begin__

class tex_batcher
{
public:
    typedef unordered_map<const image*, rectf> location_map;

public:
    tex_batcher();
    bool is_empty() const { return _rect_packer.is_empty(); }
    float get_width() const { return _rect_packer.get_width() + _gap; }
    float get_height() const { return _rect_packer.get_height() + _gap; }
    void add_image(const image* p);
    void arrange();
    const location_map& get_location_map() const { return _location_map; }
    render_texture2d* create_texture(rendersys* rsys) const;

protected:
    rect_packer         _rect_packer;
    location_map        _location_map;
    float               _gap;

private:
    void prepare_input_list(rp_input_list& inputs);
};

__ariel_end__

#endif
