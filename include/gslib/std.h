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
#include <comip.h>

#include <gslib/type.h>
#include <gslib/pool.h>
#include <gslib/string.h>

__gslib_begin__

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

#if defined(_MSC_VER) && (_MSC_VER < 1910)
/* the same as the default std version */
inline size_t hash_bytes(const byte str[], int size)
{
    size_t val = 2166136261U;
    size_t first = 0;
    size_t last = (size_t)size;
    size_t stride = 1 + last / 10;
    for( ; first < last; first += stride)
        val = 16777619U * val ^ (size_t)str[first];
    return val;
}
#else
inline size_t hash_bytes(const byte str[], int size)
{ return std::_Hash_bytes(str, size); }
#endif

#ifdef _UNICODE
inline size_t string_hash(const string& str) { return hash_bytes((const byte*)str.c_str(), str.size() * sizeof(wchar)); }
#else
inline size_t string_hash(const string& str) { return hash_bytes((const byte*)str.c_str(), str.size()); }
#endif

class hasher:
    public std::_Fnv1a_hasher
{
public:
    size_t add_bytes(const byte* first, int len) { return _Add_bytes(first, first + len); }
    size_t get_value() const { return _Val; }
};

/* switch to namespace tr1 and replace the hash for string */
__gslib_end__

namespace std {

template<>
class hash<gs::string>
#if defined(_MSC_VER) && (_MSC_VER < 1914)
    : public unary_function<gs::string, size_t>
#endif
{
public:
    size_t operator()(const gs::string& kval) const { return string_hash(kval); }
};

};
__gslib_begin__

/* wrappers for all kinds of vessels */
template<class _ty>
using list = std::list<_ty>;
template<class _ty>
using vector = std::vector<_ty>;
template<class _ty>
using deque = std::deque<_ty>;
template<class _ty>
using stack = std::stack<_ty>;

template<class _kty, class _ty,
    class _pr = std::less<_kty>
    >
using map = std::map<_kty, _ty, _pr>;
template<class _ty, class _pr = std::less<_ty> >
using set = std::set<_ty, _pr>;

template<class _kty, class _ty,
    class _pr = std::less<_kty>
    >
using multimap = std::multimap<_kty, _ty, _pr>;
template<class _ty, class _pr = std::less<_ty> >
using multiset = std::multiset<_ty, _pr>;

template<class _kty, class _ty,
    class _hs = std::hash<_kty>
    >
using unordered_map = std::unordered_map<_kty, _ty, _hs>;

template<class _kty,
    class _hs = std::hash<_kty>,
    class _equ = std::equal_to<_kty>
    >
using unordered_set = std::unordered_set<_kty, _hs, _equ>;

template<class _kty, class _ty,
    class _hs = std::hash<_kty>
    >
using unordered_multimap = std::unordered_multimap<_kty, _ty, _hs>;

template<class _kty, class _hs, class _equ>
using unordered_multiset = std::unordered_multiset<_kty, _hs, _equ>;

template<class _cls>
class com_ptr
{
public:
    typedef _cls* ptr_type;
    typedef com_ptr<_cls> myref;

public:
    com_ptr() { _ptr = nullptr; }
    ~com_ptr()
    {
        if(_ptr) {
            _ptr->Release();
            _ptr = nullptr;
        }
    }
    com_ptr(_cls* p)
    {
        _ptr = p;
        if(_ptr)
            _ptr->AddRef();
    }
    com_ptr(const myref& p)
    {
        _ptr = p;
        if(_ptr)
            _ptr->AddRef();
    }
    com_ptr(IUnknown* p): _ptr(nullptr)
    {
        if(p != nullptr)
            p->QueryInterface(__uuidof(_cls), (void**)&_ptr);
    }
    void attach(_cls* p)
    {
        if(_ptr)
            _ptr->Release();
        _ptr = p;
    }
    _cls* detach()
    {
        _cls* p = _ptr;
        _ptr = nullptr;
        return p;
    }
    void clear()
    {
        if(_ptr) {
            _ptr->Release();
            _ptr = nullptr;
        }
    }
    _cls* operator=(_cls* p)
    {
        if(p) p->AddRef();
        if(_ptr)
            _ptr->Release();
        return _ptr = p;
    }
    _cls* operator=(const myref& p)
    {
        if(p._ptr)
            p._ptr->AddRef();
        if(_ptr)
            _ptr->Release();
        return _ptr = p._ptr;
    }
    _cls* operator=(IUnknown* p)
    {
        _cls* tmp = nullptr;
        if(p)
            p->QueryInterface(__uuidof(_cls), (void**)&tmp);
        if(_ptr)
            _ptr->Release();
        return _ptr = tmp;
    }
    _cls** operator&()
    {
        assert(!_ptr);
        return &_ptr;
    }
    _cls* get() const { return _ptr; }
    _cls* operator->() const { return _ptr; }
    bool operator!() const { return !_ptr; }
    bool operator == (_cls* p) const { return _ptr == p; }
    bool operator != (_cls* p) const { return _ptr != p; }

private:
    _cls*           _ptr;
};

__gslib_end__

#endif