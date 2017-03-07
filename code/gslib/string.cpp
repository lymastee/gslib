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

#include <windows.h>
#include <gslib/type.h>
#include <gslib/string.h>

__gslib_begin__

#pragma warning(disable: 4996)

union mbcselem
{
    struct  { char high, low; };
    word    elem;
};

const char* get_mbcs_elem(const char* str, uint& c)
{
    if(!str) { c = 0; return 0; }
    if(str[0] < 0) {
        ((mbcselem*)&c)->high = str[1];
        ((mbcselem*)&c)->low = str[0];
        return str += 2;
    }
    c = (uint)str[0];
    return ++ str;
}

const char* get_mbcs_elem(const char* str, uint& c, const char* end)
{
    if(!str || str == end) { c = 0; return 0; }
    if(str[0] < 0) {
        ((mbcselem*)&c)->high = str[1];
        ((mbcselem*)&c)->low = str[0];
        return str += 2;
    }
    c = (uint)str[0];
    return ++ str;
}

int convert_to_wide(wchar out[], int size, const char* str, int len)
{
    return MultiByteToWideChar(CP_ACP, 0, str, len, out, size);
}

int convert_to_byte(char out[], int size, const wchar* str, int len)
{
    return WideCharToMultiByte(CP_ACP, 0, str, len, out, size, 0, 0);
}

int convert_utf8_to_wide(wchar out[], int size, const char* str, int len)
{
    return MultiByteToWideChar(CP_UTF8, 0, str, len, out, size);
}

int convert_wide_to_utf8(char out[], int size, const wchar* str, int len)
{
    return WideCharToMultiByte(CP_UTF8, 0, str, len, out, size, 0, 0);
}

#define LEFT    0x01
#define PLUS    0x02
#define SPACE   0x04
#define SPECIAL 0x08
#define ZERO    0x10
#define SIGN    0x20    /* signed if set */
#define SMALL   0x40    /* 'abcdef' if set, 'ABCDEF' otherwise */

static int get_wide(const char **s)
{
    assert(s);
    int res = 0;
    while(isdigit (**s)) res = 10 * res + *((*s)++) - '0';
    return res;
}

static int get_wide(const wchar **s)
{
    assert(s);
    int res = 0;
    while(iswdigit (**s)) res = 10 * res + *((*s)++) - L'0';
    return res;
}

#define LONG_STRSIZE_BASE_2 32

template<class _type>
struct unsigned_type {};

template<>
struct unsigned_type<int16>
{
    typedef uint16 acquire;
};

template<>
struct unsigned_type<int>
{
    typedef uint acquire;
};

template<>
struct unsigned_type<int64>
{
    typedef uint64 acquire;
};

template<class _number>
static void number_to_string(_std_string<char>& str, _number num, int base, int flags, int wide, int precision)
{
    char sign;  /* sign printed : '+', '-', ' ', or 0 (no sign) */
    _number num_cpy = num;
    auto ul_num = (unsigned_type<_number>::acquire)num;
    /* string representation of num */
    char tmp[LONG_STRSIZE_BASE_2];
    int i = 0;
    const char* digits = "0123456789ABCDEF";
    if(flags & SMALL)
        digits = "0123456789abcdef";
    if((base < 2) || (base > 16))
        return;
    if((flags & SIGN) && (num < 0)) {
        sign = '-';
        num = -num;
    }
    else {
        sign = (flags & PLUS) ? '+' : ((flags & SPACE) ? ' ' : 0);
    }
    if(sign)
        wide --;
    if(flags & SPECIAL) {
        if((base == 16) && (num != 0)) wide -= 2;   /* '0x' or '0X' */
        if(base == 8) { wide --; precision --; }    /* '0' */
    }
    if(num == 0)
        tmp[i++] = '0';
    /* signed format */
    if(flags & SIGN) {
        while(num != 0) {
            tmp[i ++] = digits[num % base];
            num /= base;
        }
    }
    /* unsigned format */
    else {
        while(ul_num != 0) {
            tmp[i ++] = digits[ul_num % base];
            ul_num /= base;
        }
    }
    if(i > precision)
        precision = i;
    wide -= precision;
    /* wide = number of padding chars */
    /* precision = number of figures after the sign and the special chars */
    /* right justified and no zeropad : pad with spaces */
    if(!(flags & (LEFT + ZERO))) {
        if(wide > 0)
            str.append(wide, ' ');
        wide = 0;
    }
    if(sign)
        str.push_back(sign);
    if((flags & SPECIAL) && (num_cpy != 0)) {
        if(base == 8)
            str.push_back('0');
        else if(base == 16) {
            if(flags & SMALL) str.append("0x");
            else str.append("0X");
        }
    }
    /* right justified and zeropad : pad with 0 */
    if(!(flags & LEFT)) {
        if(wide > 0) {
            str.append(wide, '0');
            wide --;
        }
    }
    /* print num */
    if(i < precision)
        str.append(precision - i, '0');
    if(i > 0) {
        for(i --; i >= 0; i --)
            str.push_back(tmp[i]);
    }
    /* left justified : pad with spaces */
    if(wide > 0)
        str.append(wide, ' ');
}

template<class _number>
static void number_to_string(_std_string<wchar>& str, _number num, int base, int flags, int wide, int precision)
{
    wchar sign;  /* sign printed : '+', '-', ' ', or 0 (no sign) */
    _number num_cpy = num;
    auto ul_num = (unsigned_type<_number>::acquire)num;
    /* string representation of num */
    wchar tmp[LONG_STRSIZE_BASE_2];
    int i = 0;
    const wchar* digits = L"0123456789ABCDEF";
    if(flags & SMALL)
        digits = L"0123456789abcdef";
    if((base < 2) || (base > 16))
        return;
    if((flags & SIGN) && (num < 0)) {
        sign = L'-';
        num = -num;
    }
    else {
        sign = (flags & PLUS) ? L'+' : ((flags & SPACE) ? L' ' : 0);
    }
    if(sign)
        wide --;
    if(flags & SPECIAL) {
        if((base == 16) && (num != 0)) wide -= 2;   /* '0x' or '0X' */
        if(base == 8) { wide --; precision --; }    /* '0' */
    }
    if(num == 0)
        tmp[i++] = L'0';
    /* signed format */
    if(flags & SIGN) {
        while(num != 0) {
            tmp[i ++] = digits[num % base];
            num /= base;
        }
    }
    /* unsigned format */
    else {
        while(ul_num != 0) {
            tmp[i ++] = digits[ul_num % base];
            ul_num /= base;
        }
    }
    if(i > precision)
        precision = i;
    wide -= precision;
    /* wide = number of padding chars */
    /* precision = number of figures after the sign and the special chars */
    /* right justified and no zeropad : pad with spaces */
    if(!(flags & (LEFT + ZERO))) {
        if(wide > 0)
            str.append(wide, L' ');
        wide = 0;
    }
    if(sign)
        str.push_back(sign);
    if((flags & SPECIAL) && (num_cpy != 0)) {
        if(base == 8)
            str.push_back(L'0');
        else if(base == 16) {
            if(flags & SMALL) str.append(L"0x");
            else str.append(L"0X");
        }
    }
    /* right justified and zeropad : pad with 0 */
    if(!(flags & LEFT)) {
        if(wide > 0) {
            str.append(wide, L'0');
            wide --;
        }
    }
    /* print num */
    if(i < precision)
        str.append(precision - i, L'0');
    if(i > 0) {
        for(i --; i >= 0; i --)
            str.push_back(tmp[i]);
    }
    /* left justified : pad with spaces */
    if(wide > 0)
        str.append(wide, L' ');
}

static void frac_to_string(_std_string<char>& str, double frac, int precision)
{
    assert(frac < 1.0);
    int ptd, sign;
    char* szf = ecvt(frac, precision, &ptd, &sign);
    int len = strlen(szf);
    if(len > precision) {
        assert(!"unexpected.");
        return;
    }
    assert(ptd <= 0);
    if(ptd < 0) {
        str.append(-ptd, '0');
        len += ptd;
    }
    str.append(szf, len);
}

static void floatp_to_string(_std_string<char>& str, double num, int wide, int precision)
{
    if(num < 0) {
        str.push_back('-');
        num = -num;
    }
    if(wide == 0)
        wide = -1;
    double i, f;
    f = modf(num, &i);
    _string<char> s;
    s.from_int((int)i);
    if(wide < 0) {
        str.append(s);
        if(precision < 0)
            precision = 6;
        str.push_back('.');
        frac_to_string(str, f, precision);
    }
    else {
        if(s.length() > wide) {
            str.push_back(s.front());
            str.push_back('.');
            if(wide - 1 > 0)
                str.append(s.c_str() + 1, wide - 1);
            str.push_back('e');
            _string<char> r;
            r.from_int(s.length() - 1);
            str.append(r);
            return;
        }
        if(precision < 0)
            precision = wide;
        str.append(s);
        str.push_back('.');
        int premax = wide - s.length();
        assert(premax >= 0);
        precision = gs_min(precision, premax);
        if(precision > 0)
            frac_to_string(str, f, precision);
    }
}

static void frac_to_string(_std_string<wchar>& str, double frac, int precision)
{
    assert(frac < 1.0);
    int ptd, sign;
    char* szf = ecvt(frac, precision, &ptd, &sign);
    int len = strlen(szf);
    if(len > precision) {
        assert(!"unexpected.");
        return;
    }
    assert(ptd <= 0);
    if(ptd < 0) {
        str.append(-ptd, L'0');
        len += ptd;
    }
    for(int i = 0; i < len; i ++)
        str.push_back(szf[i]);
}

static void floatp_to_string(_std_string<wchar>& str, double num, int wide, int precision)
{
    if(num < 0) {
        str.push_back(L'-');
        num = -num;
    }
    if(wide == 0)
        wide = -1;
    double i, f;
    f = modf(num, &i);
    _string<wchar> s;
    s.from_int((int)i);
    if(wide < 0) {
        str.append(s);
        if(precision < 0)
            precision = 6;
        str.push_back(L'.');
        frac_to_string(str, f, precision);
    }
    else {
        if(s.length() > wide) {
            str.push_back(s.front());
            str.push_back(L'.');
            if(wide - 1 > 0)
                str.append(s.c_str() + 1, wide - 1);
            str.push_back(L'e');
            _string<wchar> r;
            r.from_int(s.length() - 1);
            str.append(r);
            return;
        }
        if(precision < 0)
            precision = wide;
        str.append(s);
        str.push_back(L'.');
        int premax = wide - s.length();
        assert(premax >= 0);
        precision = gs_min(precision, premax);
        if(precision > 0)
            frac_to_string(str, f, precision);
    }
}

int _vsprintf(_std_string<char>& str, const char* fmt, va_list ap)
{
    assert(fmt);
    str.clear();
    char c, *s;
    wchar* ws;
    int flags, wide, precision, qualifier;
    int len, base;
    _string<char> cvt;
    while((c = *fmt++) != 0) {
        if(c != '%') { str.push_back(c); continue; }
        if(*fmt == '%') { str.push_back('%'); fmt ++; continue; }
        /* get flags */
        flags = 0;
        for(;;) {
            if(*fmt == '-') { flags |= LEFT; fmt ++; continue; }
            if(*fmt == '+') { flags |= PLUS; fmt ++; continue; }
            if(*fmt == ' ') { flags |= SPACE; fmt ++; continue; }
            if(*fmt == '#') { flags |= SPECIAL; fmt ++; continue; }
            if(*fmt == '0') { flags |= ZERO; fmt ++; continue; }
            break;
        }
        /* get wide */
        wide = -1;
        if(isdigit(*fmt)) wide = get_wide((const char **)(&fmt));
        else if(*fmt == '*') { wide = va_arg(ap, int); fmt ++; }
        /* get precision */
        precision = -1;
        if(*fmt == '.') {
            fmt ++;
            if(isdigit(*fmt))
                precision = get_wide((const char**)(&fmt));
            else if(*fmt == '*') {
                precision = va_arg(ap, int);
                fmt ++;
            }
            else
                precision = 0;
        }
        /* get qualifier */
        qualifier = -1;
        if((*fmt == 'h') || (*fmt == 'l'))
            qualifier = *fmt ++;
        /* get format */
        switch(*fmt ++)
        {
        case 'i':
        case 'd':
            flags |= SIGN;
            if(precision != -1)
                flags &= ~ZERO;
            switch(qualifier)
            {
            case 'h':
                number_to_string<int16>(str, (int16)va_arg(ap, int), 10, flags, wide, precision);
                break;
            case 'l':
                number_to_string<int64>(str, va_arg(ap, int64), 10, flags, wide, precision);
                break;
            default:
                number_to_string<int>(str, va_arg(ap, int), 10, flags, wide, precision);
                break;
            }
            break;
        case 'f':
            floatp_to_string(str, va_arg(ap, double), wide, precision);
            break;
        case 'u':
            base = 10;
            goto num_to_str_without_sign;
        case 'o':
            base = 8;
            goto num_to_str_without_sign;
        case 'x':
            flags |= SMALL;
        case 'X':
            base = 16;
        num_to_str_without_sign:
            flags &= (~PLUS & ~SPACE);
            if(precision != -1)
                flags &= ~ZERO;
            switch(qualifier)
            {
            case 'h':
                number_to_string<int16>(str, (int16)va_arg(ap, int), base, flags, wide, precision);
                break;
            case 'l':
                number_to_string<int64>(str, va_arg(ap, int64), base, flags, wide, precision);
                break;
            default:
                number_to_string<int>(str, va_arg(ap, int), base, flags, wide, precision);
                break;
            }
            break;
        case 's':
            if(qualifier == 'l') {
                ws = va_arg(ap, wchar*);
                len = wcslen(ws);
                if((precision >= 0) && (len > precision))
                    len = precision;
                /* right justified, pad with spaces */
                if(!(flags & LEFT)) {
                    if(len < wide)
                        str.append(wide - len, ' ');
                    wide = len;
                }
                cvt.from(ws);
                str.append(cvt);
                /* left justified, pad with spaces */
                if(len < wide)
                    str.append(wide - len, ' ');
            }
            else {
                s = va_arg(ap, char*);
                len = strlen(s);
                if((precision >= 0) && (len > precision))
                    len = precision;
                /* right justified, pad with spaces */
                if(!(flags & LEFT)) {
                    if(len < wide)
                        str.append(wide - len, ' ');
                    wide = len;
                }
                str.append(s, len);
                /* left justified, pad with spaces */
                if(len < wide)
                    str.append(wide - len, ' ');
            }
            break;
        case 'c':
            if(qualifier == 'l') {
                /* right justified, pad with spaces */
                if(!(flags & LEFT)) {
                    if(1 < wide)
                        str.append(wide - 1, ' ');
                    wide = 1;
                }
                str.push_back((unsigned char)va_arg(ap, int));
                /* left justified, pad with spaces */
                if(1 < wide)
                    str.append(wide - 1, ' ');
            }
            else {
                /* right justified, pad with spaces */
                if(!(flags & LEFT)) {
                    if(1 < wide)
                        str.append(wide - 1, ' ');
                    wide = 1;
                }
                str.push_back((unsigned char)va_arg(ap, int));
                /* left justified, pad with spaces */
                if(1 < wide)
                    str.append(wide - 1, ' ');
            }
            break;
        default:
            return -1;
        }
    }
    return str.length();
}

int _vsprintf(_std_string<wchar>& str, const wchar* fmt, va_list ap)
{
    assert(fmt);
    str.clear();
    wchar c, *s;
    int flags, wide, precision, qualifier;
    int len, base;
    _string<wchar> cvt;
    while((c = *fmt++) != 0) {
        if(c != L'%') { str.push_back(c); continue; }
        if(*fmt == L'%') { str.push_back(L'%'); fmt ++; continue; }
        /* get flags */
        flags = 0;
        for(;;) {
            if(*fmt == L'-') { flags |= LEFT; fmt ++; continue; }
            if(*fmt == L'+') { flags |= PLUS; fmt ++; continue; }
            if(*fmt == L' ') { flags |= SPACE; fmt ++; continue; }
            if(*fmt == L'#') { flags |= SPECIAL; fmt ++; continue; }
            if(*fmt == L'0') { flags |= ZERO; fmt ++; continue; }
            break;
        }
        /* get wide */
        wide = -1;
        if(iswdigit(*fmt)) wide = get_wide((const wchar **)(&fmt));
        else if(*fmt == L'*') { wide = va_arg(ap, int); fmt ++; }
        /* get precision */
        precision = -1;
        if(*fmt == L'.') {
            fmt ++;
            if(iswdigit(*fmt))
                precision = get_wide((const wchar**)(&fmt));
            else if(*fmt == L'*') {
                precision = va_arg(ap, int);
                fmt ++;
            }
            else
                precision = 0;
        }
        /* get qualifier */
        qualifier = -1;
        if((*fmt == L'h') || (*fmt == L'l'))
            qualifier = *fmt ++;
        /* get format */
        switch(*fmt ++)
        {
        case L'i':
        case L'd':
            flags |= SIGN;
            if(precision != -1)
                flags &= ~ZERO;
            switch(qualifier)
            {
            case L'h':
                number_to_string<int16>(str, (int16)va_arg(ap, int), 10, flags, wide, precision);
                break;
            case L'l':
                number_to_string<int64>(str, va_arg(ap, int64), 10, flags, wide, precision);
                break;
            default:
                number_to_string<int>(str, va_arg(ap, int), 10, flags, wide, precision);
                break;
            }
            break;
        case L'f':
            floatp_to_string(str, va_arg(ap, double), wide, precision);
            break;
        case L'u':
            base = 10;
            goto num_to_str_without_sign;
        case L'o':
            base = 8;
            goto num_to_str_without_sign;
        case L'x':
            flags |= SMALL;
        case L'X':
            base = 16;
        num_to_str_without_sign:
            flags &= (~PLUS & ~SPACE);
            if(precision != -1)
                flags &= ~ZERO;
            switch(qualifier)
            {
            case L'h':
                number_to_string<int16>(str, (int16)va_arg(ap, int), base, flags, wide, precision);
                break;
            case L'l':
                number_to_string<int64>(str, va_arg(ap, int64), base, flags, wide, precision);
                break;
            default:
                number_to_string<int>(str, va_arg(ap, int), base, flags, wide, precision);
                break;
            }
            break;
        case L's':
            s = va_arg(ap, wchar*);
            len = wcslen(s);
            if((precision >= 0) && (len > precision))
                len = precision;
            /* right justified, pad with spaces */
            if(!(flags & LEFT)) {
                if(len < wide)
                    str.append(wide - len, L' ');
                wide = len;
            }
            str.append(s, len);
            /* left justified, pad with spaces */
            if(len < wide)
                str.append(wide - len, L' ');
            /*
            ss = va_arg(ap, char*);
            len = strlen(ss);
            if((precision >= 0) && (len > precision))
                len = precision;
            if(!(flags & LEFT)) {
                if(len < wide)
                    str.append(wide - len, L' ');
                wide = len;
            }
            cvt.from(ss);
            str.append(cvt);
            if(len < wide)
                str.append(wide - len, L' ');
            */
            break;
        case L'c':
            if(qualifier == L'l') {
                /* right justified, pad with spaces */
                if(!(flags & LEFT)) {
                    if(1 < wide)
                        str.append(wide - 1, L' ');
                    wide = 1;
                }
                str.push_back(0x00ff & (wchar)va_arg(ap, int));
                /* left justified, pad with spaces */
                if(1 < wide)
                    str.append(wide - 1, L' ');
            }
            else {
                /* right justified, pad with spaces */
                if(!(flags & LEFT)) {
                    if(1 < wide)
                        str.append(wide - 1, L' ');
                    wide = 1;
                }
                str.push_back((wchar)va_arg(ap, int));
                /* left justified, pad with spaces */
                if(1 < wide)
                    str.append(wide - 1, L' ');
            }
            break;
        default:
            return -1;
        }
    }
    return str.length();
}

__gslib_end__
