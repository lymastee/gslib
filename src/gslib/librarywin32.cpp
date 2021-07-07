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

#include <windows.h>
#include <gslib/error.h>
#include <gslib/library.h>

__gslib_begin__

bool library::load(const string& path)
{
    if(is_loaded())
        unload();
    assert(!_handle);
#if defined(UNICODE) || defined(_UNICODE)
    _handle = (addrptr)::LoadLibraryW(path.c_str());
#else
    _handle = (addrptr)::LoadLibraryA(path.c_str());
#endif
    _is_loaded = true;
    return is_valid();
}

void library::unload()
{
    if(_handle) {
        ::FreeLibrary((HMODULE)_handle);
        _handle = nullptr;
    }
    _is_loaded = false;
}

addrptr library::resolve(const string& symbol)
{
    addrptr fn = nullptr;
    if(!is_valid()) {
        set_error(_t("invalid library."));
        return nullptr;
    }
#if defined(UNICODE) || defined(_UNICODE)
    fn = (addrptr)::GetProcAddress((HMODULE)_handle, _string<char>().from(symbol.c_str(), symbol.length()).c_str());
#else
    fn = (addrptr)::GetProcAddress((HMODULE)_handle, symbol.c_str());
#endif
    return fn;
}

__gslib_end__