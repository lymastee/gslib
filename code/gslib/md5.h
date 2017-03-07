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

#ifndef md5_3573c73e_b471_4b88_be1e_4be40dd8e24f_h
#define md5_3573c73e_b471_4b88_be1e_4be40dd8e24f_h

#include <gslib/type.h>
#include <gslib/string.h>

__gslib_begin__

class md5
{
public:
    md5() { reset(); }
    md5(const void* src, int len)
    {
        reset();
        update((const byte*)src, len);
        finalize();
    }
    void reset();
    void update(const byte* src, int len);
    const byte* get_digest() const { return _digest; }
    void get_string(string& str) const;

protected:
    void finalize();
    void transform(const byte block[64]);
    void encode(const uint* src, byte* des, int len);
    void decode(const byte* src, uint* des, int len);

private:
    md5(const md5&);
    md5& operator=(const md5&);

protected:
    uint        _state[4];
    uint        _count[2];
    byte        _buffer[64];
    byte        _digest[16];
};

__gslib_end__

#endif
