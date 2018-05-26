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

#ifndef string_81448b68_30d7_4d64_a67a_cecef7c90139_h
#define string_81448b68_30d7_4d64_a67a_cecef7c90139_h

#include <string>
#include <assert.h>
#include <gslib/type.h>

__gslib_begin__

template<class _element>
class _std_string:
    public std::basic_string<_element, std::char_traits<_element>, std::allocator<_element> >
{
public:
    typedef _element protoch;
    typedef std::char_traits<protoch> prototr;
    typedef std::allocator<protoch> protoalloc;
    typedef std::basic_string<protoch, prototr, protoalloc> protoref;
    friend int _vsprintf(_std_string<char>& str, const char* fmt, va_list ap);
    friend int _vsprintf(_std_string<wchar>& str, const wchar* fmt, va_list ap);
};

template<class _element>
struct _string_tool
{
    typedef _element element;
};

template<>
struct _string_tool<char>
{
    static void copy(char* des, size_t size, const char* src) { strcpy_s(des, size, src); }
    static int length(const char* str) { return (int)strlen(str); }
    static int compare(const char* s1, const char* s2) { return (int)strcmp(s1, s2); }
    static int compare_cl(const char* s1, const char* s2) { return (int)_stricmp(s1, s2); }
    static int compare(const char* s1, const char* s2, int cnt) { return (int)strncmp(s1, s2, cnt); }
    static int compare_cl(const char* s1, const char* s2, int cnt) { return (int)_strnicmp(s1, s2, cnt); }
    static int vprintf(char* des, int size, const char* fmt, va_list ap) { return _vsprintf_s_l(des, size, fmt, 0, ap); }
    static int printf(char* des, int size, const char* fmt, ...)
    {
        va_list ptr;
        va_start(ptr, fmt);
        return _vsprintf_s_l(des, size, fmt, 0, ptr);
    }
    static int vsscanf(const char* src, const char* fmt, va_list ap) { return vsscanf_s(src, fmt, ap); }
    static int sscanf(const char* src, const char* fmt, ...)
    {
        va_list ptr;
        va_start(ptr, fmt);
        return vsscanf_s(src, fmt, ptr);
    }
    static int ctlprintf(const char* src, ...)
    {
        va_list ptr;
        va_start(ptr, src);
        return ::vprintf(src, ptr);
    }
    static int to_int(const char* src) { return atoi(src); }
    static int to_int(const char* src, int radix) { return strtol(src, 0, radix); }
    static uint to_uint(const char* src, int radix) { return strtoul(src, 0, radix); }
    static int64 to_int64(const char* src) { return _atoi64(src); }
    static int64 to_int64(const char* src, int radix) { return _strtoi64(src, 0, radix); }
    static uint64 to_uint64(const char* src, int radix) { return _strtoui64(src, 0, radix); }
    static real to_real(const char* src) { return atof(src); }
    static void from_int(int i, char* des, int size, int radix) { _itoa_s(i, des, size, radix); }
    static void from_int64(int64 i, char* des, int size, int radix) { _i64toa_s(i, des, size, radix); }
    static void from_real(char* des, int size, real d) { from_real(des, size, d, size-2); }
    static void from_real(char* des, int size, real d, int precis) { _gcvt_s(des, size, d, precis); }
    static void to_lower(char* str, int size) { _strlwr_s(str, size); }
    static void to_upper(char* str, int size) { _strupr_s(str, size); }
    static const char* find(const char* src, const char* tar) { return strstr(src, tar); }
    static int test(const char* src, const char* cpset) { return (int)strcspn(src, cpset); }
    static const char* _test(const char* src, const char* cpset) { return strpbrk(src, cpset); }
};

template<>
struct _string_tool<wchar>
{
    static void copy(wchar* des, size_t size, const wchar* src) { wcscpy_s(des, size, src); }
    static int length(const wchar* str) { return (int)wcslen(str); }
    static int compare(const wchar* s1, const wchar* s2) { return (int)wcscmp(s1, s2); }
    static int compare_cl(const wchar* s1, const wchar* s2) { return (int)_wcsicmp(s1, s2); }
    static int compare(const wchar* s1, const wchar* s2, int cnt) { return (int)wcsncmp(s1, s2, cnt); }
    static int compare_cl(const wchar* s1, const wchar* s2, int cnt) { return (int)_wcsnicmp(s1, s2, cnt); }
    static int vprintf(wchar* des, int size, const wchar* fmt, va_list ap) { return _vswprintf_s_l(des, size, fmt, 0, ap); }
    static int printf(wchar* des, int size, const wchar* fmt, ...)
    {
        va_list ptr;
        va_start(ptr, fmt);
        return _vswprintf_s_l(des, size, fmt, 0, ptr);
    }
    static int vsscanf(const wchar* src, const wchar* fmt, va_list ap) { return vswscanf_s(src, fmt, ap); }
    static int sscanf(const wchar* src, const wchar* fmt, ...)
    {
        va_list ptr;
        va_start(ptr, fmt);
        return vswscanf_s(src, fmt, ptr);
    }
    static int ctlprintf(const wchar* src, ...)
    {
        va_list ptr;
        va_start(ptr, src);
        return ::vwprintf(src, ptr);
    }
    static int to_int(const wchar* src) { return _wtoi(src); }
    static int to_int(const wchar* src, int radix) { return wcstol(src, 0, radix); }
    static uint to_uint(const wchar* src, int radix) { return wcstoul(src, 0, radix); }
    static int64 to_int64(const wchar* src) { return _wtoi64(src); }
    static int64 to_int64(const wchar* src, int radix) { return _wcstoi64(src, 0, radix); }
    static uint64 to_uint64(const wchar* src, int radix) { return _wcstoui64(src, 0, radix); }
    static real to_real(const wchar* src) { return _wtof(src); }
    static void from_int(int i, wchar* des, int size, int radix) { _itow_s(i, des, size, radix); }
    static void from_int64(int64 i, wchar* des, int size, int radix) { _i64tow_s(i, des, size, radix); }
    static void from_real(wchar* des, int size, real d) { from_real(des, size, d, size-2); }
    static void from_real(wchar* des, int size, real d, int precis)
    {
        _gcvt_s((char*)des, size, d, precis);
        int len = (int)strlen((char*)des); /* call it in asm wasn't available in /o2 */
        __asm
        {
            mov  ecx, len;
            mov  esi, des;
            add  esi, ecx;
            mov  edi, esi;
            add  edi, ecx;
            mov  word ptr[edi], 0;
            std;
        rptag:
            movsb;
            mov  byte ptr[edi], 0;
            dec  edi;
            loop rptag;
            cld;
        }
    }
    static void to_lower(wchar* str, int size) { _wcslwr_s(str, size); }
    static void to_upper(wchar* str, int size) { _wcsupr_s(str, size); }
    static const wchar* find(const wchar* src, const wchar* tar) { return wcsstr(src, tar); }
    static int test(const wchar* src, const wchar* cpset) { return (int)wcsspn(src, cpset); }
    static const wchar* _test(const wchar* src, const wchar* cpset) { return wcspbrk(src, cpset); }
};

extern const char* get_mbcs_elem(const char* str, uint& c);
extern const char* get_mbcs_elem(const char* str, uint& c, const char* end);
extern int convert_to_wide(wchar out[], int size, const char* str, int len = -1);
extern int convert_to_byte(char out[], int size, const wchar* str, int len = -1);
extern int convert_utf8_to_wide(wchar out[], int size, const char* str, int len = -1);
extern int convert_wide_to_utf8(char out[], int size, const wchar* str, int len = -1);

struct _string_caseful {};
struct _string_caseless {};

template<class _element, 
    class _cpcase = _string_caseful>
class _string:
    public _std_string<_element>
{
public:
    typedef _element element;
    typedef _std_string<element> inheritref;
    typedef _string_tool<element> _strtool;
    typedef typename inheritref::protoref protoref;
    typedef _string<_element, _cpcase> myref;

    template<class _cpcase> static int comparefunc(const _string* s, const element* str);
    template<class _cpcase> static  int comparefunc(const _string* s, const element* str, int len);
    template<> static int comparefunc<_string_caseful>(const _string* s, const element* str) { return _strtool::compare(s->c_str(), str); }
    template<> static int comparefunc<_string_caseless>(const _string* s, const element* str) { return _strtool::compare_cl(s->c_str(), str); }
    template<> static int comparefunc<_string_caseful>(const _string* s, const element* str, int len) { return _strtool::compare(s->c_str(), str, len); }
    template<> static int comparefunc<_string_caseless>(const _string* s, const element* str, int len) { return _strtool::compare_cl(s->c_str(), str, len); }

protected:
    void _stl_tidy(bool built, int newsize) { _Tidy(built, newsize); }
#if defined(_MSC_VER) && (_MSC_VER >= 1910)
    bool _stl_grow(int newsize, bool trim = false) { return _Grow(newsize); }
#else
    bool _stl_grow(int newsize, bool trim = false) { return _Grow(newsize, trim); }
#endif
    element* _stl_rawstr() { return _Myptr(); }
    const element* _stl_rawstr() const { return _Myptr(); }
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
    int& _stl_cap() { return (int&)_Myres(); }
    int& _stl_size() { return (int&)_Mysize(); }
    void _stl_fix() { _Mysize() = (size_t)_strtool::length(c_str()); }
#else
    int& _stl_cap() { return (int&)_Myres; }
    int& _stl_size() { return (int&)_Mysize; }
    void _stl_fix() { _Mysize = (size_t)_strtool::length(c_str()); }
#endif
    void _stl_eos(int pos) { _Eos(pos); }

private:
    template<class _element>
    void convert_from(const wchar* str);
    template<class _element>
    void convert_from(const char* str);
    template<>
    void convert_from<wchar>(const wchar* str) { assign(str); }
    template<>
    void convert_from<char>(const wchar* str)
    {
        int len = convert_to_byte(0, 0, str);
        _stl_grow(len);
        convert_to_byte(_stl_rawstr(), len, str);
        _stl_fix();
    }
    template<>
    void convert_from<wchar>(const char* str)
    {
        int len = convert_to_wide(0, 0, str);
        _stl_grow(len);
        convert_to_wide(_stl_rawstr(), len, str);
        _stl_fix();
    }
    template<>
    void convert_from<char>(const char* str) { assign(str); }
    template<class _element>
    void convert_from(const char* str, int len);
    template<class _element>
    void convert_from(const wchar* str, int len);
    template<>
    void convert_from<wchar>(const wchar* str, int len) { assign(str, len); }
    template<>
    void convert_from<wchar>(const char* str, int len)
    {
        int l = convert_to_wide(0, 0, str, len);
        _stl_grow(l + 1);
        convert_to_wide(_stl_rawstr(), l, str, len);
        _stl_eos(l);
    }
    template<>
    void convert_from<char>(const wchar* str, int len)
    {
        int l = convert_to_byte(0, 0, str, len);
        _stl_grow(l + 1);
        convert_to_byte(_stl_rawstr(), l, str, len);
        _stl_eos(l);
    }
    template<>
    void convert_from<char>(const char* str, int len) { assign(str, len); }

public:
    _string() {}
    _string(const element* str) { if(str) assign(str); }
    _string(const element* str, int len) { if(str) assign(str, len); }
    _string(element c, int ctr) { assign(ctr, c); }
    void destroy() { _stl_tidy(true, 0); }
    int length() const { return (int)size(); }
    element& front() { return at(0); }
    const element& front() const { return at(0); }
    element& back() { return at(size()-1); }
    const element& back() const { return at(size()-1); }
    element pop_back()
    {
        element e = back();
        resize(size()-1);
        return e;
    }
    int test(const element* cpset) const
    {
        if(empty())
            return (int)npos;
        int pos = _strtool::test(c_str(), cpset);
        if(pos == length())
            return (int)npos;
        return pos;
    }
    const element* _test(const element* cpset) const
    {
        if(empty())
            return 0;
        return _strtool::_test(c_str(), cpset);
    }
    void format(const element* fmt, ...)
    {
        va_list ptr;
        va_start(ptr, fmt);
        _vsprintf(*this, fmt, ptr);
    }
    void formatv(const element* fmt, va_list ptr)
    {
        _vsprintf(*this, fmt, ptr);
    }
    /*
    void format(int size, const element* fmt, ...)
    {
        _stl_grow(size, false);
        va_list ptr;
        va_start(ptr, fmt);
        _stl_size() = _strtool::vprintf(_stl_rawstr(), _stl_cap(), fmt, ptr);
    }
    */
    void to_lower()
    {
        if(empty())
            return;
        _strtool::to_lower(_stl_rawstr(), _stl_cap());
    }
    void to_upper()
    {
        if(empty())
            return;
        _strtool::to_upper(_stl_rawstr(), _stl_cap());
    }
    int to_int() const { return _strtool::to_int(_stl_rawstr()); }
    int64 to_int64() const { return _strtool::to_int64(_stl_rawstr()); }
    real to_real() const { return _strtool::to_real(_stl_rawstr()); }
    void from(const char* str) { convert_from<element>(str); }
    void from(const wchar* str) { convert_from<element>(str); }
    void from(const char* str, int len) { convert_from<element>(str, len); }
    void from(const wchar* str, int len) { convert_from<element>(str, len); }
    void from_int(int i, int radix = 10)
    {
        _stl_grow(12);
        _strtool::from_int(i, _stl_rawstr(), _stl_cap(), radix);
        _stl_fix();
    }
    void from_int64(int64 i, int radix = 10)
    {
        _stl_grow(24);
        _strtool::from_int64(i, _stl_rawstr(), _stl_cap(), radix);
        _stl_fix();
    }
    void from_real(real d, int precis = 22)
    {
        _stl_grow(precis+2);
        _strtool::from_real(_stl_rawstr(), _stl_cap(), d, precis);
        _stl_fix();
    }
    int compare(const element* str) const { return _strtool::compare(_stl_rawstr(), str); }
    int compare(const element* str, int len) const { return _strtool::compare(_stl_rawstr(), str, len); }
    int compare_cl(const element* str) const { return _strtool::compare_cl(_stl_rawstr(), str); }
    int compare_cl(const element* str, int len) const { return _strtool::compare_cl(_stl_rawstr(), str, len); }
    bool greater(const element* str) const { return comparefunc<_cpcase>(this, str) > 0; }
    bool greater(const element* str, int len) const { return comparefunc<_cpcase>(this, str, len) > 0; }
    bool equal(const element* str) const { return comparefunc<_cpcase>(this, str) == 0; }
    bool equal(const element* str, int len) const { return comparefunc<_cpcase>(this, str, len) == 0; }
    bool less(const element* str) const { return comparefunc<_cpcase>(this, str) < 0; }
    bool less(const element* str, int len) const { return comparefunc<_cpcase>(this, str, len) < 0; }
    bool operator < (const element* str) const { return less(str); }
    bool operator < (const _string* str) const { return less(str->c_str(), str->length()); }
    bool operator < (const _string& str) const { return less(str.c_str(), str.length()); }
    bool operator == (const element* str) const { return equal(str); }
    bool operator == (const _string* str) const { return equal(str->c_str(), str->length()); }
    bool operator == (const _string& str) const { return equal(str.c_str(), str.length()); }
    bool operator != (const element* str) const { return !equal(str); }
    bool operator != (const _string* str) const { return !equal(str->c_str(), str->length()); }
    bool operator != (const _string& str) const { return !equal(str.c_str(), str.length()); }
    bool operator > (const element* str) const { return greater(str); }
    bool operator > (const _string* str) const { return greater(str->c_str(), str->length()); }
    bool operator > (const _string& str) const { return greater(str.c_str(), str.length()); }
};

typedef _string_tool<gchar> strtool;

/*
 * string compare tool for short const string, notify that the s1 increase:
 * string symmetrical check
 */
template<class _element>
class strsymcheck
{
public:
    typedef _element mchar;
    typedef strsymcheck<mchar> mref;

public:
    template<int _len>
    static bool check(const mchar* s1, const mchar* s2) { return *s1 ++ == *s2 ++ && mref::check<_len-1>(s1, s2); }
    template<>
    static bool check<0>(const mchar* s1, const mchar* s2) { return true; }
    template<>
    static bool check<1>(const mchar* s1, const mchar* s2) { return *s1 ++ == *s2 ++; }
    template<int _len>
    static bool check_run(const mchar*& s1, const mchar* s2)
    {
        if(check<_len>(s1, s2)) {
            s1 += _len;
            return true;
        }
        return false;
    }
};

#ifdef _cstrcmp
#undef _cstrcmp
#endif
#define _cstrcmp(sr, cmp)    strsymcheck<gchar>::check<_cststrlen(cmp)>((sr), (cmp))

#define _cstrncmp(sr, len, cmp) \
    ((len >= _cststrlen(cmp)) && strsymcheck<gchar>::check<_cststrlen(cmp)>((sr), (cmp)))

#ifdef _cstrcmprun
#undef _cstrcmprun
#endif
#define _cstrcmprun(sr, cmp) strsymcheck<gchar>::check_run<_cststrlen(cmp)>((sr), (cmp))

#define mkstr(str1, str2) (((string(str1) += str2)).c_str())
#define mkstr3(str1, str2, str3) (((string(str1) += str2) += str3).c_str())
#define mkstr4(str1, str2, str3, str4) ((((string(str1) += str2) += str3) += str4).c_str())

template<int _cmpstr> inline
bool strequ(const char* str) { return *(const int*)str == _cmpstr; }
template<char _cmpch> inline
bool chequ(const char* str) { return *str == _cmpch; }
template<char _c1, char _c2>
struct chless { static const bool result = _c1 < _c2; };
template<char _c1, char _c2, bool _less = chless<_c1, _c2>::result>
struct chmin {};
template<char _c1, char _c2>
struct chmin<_c1, _c2, true> { static const char value = _c1; };
template<char _c1, char _c2>
struct chmin<_c1, _c2, false> { static const char value = _c2; };
template<char _c1, char _c2, bool _less = chless<_c1, _c2>::result>
struct chmax {};
template<char _c1, char _c2>
struct chmax<_c1, _c2, true> { static const char value = _c2; };
template<char _c1, char _c2>
struct chmax<_c1, _c2, false> { static const char value = _c1; };
template<int _len> inline
char strhit(const char* str, const char* cmp) { return *str == *cmp++ ? *str : strhit<_len + 1>(str, cmp); }
template<> inline
char strhit<1>(const char* str, const char* cmp) { return *str == *cmp ? *str : 0; }
template<> inline
char strhit<0>(const char* str, const char* cmp) { return *str; }
template<char _from, char _to>
char strhit(const char* str)
{
    return (*str >= chmin<_from, _to>::value && 
        *str <= chmax<_from, _to>::value) ?
        *str : 0;
}

/* used by the tool chain. */
inline int conv_quadstr(const char* str)
{
    int n = 0;
    if(str[0] == 0)
        return n;
    n |= str[0];
    if(str[1] == 0)
        return n;
    n |= ((uint)str[1]) << 8;
    if(str[2] == 0)
        return n;
    n |= ((uint)str[2]) << 16;
    if(str[3] == 0)
        return n;
    n |= ((uint)str[3]) << 24;
    return n;
}
inline char _1of_quadstr(int quadstr) { return (char)(quadstr & 0xff); }
inline char _2of_quadstr(int quadstr) { return (char)((quadstr & 0xff00) >> 8); }
inline char _3of_quadstr(int quadstr) { return (char)((quadstr & 0xff0000) >> 16); }
inline char _4of_quadstr(int quadstr) { return (char)((quadstr & 0xff000000) >> 24); }
inline int len_of_quadstr(int quadstr)
{
    if(!(quadstr & 0xff))
        return 0;
    if(!(quadstr & 0xff00))
        return 1;
    if(!(quadstr & 0xff0000))
        return 2;
    return (quadstr & 0xff000000) ? 4 : 3;
}
inline int utf8_unit_len(const char* str)
{
    if(str[0] == 0)
        return 0;
    if(!(str[0] & 0x80))
        return 1;
    if((str[0] & 0xe0) == 0xc0)
        return 2;
    if((str[0] & 0xf0) == 0xe0)
        return 3;
    if((str[0] & 0xf8) == 0xf0)
        return 4;
    if((str[0] & 0xfc) == 0xf8)
        return 5;
    if((str[0] & 0xfe) == 0xfc)
        return 6;
    return -1;
}
inline bool is_utf8_head(const char* str)
{
    assert(str);
    return (str[0] & 0xc0) != 0x80;
}
inline const char* to_utf8_head(const char* str)
{
    assert(str);
    for( ; str[0] && !is_utf8_head(str); str --);
    return str;
}
inline const char* next_utf8_unit(const char* str)
{
    assert(str);
    if(str[0] == 0)
        return str;
    for(str ++; !is_utf8_head(str); str ++);
    return str;
}
inline const char* prev_utf8_unit(const char* str)
{
    assert(str);
    return to_utf8_head(-- str);
}
inline bool utf8_unit_valid(const char* str)
{
    assert(str);
    return !is_utf8_head(str) ? false :
        next_utf8_unit(str) - str == utf8_unit_len(str);
}
inline int utf8_len(const char* str)
{
    assert(str);
    int len = 0;
    for( ; str[0]; str = next_utf8_unit(str), len ++);
    return len;
}

class string:
    public _string<gchar>
{
public:
    typedef std::basic_string<gchar> stdstr;
    typedef _string<gchar> superref;
    typedef superref::iterator iterator;
    typedef superref::const_iterator const_iterator;
    typedef superref::protoref protoref;

public:
    using superref::npos;
    using superref::begin;
    using superref::c_str;
    using superref::capacity;
    using superref::compare;
    using superref::copy;
    using superref::empty;
    using superref::find;
    using superref::find_first_not_of;
    using superref::find_first_of;
    using superref::find_last_not_of;
    using superref::find_last_of;
    using superref::max_size;
    using superref::operator+=;
    using superref::operator=;
    using superref::rbegin;
    using superref::reserve;
    using superref::rfind;
    using superref::substr;
    using superref::test;
    using superref::_test;
    using superref::to_lower;
    using superref::to_upper;
    using superref::to_int;
    using superref::to_int64;
    using superref::to_real;
    using superref::compare_cl;
    using superref::greater;
    using superref::less;
    using superref::equal;
    using superref::operator<;
    using superref::operator>;
    using superref::operator!=;
    using superref::operator==;
    using superref::front;
    using superref::back;
    using superref::assign;
    using superref::append;
    using superref::clear;
    using superref::erase;
    using superref::insert;
    using superref::push_back;
    using superref::replace;
    using superref::resize;
    using superref::swap;
    using superref::at;
    using superref::operator[];
    using superref::format;
    using superref::from_int;
    using superref::from_int64;
    using superref::from_real;
    using superref::length;
    using superref::pop_back;

public:
    string() {}
    string(const gchar* str): superref(str) {}
    string(const gchar* str, int len): superref(str, len) {}
    string(gchar ch, int cnt): superref(ch, cnt) {}
    string(protoref& rhs) { assign(rhs.c_str(), (int)rhs.size()); }
    int size() const { return (int)superref::size(); } 
    bool operator<(const string& str) const { return less(str.c_str(), str.length()); }
    bool operator>(const string& str) const { return greater(str.c_str(), str.length()); }
    bool operator==(const string& str) const { return equal(str.c_str(), str.length()); }
    bool operator!=(const string& str) const { return !equal(str.c_str(), str.length()); }
};

inline bool is_mbcs_half(const char* str)
{
    if(!str || !(str[0] & 0x80))
        return false;
    int i = 1;
    for( ; str[i] && (str[i] & 0x80); i ++);
    return i % 2 != 0;
}

inline const gchar* get_next_char(const gchar* str, uint& c)
{
#ifdef _UNICODE
    if(!str || !str[0]) { c = 0; return 0; }
    c = (uint)str[0];
    return ++ str;
#else
    return get_mbcs_elem(str, c);
#endif
}

inline const gchar* get_next_char(const gchar* str, uint& c, const gchar* end)
{
#ifdef _UNICODE
    if(!str || str == end) { c = 0; return 0; }
    c = (uint)str[0];
    return ++ str;
#else
    return get_mbcs_elem(str, c, end);
#endif
}

__gslib_end__

#endif

