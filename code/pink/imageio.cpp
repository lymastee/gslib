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

#include <pink/imageio.h>
#include <pink/image.h>
#include <gslib/file.h>

__pink_begin__

static image_format_type detect_image_format(const string& path)
{
    int size = path.length();
    const gchar* dir = path.c_str();
    const gchar* pf = dir + size;
    for(; (pf - 1 != dir) && (pf[-1] != _t('.')); pf --);
    if(strtool::compare_cl(pf, _t("bmp"), 3) == 0 ||
        strtool::compare_cl(pf, _t("dib"), 3) == 0
        )
        return image_format_bmp;
    return image_format_unknown;
}

bool imageio::read_image(image& img, const string& path)
{
    file f(path.c_str(), _t("rb"));
    if(!f.is_valid())
        return false;
    int size = f.size();
    byte* buf = new byte[size];
    f.get(buf, size);
    auto fmt = detect_image_format(path);
    bool ret = false;
    switch(fmt)
    {
    case image_format_bmp:
        ret = read_bmp_image(img, buf, size);
        break;
    default:
        assert(!"unknown format.");
        ret = false;
    }
    delete [] buf;
    return ret;
}

struct bmp_file_hdr
{
    char    bf_type[2];         /* "BM" */
    int32   bf_size;
    int16   bf_reserved1;
    int16   bf_reserved2;
    int32   bf_off_bits;
};

struct bmp_info_hdr
{
    int32   bi_size;
    int32   bi_width;
    int32   bi_height;
    int16   bi_planes;
    int16   bi_bit_count;
    int32   bi_compression;
    int32   bi_size_image;
    int32   bi_xppm;            /* xpels per meter */
    int32   bi_yppm;            /* ypels per meter */
    int32   bi_clr_used;
    int32   bi_clr_important;
};

/* This structure is not fully supported, only used to support the alpha channel. */
struct bmp_info_hdr_v4:
    public bmp_info_hdr
{
    int32   bi_red_mask;
    int32   bi_green_mask;
    int32   bi_blue_mask;
    int32   bi_alpha_mask;      /* support alpha channel */
    int32   bi_cs_type;
    int32   bi_endpoints[9];    /* struct CIEXYZTRIPLE */
    int32   bi_gamma_red;
    int32   bi_gamma_green;
    int32   bi_gamma_blue;
};

struct bmp_info_hdr_v5:
    public bmp_info_hdr_v4
{
    int32   bi_intent;
    int32   bi_profile_data;
    int32   bi_profile_size;
    int32   bi_reserved;
};

struct bmp_handler_args
{
    int     width;
    int     height;
    int     comp;               /* compression */
    int     cr_depth;
    int     cr_table_size;
    uint*   cr_table;           /* NULL if doesn't exist */
    uint    red_mask;
    uint    green_mask;
    uint    blue_mask;
    uint    alpha_mask;
    int     red_shift;
    int     green_shift;
    int     blue_shift;
    int     alpha_shift;
    int     red_scale;
    int     green_scale;
    int     blue_scale;
    int     alpha_scale;
    bool    has_alpha_channel;
};

const int BMP_OLD  = 12;                        /* old Windows/OS2 BMP size */
const int BMP_WIN  = 40;                        /* Windows BMP v3 size */
const int BMP_OS2  = 64;                        /* new OS/2 BMP size */
const int BMP_WIN4 = 108;                       /* Windows BMP v4 size */
const int BMP_WIN5 = 124;                       /* Windows BMP v5 size */

const int BMP_RGB  = 0;                         /* no compression */
const int BMP_RLE8 = 1;                         /* run-length encoded, 8 bits */
const int BMP_RLE4 = 2;                         /* run-length encoded, 4 bits */
const int BMP_BITFIELDS = 3;                    /* RGB values encoded in data as bit-fields */

static bool read_dib_fileheader(bmp_file_hdr& fileheader, const void* ptr, int size)
{
    assert(ptr);
    if(size <= sizeof(bmp_file_hdr))
        return false;
    const char* bm = (const char*)ptr;
    if((bm[0] != 'B') || (bm[1] != 'M'))
        return false;
    memcpy_s(&fileheader, sizeof(bmp_file_hdr), ptr, size);
    return true;
}

template<class _info_hdr>
static bool read_dib_infoheader(_info_hdr& infoheader, const void* ptr, int size)
{
    assert(ptr);
    if(size <= sizeof(_info_hdr))
        return false;
    memcpy_s(&infoheader, sizeof(_info_hdr), ptr, size);
    int nbits = infoheader.bi_bit_count;
    int comp = infoheader.bi_compression;
    if (!(nbits == 1 || nbits == 4 || nbits == 8 || nbits == 16 || nbits == 24 || nbits == 32) ||
        infoheader.bi_planes != 1 || comp > BMP_BITFIELDS
        )
        return false;                               /* weird BMP image */
    if (!(comp == BMP_RGB || (nbits == 4 && comp == BMP_RLE4) ||
        (nbits == 8 && comp == BMP_RLE8) || ((nbits == 16 || nbits == 32) && comp == BMP_BITFIELDS))
        )
         return false;                              /* weird compression type */
    return true;
}

static inline int calc_shift(uint mask)
{
    int result = 0;
    __asm {
        bsf  eax, mask;
        mov  result, eax;
    }
    return result;
}

static bool setup_bmp_handler_args(bmp_handler_args& bhargs, const bmp_info_hdr& header, const byte* ptr, int size)
{
    assert(ptr);
    int nbits = header.bi_bit_count;
    int comp = header.bi_compression;
    switch(nbits)
    {
    case 32:
    case 24:
    case 16:
    case 8:
    case 4:
        bhargs.cr_depth = nbits;
        break;
    default:
        assert(!"unsupported format.");
        return false;
    }
    int ncols = 0;              /* color table */
    if(nbits <= 8) {            /* indexed color bmp */
        ncols = header.bi_clr_used ? header.bi_clr_used : (1 << nbits);
        assert(ncols > 0);
        if(ncols > 256)         /* sanity check */
            return false;
        bhargs.cr_table_size = ncols;
    }
    if(ncols > 0) {
        int rgb_len = (header.bi_size == BMP_OLD) ? 3 : 4;
        if(size < rgb_len * ncols) {
            assert(!"color table broken.");
            return false;
        }
        uint* crtable = new uint[ncols];
        const byte* p = ptr;
        /* probably should optimize with SIMD instructions */
        if(rgb_len == 3) {
            bhargs.has_alpha_channel = false;
            for(int i = 0; i < ncols; i ++) {
                crtable[i] = pink::color(p[2], p[1], p[0]).data();
                p += 3;
            }
        }
        else {
            assert(rgb_len == 4);
            bhargs.has_alpha_channel = true;
            for(int i = 0; i < ncols; i ++) {
                crtable[i] = pink::color(p[3], p[2], p[1], p[0]).data();
                p += 3;
            }
        }
        assert(!bhargs.cr_table);
        bhargs.cr_table = crtable;
    }
    else if((comp == BMP_BITFIELDS) && (nbits == 16 || nbits == 32)) {
        bhargs.red_shift = calc_shift(bhargs.red_mask);
        bhargs.red_scale = 256 / ((bhargs.red_mask >> bhargs.red_shift) + 1);
        bhargs.green_shift = calc_shift(bhargs.green_mask);
        bhargs.green_scale = 256 / ((bhargs.green_mask >> bhargs.green_shift) + 1);
        bhargs.blue_shift = calc_shift(bhargs.blue_mask);
        bhargs.blue_scale = 256 / ((bhargs.blue_mask >> bhargs.blue_shift) + 1);
        bhargs.alpha_shift = calc_shift(bhargs.alpha_mask);
        bhargs.alpha_scale = 256 / ((bhargs.alpha_mask >> bhargs.alpha_shift) + 1);
        bhargs.has_alpha_channel = (bhargs.alpha_mask != 0);
    }
    else if((comp == BMP_RGB) && (nbits == 24)) {
        bhargs.blue_mask = 0x000000ff;
        bhargs.green_mask = 0x0000ff00;
        bhargs.red_mask = 0x00ff0000;
        bhargs.blue_shift = 0;
        bhargs.green_shift = 8;
        bhargs.red_shift = 16;
        bhargs.blue_scale = bhargs.green_scale = bhargs.red_scale = 1;
        bhargs.has_alpha_channel = false;
    }
    else if((comp == BMP_RGB) && (nbits == 32)) {
        bhargs.blue_mask = 0x000000ff;
        bhargs.green_mask = 0x0000ff00;
        bhargs.red_mask = 0x00ff0000;
        bhargs.alpha_mask = 0xff000000;
        bhargs.blue_shift = 0;
        bhargs.green_shift = 8;
        bhargs.red_shift = 16;
        bhargs.alpha_shift = 24;
        bhargs.blue_scale = bhargs.green_scale = bhargs.red_scale = bhargs.alpha_scale = 1;
        bhargs.has_alpha_channel = true;
    }
    else if(comp == BMP_RGB && nbits == 16) {
        bhargs.blue_mask = 0x001f;
        bhargs.green_mask = 0x03e0;
        bhargs.red_mask = 0x7c00;
        bhargs.blue_shift = 0;
        bhargs.green_shift = 2;
        bhargs.red_shift = 7;
        bhargs.red_scale = 1;
        bhargs.green_scale = 1;
        bhargs.blue_scale = 8;
        bhargs.has_alpha_channel = false;
    }
    else {
        assert(!"unsupported format.");
        return false;
    }
    return true;
}

static bool setup_bmp_handler_args_v4(bmp_handler_args& bhargs, const bmp_info_hdr_v4& header, const byte* ptr, int size)
{
    bhargs.red_mask = header.bi_red_mask;
    bhargs.green_mask = header.bi_green_mask;
    bhargs.blue_mask = header.bi_blue_mask;
    bhargs.alpha_mask = header.bi_alpha_mask;
    return setup_bmp_handler_args(bhargs, header, ptr, size);
}

class image_data_stream
{
public:
    image_data_stream(const byte* ptr, int size)
    {
        assert(ptr);
        _ptr = ptr;
        _size = size;
        _pos = 0;
    }
    bool read_byte(byte& data) const
    {
        assert(_ptr);
        if(_pos < _size) {
            data = _ptr[_pos ++];
            return true;
        }
        return false;
    }
    int read_bytes(byte* p, int size) const
    {
        assert(p);
        int left = _size - _pos;
        if(left <= 0)
            return 0;
        if(size > left)
            size = left;
        memcpy_s(p, size, _ptr + _pos, size);
        _pos += size;
        return size;
    }

protected:
    const byte*             _ptr;
    int                     _size;
    mutable int             _pos;
};

static bool convert_indices_to_colors(image& img, const bmp_handler_args& bhargs, const byte* indices, int bpl)
{
    assert(indices);
    assert(bhargs.cr_table && "indexed bmp must have a color table.");
    int w = bhargs.width, h = bhargs.height;
    if(bhargs.cr_depth == 24) {
        for(int y = 0; y < h; y ++) {
            int i = 0;
            byte* d = img.get_data(0, y);
            const byte* n = indices + (y * bpl);
            for(int x = 0; x < w; x += 3) {
                color cr;
                cr.data() = bhargs.cr_table[n[i ++]];
                d[x] = cr.red;
                d[x + 1] = cr.green;
                d[x + 2] = cr.blue;
            }
        }
    }
    else if(bhargs.cr_depth == 32) {
        for(int y = 0; y < h; y ++) {
            int i = 0;
            byte* d = img.get_data(0, y);
            const byte* n = indices + (y * bpl);
            for(int x = 0; x < w; x += 4)
                *(uint*)(d + x) = bhargs.cr_table[n[i ++]];
        }
    }
    else {
        assert(!"unexpected error.");
        return false;
    }
    return true;
}

static bool read_dib_rle4(image& img, const bmp_handler_args& bhargs, const byte* src, int size)
{
    assert(src && (size > 0));
    int w = bhargs.width, h = bhargs.height;
    int bpl = ((w * sizeof(byte) + 31) >> 5) << 2;
    byte* data = new byte[bpl * h];
    int x = 0, y = 0, c, i;
    byte b;
    image_data_stream d(src, size);
    register byte* p = data + (h - 1) * bpl;
    const byte* endp = p + w;
    while(y < h) {
        if(!d.read_byte(b))
            break;
        if(b == 0) {                /* escape code */
            if(!d.read_byte(b) || b == 1)
                y = h;              /* exit loop */
            else {
                switch(b)
                {
                case 0:             /* end of line */
                    x = 0;
                    y ++;
                    p = data + (h - y - 1) * bpl;
                    break;
                case 2:             /* delta (jump) */
                    {
                        byte tmp;
                        d.read_byte(tmp);
                        x += tmp;
                        d.read_byte(tmp);
                        y += tmp;
                    }
                    /* protection */
                    if((uint)x >= (uint)w)
                        x = w - 1;
                    if((uint)y >= (uint)h)
                        y = h - 1;
                    p = data + (h - y - 1) * bpl + x;
                    break;
                default:            /* absolute mode */
                    /* protection */
                    if(p + b > endp)
                        b = endp - p;
                    i = (c = b) / 2;
                    while(i --) {
                        d.read_byte(b);
                        *p++ = b >> 4;
                        *p++ = b & 0x0f;
                    }
                    if(c & 1) {
                        byte tmp;
                        d.read_byte(tmp);
                        *p++ = tmp >> 4;
                    }
                    if((((c & 3) + 1) & 2) == 2) {
                        byte cast;
                        d.read_byte(cast);      /* align on word boundary */
                    }
                    x += c;
                }
            }
        }
        else {                      /* encoded mode */
            /* protection */
            if(p + b > endp)
                b = endp - p;
            i = (c = b) / 2;
            d.read_byte(b);         /* 2 pixels to be repeated */
            while(i --) {
                *p++ = b >> 4;
                *p++ = b & 0x0f;
            }
            if(c & 1)
                *p++ = b >> 4;
            x += c;
        }
    }
    bool ret = convert_indices_to_colors(img, bhargs, data, bpl);
    delete [] data;
    return ret;
}

static bool read_dib_uncomp4(image& img, const bmp_handler_args& bhargs, const byte* src, int size)
{
    assert(src && (size > 0));
    int w = bhargs.width, h = bhargs.height;
    int bpl = ((w * sizeof(byte) + 31) >> 5) << 2;
    int datalen = bpl * h;
    byte* data = new byte[datalen];
    memset(data, 0, datalen);
    int buflen = ((w + 7) / 8) * 4;
    byte* buf = new byte[buflen];
    image_data_stream d(src, size);
    while(--h >= 0) {
        if(d.read_bytes(buf, buflen) != buflen)
            break;
        register byte* p = data + h * bpl;
        byte* b = buf;
        for(int i = 0; i < w / 2; i ++) {
            *p++ = *b >> 4;
            *p++ = *b++ & 0x0f;
        }
        if(w & 1)
            *p = *b >> 4;
    }
    bool ret = convert_indices_to_colors(img, bhargs, data, bpl);
    delete [] buf;
    delete [] data;
    return ret;
}

static bool read_dib_rle8(image& img, const bmp_handler_args& bhargs, const byte* src, int size)
{
    assert(src && (size > 0));
    int w = bhargs.width, h = bhargs.height;
    int bpl = ((w * sizeof(byte) + 31) >> 5) << 2;
    byte* data = new byte[bpl * h];
    int x = 0, y = 0;
    byte b;
    image_data_stream d(src, size);
    register byte* p = data + (h - 1) * bpl;
    const byte* endp = p + w;
    while(y < h) {
        if(!d.read_byte(b))
            break;
        if(b == 0) {                /* escape code */
            if(!d.read_byte(b) || b == 1)
                y = h;              /* exit loop */
            else {
                switch(b)
                {
                case 0:             /* end of line */
                    x = 0;
                    y ++;
                    p = data + (h - y - 1) * bpl;
                    break;
                case 2:             /* delta (jump) */
                    /* protection */
                    if((uint)x >= (uint)w)
                        x = w - 1;
                    if((uint)y >= (uint)h)
                        y = h - 1;
                    {
                        byte tmp;
                        d.read_byte(tmp);
                        x += tmp;
                        d.read_byte(tmp);
                        y += tmp;
                    }
                    p = data + (h - y - 1) * bpl + x;
                    break;
                default:            /* absolute mode */
                    /* protection */
                    if(p + b > endp)
                        b = endp - p;
                    if(d.read_bytes(p, b) != b)
                        return false;
                    if((b & 1) == 1) {
                        byte cast;
                        d.read_byte(cast);      /* align on word boundary */
                    }
                    x += b;
                    p += b;
                }
            }
        }
        else {                      /* encoded mode */
            /* protection */
            if(p + b > endp)
                b = endp - p;
            byte tmp;
            d.read_byte(tmp);
            memset(p, tmp, b);      /* repeat pixel */
            x += b;
            p += b;
        }
    }
    bool ret = convert_indices_to_colors(img, bhargs, data, bpl);
    delete [] data;
    return ret;
}

static bool read_dib_uncomp8(image& img, const bmp_handler_args& bhargs, const byte* src, int size)
{
    assert(src && (size > 0));
    int w = bhargs.width, h = bhargs.height;
    int bpl = ((w * sizeof(byte) + 31) >> 5) << 2;
    byte* data = new byte[bpl * h];
    image_data_stream d(src, size);
    while(--h >= 0) {
        if(d.read_bytes(data + h * bpl, bpl) != bpl)
            break;
    }
    bool ret = convert_indices_to_colors(img, bhargs, data, bpl);
    delete [] data;
    return ret;
}

static bool read_dib_colors(image& img, const bmp_handler_args& bhargs, const byte* src, int size)
{
    assert(src && (size > 0));
    int w = bhargs.width, h = bhargs.height;
    int nbits = bhargs.cr_depth;
    int bpl = img.get_bytes_per_line();
    byte* buf24 = new byte[bpl];
    int bpl24 = ((w * nbits + 31) / 32) * 4;
    byte* data = img.get_data(0, 0);
    image_data_stream d(src, size);
    auto red_mask = bhargs.red_mask, green_mask = bhargs.green_mask, blue_mask = bhargs.blue_mask, alpha_mask = bhargs.alpha_mask;
    auto red_shift = bhargs.red_shift, green_shift = bhargs.green_shift, blue_shift = bhargs.blue_shift, alpha_shift = bhargs.alpha_shift;
    auto red_scale = bhargs.red_scale, green_scale = bhargs.green_scale, blue_scale = bhargs.blue_scale, alpha_scale = bhargs.alpha_scale;
    if(bhargs.has_alpha_channel) {
        assert(nbits == 32);
        while(--h >= 0) {
            register uint* p = (uint*)(data + h * bpl);
            uint* end = p + w;
            if(d.read_bytes(buf24, bpl24) != bpl24)
                break;
            byte* b = buf24;
            while(p < end) {
                uint c = *(byte*)b | (*(byte*)(b + 1) << 8);
                if(nbits != 16)
                    c |= *(byte*)(b + 2) << 16;
                *p++ = color(
                    ((c & red_mask) >> red_shift) * red_scale,
                    ((c & green_mask) >> green_shift) * green_scale,
                    ((c & blue_mask) >> blue_shift) * blue_scale,
                    ((c & alpha_mask) >> alpha_shift) * alpha_scale
                    ).data();
                b += nbits / 8;
            }
        }
    }
    else {
        while(--h >= 0) {
            register byte* p = data + h * bpl;
            byte* end = p + w;
            if(d.read_bytes(buf24, bpl24) != bpl24)
                break;
            byte* b = buf24;
            while(p < end) {
                uint c = *(byte*)b | (*(byte*)(b + 1) << 8);
                if(nbits != 16)
                    c |= *(byte*)(b + 2) << 16;
                *p++ = ((c & red_mask) >> red_shift) * red_scale;
                *p++ = ((c & green_mask) >> green_shift) * green_scale;
                *p++ = ((c & blue_mask) >> blue_shift) * blue_scale;
                b += nbits / 8;
            }
        }
    }
    delete [] buf24;
    return true;
}

bool imageio::read_bmp_image(image& img, const void* ptr, int size)
{
    auto* p = static_cast<const byte*>(ptr);
    bmp_file_hdr bf;
    if(!read_dib_fileheader(bf, p, size))
        return false;
    p += sizeof(bmp_file_hdr);
    size -= sizeof(bmp_file_hdr);
    bmp_info_hdr_v5 bi;
    if(!read_dib_infoheader(static_cast<bmp_info_hdr&>(bi), p, size))   /* pre-read a normal bmp header */
        return false;
    int comp = bi.bi_compression;
    int nbits = bi.bi_bit_count;
    bmp_handler_args bhargs;
    memset(&bhargs, 0, sizeof(bhargs));
    if((bi.bi_size >= BMP_WIN4) || (comp == BMP_BITFIELDS && (nbits == 16 || nbits == 32))) {
        const byte* palasu_pos = 0;
        int pal_size = 0;
        if(bi.bi_size == BMP_WIN5) {
            if(!read_dib_infoheader(bi, p, size))
                return false;
            palasu_pos = p + sizeof(bmp_info_hdr_v5);
            pal_size = bf.bf_off_bits - sizeof(bmp_info_hdr_v5);
        }
        else {
            if(!read_dib_infoheader(static_cast<bmp_info_hdr_v4>(bi), p, size))
                return false;
            palasu_pos = p + sizeof(bmp_info_hdr_v4);
            pal_size = bf.bf_off_bits - sizeof(bmp_info_hdr_v4);
        }
        if(pal_size <= 0) {
            pal_size = 0;
            palasu_pos = 0;
        }
        if(!setup_bmp_handler_args_v4(bhargs, bi, palasu_pos, pal_size))
            return false;
    }
    else {
        const byte* palasu_pos = p + sizeof(bmp_info_hdr);
        int pal_size = bf.bf_off_bits - sizeof(bmp_info_hdr);
        if(!setup_bmp_handler_args(bhargs, bi, palasu_pos, pal_size))
            return false;
    }
    p += bf.bf_off_bits;
    size -= bf.bf_off_bits;
    /* create image data here */
    auto fmt = bhargs.has_alpha_channel ? image::fmt_rgba : image::fmt_rgb;
    img.create(fmt, bhargs.width, bhargs.height);
    img._xpels_per_meter = bi.bi_xppm;
    img._ypels_per_meter = bi.bi_yppm;
    if(nbits == 4) {
        if(comp == BMP_RLE4) {
            if(!read_dib_rle4(img, bhargs, p, size))
                return false;
        }
        else if(comp == BMP_RGB) {
            if(!read_dib_uncomp4(img, bhargs, p, size))
                return false;
        }
        else {
            assert(!"unexpected error.");
            return false;
        }
    }
    else if(nbits == 8) {
        if(comp == BMP_RLE8) {
            if(!read_dib_rle8(img, bhargs, p, size))
                return false;
            else if(!read_dib_uncomp8(img, bhargs, p, size))
                return false;
            else {
                assert(!"unexpected error.");
                return false;
            }
        }
    }
    else if(nbits == 16 || nbits == 24 || nbits == 32) {
        if(!read_dib_colors(img, bhargs, p, size))
            return false;
    }
    else {
        assert(!"bad image format.");
        return false;
    }
    return true;
}

__pink_end__
