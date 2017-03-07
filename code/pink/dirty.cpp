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

#include <pink/dirty.h>

__pink_begin__

#define max_dirty   64

dirty_list::dirty_list()
{
    _width = 0;
    _height = 0;
    _whole = false;
    _cap = max_dirty;
}

dirty_list::dirty_list(int w, int h)
{
    _width = w;
    _height = h;
    _whole = false;
    _cap = max_dirty;
}

void dirty_list::clear()
{
    _rclist.clear();
    _whole = false;
}

void dirty_list::set_dimension(int w, int h)
{
    _width = w;
    _height = h;
    _whole = true;
}

void dirty_list::add(rect rc)
{
    if(_whole || rc.left >= _width || rc.top >= _height ||
        rc.left >= rc.right || rc.top >= rc.bottom
        )
        return;

    if((int)_rclist.size() == _cap) {
        _whole = true;
        return;
    }

    if(rc.right > _width)
        rc.right = _width;
    if(rc.bottom > _height)
        rc.bottom = _height;

    for(iterator i = _rclist.begin(); i != _rclist.end(); ++ i) {
        /* inside the rect */
        if(rc.left >= i->left && rc.top >= i->top && rc.right <= i->right &&
            rc.bottom <= i->bottom
            )
            return;
        /* intersect */
        if(is_rect_intersected(rc, *i)) {
            rect rc1;
            union_rect(rc1, rc, *i);
            _rclist.erase(i);
            add(rc1);
            return;
        }
    }

    /* no intersection */
    _rclist.push_back(rc);
}

bool dirty_list::is_dirty(const rect& rc) const
{
    if(rc.left >= _width || rc.top >= _height || rc.left >= rc.right ||
        rc.top >= rc.bottom
        )
        return false;

    if(_whole)
        return true;

    for(const_iterator i = _rclist.begin(); i != _rclist.end(); ++ i) {
        if(is_rect_intersected(rc, *i))
            return true;
    }

    return false;
}

__pink_end__
