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

#ifndef dirty_08c91b95_68e8_437c_b721_3ad67fdc25ac_h
#define dirty_08c91b95_68e8_437c_b721_3ad67fdc25ac_h

#include <gslib/std.h>
#include <pink/type.h>

__pink_begin__

class dirty_list
{
public:
    typedef list<rect> rect_list;
    typedef rect_list::iterator iterator;
    typedef rect_list::const_iterator const_iterator;

protected:
    int         _width, _height;
    int         _cap;
    bool        _whole;

private:
    rect_list   _rclist;

public:
    dirty_list();
    dirty_list(int w, int h);
    int size() const { return (int)_rclist.size(); }
    void clear();
    void set_dimension(int w, int h);
    void add(rect rc);
    void set_whole() { _whole = true;}
    bool is_whole() const { return _whole; }
    bool is_dirty(const rect& rc) const;
    iterator begin() { return _rclist.begin(); }
    const_iterator begin() const { return _rclist.begin(); }
    iterator end() { return _rclist.end(); }
    const_iterator end() const { return _rclist.end(); }
};

__pink_end__

#endif
