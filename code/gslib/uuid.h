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

#ifndef uuid_93a5858f_7040_4f67_9767_808d2a4f7ff9_h
#define uuid_93a5858f_7040_4f67_9767_808d2a4f7ff9_h

#include <gslib/type.h>
#include <gslib/string.h>

__gslib_begin__

struct uuid_timestamp
{
    byte        tm_sec;
    byte        tm_min;
    byte        tm_hour;
    byte        tm_mday;    /* day of month */
    byte        tm_mon;
    byte        tm_wday;    /* day of week */
    int16       tm_year;
    int16       tm_yday;    /* day of year */
    int32       tm_fraction;
};

enum uuid_version
{
    uuid_v1,                /* date-time & MAC address */
    uuid_v2,                /* DCE security */
    uuid_v3,                /* MD5 hash & namespace */
    uuid_v4,                /* random */
    uuid_v5,                /* SHA-1 hash & namespace */
    uuid_ver_default = uuid_v4,
    uuid_ver_invalid = uuid_v2,
};

struct uuid_raw
{
    uint        data1;
    uint16      data2;
    uint16      data3;
    byte        data4[8];
};

class uuid:
    protected uuid_raw
{
public:
    uuid();
    uuid(uuid_version ver) { generate(ver); }
    uuid(uuid_version ver, const gchar* name, int len) { generate(ver, name, len); }
    uuid(const string& str) { from_string(str.c_str(), str.length()); }
    uuid(const gchar* str, int len) { from_string(str, len); }
    bool is_valid() const;
    void generate(uuid_version ver);
    void generate(uuid_version ver, const gchar* name, int len);
    void from_string(const gchar* str, int len);
    const gchar* to_string(string& str) const;
    void get_timestamp(uuid_timestamp& ts) const;
};

__gslib_end__

#endif
