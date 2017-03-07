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

#include <pink\image.h>
#include <pink\widget.h>
#include <pink\dataxchg.h>

/*
 * DONOT use BITMAPINFO structure to retrieve the bitmap infomations,
 * when BITMAPINFOHEADER::biCompression = 3, which means there was 3 mask
 * values in the color table returned after the BITMAPINFOHEADER
 */
typedef struct tagBITMAPINFOWITHCRTABLE
{
    BITMAPINFOHEADER    bmiHeader;
    DWORD               crMaskRed;
    DWORD               crMaskGreen;
    DWORD               crMaskBlue;
    DWORD               crReserved;
}
BitmapInfoWithCrtable, *LPBitmapInfoWithCrtable;

/*
 * get raw image datas from bitmap, @dc must be @bmp's container dc,
 * @info is output argument, and the pointer returned must be deleted
 */
static BYTE* get_raw_image_data(HDC dc, HBITMAP bmp, BitmapInfoWithCrtable& info)
{
    memset(&info, 0, sizeof(BitmapInfoWithCrtable));
    info.bmiHeader.biSize = sizeof(BITMAPINFO);
    if(!GetDIBits(dc, bmp, 0, 0, 0, (BITMAPINFO*)&info, DIB_RGB_COLORS))
        return 0;
    BYTE* buf = new BYTE[info.bmiHeader.biSizeImage];   /* caution! */
    if(!GetDIBits(dc, bmp, 0, info.bmiHeader.biHeight, buf, (BITMAPINFO*)&info, DIB_RGB_COLORS))
        return 0;
    return buf;
}

bool _get_image_by_bmpinfo(image* img, const BITMAPINFO* pinfo)
{
    /* check up */
    if(!img || !pinfo || img->is_valid())
        return false;
    const BITMAPINFOHEADER* hdr = &pinfo->bmiHeader;
    int w = hdr->biWidth, h = hdr->biHeight;
    if(!w || !h)
        return false;
    /* buffering */
    HDC smpdc = GetDC(0);
    HDC ctndc = CreateCompatibleDC(smpdc);
    HBITMAP bmp = CreateCompatibleBitmap(smpdc, w, h);
    HBITMAP oldbmp = (HBITMAP)SelectObject(ctndc, bmp);
    /* when retrieve a bitmap from rtf stream, the alpha infomations were already lost */
    SetBkMode(ctndc, OPAQUE);
    SetBkColor(ctndc, RGB(0,0,0));
    SetDIBitsToDevice(ctndc, 0, 0, w, h, 0, 0, 0, h, (const void*)pinfo->bmiColors, pinfo, DIB_RGB_COLORS);
    BitmapInfoWithCrtable info;
    BYTE* crdatbk = get_raw_image_data(ctndc, bmp, info);
    int* crdat = (int*)crdatbk;
    assert(info.bmiHeader.biCompression == 3 && info.bmiHeader.biBitCount == 32);
    img->create(w, h, false);
    int alphacap = (int)info.bmiHeader.biSizeImage / 4;
    for(int i = 0; i < h; i ++) {
        pixel* p = img->get_color(i, false);
        for(int j = 0; j < w; j ++)
            (p++)->from_xrgb(*(crdat++));
    }
    delete [] crdatbk;
    /* clear */
    SelectObject(ctndc, oldbmp);
    DeleteObject(bmp);
    DeleteDC(ctndc);
    ReleaseDC(0, smpdc);
    return true;
}

struct mfparam
{
    clipboard_list* clist;
    int cap;
    int count;
};

int CALLBACK enh_metafile_proc(HDC hdc, HANDLETABLE* lptable, const ENHMETARECORD* lpemfr, int objc, LPARAM lparam)
{
    mfparam* mfp = (mfparam*)lparam;
    assert(mfp != 0);
    if(mfp->count >= mfp->cap)
        return 1;
    if(lpemfr->iType == EMR_EXTTEXTOUTA) {
    }
    else if(lpemfr->iType == EMR_EXTTEXTOUTW) {
        int c = *(int*)(lpemfr->dParm+0x09);
        wchar* str = (wchar*)(lpemfr->dParm+0x11);
        if(c == 0)
            return 1;
        /* convert from wchar to char */
//         clipboard_text* p = gs_new(clipboard_text);
//         int sc = c << 1;
//         char* sstr = new char[sc];
//         BOOL brpl = FALSE;
//         sc = WideCharToMultiByte(CP_OEMCP, WC_NO_BEST_FIT_CHARS, str, c, sstr, sc, "?", &brpl);
//         p->assign(sstr, sc);
//         p->push_back(_t('\n'));
//         delete [] sstr;
//         mfp->clist->push_back(p);
//         mfp->count ++;
    }
    else if(lpemfr->iType == EMR_STRETCHDIBITS) {
        clipboard_bitmap* p = gs_new(clipboard_bitmap);
        BITMAPINFO* pinfo = (BITMAPINFO*)(lpemfr->dParm+0x12);
        verify(_get_image_by_bmpinfo(p, pinfo));
        mfp->clist->push_back(p);
        mfp->count ++;
    }
    return 2;
}

int _get_clipdata_by_metafile(clipboard_list& cl, int c, HENHMETAFILE hemf)
{
    if(c < 0)
        c = 0x7fffffff;
    RECT rc = { 0,0,1,1 };
    mfparam mp = { &cl, c, 0 };
    HENHMETAFILE cpyhemf = CopyEnhMetaFile(hemf, 0);
    BOOL b = EnumEnhMetaFile(0, cpyhemf, enh_metafile_proc, (void*)&mp, &rc);
    assert(b);
    DeleteEnhMetaFile(cpyhemf);
    return mp.count;
}

int _get_clipdata_by_hwnd(clipboard_list& cl, int c, HWND hwnd)
{
    if(c < 0)
        c = 0x7fffffff;
    BOOL b = OpenClipboard(hwnd);
    assert(b);
    /* try to get metafile data */
    HANDLE h = GetClipboardData(CF_ENHMETAFILE);
    if(h && h != INVALID_HANDLE_VALUE) {
        int r = _get_clipdata_by_metafile(cl, c, (HENHMETAFILE)h);
        CloseClipboard();
        return r;
    }
    UINT ufmt = EnumClipboardFormats(0);
    int i = 0;
    while(ufmt && i < c) {
        if(ufmt == CF_TEXT) {
            h = GetClipboardData(CF_TEXT);
            clipboard_text* p = gs_new(clipboard_text);
            const void* ptr = GlobalLock(h);
            p->assign((const gchar*)ptr);
            GlobalUnlock(h);
            cl.push_back(p);
            i ++;
        }
        else if(ufmt == CF_DIB) {
            h = GetClipboardData(CF_DIB);
            clipboard_bitmap* p = gs_new(clipboard_bitmap);
            BITMAPINFO* ptr = (BITMAPINFO*)GlobalLock(h);
            verify(_get_image_by_bmpinfo(p, ptr));
            GlobalUnlock(h);
            cl.push_back(p);
            i ++;
        }
        ufmt = EnumClipboardFormats(ufmt);
    }
    CloseClipboard();
    return i;
}
