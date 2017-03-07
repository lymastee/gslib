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

#ifndef dataxchg_9639a1c0_f1fd_42f2_b870_5f7762926b4a_h
#define dataxchg_9639a1c0_f1fd_42f2_b870_5f7762926b4a_h

#include <windows.h>

using namespace gs;
using namespace gs::pink;

class gs::pink::image;
class gs::pink::clipboard_list;

extern bool _get_image_by_bmpinfo(image* img, const BITMAPINFO* pinfo);
extern int _get_clipdata_by_metafile(clipboard_list& cl, int c, HENHMETAFILE hemf);
extern int _get_clipdata_by_hwnd(clipboard_list& cl, int c, HWND hwnd);
extern int _get_clipdata_by_ole(clipboard_list& cl, int c, HWND hwnd);

#endif