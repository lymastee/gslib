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

#ifndef sysop_eb634adc_cca3_4f8f_853c_7852abe06d2d_h
#define sysop_eb634adc_cca3_4f8f_853c_7852abe06d2d_h

#include <gslib/type.h>
#include <pink/type.h>

__gslib_begin__

using pink::point;
using pink::rect;
using pink::font;

enum unimask
{
    declare_mask(um_shift,      0),
    declare_mask(um_sshift,     1),
    declare_mask(um_control,    2),
    declare_mask(um_scontrol,   3),
    declare_mask(um_reserve,    4),
    declare_mask(um_alter,      6),
    declare_mask(um_salter,     7),
    declare_mask(um_lmouse,     8),
    declare_mask(um_rmouse,     9),
};

enum stylemask
{
    declare_mask(sm_movable,    0),
    declare_mask(sm_hitable,    1),
    declare_mask(sm_visible,    2),
};

/*
 * part1. ascii codes
 * part2. reserved
 * part3. mouse keys
 * part4. joystick keys
 * part5. keyboard controls
 */
enum unikey
{
    uk_null = 0,        /* null */
    uk_soh,             /* start of heading */
    uk_stx,             /* start of text */
    uk_etx,             /* end of text */
    uk_eot,             /* end of transmission */
    uk_enq,             /* enquiry */
    uk_ack,             /* acknowledge */
    uk_bel,             /* bell */
    uk_bs,              /* backspace */
    uk_tab,             /* horizontal tab */
    uk_lf,              /* nl line feed, new line */
    uk_vt,              /* vertical tab */
    uk_ff,              /* np form feed, new page */
    uk_cr,              /* carriage return */
    uk_so,              /* shift out */
    uk_si,              /* shift in */
    uk_dle,             /* data link escape */
    uk_dc1,             /* device control 1 */
    uk_dc2,             /* device control 2 */
    uk_dc3,             /* device control 3 */
    uk_dc4,             /* device control 4 */
    uk_nak,             /* negative acknowledge */
    uk_syn,             /* synchronous idle */
    uk_etb,             /* end of trans. block */
    uk_can,             /* cancel */
    uk_em,              /* end of medium */
    uk_sub,             /* substitute */
    uk_esc,             /* escape */
    uk_fs,              /* file separator */
    uk_gs,              /* group separator */
    uk_rs,              /* record separator */
    uk_us,              /* unit separator */
    uk_sp,              /* space, blank */
                        /* ! - ~ */
    uk_del  = 0x7f,     /* delete */

    mk_left,
    mk_center,
    mk_right,

    vk_insert,          /* insert */
    vk_caps,            /* caps lock */
    vk_pscr,            /* print screen */
    vk_numlock,         /* number lock */
    vk_home,            /* home */
    vk_end,             /* end */
    vk_pageup,          /* page up */
    vk_pagedown,        /* page down */
    vk_left,            /* left arrow */
    vk_up,              /* up arrow */
    vk_right,           /* right arrow */
    vk_down,            /* down arrow */

    vk_shift,
    vk_control,
    vk_alter,

    vk_f1,              /* f1 - f12 */
    vk_f2,
    vk_f3,
    vk_f4,
    vk_f5,
    vk_f6,
    vk_f7,
    vk_f8,
    vk_f9,
    vk_f10,
    vk_f11,
    vk_f12,
};

class system_driver;

class __gs_novtable system_notify abstract
{
public:
    virtual void on_show(bool b) = 0;
    virtual void on_create(system_driver* ptr, const rect& rc) = 0;
    virtual void on_close() = 0;
    virtual void on_size(const rect& rc) = 0;
    virtual void on_paint(const rect& rc) = 0;
    virtual void on_halt() = 0;
    virtual void on_resume() = 0;
    virtual bool on_mouse_down(uint um, unikey uk, const point& pt) = 0;
    virtual bool on_mouse_up(uint um, unikey uk, const point& pt) = 0;
    virtual bool on_move(uint um, const point& pt) = 0;
    virtual bool on_key_down(uint um, unikey uk) = 0;
    virtual bool on_key_up(uint um, unikey uk) = 0;
    virtual bool on_char(uint um, uint ch) = 0;
    virtual void on_timer(uint tid) = 0;
};

struct system_context
{
    enum
    {
        sct_notify      = 0x01,
        sct_painter     = 0x02,
        sct_rectangle   = 0x04,
        sct_hwnd        = 0x08,
        sct_everything  = 0xffffffff,
    };

    uint                mask;
    system_notify*      notify;
    void*               painter;
    rect                rectangle;
    uint                hwnd;
};

class __gs_novtable system_driver abstract
{
public:
    virtual ~system_driver() {}
    virtual void initialize(const system_context& ctx) = 0;
    virtual void setup() = 0;
    virtual void close() = 0;
    virtual void set_timer(uint tid, int t) = 0;
    virtual void kill_timer(uint tid) = 0;
    virtual void update() = 0;
    virtual void emit(int msgid, void* msg, int size) = 0;
    virtual void set_ime(point pt, const font& ft) = 0;
    //virtual void set_clipboard(const gchar* fmt, const void* ptr, int size) = 0;
    //virtual int get_clipboard(const gchar* fmt, const void*& ptr) = 0;
    //virtual int get_clipboard(clipboard_list& cl, int c) = 0;
};

__gslib_end__

#endif
