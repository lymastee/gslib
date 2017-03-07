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

#ifndef std_9154454e_8e1d_4505_9034_5c7a71b77dd0_h
#define std_9154454e_8e1d_4505_9034_5c7a71b77dd0_h

#include <xutility>
#include <algorithm>
#include <memory>
#include <list>
#include <vector>
#include <deque>
#include <queue>
#include <map>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include <gslib/type.h>
#include <gslib/pool.h>
#include <gslib/string.h>

__gslib_begin__

template<class _ty> inline
_ty* _allocate(size_t c) { return c <= 0 ? 0 : (_ty*)_pool->born((int)(sizeof(_ty)*c)); }

template<class _ty> inline
_ty* _allocate(size_t c, _ty* ptr) { return c <= 0 ? 0 : (_ty*)_pool->flex(ptr, (int)(sizeof(_ty)*c)); }

template<class _t1, class _t2> inline
void _construct(_t1* ptr, const _t2& v) { new(ptr)_t1(v); }

template<class _ty>
void _destroy(_ty* ptr) { ptr->~_ty(); }

template<> inline
void _destroy(char*) {}

template<> inline
void _destroy(wchar_t*) {}

template<class _ty>
void _deallocate(_ty* ptr) { _pool->kill(ptr); }

template<class _ty>
class allocator:
#if defined(_MSC_VER) &&(_MSC_VER >= 1900)
    public std::allocator<_ty>
#else
    public std::_Allocator_base<_ty>
#endif
{
public:
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
    typedef std::allocator<_ty> _Mybase;
#else
    typedef std::_Allocator_base<_ty> _Mybase;
#endif
    typedef typename _Mybase::value_type value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef const value_type* const_pointer;
    typedef const value_type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

public:
    template<class _other>
    struct rebind { typedef allocator<_other> other; };
    pointer address(reference v) const { return &v; }
    const_pointer address(const_reference v) const { return &v; }

public:
    allocator() {}
    allocator(const allocator<_ty>&) {}
    template<class _other>
    allocator(const allocator<_other>&) {}
    template<class _other>
    allocator<_ty>& operator=(const allocator<_other>&) { return *this; }

public:
    void deallocate(pointer ptr, size_type) { _deallocate<void>(ptr); }
    pointer allocate(size_type c) { return _allocate<_ty>(c); }
    pointer allocate(size_type c, const void* ptr) { return _allocate(c, ptr); }
    void construct(pointer ptr, const _ty& v) { _construct(ptr, v); }
    void destroy(pointer ptr) { _destroy(ptr); }
    size_t max_size() const
    {
        size_t c = (size_t)(-1) / sizeof(_ty);
        return 0 < c ? c : 1;
    }
};

template<class _ty, class _other> inline
bool operator==(const allocator<_ty>&, const allocator<_other>&) { return true; }

template<class _ty, class _other> inline
bool operator!=(const allocator<_ty>&, const allocator<_other>&) { return false; }

class string;

/* used for base type delegate */
template<class _inner>
struct base_delegator
{
    _inner          _puppet;

    base_delegator() {}
    base_delegator(const _inner& i): _puppet(i) {}
    operator _inner() const { return _puppet; }
    _inner& operator=(const _inner& i) { return _puppet = i; }
};

template<class _inner>
struct base_ptr_delegator
{
    _inner*         _puppet;

    base_ptr_delegator() { _puppet = 0; }
    base_ptr_delegator(_inner* ptr) { _puppet = ptr; }
    operator _inner*() { return _puppet; }
    operator const _inner*() const { return _puppet; }
    _inner* operator=(_inner* ptr) { return _puppet = ptr; }
};

/* the same as the default std version */
inline size_t string_hash(const gchar str[], int size)
{
    size_t val = 2166136261U;
    size_t first = 0;
    size_t last = (size_t)size;
    size_t stride = 1 + last / 10;
    for( ; first < last; first += stride)
        val = 16777619U * val ^ (size_t)str[first];
    return val;
}

inline size_t string_hash(const string& str) { return string_hash(str.c_str(), str.size()); }
inline size_t string_hash(const gchar str[]) { return string_hash(str, strtool::length(str)); }

/* switch to namespace tr1 and replace the hash for string */
__gslib_end__

namespace std {

template<>
class hash<gs::string>:
    public unary_function<gs::string, size_t>
{
public:
    size_t operator()(const gs::string& kval) const
    { return string_hash(kval); }
};

};
__gslib_begin__

/* wrappers for all kinds of vessels */

#ifndef _GS_THREADSAFE

template<class _ty>
class list: public std::list<_ty, gs::allocator<_ty> > {};
template<class _ty>
class vector: public std::vector<_ty, gs::allocator<_ty> > {};
template<class _ty>
class deque: public std::deque<_ty, gs::allocator<_ty> > {};
template<class _ty>
class stack: public std::stack<_ty, gs::deque<_ty> > {};

template<class _kty, class _ty,
    class _pr = std::less<_kty>
    >
class map: public std::map<_kty, _ty, _pr,
    gs::allocator<std::pair<const _kty, _ty> >
    >
{};
template<class _ty, class _pr = std::less<_ty> >
class set: public std::set<_ty, _pr, gs::allocator<_ty> > {};

template<class _kty, class _ty,
    class _pr = std::less<_kty>
    >
class multimap: public std::multimap<_kty, _ty, _pr,
    gs::allocator<std::pair<const _kty, _ty> >
    >
{};
template<class _ty, class _pr = std::less<_ty> >
class multiset: public std::multiset<_ty, _pr, gs::allocator<_ty> > {};

template<class _kty, class _ty,
    class _hs = std::tr1::hash<_kty>, class _equ = std::equal_to<_kty>
    >
class unordered_map:
    public std::unordered_map<_kty, _ty, _hs, _equ,
        gs::allocator<std::pair<const _kty, _ty> >
    >
{};
template<class _kty,
    class _hs = std::tr1::hash<_kty>,
    class _equ = std::equal_to<_kty>
    >
class unordered_set:
    public std::unordered_set<_kty, _hs, _equ, gs::allocator<_kty>
    >
{};

template<class _kty, class _ty,
    class _hs = std::tr1::hash<_kty>, class _equ = std::equal_to<_kty>
    >
class unordered_multimap:
    public std::unordered_multimap<_kty, _ty, _hs, _equ, 
        gs::allocator<std::pair<const _kty, _ty> > 
    >
{};
template<class _kty, class _hs, class _equ>
class unordered_multiset:
    public std::unordered_multiset<_kty, _hs, _equ, gs::allocator<_kty>
    >
{};

#else

template<class _ty>
class list: public std::list<_ty> {};
template<class _ty>
class vector: public std::vector<_ty> {};
template<class _ty>
class deque: public std::deque<_ty> {};
template<class _ty>
class stack: public std::stack<_ty> {};

template<class _kty, class _ty,
    class _pr = std::less<_kty>
    >
class map: public std::map<_kty, _ty, _pr> {};
template<class _ty, class _pr = std::less<_ty> >
class set: public std::set<_ty, _pr> {};

template<class _kty, class _ty,
    class _pr = std::less<_kty>
    >
class multimap: public std::multimap<_kty, _ty, _pr>
{};
template<class _ty, class _pr = std::less<_ty> >
class multiset: public std::multiset<_ty, _pr>
{};

template<class _kty, class _ty,
    class _hs = std::tr1::hash<_kty>
    >
class unordered_map:
    public std::unordered_map<_kty, _ty, _hs> {};

template<class _kty,
    class _hs = std::tr1::hash<_kty>,
    class _equ = std::equal_to<_kty>
    >
class unordered_set:
    public std::unordered_set<_kty, _hs, _equ> {};

template<class _kty, class _ty,
    class _hs = std::tr1::hash<_kty>
    >
class unordered_multimap:
    public std::unordered_multimap<_kty, _ty, _hs> {};

template<class _kty, class _hs, class _equ>
class unordered_multiset:
    public std::unordered_multiset<_kty, _hs, _equ> {};

#endif

__gslib_end__

#endif