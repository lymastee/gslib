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

#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include <windows.h>
#include <pink/image.h>
#include <pink/imgloader.h>
#include "ext/cximage/ximage.h"

#pragma warning(disable: 4099)

#ifdef _DEBUG
#pragma comment(lib, "ext/cximage/cximage_d.lib")
#pragma comment(lib, "ext/cximage/zlib_d.lib")
#pragma comment(lib, "ext/cximage/jpeg_d.lib")
#pragma comment(lib, "ext/cximage/png_d.lib")
#pragma comment(lib, "ext/cximage/tiff_d.lib")
#else
#pragma comment(lib, "ext/cximage/cximage.lib")
#pragma comment(lib, "ext/cximage/zlib.lib")
#pragma comment(lib, "ext/cximage/jpeg.lib")
#pragma comment(lib, "ext/cximage/png.lib")
#pragma comment(lib, "ext/cximage/tiff.lib")
#endif

__pink_begin__

static uint _get_cximage_type(const gchar* dir)
{
    int size = strtool::length(dir);
    const gchar* pf;
    for(pf = dir + size; pf - 1 != dir && pf[-1] != _t('.'); pf --);
    if(strtool::compare_cl(pf, _t("bmp")) == 0 || 
        strtool::compare_cl(pf, _t("dib")) == 0)
        return CXIMAGE_FORMAT_BMP;
    else if(strtool::compare_cl(pf, _t("png")) == 0)
        return CXIMAGE_FORMAT_PNG;
    else if(strtool::compare_cl(pf, _t("jpg")) == 0 ||
        strtool::compare_cl(pf, _t("jpeg")) == 0 ||
        strtool::compare_cl(pf, _t("jfif")) == 0)
        return CXIMAGE_FORMAT_JPG;
    else if(strtool::compare_cl(pf, _t("gif")) == 0)
        return CXIMAGE_FORMAT_GIF;
    else if(strtool::compare_cl(pf, _t("tif")) == 0 ||
        strtool::compare_cl(pf, _t("tiff")) == 0)
        return CXIMAGE_FORMAT_TIF;
    else if(strtool::compare_cl(pf, _t("tga")) == 0)
        return CXIMAGE_FORMAT_TGA;
    else if(strtool::compare_cl(pf, _t("ico")) == 0 ||
        strtool::compare_cl(pf, _t("cur")) == 0)
        return CXIMAGE_FORMAT_ICO;
    return CXIMAGE_FORMAT_UNKNOWN;
}

bool load_image(const gchar* dir, image& img)
{
    CxImage rdi;
    if(!rdi.Load(dir, _get_cximage_type(dir)))
        return false;
    assert(rdi.GetBpp() == 24 && "unsupported format.");
    img.destroy();
    byte* alpha_ptr = rdi.AlphaGetPointer();
    int w = rdi.GetWidth(), h = rdi.GetHeight();
    int size = w * h;
    if(alpha_ptr) {
        int ccl = size >> 2, rnd = size - (ccl << 2);
        for( ; ccl; ccl --) {
            if(*(int*)alpha_ptr != -1)
                break;
            alpha_ptr += sizeof(int);
        }
        if(ccl == 0) {
            for( ; rnd; rnd --) {
                if(*(alpha_ptr++) != 0xff)
                    break;
            }
        }
        if(!ccl && !rnd)
            alpha_ptr = 0;
    }
    img.create(w, h, alpha_ptr != 0);
    bitmap_header* hdr = (bitmap_header*)img.get_header();
    bitmap_header* hdr1 = (bitmap_header*)rdi.GetDIB();
    hdr->x_pels = hdr1->x_pels;
    hdr->y_pels = hdr1->y_pels;
    assert(hdr->image_size == hdr1->image_size);
    if(alpha_ptr != 0)
        memcpy_s(img.get_alpha(0, false), size, rdi.AlphaGetPointer(), size);
    size = hdr1->image_size;
    memcpy_s(img.get_color(0, false), size, rdi.GetBits(0), size);
    return true;
}

bool save_image(const gchar* dir, const image& img)
{
    assert(img.is_valid());
    CxImage wdi;
    uint imgtype = _get_cximage_type(dir);
    verify(wdi.Create(img.get_width(), img.get_height(), 24, imgtype) != 0);
    int size = ((bitmap_header*)wdi.GetDIB())->image_size;
    memcpy_s(wdi.GetBits(0), size, img.get_color(0, false), size);
    if(byte* alpha_ptr = (byte*)img.get_alpha(0, false)) {
        verify(wdi.AlphaCreate());
        size = img.get_width() * img.get_height();
        memcpy_s(wdi.AlphaGetPointer(), size, alpha_ptr, size);
    }
    return wdi.Save(dir, imgtype);
}

__pink_end__
