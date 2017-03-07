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

#ifndef mclass_109a4180_a67d_47da_b8ab_2bb1c667bbfe_h
#define mclass_109a4180_a67d_47da_b8ab_2bb1c667bbfe_h

#include <gslib/type.h>

__gslib_begin__

template<class _tag, class _c1, class _c2>
class multi_class
{
public:
    typedef _tag tag;
    typedef _c1 class1;
    typedef _c2 class2;

public:
    multi_class(tag t) { initialize(t); }
    ~multi_class() { destroy(); }
    tag get_tag() const { return _tag; }
    explicit multi_class(const multi_class& that)
    {
        auto t = that.get_tag();
        _tag = t;
        switch(t)
        {
        case 0:
            new (_holder) class1(that.to_const_class1());
            break;
        case 1:
            new (_holder) class2(that.to_const_class2());
            break;
        default:
            assert(!"unexpected tag for multi_class.");
            return;
        }
    }
    class1& to_class1()
    {
        assert(_tag == 0);
        return *reinterpret_cast<class1*>(_holder);
    }
    class2& to_class2()
    {
        assert(_tag == 1);
        return *reinterpret_cast<class2*>(_holder);
    }
    const class1& to_const_class1() const
    {
        assert(_tag == 0);
        return *reinterpret_cast<const class1*>(_holder);
    }
    const class2& to_const_class2() const
    {
        assert(_tag == 1);
        return *reinterpret_cast<const class2*>(_holder);
    }

protected:
    tag                 _tag;
    byte                _holder[max_sizeof(class1, class2)];

protected:
    void initialize(tag t)
    {
        _tag = t;
        switch(t)
        {
        case 0:
            new (_holder) class1();
            break;
        case 1:
            new (_holder) class2();
            break;
        }
    }
    void destroy()
    {
        switch(_tag)
        {
        case 0:
            to_class1().~class1();
            break;
        case 1:
            to_class2().~class2();
            break;
        }
    }
};

template<class _tag, class _c1, class _c2, class _c3>
class multi_class_3
{
public:
    typedef _tag tag;
    typedef _c1 class1;
    typedef _c2 class2;
    typedef _c3 class3;

public:
    multi_class_3(tag t) { initialize(t); }
    ~multi_class_3() { destroy(); }
    tag get_tag() const { return _tag; }
    explicit multi_class_3(const multi_class_3& that)
    {
        auto t = that.get_tag();
        _tag = t;
        switch(t)
        {
        case 0:
            new (_holder) class1(that.to_const_class1());
            break;
        case 1:
            new (_holder) class2(that.to_const_class2());
            break;
        case 2:
            new (_holder) class3(that.to_const_class3());
            break;
        default:
            assert(!"unexpected tag for multi_class.");
            return;
        }
    }
    class1& to_class1()
    {
        assert(_tag == 0);
        return *reinterpret_cast<class1*>(_holder);
    }
    class2& to_class2()
    {
        assert(_tag == 1);
        return *reinterpret_cast<class2*>(_holder);
    }
    class3& to_class3()
    {
        assert(_tag == 2);
        return *reinterpret_cast<class3*>(_holder);
    }
    const class1& to_const_class1() const
    {
        assert(_tag == 0);
        return *reinterpret_cast<const class1*>(_holder);
    }
    const class2& to_const_class2() const
    {
        assert(_tag == 1);
        return *reinterpret_cast<const class2*>(_holder);
    }
    const class3& to_const_class3() const
    {
        assert(_tag == 2);
        return *reinterpret_cast<const class3*>(_holder);
    }

protected:
    tag             _tag;
    byte            _holder[max_sizeof3(class1, class2, class3)];

protected:
    void initialize(tag t)
    {
        _tag = t;
        switch(t)
        {
        case 0:
            new (_holder) class1();
            break;
        case 1:
            new (_holder) class2();
            break;
        case 2:
            new (_holder) class3();
            break;
        }
    }
    void destroy()
    {
        switch(_tag)
        {
        case 0:
            to_class1().~class1();
            break;
        case 1:
            to_class2().~class2();
            break;
        case 2:
            to_class3().~class3();
            break;
        }
    }
};

/*
 * for more arguments and so on..
 * if you need a multi_class for more modes, complete it yourself.
 */

__gslib_end__

#endif
