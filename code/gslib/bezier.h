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

#ifndef bezier_f48ccc1f_1db9_403f_8be0_d44f5cbfc6dc_h
#define bezier_f48ccc1f_1db9_403f_8be0_d44f5cbfc6dc_h

#include <assert.h>
#include <gslib/type.h>
#include <gslib/graphic.h>

__gslib_begin__

template<class _oprt, int _order>
struct _bezier_factor
{
    _oprt   factor[_order+1];
    _oprt   ttable1[_order], ttable2[_order];

    static void normalize(_oprt ttable[], _oprt t)
    {
        /* the start */
        ttable[0] = t;
        normalize<_order-1>(++ ttable, t);
    }
    template<int _order>
    static void normalize(_oprt ttable[], _oprt t)
    {
        ttable[0] = ttable[-1] * t;
        normalize<_order-1>(++ ttable, t);
    }
    template<>
    static void normalize<0>(_oprt ttable[], _oprt t)
    {
        /* the end */
    }
    static int get_size() { return _order + 1; }
    /* original entry */
    void get_factor(_oprt t)
    {
        assert(t >= (_oprt)0 && t < (_oprt)1);
        _oprt t2 = (_oprt)1 - t;
        normalize(ttable1, t);
        normalize(ttable2, t2);
		assert(_order > 1);
		int i = _order >> 1;
		int cmlt = _order << (i - 1);
		if(_order % 2 == 0)
			factor[i] = (_oprt)cmlt * ttable2[i-1] * ttable1[i-1];
		calc_factor< ((_order-1)>>1), (_order<<(((_order-1)>>1)-1)) >();
    }
    template<int _idx, int _cmlt>
    void calc_factor()
    {
		int i = _order - _idx;
		factor[_idx] = (_oprt)_cmlt * ttable1[_idx-1] * ttable2[i];
		factor[i] = (_oprt)_cmlt * ttable1[i] * ttable2[_idx-1];
		calc_factor<_idx-1, (_cmlt>>1)>();
    }
    template<>
    void calc_factor<0, 1>()
    {
		factor[0] = ttable2[_order-1];
		factor[_order] = ttable1[_order-1];
		/* the end */
    }
};

template<class _oprt>
struct _bezier_factor<_oprt, 1>
{
};

template<class _oprt>
struct _bezier_factor<_oprt, 2>
{
};

/*
 * It seems like a beautiful solution to make the bezier factors, but we would need a powerful
 * compiler to support it, much more powerful then visual c++...
 */

template<class _oprt, int _order>
struct bezier_factor
{
    _oprt   factor[_order+1];
    _oprt   ttable1[_order], ttable2[_order];

    void normalize(_oprt t)
    {
        assert(t >= (_oprt)0 && t < (_oprt)1);
        _oprt t2 = (_oprt)1 - t;
        _oprt *ptr1 = ttable1, *ptr2 = ttable2;
        ptr1[0] = t;
        ptr2[0] = t2;
        for(int i = 1; i < _order; i ++) {
            ptr1[i] = ptr1[i-1] * t;
            ptr2[i] = ptr2[i-1] * t2;
        }
    }
    void get_factor(_oprt t)
    {
        normalize(t);
        assert(_order > 2);
        int i = _order >> 1;
        int cmlt = _order << (i - 1);
        _oprt *ptr1 = ttable1, *ptr2 = ttable2;
        if(_order % 2 == 0) {
            factor[i] = ptr2[i-1] * ptr1[i-1] * cmlt;
            i --;
            cmlt >>= 1;
        }
        for( ; i > 0; i --, cmlt >>= 1) {
            int i1 = _order - i;
            int p1 = i - 1, p2 = i1 - 1;
            factor[i] = ptr1[p1] * ptr2[p2] * cmlt;
            factor[i1] = ptr1[p2] * ptr2[p1] * cmlt;
        }
        factor[0] = ptr2[_order-1];
        factor[_order] = ptr1[_order-1];
    }
    static int get_size()
    {
        return _order + 1;
    }
};

template<class _oprt>
struct bezier_factor<_oprt, 1>
{
    _oprt   factor[2];

    static int get_size() { return 2; }
    void get_factor(_oprt t)
    {
        assert(t >= (_oprt)0 && t < (_oprt)1);
        factor[0] = (_oprt)1 - t;
        factor[1] = t;
    }
};

template<class _oprt>
struct bezier_factor<_oprt, 2>
{
    _oprt   factor[3];

    static int get_size() { return 3; }
    void get_factor(_oprt t)
    {
        assert(t >= (_oprt)0 && t < (_oprt)1);
        factor[0] = (_oprt)1 - t;
        factor[1] = factor[0];
        factor[0] *= factor[0];
        factor[1] *= t;
        factor[1] *= 2;
        factor[2] = t * t;
    }
};

template<class _oprt, int _order>
struct bezier_tool
{
    point   pt[_order+1];

    void set_point(const point p[], int s)
    {
        assert(s == _order+1);
        memcpy_s(pt, sizeof(pt), p, sizeof(pt));
    }
    point get_point(_oprt t)
    {
        bezier_factor<_oprt, _order> bfct;
        bfct.get_factor(t);
        _oprt x = 0, y = 0;
        for(int i = 0; i < _order+1; i ++) {
            x += bfct.factor[i] * pt[i].x;
            y += bfct.factor[i] * pt[i].y;
        }
        return point((int)x, (int)y);
    }
};

/* n-order version */

template<class _oprt>
struct bezier_factor_n
{
    _oprt*  factor;
    int     order;

    bezier_factor_n(int o)
    {
        assert(o > 0);
        order = o;
        factor = new _oprt[o+1];
        assert(factor);
    }
    ~bezier_factor_n()
    {
        if(factor) {
            delete [] factor;
            factor = 0;
        }
    }
    int get_size() const
    {
        return order + 1;
    }
    void get_factor(_oprt t)
    {
        assert(t >= (_oprt)0 && t < (_oprt)1);
        if(order == 0) {
            /* never = 0 */
            assert(0);
        }
        else if(order == 1) {
            factor[0] = (_oprt)1 - t;
            factor[1] = t;
        }
        else if(order == 2) {
            factor[0] = (_oprt)1 - t;
            factor[1] = factor[0];
            factor[0] *= factor[0];
            factor[1] *= t;
            factor[1] *= 2;
            factor[2] = t * t;
        }
        else {
            _oprt *ttable1 = new _oprt[order],
                *ttable2 = new _oprt[order];
            _oprt t2 = (_oprt)1 - t;
            _oprt *ptr1 = ttable1, *ptr2 = ttable2;
            ptr1[0] = t;
            ptr2[0] = t2;
            for(int i = 1; i < order; i ++) {
                ptr1[i] = ptr1[i-1] * t;
                ptr2[i] = ptr2[i-1] * t2;
            }
            int i = order >> 1;
            int cmlt = order << (i - 1);
            ptr1 = ttable1, ptr2 = ttable2;
            if(order % 2 == 0) {
                factor[i] = ptr2[i-1] * ptr1[i-1] * cmlt;
                i --;
                cmlt >>= 1;
            }
            for( ; i > 0; i --, cmlt >>= 1) {
                int i1 = order - i;
                int p1 = i - 1, p2 = i1 - 1;
                factor[i] = ptr1[p1] * ptr2[p2] * cmlt;
                factor[i1] = ptr1[p2] * ptr2[p1] * cmlt;
            }
            factor[0] = ptr2[order-1];
            factor[order] = ptr1[order-1];
            /* donot forget to deallocate */
            delete [] ttable1;
            delete [] ttable2;
        }
    }
};

template<class _oprt>
struct bezier_tool_n:
    public bezier_factor_n<_oprt>
{
    point*  pt;

    bezier_tool_n(int o):
        bezier_factor_n<_oprt>(o)
    {
        assert(o > 0);
        pt = new point[o+1];
        assert(pt);
    }
    ~bezier_tool_n()
    {
        if(pt) {
            delete [] pt;
            pt = 0;
        }
    }
    void set_point(const point p[], int s)
    {
        assert(s == order+1);
        int size = sizeof(point) * (order+1);
        memcpy_s(pt, size, p, size);
    }
    point get_point(_oprt t)
    {
        get_factor(t);
        _oprt x = 0, y = 0;
        int cap = order + 1;
        for(int i = 0; i < cap; i ++) {
            x += factor[i] * pt[i].x;
            y += factor[i] * pt[i].y;
        }
        return point((int)x, (int)y);
    }
};

__gslib_end__

#endif
