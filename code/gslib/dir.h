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

#ifndef dir_710838b0_50ee_47d4_bb24_95e5ded5be0b_h
#define dir_710838b0_50ee_47d4_bb24_95e5ded5be0b_h

#include <direct.h>
#include <io.h>
#include <gslib/type.h>

#ifdef _UNICODE

#define _gs_getcwd _wgetcwd
#define _gs_chdir _wchdir
#define _gs_mkdir _wmkdir
#define _gs_rmdir _wrmdir
#define _gs_finddata_t _wfinddata_t
#define _gs_findfirst _wfindfirst
#define _gs_findnext _wfindnext
#define _gs_findclose _findclose

#else

#define _gs_getcwd _getcwd
#define _gs_chdir _chdir
#define _gs_mkdir _mkdir
#define _gs_rmdir _rmdir
#define _gs_finddata_t _finddata_t
#define _gs_findfirst _findfirst
#define _gs_findnext _findnext
#define _gs_findclose _findclose

#endif

#endif
