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

#ifndef linequ_546582d4_5f60_4938_9ce8_1beb1678ba61_h
#define linequ_546582d4_5f60_4938_9ce8_1beb1678ba61_h

#include <gslib/type.h>
#include <gslib/std.h>
#include <gslib/error.h>

__gslib_begin__

class _trait_dimcol {};
class _trait_dimrow {};

template<class _fty, class _mty, class _trdim>
class ledimsel
{
public:
    typedef _fty type;
    typedef _mty matrix;
    typedef ledimsel<_fty, _mty, _trdim> selection;

public:
    ledimsel(matrix& mat, int cr): _mat(mat), _sel(cr) {}
    int index() const { return _sel; }
    int capacity() const { return capacity<_trdim>(); }
    int cols() const { return _mat.cols(); }
    int rows() const { return _mat.rows(); }
    type& get(int i) { return get<_trdim>(i); }
    const type& get_const(int i) const { return get_const<_trdim>(i); }
    void set(int i, const type& t) { get(i) = t; }

protected:
    template<class _trdim>
    int capacity() const;
    template<class _trdim>
    type& get(int i);
    template<class _trdim>
    const type& get_const(int i) const;
    template<>
    int capacity<_trait_dimcol>() const { return rows(); }
    template<>
    int capacity<_trait_dimrow>() const { return cols(); }
    template<>
    type& get<_trait_dimcol>(int i) { return _mat.get(i, _sel); }
    template<>
    type& get<_trait_dimrow>(int i) { return _mat.get(_sel, i); }
    template<>
    const type& get_const<_trait_dimcol>(int i) const { return _mat.get_const(i, _sel); }
    template<>
    const type& get_const<_trait_dimrow>(int i) const { return _mat.get_const(_sel, i); }

protected:
    matrix&         _mat;
    int             _sel;

public:
    template<class _op>
    ledimsel& operator*=(_op t)
    {
        for_each([&t](type& n) { n *= t; });
        return *this;
    }
    template<class _op>
    ledimsel& operator/=(_op t) { return operator*=(t); }
    template<class _opset>
    ledimsel& operator+=(const _opset& rhs)
    {
        assert(capacity() == rhs.capacity());
        int cap = capacity();
        for(int i = 0; i < cap; i ++)
            get(i) += rhs.get_const(i);
        return *this;
    }
    template<class _opset>
    ledimsel& operator-=(const _opset& rhs)
    {
        assert(capacity() == rhs.capacity());
        int cap = capacity();
        for(int i = 0; i < cap; i ++)
            get(i) -= rhs.get_const(i);
        return *this;
    }

public:
    template<class _lambda>
    void for_each(_lambda lam)
    {
        int cap = capacity();
        for(int i = 0; i < cap; i ++)
            lam(get(i));
    }
    template<class _lambda>
    void const_for_each(_lambda lam) const
    {
        int cap = capacity();
        for(int i = 0; i < cap; i ++)
            lam(get_const(i));
    }
};

/* optimize for row selector */
template<class _fty, class _mty>
class ledimsel<_fty, _mty, _trait_dimrow>
{
public:
    typedef _fty type;
    typedef _mty matrix;
    typedef ledimsel<_fty, _mty, _trait_dimrow> selection;

public:
    ledimsel(matrix& mat, int cr): _mat(mat), _sel(cr), _data(mat.data() + mat.accpos(cr, 0)) {}
    int index() const { return _sel; }
    int capacity() const { return cols(); }
    int cols() const { return _mat.cols(); }
    int rows() const { return _mat.rows(); }
    type& get(int i) { return _data[i]; }
    const type& get_const(int i) const { return _data[i]; }
    void set(int i, const type& t) { get(i) = t; }

protected:
    matrix&         _mat;
    int             _sel;
    type*           _data;

public:
    template<class _op>
    ledimsel& operator*=(_op t)
    {
        for_each([&t](type& n) { n *= t; });
        return *this;
    }
    template<class _op>
    ledimsel& operator/=(_op t) { return operator*=(t); }
    template<class _opset>
    ledimsel& operator+=(const _opset& rhs)
    {
        assert(capacity() == rhs.capacity());
        int cap = capacity();
        for(int i = 0; i < cap; i ++)
            get(i) += rhs.get_const(i);
        return *this;
    }
    template<class _opset>
    ledimsel& operator-=(const _opset& rhs)
    {
        assert(capacity() == rhs.capacity());
        int cap = capacity();
        for(int i = 0; i < cap; i ++)
            get(i) -= rhs.get_const(i);
        return *this;
    }

public:
    template<class _lambda>
    void for_each(_lambda lam)
    {
        int cap = capacity();
        for(int i = 0; i < cap; i ++)
            lam(get(i));
    }
    template<class _lambda>
    void const_for_each(_lambda lam) const
    {
        int cap = capacity();
        for(int i = 0; i < cap; i ++)
            lam(get_const(i));
    }
};

template<class _opset1, class _opset2>
static void swap_opset(_opset1& ops1, _opset2& ops2)
{
    assert(ops1.capacity() == ops2.capacity());
    int cap = ops1.capacity();
    for(int i = 0; i < cap; i ++)
        std::swap(ops1.get(i), ops2.get(i));
}

template<class _opset>
static int first_non_zero(const _opset& ops)
{
    int cap = ops.capacity();
    for(int i = 0; i < cap; i ++) {
        if(ops.get_const(i) != 0)
            return i;
    }
    return cap;
}

template<class _opset>
static int last_non_zero(const _opset& ops)
{
    for(int i = ops.capacity() - 1; i >= 0; i --) {
        if(ops.get_const(i) != 0)
            return i;
    }
    return ops.capacity();
}

template<class _fty>
class levector
{
public:
    typedef _fty type;
    typedef levector<_fty> vector;

protected:
    int             _size;
    type*           _data;

public:
    levector() { _data = 0, _size = 0; }
    levector(int s)
    {
        assert(s > 0);
        _data = 0;
        setup(s);
    }
    ~levector()
    {
        if(_data) {
            delete [] _data;
            _data = 0;
        }
    }
    void setup(int s)
    {
        if(_data != 0)
            delete [] _data;
        _size = s;
        _data = new type[s];
    }
    int capacity() const { return _size; }
    type& get(int i) { return _data[i]; }
    const type& get_const(int i) const { return _data[i]; }
    type* data() const { return _data; }
    void set(int i, const type& t) { get(i) = t; }

public:
    template<class _lambda>
    void for_each(_lambda lam)
    {
        int cap = capacity();
        for(int i = 0; i < cap; i ++)
            lam(get(i));
    }
    template<class _lambda>
    void const_for_each(_lambda lam) const
    {
        int cap = capacity();
        for(int i = 0; i < cap; i ++)
            lam(get_const(i));
    }
};

template<class _fty>
class lematrix
{
public:
    typedef _fty type;
    typedef lematrix<_fty> matrix;
    typedef levector<_fty> vector;
    typedef ledimsel<_fty, lematrix, _trait_dimrow> rowselector;
    typedef ledimsel<_fty, const lematrix, _trait_dimrow> const_rowselector;
    typedef ledimsel<_fty, lematrix, _trait_dimcol> colselector;
    typedef ledimsel<_fty, const lematrix, _trait_dimcol> const_colselector;

protected:
    int             _rows;
    int             _cols;
    type*           _data;

public:
    lematrix()
    {
        _rows = _cols = 0;
        _data = 0;
    }
    lematrix(int r, int c)
    {
        assert(r > 0 && c > 0);
        _data = 0;
        set_dim(r, c);
    }
    ~lematrix()
    {
        if(_data != 0) {
            delete [] _data;
            _data = 0;
        }
    }
    void set_dim(int r, int c)
    {
        if(_data != 0)
            delete [] _data;
        assert(r > 0 && c > 0);
        _rows = r;
        _cols = c;
        _data = new type[r * c];
    }
    int accpos(int r, int c) const
    {
        assert(r >= 0 && r < _rows && c >= 0 && c < _cols);
        int p = r * _cols + c;
        return p;
    }
    type& get(int r, int c)
    {
        assert(r >= 0 && r < _rows && c >= 0 && c < _cols);
        int p = accpos(r, c);
        return _data[p];
    }
    const type& get_const(int r, int c) const
    {
        assert(r >= 0 && r < _rows && c >= 0 && c < _cols);
        int p = accpos(r, c);
        return _data[p];
    }
    int dump_row(vector& v, int r)
    {
        int s = cols();
        v.setup(s);
        memcpy_s(v.data(), sizeof(type) * s, &get(r, 0), sizeof(type) * s);
        return s;
    }
    int dump_col(vector& v, int c)
    {
        int s = rows();
        v.setup(s);
        auto& sel = select_col(c);
        assert(s == sel.capacity());
        for(int i = 0; i < s; i ++)
            v.set(i, sel.get_const(i));
        return s;
    }
    bool gauss_elim()
    {
        assert(rows() >= 2);
        int cap = rows();
        arrange(0, cap);
        for(int i = 1; i < cap; i ++)
            gauss_elim(i);
        return row_rank() <= col_rank();
    }

public:
    int rows() const { return _rows; }
    int cols() const { return _cols; }
    type* data() const { return _data; }
    colselector select_col(int c) { return colselector(*this, c); }
    const_colselector select_col(int c) const { return const_colselector(*this, c); }
    rowselector select_row(int r) { return rowselector(*this, r); }
    const_rowselector select_row(int r) const { return const_rowselector(*this, r); }
    bool fuzzy_solvable() const { return rows() >= cols() - 1; }
    int row_rank() const { return cols() - first_non_zero(select_row(0)) - 1; }
    int col_rank() const { return last_non_zero(select_col(cols() - 1)) + 1; }

protected:
    void arrange(int start, int end)
    {
        assert(start >= 0 && end <= rows());
        if(end - start <= 1)
            return;
        struct node
        {
            int index, first;
            node(int i, int f): index(i), first(f) {}
        };
        list<node> stlist;
        for(int i = start; i < end; i ++)
            stlist.push_back(node(i, first_non_zero(select_row(i))));
        stlist.sort([](const node& n1, const node& n2)->bool { return n1.first < n2.first; });
        assert(!stlist.empty());
        matrix mt;
        mt.set_dim(end - start, cols());
        auto j = stlist.begin();
        for(int i = 0; j != stlist.end(); ++ i, ++ j) {
            auto& r = mt.select_row(i);
            memcpy_s(&r.get(0), sizeof(type)*cols(), &get_const(j->index, 0), sizeof(type)*cols());
        }
        int cpsize = sizeof(type) * cols() * (end - start);
        memcpy_s(&get(start, 0), cpsize, mt.data(), cpsize);
    }
    void gauss_elim(int i)
    {
        assert(i > 0 && i < rows());
        auto& select1 = select_row(i - 1);
        auto& select2 = select_row(i);
        int first1 = first_non_zero(select1);
        if(select2.get(first1) == 0)
            return;
        int cap = rows();
        for(int j = i; j < cap; j ++) {
            if(!gauss_elim(i - 1, j, first1))
                break;
        }
        arrange(i, cap);
    }
    bool gauss_elim(int i, int j, int first)
    {
        assert(i < j);
        auto& select1 = select_row(i);
        auto& select2 = select_row(j);
        if(select2.get(first) == 0)
            return false;
        select2 *= (select1.get_const(first) / select2.get_const(first));
        select2 -= select1;
        return true;
    }

protected:
    void trace_data() const
    {
        for(int i = 0; i < rows(); i ++) {
            auto& sel = select_row(i);
            for(int j = 0; j < sel.capacity(); j ++)
                trace(_t("%lf    "), sel.get_const(j));
            trace(_t("\r"));
        }
    }
};

template<class _fty>
class linequ
{
public:
    typedef _fty type;
    typedef linequ<_fty> myref;
    typedef levector<_fty> vector;
    typedef lematrix<_fty> matrix;
    static double qnan()
    {
        static const struct nan_struct { union { uint a[2]; double b; }; } qnan_struct = { 0xffffffff, 0x7fffffff };
        return qnan_struct.b;
    }

public:
    void set_variable_count(int c) { _variables.setup(c); }
    int get_variable_count() const { return _variables.capacity(); }
    void set_equation_count(int c) { _matrix.set_dim(c, get_variable_count() + 1); }
    int get_equation_count() const { return _matrix.rows(); }
    const vector& get_results() const { return _variables; }
    type get_variable(int i) const { return _variables.get_const(i); }
    void set_equation(int i, ...)
    {
        va_list ptr;
        va_start(ptr, i);
        auto& selection = _matrix.select_row(i);
        int cap = _matrix.cols();
        for(int j = 0; j < cap; j ++)
            selection.set(j, va_arg(ptr, type));
    }
    void set_equation(int i, const type d[], int s)
    {
        auto& selection = _matrix.select_row(i);
        if(s > selection.capacity())
            s = selection.capacity();
        for(int j = 0; j < s; j ++)
            selection.set(j, d[j]);
    }
    int solve()
    {
        if(!_matrix.fuzzy_solvable() || !_matrix.gauss_elim())
            return 0;
        set_unsolved<_fty>();
        int rkcol = _matrix.col_rank();
        int c = 0;
        for(int i = rkcol - 1; i >= 0; i --, c ++) {
            if(!solve(i))
                break;
        }
        return c;
    }

protected:
    vector          _variables;
    matrix          _matrix;

protected:
    template<class _fty>
    void set_unsolved();
    template<>
    void set_unsolved<double>() { _variables.for_each([](type& v) { v = qnan(); }); }
    template<>
    void set_unsolved<float>()
    {
        float fqnan = static_cast<float>(qnan());
        assert(sizeof(fqnan) == sizeof(int));
        memset(_variables.data(), *(int*)&fqnan, _variables.capacity());
    }
    bool solve(int c)
    {
        auto& selection = _matrix.select_row(c);
        int first = first_non_zero(selection);
        for(int i = get_variable_count() - 1; i > c; i --) {
            if((((type)qnan())) == get_variable(i))
                return false;
        }
        int cap = get_variable_count();
        type s = selection.get_const(cap);
        for(int i = c + 1; i < cap; i ++)
            s += (selection.get_const(i) * get_variable(i));
        _variables.set(c, -s / selection.get_const(c));
        return true;
    }
};

__gslib_end__

#endif
