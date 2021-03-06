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

#pragma once

#ifndef base64_08423729_6a46_4aa0_82bc_18c75b2c8762_h
#define base64_08423729_6a46_4aa0_82bc_18c75b2c8762_h

#include <gslib/config.h>
#include <gslib/pool.h>
#include <gslib/string.h>

__gslib_begin__

extern int base64_encode(string& output, const void* data, int len);
extern int base64_decode(vessel& output, const gchar* str, int len);

__gslib_end__

#endif
