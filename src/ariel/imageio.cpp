/*
 * Copyright (c) 2016-2021 lymastee, All rights reserved.
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

#include <ariel/imageio.h>
#include <ariel/image.h>
#include <gslib/file.h>
#include <gslib/error.h>

extern "C" {
#include <libjpeg/jpeglib.h>
#include <libpng/png.h>
#include <libpng/pngconf.h>
}

__ariel_begin__

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
    else if(strtool::compare_cl(pf, _t("jpg"), 3) == 0 ||
        strtool::compare_cl(pf, _t("jpeg"), 4) == 0 ||
        strtool::compare_cl(pf, _t("jfif"), 4) == 0
        )
        return image_format_jpg;
    else if(strtool::compare_cl(pf, _t("png"), 3) == 0)
        return image_format_png;
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
    case image_format_png:
        ret = read_png_image(img, buf, size);
        break;
    case image_format_jpg:
        ret = read_jpg_image(img, buf, size);
        break;
    default:
        assert(!"unknown format.");
        ret = false;
    }
    delete [] buf;
    return ret;
}

bool imageio::save_image(const image& img, const string& path)
{
    auto fmt = detect_image_format(path);
    switch(fmt)
    {
    case image_format_bmp:
        return save_bmp_image(img, path);
    case image_format_png:
        return save_png_image(img, path);
    case image_format_jpg:
        return save_jpg_image(img, path);
    default:
        assert(!"unknown image type.");
        return false;
    }
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

const int BMP_FILEHDR_SIZE = 14;

const int BMP_OLD  = 12;                        /* old Windows/OS2 BMP size */
const int BMP_WIN  = 40;                        /* Windows BMP v3 size */
const int BMP_OS2  = 64;                        /* new OS/2 BMP size */
const int BMP_WIN4 = 108;                       /* Windows BMP v4 size */
const int BMP_WIN5 = 124;                       /* Windows BMP v5 size */

const int BMP_RGB  = 0;                         /* no compression */
const int BMP_RLE8 = 1;                         /* run-length encoded, 8 bits */
const int BMP_RLE4 = 2;                         /* run-length encoded, 4 bits */
const int BMP_BITFIELDS = 3;                    /* RGB values encoded in data as bit-fields */

static int convert_dpi_to_dpm(int dpi)
{
    return (int)((float)dpi / 0.0254f + 0.5f);
}

static int convert_dpm_to_dpi(int dpm)
{
    return (int)((float)dpm * 0.0254f + 0.5f);
}

static bool read_dib_fileheader(bmp_file_hdr& fileheader, const void* ptr, int size)
{
    assert(ptr);
    if(size <= sizeof(bmp_file_hdr))
        return false;
    const char* bm = (const char*)ptr;
    if((bm[0] != 'B') || (bm[1] != 'M'))
        return false;
    memcpy_s(&fileheader.bf_type, 2, "BM", 2);
    memcpy_s(&fileheader.bf_size, 12, (char*)ptr + 2, 12);
    return true;
}

template<class _info_hdr>
static bool read_dib_infoheader(_info_hdr& infoheader, const void* ptr, int size)
{
    assert(ptr);
    if(size <= sizeof(_info_hdr))
        return false;
    memcpy_s(&infoheader, sizeof(_info_hdr), ptr, sizeof(_info_hdr));
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
    unsigned long result = 0;
    _BitScanForward(&result, mask);
    return (int)result;
}

static bool setup_bmp_handler_args(bmp_handler_args& bhargs, const bmp_info_hdr& header, const byte* ptr, int size)
{
    assert(ptr);
    bhargs.width = header.bi_width;
    bhargs.height = header.bi_height;
    bhargs.comp = header.bi_compression;
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
                crtable[i] = color(p[2], p[1], p[0]).data();
                p += 3;
            }
        }
        else {
            assert(rgb_len == 4);
            bhargs.has_alpha_channel = true;
            for(int i = 0; i < ncols; i ++) {
                crtable[i] = color(p[3], p[2], p[1], p[0]).data();
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
    if(!setup_bmp_handler_args(bhargs, header, ptr, size))
        return false;
    bhargs.red_mask = header.bi_red_mask;
    bhargs.green_mask = header.bi_green_mask;
    bhargs.blue_mask = header.bi_blue_mask;
    bhargs.alpha_mask = header.bi_alpha_mask;
    return true;
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
        memcpy(p, _ptr + _pos, size);
        _pos += size;
        return size;
    }
    void seek(int p) { _pos = p; }
    int current_pos() const { return _pos; }
    const byte* get_data() const { return _ptr; }
    int get_size() const { return _size; }

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
    int bpl = ((w * 32 + 31) >> 5) << 2;
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
    int bpl = ((w * 32 + 31) >> 5) << 2;
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
    int bpl = ((w * 32 + 31) >> 5) << 2;
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
    int bpl = ((w * 32 + 31) >> 5) << 2;
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
                    0xff
                    ).data();
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
    p += BMP_FILEHDR_SIZE;
    size -= BMP_FILEHDR_SIZE;
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
    int offset = bf.bf_off_bits - BMP_FILEHDR_SIZE;
    p += offset;
    size -= offset;
    /* create image data here */
    auto fmt = image::fmt_rgba;
    img.create(fmt, bhargs.width, bhargs.height);
    img._xdpi = convert_dpm_to_dpi(bi.bi_xppm);
    img._ydpi = convert_dpm_to_dpi(bi.bi_yppm);
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
    img.enable_alpha_channel(bhargs.has_alpha_channel);
    return true;
}

bool imageio::save_bmp_image(const image& img, const string& path)
{
    assert(img.get_depth() == 32 && "currently support 32 bits only.");
    /* write file header */
    int bpl = img.get_bytes_per_line();
    int bpl_bmp = bpl;
    if(!img.has_alpha())
        bpl_bmp = ((img.get_width() * 24 + 31) / 32) * 4;
    int nbits = img.has_alpha() ? 32 : 24;
    bmp_file_hdr fileheader;
    memcpy(fileheader.bf_type, "BM", 2);
    fileheader.bf_reserved1 = fileheader.bf_reserved2 = 0;
    fileheader.bf_off_bits = BMP_FILEHDR_SIZE + BMP_WIN;        /* no color table */
    fileheader.bf_size = fileheader.bf_off_bits + bpl_bmp * img.get_height();
    file f;
    f.open(path.c_str(), _t("wb"));
    f.put((byte*)"BM", 2);
    f.put(reinterpret_cast<byte*>(&fileheader.bf_size), sizeof(fileheader) - 4);
    /* write image header */
    bmp_info_hdr bmpheader;
    bmpheader.bi_size = BMP_WIN;
    bmpheader.bi_width = img.get_width();
    bmpheader.bi_height = img.get_height();
    bmpheader.bi_planes = 1;
    bmpheader.bi_bit_count = nbits;
    bmpheader.bi_compression = BMP_RGB;
    bmpheader.bi_size_image = bpl_bmp * img.get_height();
    bmpheader.bi_xppm = convert_dpi_to_dpm(img.get_xdpi());
    bmpheader.bi_yppm = convert_dpi_to_dpm(img.get_ydpi());
    bmpheader.bi_clr_used = bmpheader.bi_clr_important = 0;
    f.put(reinterpret_cast<byte*>(&bmpheader), sizeof(bmpheader));
    /* write image data */
    if(img.has_alpha()) {
        f.put(img.get_data(0, 0), img.get_size());
        return true;
    }
    /* convert to 24bit bmp */
    byte* buf = new byte[bpl_bmp];
    for(int y = img.get_height() - 1; y >= 0; y --) {
        byte* p = img.get_data(0, y);
        byte* end = p + (img.get_width() * 4);
        byte* b = buf;
        while(p < end) {
            *b++ = p[2], *b++ = p[1], *b++ = p[0];
            p += 4;
        }
        f.put(buf, bpl_bmp);
    }
    delete [] buf;
    return true;
}

void my_jpeg_init_source(j_decompress_ptr);
boolean my_jpeg_fill_input_buffer(j_decompress_ptr cinfo);
void my_jpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes);
void my_jpeg_term_source(j_decompress_ptr cinfo);

struct my_jpeg_source_mgr:
    public jpeg_source_mgr
{
    static const int max_buf = 4096;
    image_data_stream*  device;
    JOCTET  buffer[max_buf];

public:
    my_jpeg_source_mgr(image_data_stream* p)
    {
        assert(p);
        jpeg_source_mgr::init_source = my_jpeg_init_source;
        jpeg_source_mgr::fill_input_buffer = my_jpeg_fill_input_buffer;
        jpeg_source_mgr::skip_input_data = my_jpeg_skip_input_data;
        jpeg_source_mgr::resync_to_restart = jpeg_resync_to_restart;
        jpeg_source_mgr::term_source = my_jpeg_term_source;
        device = p;
        bytes_in_buffer = 0;
        next_input_byte = buffer;
    }
};

static void my_jpeg_init_source(j_decompress_ptr)
{
}

static boolean my_jpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
    auto* src = reinterpret_cast<my_jpeg_source_mgr*>(cinfo->src);
    src->next_input_byte = (const JOCTET*)(src->device->get_data() + src->device->current_pos());
    int num_read = src->device->get_size() - src->device->current_pos();
    src->device->seek(src->device->get_size());
    if(num_read <= 0) {
        /* Insert a fake EOI marker - as per jpeglib recommendation */
        src->next_input_byte = src->buffer;
        src->buffer[0] = (JOCTET)0xff;
        src->buffer[1] = (JOCTET)JPEG_EOI;
        src->bytes_in_buffer = 2;
    }
    else {
        src->bytes_in_buffer = num_read;
    }
    return true;
}

static void my_jpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    auto* src = reinterpret_cast<my_jpeg_source_mgr*>(cinfo->src);
    if(num_bytes > 0) {
        while(num_bytes > (long)src->bytes_in_buffer) {
            num_bytes -= (long)src->bytes_in_buffer;
            (void)my_jpeg_fill_input_buffer(cinfo);
        }
        src->next_input_byte += (size_t)num_bytes;
        src->bytes_in_buffer -= (size_t)num_bytes;
    }
}

static void my_jpeg_term_source(j_decompress_ptr cinfo)
{
    auto* src = reinterpret_cast<my_jpeg_source_mgr*>(cinfo->src);
    assert(src);
    src->device->seek(src->device->current_pos() - src->bytes_in_buffer);
}

struct my_jpeg_error_mgr:
    public jpeg_error_mgr
{
    jmp_buf setjmp_buffer;
};

static void my_jpeg_error_exit(j_common_ptr cinfo)
{
    my_jpeg_error_mgr* myerr = reinterpret_cast<my_jpeg_error_mgr*>(cinfo->err);
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);
    longjmp(myerr->setjmp_buffer, 1);
}

static bool read_jpeg_size(int &w, int &h, j_decompress_ptr cinfo)
{
    (void)jpeg_calc_output_dimensions(cinfo);
    w = cinfo->output_width;
    h = cinfo->output_height;
    return true;
}

static bool read_jpeg_format(image::image_format& format, j_decompress_ptr cinfo)
{
    bool result = true;
    switch(cinfo->output_components)
    {
    case 1:
        format = image::fmt_gray;
        break;
    case 3:
    case 4:
        format = image::fmt_rgba;
        break;
    default:
        result = false;
        break;
    }
    cinfo->output_scanline = cinfo->output_height;
    return result;
}

class jpeg_reader
{
public:
    jpeg_reader():
        quality(75), iod_src(nullptr), width(0), height(0), format(image::fmt_rgba)
    {
    }
    ~jpeg_reader()
    {
        if(iod_src) {
            jpeg_destroy_decompress(&info);
            delete iod_src;
            iod_src = nullptr;
        }
    }
    bool read_header(image_data_stream& ds);
    bool read(image_data_stream& ds, image& img);

    int quality;
    int width;
    int height;
    image::image_format format;
    jpeg_decompress_struct info;
    my_jpeg_source_mgr* iod_src;
    my_jpeg_error_mgr err;
};

bool jpeg_reader::read_header(image_data_stream& ds)
{
    iod_src = new my_jpeg_source_mgr(&ds);
    jpeg_create_decompress(&info);
    info.src = iod_src;
    info.err = jpeg_std_error(&err);
    err.error_exit = my_jpeg_error_exit;
    if(!setjmp(err.setjmp_buffer)) {
        (void)jpeg_read_header(&info, true);
        width = height = 0;
        return read_jpeg_size(width, height, &info) && read_jpeg_format(format, &info);
    }
    return false;
}

static bool ensure_valid_image(image& img, jpeg_decompress_struct* info, int width, int height)
{
    image::image_format format;
    switch(info->output_components)
    {
    case 1:
        format = image::fmt_gray;
        break;
    case 3:
    case 4:
        format = image::fmt_rgba;
        break;
    default:
        return false;
    }
    if(img.is_valid()) {
        if((img.get_width() != width) || (img.get_height() != height) ||
            (img.get_format() != format)
            )
            img.destroy();
    }
    if(!img.is_valid())
        img.create(format, width, height);
    return img.is_valid();
}

static void convert_rgb888_to_rgb32(uint* dst, const byte* src, int len)
{
    for(int i = 0; i < len; i ++) {
        *dst++ = color(src[0], src[1], src[2]).data();
        src += 3;
    }
}

bool jpeg_reader::read(image_data_stream& ds, image& img)
{
    if(!read_header(ds))
        return false;
    if(!setjmp(err.setjmp_buffer)) {
        /* disable scaling */
        info.scale_num = info.scale_denom = 1;
        /* if high quality not required, use fast decompression */
        if(quality < 50) {
            info.dct_method = JDCT_IFAST;
            info.do_fancy_upsampling = false;
        }
        (void)jpeg_calc_output_dimensions(&info);
        if((width != info.output_width) || (height != info.output_height))
            longjmp(err.setjmp_buffer, 1);
        if(!ensure_valid_image(img, &info, width, height))
            longjmp(err.setjmp_buffer, 1);
        if(info.output_components != 1) {
            JSAMPARRAY rows = (info.mem->alloc_sarray)((j_common_ptr)&info, JPOOL_IMAGE, width * info.output_components, 1);
            (void)jpeg_start_decompress(&info);
            while(info.output_scanline < info.output_height) {
                int y = info.output_scanline;
                (void)jpeg_read_scanlines(&info, rows, 1);
                byte* in = rows[0];
                uint* out = (uint*)img.get_data(0, y);
                if(info.output_components == 3)
                    convert_rgb888_to_rgb32(out, in, width);
                else if(info.out_color_space == JCS_CMYK) {
                    /* convert CMYK to RGB */
                    for(int i = 0; i < width; i ++) {
                        int k = in[3];
                        *out ++ = color(k * in[0] / 255, k * in[1] / 255, k * in[2] / 255).data();
                        in += 4;
                    }
                }
            }
        }
        else {      /* info.output_components == 1 */
            (void)jpeg_start_decompress(&info);
            while(info.output_scanline < info.output_height) {
                byte* row = img.get_data(0, info.output_scanline);
                (void)jpeg_read_scanlines(&info, &row, 1);
            }
        }
        (void)jpeg_finish_decompress(&info);
        if(info.density_unit <= 1) {
            img.set_xdpi(info.X_density);
            img.set_ydpi(info.Y_density);
        }
        else if(info.density_unit == 2) {
            img.set_xdpi((int)(2.54f * info.X_density + 0.5f));
            img.set_ydpi((int)(2.54f * info.Y_density + 0.5f));
        }
        img.enable_alpha_channel(false);    /* jpeg DOESNOT have alpha channel */
        return img.is_valid();
    }
    return false;
}

bool imageio::read_jpg_image(image& img, const void* ptr, int size)
{
    image_data_stream ds(reinterpret_cast<const byte*>(ptr), size);
    jpeg_reader reader;
    return reader.read(ds, img);
}

void my_jpeg_init_destination(j_compress_ptr);
boolean my_jpeg_empty_output_buffer(j_compress_ptr);
void my_jpeg_term_destination(j_compress_ptr);

struct my_jpeg_destination_mgr:
    public jpeg_destination_mgr
{
    static const int max_buf = 4096;
    file&   device;
    JOCTET  buffer[max_buf];

public:
    my_jpeg_destination_mgr(file& f): device(f)
    {
        jpeg_destination_mgr::init_destination = my_jpeg_init_destination;
        jpeg_destination_mgr::empty_output_buffer = my_jpeg_empty_output_buffer;
        jpeg_destination_mgr::term_destination = my_jpeg_term_destination;
        next_output_byte = buffer;
        free_in_buffer = max_buf;
    }
};

static void my_jpeg_init_destination(j_compress_ptr)
{
}

static boolean my_jpeg_empty_output_buffer(j_compress_ptr cinfo)
{
    auto* dest = reinterpret_cast<my_jpeg_destination_mgr*>(cinfo->dest);
    int written = dest->device.put((byte*)dest->buffer, my_jpeg_destination_mgr::max_buf);
    if(written != my_jpeg_destination_mgr::max_buf)
        (*cinfo->err->error_exit)((j_common_ptr)cinfo);
    dest->next_output_byte = dest->buffer;
    dest->free_in_buffer = my_jpeg_destination_mgr::max_buf;
    return true;
}

static void my_jpeg_term_destination(j_compress_ptr cinfo)
{
    auto* dest = reinterpret_cast<my_jpeg_destination_mgr*>(cinfo->dest);
    int n = my_jpeg_destination_mgr::max_buf - dest->free_in_buffer;
    int written = dest->device.put((byte*)dest->buffer, n);
    if(written != n)
        (*cinfo->err->error_exit)((j_common_ptr)cinfo);
}

bool imageio::save_jpg_image(const image& img, const string& path)
{
    bool success;
    jpeg_compress_struct cinfo;
    JSAMPROW row_pointer[1];
    row_pointer[0] = nullptr;
    file f;
    f.open(path.c_str(), _t("wb"));
    my_jpeg_destination_mgr* iod_dest = new my_jpeg_destination_mgr(f);
    my_jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = my_jpeg_error_exit;
    if(!setjmp(jerr.setjmp_buffer)) {
        jpeg_create_compress(&cinfo);
        cinfo.dest = iod_dest;
        cinfo.image_width = img.get_width();
        cinfo.image_height = img.get_height();
        bool gray = false;
        switch(img.get_format())
        {
        case image::fmt_gray:
            gray = true;
            cinfo.input_components = 1;
            cinfo.in_color_space = JCS_GRAYSCALE;
            break;
        case image::fmt_rgba:
        default:
            cinfo.input_components = 3;
            cinfo.in_color_space = JCS_RGB;
            break;
        }
        jpeg_set_defaults(&cinfo);
        cinfo.density_unit = 1;
        cinfo.X_density = img.get_xdpi();
        cinfo.Y_density = img.get_ydpi();
        int quality = 75;   /* default quality */
        jpeg_set_quality(&cinfo, quality, true);
        jpeg_start_compress(&cinfo, true);
        row_pointer[0] = new byte[cinfo.image_width * cinfo.input_components];
        int w = cinfo.image_width;
        while(cinfo.next_scanline < cinfo.image_height) {
            byte* row = row_pointer[0];
            switch(img.get_format())
            {
            case image::fmt_gray:
                memcpy(row, img.get_data(0, cinfo.next_scanline), w);
                break;
            case image::fmt_rgba:
            default:
                {
                    const color* cr = (const color*)img.get_data(0, cinfo.next_scanline);
                    for(int i = 0; i < w; i ++) {
                        *row++ = cr->red;
                        *row++ = cr->green;
                        *row++ = cr->blue;
                        ++ cr;
                    }
                    break;
                }
            }
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }
        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
        success = true;
    }
    else {
        jpeg_destroy_compress(&cinfo);
        success = false;
    }
    delete iod_dest;
    delete [] row_pointer[0];
    return success;
}

static void my_png_warning(png_structp, png_const_charp message)
{
    string str;
    str.from(message);
    trace(_t("libpng warning: %s\n"), str.c_str());
}

void my_png_read_fn(png_structp png_ptr, png_bytep data, png_size_t length);
void my_png_setup_image(image& img, png_structp png_ptr, png_infop info_ptr, float gamma);

struct png_reader
{
    enum read_state
    {
        rs_read_header,
        rs_reading_end,
        rs_error,
    };

    float               gamma;
    int                 quality;
    png_struct*         png_ptr;
    png_info*           info_ptr;
    png_info*           end_info;
    png_byte**          row_pointers;
    read_state          state;
    image_data_stream&  device;

public:
    png_reader(image_data_stream& ds):
        device(ds)
    {
        gamma = 0.f;
        quality = 2;
        png_ptr = nullptr;
        info_ptr = nullptr;
        end_info = nullptr;
        row_pointers = nullptr;
    }
    ~png_reader()
    {
        if(png_ptr) {
            if(end_info) {
                png_destroy_info_struct(png_ptr, &end_info);
                end_info = nullptr;
            }
            if(info_ptr) {
                png_destroy_info_struct(png_ptr, &info_ptr);
                info_ptr = nullptr;
            }
            png_destroy_read_struct(&png_ptr, 0, 0);
            png_ptr = nullptr;
        }
        if(row_pointers) {
            delete [] row_pointers;
            row_pointers = nullptr;
        }
    }
    bool read_header()
    {
        state = rs_error;
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        if(!png_ptr)
            return false;
        png_set_error_fn(png_ptr, 0, 0, my_png_warning);
        info_ptr = png_create_info_struct(png_ptr);
        if(!info_ptr)
            return false;
        end_info = png_create_info_struct(png_ptr);
        if(!end_info)
            return false;
        if(setjmp(png_jmpbuf(png_ptr)))
            return false;
        png_set_read_fn(png_ptr, this, my_png_read_fn);
        png_read_info(png_ptr, info_ptr);
        state = rs_read_header;
        return true;
    }
    bool read(image& img)
    {
        if(!read_header())
            return false;
        row_pointers = nullptr;
        if(setjmp(png_jmpbuf(png_ptr))) {
            state = rs_error;
            return false;
        }
        my_png_setup_image(img, png_ptr, info_ptr, gamma);
        if(!img.is_valid()) {
            state = rs_error;
            return false;
        }
        png_uint_32 width;
        png_uint_32 height;
        int bit_depth;
        int color_type;
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);
        byte* data = img.get_data(0, 0);
        int bpl = img.get_bytes_per_line();
        row_pointers = new png_bytep[height];
        for(uint y = 0; y < height; y ++)
            row_pointers[y] = data + y * bpl;
        png_read_image(png_ptr, row_pointers);
        img.set_xdpi(convert_dpm_to_dpi(png_get_x_pixels_per_meter(png_ptr, info_ptr)));
        img.set_ydpi(convert_dpm_to_dpi(png_get_y_pixels_per_meter(png_ptr, info_ptr)));
        state = rs_reading_end;
        png_read_end(png_ptr, end_info);
        return true;
    }
};

static void my_png_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    png_reader* d = (png_reader*)png_get_io_ptr(png_ptr);
    image_data_stream& ds = d->device;
    if((d->state == png_reader::rs_reading_end) && (ds.get_size() - ds.current_pos() < 4) && (length == 4)) {
        /* workaround for certain malformed PNGs that lack the final crc bytes */
        byte endcrc[4] = { 0xae, 0x42, 0x60, 0x82 };
        memcpy(data, endcrc, 4);
        ds.seek(ds.get_size());
        return;
    }
    ds.read_bytes(data, length);
}

static void my_png_setup_image(image& img, png_structp png_ptr, png_infop info_ptr, float gamma)
{
    if(gamma != 0.f && png_get_valid(png_ptr, info_ptr, PNG_INFO_gAMA)) {
        double file_gamma;
        png_get_gAMA(png_ptr, info_ptr, &file_gamma);
        png_set_gamma(png_ptr, gamma, file_gamma);
    }
    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);
    png_set_interlace_handling(png_ptr);
    image::image_format format;
    bool has_alpha = false;
    if(color_type == PNG_COLOR_TYPE_GRAY) {
        if(bit_depth == 16)
            png_set_strip_16(png_ptr);
        else if(bit_depth < 8)
            png_set_packing(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
        format = image::fmt_gray;
    }
    else {
        if(bit_depth == 16)
            png_set_strip_16(png_ptr);
        png_set_expand(png_ptr);
        if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png_ptr);
        format = image::fmt_rgba;
        has_alpha = true;
        if(!(color_type & PNG_COLOR_MASK_ALPHA) && !png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
            has_alpha = false;
        }
        png_read_update_info(png_ptr, info_ptr);
    }
    if(!img.is_valid() || (img.get_width() != width) || (img.get_height() != height) || (img.get_format() != format)) {
        if(img.is_valid())
            img.destroy();
        img.create(format, width, height);
        img.enable_alpha_channel(has_alpha);
    }
}

bool imageio::read_png_image(image& img, const void* ptr, int size)
{
    image_data_stream ds((const byte*)ptr, size);
    png_reader reader(ds);
    return reader.read(img);
}

static void my_png_write_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    assert(png_ptr);
    file* device = (file*)png_get_io_ptr(png_ptr);
    assert(device);
    int n = device->put((byte*)data, length);
    if((uint)n != length) {
        png_error(png_ptr, "write error");
        return;
    }
}

static void my_png_flush_fn(png_structp)
{
}

bool imageio::save_png_image(const image& img, const string& path)
{
    file f;
    f.open(path.c_str(), _t("wb"));
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if(!png_ptr)
        return false;
    png_set_error_fn(png_ptr, 0, 0, my_png_warning);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr || setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, 0);
        return false;
    }
    png_set_compression_level(png_ptr, 9);
    png_set_write_fn(png_ptr, &f, my_png_write_fn, my_png_flush_fn);
    int color_type = 0;
    png_color_8 sig_bit;
    memset(&sig_bit, 0, sizeof(sig_bit));
    switch(img.get_format())
    {
    case image::fmt_gray:
        color_type = PNG_COLOR_TYPE_GRAY;
        sig_bit.gray = 8;
        break;
    case image::fmt_rgba:
        sig_bit.red = 8;
        sig_bit.green = 8;
        sig_bit.blue = 8;
        if(img.has_alpha()) {
            color_type = PNG_COLOR_TYPE_RGB_ALPHA;
            sig_bit.alpha = 8;
        }
        else {
            color_type = PNG_COLOR_TYPE_RGB;
        }
        break;
    default:
        assert(!"unexpected format.");
        break;
    }
    png_set_IHDR(png_ptr, info_ptr, img.get_width(), img.get_height(), 8, color_type, 0, 0, 0);
    png_set_sBIT(png_ptr, info_ptr, &sig_bit);
    png_set_pHYs(png_ptr, info_ptr, convert_dpi_to_dpm(img.get_xdpi()), convert_dpi_to_dpm(img.get_ydpi()), PNG_RESOLUTION_METER);
    png_write_info(png_ptr, info_ptr);
    png_set_packing(png_ptr);
    if(color_type == PNG_COLOR_TYPE_RGB)
        png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
    int width = img.get_width();
    int height = img.get_height();
    png_bytep* row_pointers = new png_bytep[height];
    for(int i = 0; i < height; i ++)
        row_pointers[i] = img.get_data(0, i);
    png_write_image(png_ptr, row_pointers);
    delete [] row_pointers;
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return true;
}

__ariel_end__
