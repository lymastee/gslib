/*
 * Copyright (c) 2016-2019 lymastee, All rights reserved.
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

#include <gslib/library.h>

#ifdef _WINDOWS
#include "librarywin32.cpp"
#else
!!no implementation!!
#endif

__gslib_begin__

library::library()
{
    _is_loaded = false;
    _handle = nullptr;
}

library::library(const string& path)
{
    _is_loaded = false;
    _handle = nullptr;
    load(path);
}

library::~library()
{
    unload();
}

bool library::existed(const string& path)
{
#if defined(UNICODE) || defined(_UNICODE)
    return _waccess(path.c_str(), 0) == 0;
#else
    return access(path.c_str(), 0) == 0;
#endif
}

addrptr library::resolve(const string& libpath, const string& symbol)
{
    library lib(libpath);
    return lib.resolve(symbol);
}

__gslib_end__
