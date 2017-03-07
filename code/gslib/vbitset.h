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

#ifndef vbitset_deeaeda9_1fc9_43e4_9026_c5eeb93c6ebb_h
#define vbitset_deeaeda9_1fc9_43e4_9026_c5eeb93c6ebb_h

#include <intrin.h>
#include <gslib/config.h>
#include <gslib/std.h>

__gslib_begin__

inline int bit_count(uint n)
{
    uint t = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111);
    return ((t + (t >> 3)) & 030707070707) % 63;
}

inline uint bitelim_first_nonzero(uint n) { return n & (n - 1); }
inline uint bitmask_first_nonzero(uint n) { return n ^ (n - 1); }

inline int bitscan_forward(uint n)
{
    uint p;
    return _BitScanForward((unsigned long*)&p, n) ? (int)p : -1;
}

inline int bitscan_reverse(uint n)
{
    uint p;
    return _BitScanReverse((unsigned long*)&p, n) ? (int)p : -1;
}

class vbitset
{
public:
    typedef vector<uint> container;
    static const uint knot_size = sizeof(uint) * 8;

protected:
    container           _container;
    int                 _size;

public:
    vbitset() { _size = 0; }
    int knot_count() const { return (int)_container.size(); }
    uint knot_at(int i) const
    {
        assert(i < knot_count());
        return _container.at(i);
    }
    void set_knot(int p, uint k)
    {
        if(p >= knot_count()) {
            assert(!"unexpected.");
            return;
        }
        _container.at(p) = k;
    }
    uint& ref_knot_at(int i)
    {
        assert(i < knot_count());
        return _container.at(i);
    }
    int capacity() const { return knot_size * knot_count(); }
    int size() const { return _size; }
    int length() const { return _size; }
    void set(int pos) { reset(pos, true); }
    void reset(int pos, bool b = false)
    {
        int cc = cycle_count(pos);
        int cidx = cycle_index(pos);
        ensure_container_capacity(pos, cc, cidx);
        uint& n = _container.at(cc);
        uint mask = (1 << cidx);
        b ? (n |= mask) : (n &= ~mask);
    }
    void reset(bool b)
    {
        int cc = cycle_count(_size);
        int cidx = cycle_index(_size);
        int m = b ? -1 : 0;
        memset(&_container.front(), m, cc * sizeof(uint));
        uint mask = (1 << (cidx + 1)) - 1;
        uint& n = _container.at(cc);
        b ? (n |= mask) : (n &= ~mask);
    }
    bool test(int pos)
    {
        if(pos >= _size)
            return false;
        int cc = cycle_count(pos);
        int cidx = cycle_index(pos);
        uint n = knot_at(cc);
        uint mask = (1 << cidx);
        return (n & mask) != 0;
    }
    void resize(int s)
    {
        if(s == 0) {
            _container.clear();
            _size = 0;
            return;
        }
        if(s == _size)
            return;
        else if(s < _size) {
            ensure_container_capacity(s - 1);
            return;
        }
        else {
            int cc = cycle_count(s - 1);
            _container.resize(cc + 1);
            _size = s;
        }
    }
    void rotate_left(int bias)
    {
        if(!bias)
            return;
        if(bias < 0)
            return rotate_right(-bias);
        bias %= _size;
        if(!bias)
            return;
        int cc = cycle_count(bias);
        int cm = cycle_index(bias);
        rotate_left(cc, cm);
    }
    void rotate_right(int bias)
    {
        if(!bias)
            return;
        if(bias < 0)
            return rotate_left(-bias);
        bias %= _size;
        if(!bias)
            return;
        int cc = cycle_count(bias);
        int cm = cycle_index(bias);
        rotate_right(cc, cm);
    }
    // todo: following
    void rotate_left(int cc, int cm)
    {
        assert(cc >= 0 && cm >= 0);
        if(!cm) {
            uint* ptr = new uint[cc];
            int crst = (int)_container.size() - cc;
            memcpy_s(ptr, sizeof(uint) * cc, &_container.front(), _container.capacity());
            memcpy_s(&_container.front(), sizeof(uint) * crst, &_container.at(cc), sizeof(uint) * crst);
            memcpy_s(&_container.front(), sizeof(uint) * cc, ptr, sizeof(uint) * cc);
            delete [] ptr;
        }
        else {

        }
    }
    void rotate_right(int cc, int cm)
    {
        assert(cc >= 0 && cm >= 0);
    }
    void shift_left(int bias);
    void shift_right(int bias);
    void shift_left_1(int bias);
    void shift_right_1(int bias);

public:
    static int cycle_count(int p) { return p / knot_size; }
    static int cycle_index(int p) { return p % knot_size; }

protected:
    void ensure_container_capacity(int pos)
    {
        if(pos < _size)
            return;
        int cc = cycle_count(pos);
        int cidx = cycle_index(pos);
        ensure_container_capacity(pos, cc, cidx);
    }
    void ensure_container_capacity(int pos, int cc, int cidx)
    {
        assert(cc * knot_size + cidx == pos);
        if(pos < _size)
            return;
        int kc = knot_count();
        assert(kc <= cc);
        if(kc == cc) {
            _container.push_back(0);
            _size = pos + 1;
        }
        else {
            cc ++;
            _container.resize(cc);
            int delta = cc - kc;
            memset(&_container.at(kc), 0, delta * sizeof(uint));
            _size = pos + 1;
        }
    }
};

__gslib_end__

#endif
