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

#ifndef application_89308b2a_1293_4c41_b107_31dbd1b4fa1b_h
#define application_89308b2a_1293_4c41_b107_31dbd1b4fa1b_h

#include <gslib/type.h>
#include <ariel/config.h>

__ariel_begin__

struct app_config
{
    gs::gchar           class_name[128];
    gs::gchar           window_name[128];
    gs::rect            position;
    addrptr             window_proc;
    bool                hidden;
    /* more.. */
};

struct app_data;
struct app_env;

class application
{
public:
    virtual ~application() { destroy(); }
    virtual bool setup(const app_config& cfg, const app_env& env);
    virtual void destroy();
    virtual int run();

protected:
    app_data*           _data = nullptr;

public:
    bool simple_setup(const app_config& cfg);

protected:
    int default_loop();
};

__ariel_end__

#endif
