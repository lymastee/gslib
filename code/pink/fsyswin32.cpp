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

#include <pink/fsyswin32.h>

__pink_begin__

static const int fplist_cap = 20;
static MAT2 _stand_mat = { {0,1}, {0,0}, {0,0}, {0,1} };

/* used for glyph buffers */
#define byte gs::byte
static byte* _glyph_buf = 0;
static int _gbsize = 0;

static byte* alloc_glyph_buf(int size = 1024, int predict = 0)
{
    if(!_glyph_buf) {
        _glyph_buf = (byte*)malloc(size);
        if(_glyph_buf)
            _gbsize = size;
        return _glyph_buf;
    }
    if(_gbsize >= size)
        return _glyph_buf;
    if(byte* ptr = (byte*)_expand(_glyph_buf, size)) {
        _gbsize = size;
        return _glyph_buf = ptr;
    }
    free(_glyph_buf);
    _glyph_buf = (byte*)malloc(size += predict);
    _gbsize = _glyph_buf ? size : 0;
    return _glyph_buf;
}

static void free_glyph_buf()
{
    if(_glyph_buf)
        free(_glyph_buf);
    _glyph_buf = 0;
    _gbsize = 0;
}

fsys_win32::fsys_win32()
{
    _container_dc = 0;
    _old_font = 0;
    _current = -1;
    alloc_glyph_buf();
}

fsys_win32::~fsys_win32()
{
    if(_old_font) {
        assert(_container_dc);
        SelectObject(_container_dc, _old_font);
        _old_font = 0;
    }
    if(_container_dc) {
        DeleteDC(_container_dc);
        _container_dc = 0;
    }
    for(fplist::iterator i = _fplist.begin(); i != _fplist.end(); ++ i)
        DeleteObject(i->_handle);
    _current = -1;
    free_glyph_buf();
}

void fsys_win32::initialize()
{
    HDC hdc = GetDC(0);
    assert(hdc);
    assert(!_container_dc);
    _container_dc = CreateCompatibleDC(hdc);
    ReleaseDC(0, hdc);
}

int fsys_win32::set_font(const font& f, int idx)
{
    int cap = (int)_fplist.size();
    if(idx >= 0 && idx < cap) {
        font_pair& fp = _fplist.at(idx);
        if(fp._font == f) {
            assert(_container_dc);
            HFONT h = (HFONT)SelectObject(_container_dc, fp._handle);
            if(!_old_font)
                _old_font = h;
            return _current = idx;
        }
    }
    for(int i = 0; i < cap; i ++) {
        font_pair& fp = _fplist.at(i);
        if(fp._font == f) {
            assert(_container_dc);
            HFONT h = (HFONT)SelectObject(_container_dc, fp._handle);
            if(!_old_font)
                _old_font = h;
            return _current = i;
        }
    }
    if(cap >= fplist_cap)
        _fplist.pop_front();
    int w = f.weight % 10;
    w *= 100;
    font_pair fp;
    fp._font = f;
    dword dw = f.height > 18 ? PROOF_QUALITY : DEFAULT_QUALITY;
    fp._handle = CreateFont(f.height, f.width, f.escape, f.orient, w, 
        (f.mask & font::ftm_italic) ? 1 : 0, 
        (f.mask & font::ftm_underline) ? 1 : 0, 
        (f.mask & font::ftm_strikeout) ? 1 : 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, dw, DEFAULT_PITCH, f.name.c_str()
        );
    assert(_container_dc);
    HFONT h = (HFONT)SelectObject(_container_dc, fp._handle);
    if(!_old_font)
        _old_font = h;
    _fplist.push_back(fp);
    return _current = (int)_fplist.size()-1;
}

bool fsys_win32::get_size(const gchar* str, int& w, int& h, int len)
{
    if(!str || _current == -1) {
        w = 0, h = 0;
        return false;
    }
    assert(_current >= 0 && _current < (int)_fplist.size());
    int slen = strtool::length(str);
    if(len < 0 || len > slen)
        len = slen;
    SIZE sz;
    BOOL b = GetTextExtentPoint32(_container_dc, str, len, &sz);
    assert(b);
    w = sz.cx;
    h = sz.cy;
    return true;
}

bool fsys_win32::convert(image& img, const gchar* str, int x, int y, const pixel& p, int len)
{
    if(!str || !img.is_valid() || _current == -1 || x < 0 || y < 0)
        return false;
    int w = img.get_width(), h = img.get_height();
    if(x > w || y > h)
        return false;
    assert(_glyph_buf && _gbsize);  /* out of memory! */
    assert(_current >= 0 && _current < (int)_fplist.size());
    GLYPHMETRICS gm;
    TEXTMETRIC tm;
    BOOL b = GetTextMetrics(_container_dc, &tm);
    assert(b);
    /* ensure there is enough memory buffer */
    int r = (int)GetGlyphOutline(_container_dc, 0xBCCD, GGO_GRAY8_BITMAP, &gm, 0, 0, &_stand_mat);
    if(r > _gbsize) {
        alloc_glyph_buf(r+1024);
        assert(_glyph_buf && _gbsize);
        if(!_glyph_buf || !_gbsize)
            return false;
    }
    if(len < 0)
        len = strtool::length(str);
    /* erase the background, get blank width */
    SIZE sz;
    GetTextExtentPoint32(_container_dc, str, len, &sz);
    rect rcclr(x, y, sz.cx, sz.cy);
    img.set_alpha(0, &rcclr);
    img.clear(p, &rcclr);
    GetTextExtentPoint32(_container_dc, _t(" "), 1, &sz);
    /* end condition */
    const gchar* end = str + len;
    for(;;) {
        uint c = 0;
        str = get_next_char(str, c, end);
        if(!c)  break;
        if(c == _t('\r') || c == _t('\n') || c == _t('\t'))
            continue;
        if(c == tm.tmBreakChar) {
            /* deal with blank! */
            memset(_glyph_buf, 0, _gbsize);
            memset(&gm, 0, sizeof(gm));
            gm.gmBlackBoxX = sz.cx;
            gm.gmCellIncX = (short)sz.cx;
        }
        else {
            /* normal char */
            GetGlyphOutline(_container_dc, c, GGO_GRAY8_BITMAP, &gm, _gbsize, _glyph_buf, &_stand_mat);
        }
        int u = gm.gmBlackBoxX, v = gm.gmBlackBoxY;
        int xbias = gm.gmptGlyphOrigin.x,
            ybias = tm.tmAscent - gm.gmptGlyphOrigin.y;
        if(u & 3) { u &= ~3; u += 4; }
        const byte* ptr = _glyph_buf;
        int vmax = gs_min(v, h-ybias) + y;
        for(int j = y, k = 0; j < vmax; j ++) {
            byte* des = img.get_alpha(j+ybias);
            assert(des);
            int grow = x + xbias;
            if(grow >= w)
                break;
            des += grow;
            int umax = gs_min(u, w-grow);
            for(int i = 0; i < umax; i ++) {
                int cb = (int)ptr[i];
                cb = (cb << 8) - cb;
                cb >>= 6;
                des[i] = cb;
            }
            ptr += u;
        }
        x += gm.gmCellIncX;
        y += gm.gmCellIncY;
    }
    return true;
}

void fsys_win32::draw(image& img, const gchar* str, int x, int y, const pixel& p, int len)
{
    if(!str || !img.is_valid() || _current == -1 || x < 0 || y < 0)
        return;
    int w = img.get_width(), h = img.get_height();
    if(x > w || y > h)
        return;
    assert(_glyph_buf && _gbsize);
    assert(_current >= 0 && _current < (int)_fplist.size());
    GLYPHMETRICS gm;
    TEXTMETRIC tm;
    BOOL b = GetTextMetrics(_container_dc, &tm);
    assert(b);
    /* ensure there is enough memory buffer */
    int r = (int)GetGlyphOutline(_container_dc, 0xBCCD, GGO_GRAY8_BITMAP, &gm, 0, 0, &_stand_mat);
    if(r > _gbsize) {
        alloc_glyph_buf(r+1024);
        assert(_glyph_buf && _gbsize);
        if(!_glyph_buf || !_gbsize)
            return;
    }
    /* get blank width */
    SIZE sz;
    GetTextExtentPoint32(_container_dc, _t(" "), 1, &sz);
    /* end condition */
    if(len < 0)
        len = strtool::length(str);
    const gchar* end = str + len;
    for(;;) {
        uint c = 0;
        str = get_next_char(str, c, end);
        if(!c)  break;
        if(c == _t('\r') || c == _t('\n') || c == _t('\t')) continue;
        if(c == tm.tmBreakChar) {
            /* deal with blank! */
            memset(_glyph_buf, 0, _gbsize);
            memset(&gm, 0, sizeof(gm));
            gm.gmBlackBoxX = sz.cx;
            gm.gmCellIncX = (short)sz.cx;
        }
        else {
            /* normal char */
            GetGlyphOutline(_container_dc, c, GGO_GRAY8_BITMAP, &gm, _gbsize, _glyph_buf, &_stand_mat);
        }
        int u = gm.gmBlackBoxX, v = gm.gmBlackBoxY;
        int xbias = gm.gmptGlyphOrigin.x,
            ybias = tm.tmAscent - gm.gmptGlyphOrigin.y;
        if(u & 3) { u &= ~3; u += 4; }
        const byte* ptr = _glyph_buf;
        int vmax = gs_min(y+v, h-ybias);
        for(int j = y, k = 0; j < vmax; j ++) {
            int grow = x + xbias;
            if(grow >= w)
                break;
            int umax = gs_min(u, w-grow);
            pixel* d = img.get_color(j+ybias) + grow;
            assert(d);
            for(int i = 0; i < umax; i ++, d ++) {
                if(!ptr[i]);
                else if(ptr[i] == 64)
                    *d = p;
                else {
                    byte a1 = (ptr[i] - 1) << 2;
                    byte a2 = ~a1;
                    d->red = (byte)(((int)d->red * a2 + (int)p.red * a1) >> 8);
                    d->green = (byte)(((int)d->green * a2 + (int)p.green * a1) >> 8);
                    d->blue = (byte)(((int)d->blue * a2 + (int)p.blue * a1) >> 8);
                }
            }
            ptr += u;
        }
        x += gm.gmCellIncX;
        y += gm.gmCellIncY;
    }
}

__pink_end__
