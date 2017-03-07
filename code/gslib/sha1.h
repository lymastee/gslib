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

#ifndef sha1_ae2a9a43_c706_4f24_b61f_c6a793f6bb11_h
#define sha1_ae2a9a43_c706_4f24_b61f_c6a793f6bb11_h

#include <gslib/config.h>
#include <gslib/type.h>
#include <gslib/string.h>

__gslib_begin__

class sha1
{
public:
    sha1() { reset(); }
    sha1(const void* src, int len)
    {
        reset();
        update((const byte*)src, len);
    }
    void reset();
    void update(const byte* src, int len);
    const uint* get_digest() const { return _digest; }
    void get_string(string& str) const;

private:
    sha1(const sha1&);
    sha1& operator=(const sha1&);

protected:
    uint        _digest[5];
};

__gslib_end__

#endif
