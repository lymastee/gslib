/*
 * Copyright (c) 2016-2018 lymastee, All rights reserved.
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
#include <pink/widget.h>

__pink_begin__

#define tid_refresh 0
#define tid_caret   1

vtable_ops<widget>& widget_vtable_ops()
{
    static vtable_ops<widget> inst(nullptr);
    return inst;
}

widget::widget(wsys_manager* m)
{
    _manager = m;
    _last = _next = _child = _parent = nullptr;
    _style = 0;
    _show = false;
    _enable = true;
    _backvt = nullptr;
}

widget::~widget()
{
    if(_backvt) {
        widget_vtable_ops().destroy_per_instance_vtable(this, _backvt);
        _backvt = nullptr;
    }
}

bool widget::create(widget* ptr, const gchar* name, const rect& rc, uint style)
{
    if(_parent = ptr) {
        if(!ptr->_child)
            ptr->_child = this;
        else {
            widget* c = ptr->_child;
            while(c->_next)
                c = c->_next;
            c->_next = this;
            _last = c;
        }
    }
    _name.assign(name);
    _pos = rc;
    _style = style;
    _htpos.set_point(-1,-1);
    if(style & sm_visible)
        _show = true;
    refresh(false);
    return true;
}

void widget::close()
{
    _show = false;
    _enable = false;
    capture(false);
    assert(!_child);
    if(_parent && _parent->_child == this)
        _parent->_child = _next;
    if(_last)
        _last->_next = _next;
    if(_next)
        _next->_last = _last;
}

void widget::show(bool b)
{
    if(_show != b) {
        _show = b;
        refresh(false);
    }
    if(!b)
        capture(false);
}

void widget::enable(bool b)
{
    if(_enable != b) {
        _enable = b;
        refresh(false);
    }
}

void widget::move(const rect& rc)
{
    if(rc.right < rc.left || rc.bottom < rc.top)
        return;
    refresh(false);
    _pos = rc;
    refresh(true);
}

void widget::refresh(const rect& rc, bool imm)
{
    rect rc1 = rc;
    rc1.offset(_pos.left, _pos.top);
    if(rc1.left < 0)
        rc1.left = 0;
    if(rc1.top < 0)
        rc1.top = 0;
    if(rc1.right <= rc1.left || rc1.bottom <= rc1.top)
        return;
    for(widget* p = _parent; p; p = p->_parent) {
        const rect& rc2 = p->get_rect();
        rc1.offset(rc2.left, rc2.top);
        if(rc1.right > rc2.right)
            rc1.right = rc2.right;
        if(rc1.bottom > rc2.bottom)
            rc1.bottom = rc2.bottom;
        if(rc1.left < 0)
            rc1.left = 0;
        if(rc1.top < 0)
            rc1.top = 0;
        if(rc1.right <= rc1.left || rc1.bottom <= rc1.top)
            return;
    }
    assert(_manager);
    _manager->refresh(rc1, imm);
}

void widget::make_individual_vtable()
{
    assert(!_backvt);
    _backvt = widget_vtable_ops().create_per_instance_vtable(this);
}

int widget::run_proc(int msgid, ...)
{
    va_list ptr;
    va_start(ptr, msgid);
    return proceed(msgid, ptr);
}

int widget::proceed(int msgid, va_list vlst)
{
//     /* deal with the reflection */
//     for(reflect_list::iterator i = _reflect_list.begin(); i != _reflect_list.end(); ++ i) {
//         if(i->reflect_id == msgid) {
//             i->reflect_to->on_reflect(this, msgid, vlst);
//             break;
//         }
//     }
//     /* default handles */
//     if(msgid == hid_press) {
//         uint um = va_arg(vlst, uint);
//         unikey uk = va_arg(vlst, unikey);
//         point* pt = va_arg(vlst, point*);
//         on_press(um, uk, *pt);
//         return rid_ok;
//     }
//     else if(msgid == hid_click) {
//         uint um = va_arg(vlst, uint);
//         unikey uk = va_arg(vlst, unikey);
//         point* pt = va_arg(vlst, point*);
//         on_click(um, uk, *pt);
//         return rid_ok;
//     }
//     else if(msgid == hid_hover) {
//         uint um = va_arg(vlst, uint);
//         point* pt = va_arg(vlst, point*);
//         on_hover(um, *pt);
//         return rid_ok;
//     }
//     else if(msgid == hid_leave) {
//         uint um = va_arg(vlst, uint);
//         point* pt = va_arg(vlst, point*);
//         on_leave(um, *pt);
//         return rid_ok;
//     }
//     else if(msgid == hid_keydown) {
//         uint um = va_arg(vlst, uint);
//         unikey uk = va_arg(vlst, unikey);
//         on_keydown(um, uk);
//         return rid_ok;
//     }
//     else if(msgid == hid_keyup) {
//         uint um = va_arg(vlst, uint);
//         unikey uk = va_arg(vlst, unikey);
//         on_keyup(um, uk);
//         return rid_ok;
//     }
//     else if(msgid == hid_char) {
//         uint um = va_arg(vlst, uint);
//         uint ch = va_arg(vlst, uint);
//         on_char(um, ch);
//         return rid_ok;
//     }
//     else if(msgid == hid_scroll) {
//         point* pt = va_arg(vlst, point*);
//         real scr = va_arg(vlst, real);
//         bool vert = va_arg(vlst, bool);
//         on_scroll(*pt, scr, vert);
//         return rid_ok;
//     }
    return rid_undone;
}

widget* widget::capture(bool b)
{
    assert(_manager);
    return _manager->set_capture(this, b);
}

widget* widget::focus()
{
    assert(_manager);
    return _manager->set_focus(this);
}

// void widget::reflect(widget* w, int msgid)
// {
//     assert(w);
//     reflect_node n = { this, msgid };
//     w->_reflect_list.push_back(n);
// }

void widget::lay(widget* ptr, laytag t)
{
    /* currently unsupported */
    assert(0);
}

void widget::on_press(uint um, unikey uk, const point& pt)
{
    if(uk == mk_left) {
        _htpos = pt;
        capture(true);
    }
}

void widget::on_click(uint um, unikey uk, const point& pt)
{
    if(uk == mk_left && _htpos.x >= 0 && _htpos.y >= 0) {
        _htpos.set_point(-1,-1);
        capture(false);
        /* focus! */
        focus();
    }
}

void widget::on_hover(uint um, const point& pt)
{
    if((_style & sm_movable) && _htpos.x >= 0 && _htpos.y >= 0) {
        point mt = pt;
        mt.offset(-_htpos.x, -_htpos.y);
        mt.offset(_pos.left, _pos.top);
        move(mt);
    }
}

void widget::on_leave(uint um, const point& pt)
{
}

point& widget::top_level(point& pt) const
{
    pt.offset(_pos.left, _pos.top);
    if(!_parent)
        return pt;
    return _parent->top_level(pt);
}

rect& widget::top_level(rect& rc) const
{
    rc.offset(_pos.left, _pos.top);
    if(!_parent)
        return rc;
    return _parent->top_level(rc);
}

void widget::move(const point& pt)
{
    rect rc = _pos;
    rc.move_to(pt);
    move(rc);
}

void widget::refresh(bool imm)
{
    refresh(rect(0,0,get_width(),get_height()), imm);
}

button::button(wsys_manager* m): widget(m)
{
    _source = 0;
    _allstate = false;
    _btnstate = bs_zr;
}

void button::draw(painter* paint)
{
    //if(_bkground.is_valid())
    //    paint->draw(&_bkground, 0, 0);
}

void button::enable(bool b)
{
    bool dif = _enable != b;
    superref::enable(b);
    if(dif != false)
        b ? set_normal() : set_gray();
}

void button::on_press(uint um, unikey uk, const point& pt)
{
    superref::on_press(um, uk, pt);
    if(uk == mk_left && _btnstate != bs_press) {
        set_btnstate(bs_press);
        set_press();
    }
}

void button::on_click(uint um, unikey uk, const point& pt)
{
    superref::on_click(um, uk, pt);
    if(uk == mk_left && _btnstate != bs_normal) {
        set_btnstate(bs_normal);
        set_normal();
    }
}

void button::on_hover(uint um, const point& pt)
{
    superref::on_hover(um, pt);
    if(_btnstate != bs_hover && _btnstate != bs_press) {
        set_btnstate(bs_hover);
        set_hover();
    }
}

void button::on_leave(uint um, const point& pt)
{
    superref::on_leave(um, pt);
    if(_btnstate != bs_normal && _btnstate != bs_press) {
        set_btnstate(bs_normal);
        set_normal();
    }
}

void button::set_image(const image* img, bool as)
{
    assert(!_source);
    if(_source != 0)
        return;
    _source = img;
    _allstate = as;
    refresh(false);
    int w = img->get_width(), h = img->get_height();
    if(as) w >>= 2;
    _pos.right = _pos.left + w;
    _pos.bottom = _pos.top + h;
    //_bkground.create(w, h, img->get_alpha(0) != 0);
    _enable ? set_normal() : set_gray();
}

void button::set_press()
{
    if(_source == 0)
        return;
//     _allstate ?
//         _bkground.copy(_source, 0, 0, get_width(), get_height(), get_width()<<1, 0) :
//         _bkground.set_brigntness(_source, 0.7f);
    refresh(true);
}

void button::set_normal()
{
    if(_source == 0)
        return;
    //_allstate ?
    //    _bkground.copy(_source, 0, 0, get_width(), get_height(), 0, 0) :
    //    _bkground.copy(_source, true, true);
    refresh(true);
}

void button::set_hover()
{
    if(_source == 0)
        return;
//     _allstate ? 
//         _bkground.copy(_source, 0, 0, get_width(), get_height(), get_width(), 0) :
//         _bkground.set_brigntness(_source, 1.3f);
    refresh(true);
}

void button::set_gray()
{
    if(_source == 0)
        return;
//     _allstate ? 
//         _bkground.copy(_source, 0, 0, get_width(), get_height(), get_width()*3, 0) :
//         _bkground.set_gray(_source);
    refresh(true);
}

edit::edit(wsys_manager* m): widget(m)
{
    _caret_on = false;
    _caretpos = 0;
    _bkground = 0;
    _sel_start = -1;
    _txtcolor = pixel(0,0,0);
    _selcolor = pixel(0,125,255);
    _crtcolor = pixel(0,0,0);
    _font = font(_t("ËÎÌו"), 14);
    _font_idx = -1;
}

bool edit::create(widget* ptr, const gchar* name, const rect& rc, uint style)
{
    /* should not be movable */
    style &= ~sm_movable;
    return superref::create(ptr, name, rc, style);
}

void edit::draw(painter* paint)
{
    //if(_bkground)
    //    paint->draw(_bkground, 0, 0);
    fontsys* pfs = _manager->get_fontsys();
    assert(pfs);
    _font_idx = pfs->set_font(_font, _font_idx);
    if(_sel_start < 0);
    //    paint->draw_text(_textbuf.c_str(), 0, 0, _txtcolor);
    else {
        /* 0-start, start-end, end-last */
        int start, end;
        if(_sel_start < _sel_end) {
            start = _sel_start;
            end = _sel_end;
        }
        else {
            start = _sel_end;
            end = _sel_start;
        }
        assert(start >= 0 && end >= 0);
        assert(end <= _textbuf.length());
        int w, h, bias = 0;
        string s;
        if(start) {
            s.assign(_textbuf.c_str(), start);
            pfs->get_size(s.c_str(), w, h);
            //paint->draw_text(s.c_str(), bias, 0, _txtcolor);
            bias = w;
        }
        if(start != end) {
            s.assign(_textbuf.c_str()+start, end-start);
            pfs->get_size(s.c_str(), w, h);
            //paint->draw_rect(rect(bias,0,w,h), _selcolor);
            //paint->draw_text(s.c_str(), bias, 0, pixel(255,255,255));
            bias += w;
        }
        //if(end < _textbuf.length())
        //    paint->draw_text(_textbuf.c_str()+end, bias, 0, _txtcolor);
    }
    /* draw caret */
    if(_caret_on) {
        string s;
        s.assign(_textbuf.c_str(), _caretpos);
        int w, h;
        pfs->get_size(s.c_str(), w, h);
        //paint->draw_line(point(w,0), point(w,_font.height), _crtcolor);
    }
}

void edit::on_press(uint um, unikey uk, const point& pt)
{
    if(uk == mk_left) {
        int pos = hit_char(pt);
        set_select(pos, pos);
        refresh(true);
    }
    superref::on_press(um, uk, pt);
}

void edit::on_click(uint um, unikey uk, const point& pt)
{
    if(uk == mk_left) {
        int pos = hit_char(pt);
        set_select(_sel_start, pos);
        refresh(true);
    }
    superref::on_click(um, uk, pt);
}

void edit::on_hover(uint um, const point& pt)
{
    if(um & um_lmouse) {
        int pos = hit_char(pt);
        set_select(_sel_start, pos);
        refresh(true);
    }
    superref::on_hover(um, pt);
}

void edit::on_char(uint um, uint ch)
{
    /* ascii codes */
    if((ch >= 0x20 && ch < 0x7f) || ch == 0x09 || ch == 0x0a) {
        del_select();
        _textbuf.insert((size_t)_caretpos, 1, (char)ch);
        set_caret(_caretpos+1);
        return;
    }
    /* mbcs codes */
    if(ch & 0x8080) {
        del_select();
        _textbuf.insert((size_t)_caretpos, 1, (char)((ch&0xff00)>>8));
        _textbuf.insert((size_t)_caretpos+1, 1, (char)(ch&0xff));
        set_caret(_caretpos+2);
        return;
    }
}

void edit::on_keydown(uint um, unikey uk)
{
    if(uk == uk_bs) {
        if((um & um_control) || (um & um_scontrol)) {
            set_select(
                no_select() ? _caretpos : _sel_start,
                prev_logic_char(_caretpos)
                );
            del_select();
        }
        else {
            if(no_select())
                set_select(_caretpos, prev_char(_caretpos));
            del_select();
        }
    }
    else if(uk == uk_del) {
        if((um & um_control) || (um & um_scontrol)) {
            set_select(
                no_select() ? _caretpos : _sel_start,
                next_logic_char(_caretpos)
                );
            del_select();
        }
        else {
            if(no_select())
                set_select(_caretpos, next_char(_caretpos));
            del_select();
        }
    }
    else if(uk == vk_left) {
        if((um & um_control) || (um & um_scontrol)) {
            if((um & um_shift) || (um & um_sshift)) {
                set_select(
                    no_select() ? _caretpos : _sel_start,
                    prev_logic_char(_caretpos)
                    );
            }
            else {
                _sel_start = -1;
                set_caret(prev_logic_char(_caretpos));
            }
        }
        else {
            if((um & um_shift) || (um & um_sshift)) {
                set_select(
                    no_select() ? _caretpos : _sel_start,
                    prev_char(_caretpos)
                    );
            }
            else {
                _sel_start = -1;
                set_caret(prev_char(_caretpos));
            }
        }
    }
    else if(uk == vk_right) {
        if((um & um_control) || (um & um_scontrol)) {
            if((um & um_shift) || (um & um_sshift)) {
                set_select(
                    no_select() ? _caretpos : _sel_start,
                    next_logic_char(_caretpos)
                    );
            }
            else {
                _sel_start = -1;
                set_caret(next_logic_char(_caretpos));
            }
        }
        else {
            if((um & um_shift) || (um & um_sshift)) {
                set_select(
                    no_select() ? _caretpos : _sel_start,
                    next_char(_caretpos)
                    );
            }
            else {
                _sel_start = -1;
                set_caret(next_char(_caretpos));
            }
        }
    }
    else if(uk == vk_home) {
        if((um & um_shift) || (um & um_sshift)) {
            set_select(
                no_select() ? _caretpos : _sel_start, 0
                );
        }
        else {
            _sel_start = -1;
            set_caret(0);
        }
    }
    else if(uk == vk_end) {
        if((um & um_shift) || (um & um_sshift)) {
            set_select(
                no_select() ? _caretpos : _sel_start, -1
                );
        }
        else {
            _sel_start = -1;
            set_caret(-1);
        }
    }
    else if(uk == _t('A') && ((um & um_control) || (um & um_scontrol))) {
        /* select all */
        set_select(0, -1);
    }
    else if((uk == _t('C') || uk == _t('X')) && ((um & um_control) || (um & um_scontrol))) {
        /* copy to clipboard */
        int start = gs_min(_sel_start, _sel_end),
            end = gs_max(_sel_start, _sel_end);
        if(start >= 0 && start != end) {
            int size = end - start + 1;            
            size *= sizeof(gchar);
            assert(_manager);
            _manager->set_clipboard(clipfmt_text, _textbuf.c_str()+start, size);
            if(uk == _t('X'))
                del_select();
        }
    }
    else if(uk == _t('V') && ((um & um_control) || (um & um_scontrol))) {
        /* get from clipboard */
        assert(_manager);
        const gchar* str = 0;
        clipboard_list cl;
        if(_manager->get_clipboard(cl, 1)) {
            clipboard_data* ptr = cl.front();
            if(ptr->get_format() == cf_text)
                replace_select(ptr->get_data<string>()->c_str());
        }
    }
}

void edit::on_caret()
{
    _caret_on = !_caret_on;
    refresh(false);
}

void edit::on_focus(bool b)
{
    if(!b) {
        _caret_on = false;
        refresh(false);
    }
}

void edit::set_text(const gchar* str)
{
    if(!str) {
        _textbuf.clear();
        set_caret(0);
    }
    else {
        _textbuf.assign(str);
        if(_caretpos > _textbuf.length())
            set_caret(_textbuf.length());
    }
    set_select(-1, 0);
}

void edit::set_caret(int n)
{
    _caretpos = n < 0 || n > _textbuf.length() ? 
        _textbuf.length() : n;
    string s;
    s.assign(_textbuf.c_str(), _caretpos);
    fontsys* pfs = _manager->get_fontsys();
    assert(pfs);
    int w, h;
    _font_idx = pfs->set_font(_font, _font_idx);
    pfs->get_size(s.c_str(), w, h);
    _manager->set_ime(this, point(w,0), _font);
    refresh(false);
}

void edit::set_select(int start, int end)
{
    if(end == -1)
        end = _textbuf.length();
    if(start == -1)
        _sel_start = -1;
    else {
        _sel_start = start;
        _sel_end = end;
    }
    set_caret(end);
}

void edit::replace_select(const gchar* str)
{
    del_select();
    if(str) {
        _textbuf.insert(_caretpos, str);
        set_caret(_caretpos+strtool::length(str));
    }
}

void edit::del_select()
{
    if(_sel_start < 0)
        return;
    int start, end;
    if(_sel_start < _sel_end) {
        start = _sel_start;
        end = _sel_end;
    }
    else {
        start = _sel_end;
        end = _sel_start;
    }
    assert(start >= 0 && end >= 0);
    assert(end <= _textbuf.length());
    _textbuf.erase(start, end-start);
    set_caret(start);
    assert(_caretpos >= 0 && _caretpos <= _textbuf.length());
    _sel_start = -1;
}

int edit::hit_char(point pt)
{
    if(pt.x <= 0 || !_textbuf.length())
        return 0;
    /* treat as mono font and predict the width */
    fontsys* pfs = _manager->get_fontsys();
    assert(pfs);
    int w, h, fixc;
    _font_idx = pfs->set_font(_font, _font_idx);
    pfs->get_size(_t("a"), w, h);
    fixc = pt.x / w;
    if(fixc > _textbuf.length())
        fixc = _textbuf.length();
#ifndef _UNICODE
    if(is_mbcs_half(_textbuf.c_str() + fixc)) {
        assert(fixc);  /* broken mbcs? */
        fixc --;
    }
#endif
    string s;
    s.assign(_textbuf.c_str(), fixc);
    pfs->get_size(s.c_str(), w, h);
    if(pt.x > w) {
        for(fixc ++; fixc <= _textbuf.length(); fixc ++) {
            s.push_back(_textbuf.at(fixc-1));
            if(_textbuf.at(fixc-1) & 0x80)
                s.push_back(_textbuf.at(fixc++));
            pfs->get_size(s.c_str(), w, h);
            if(pt.x < w)
                break;
        }
        if(fixc > _textbuf.length())
            return _textbuf.length();
        fixc -= (s.back()&0x80) ? 2 : 1;
    }
    else if (pt.x < w) {
        if(fixc == 0)
            return 0;
        for(fixc --; fixc > 0; fixc --) {
            if(s.pop_back() & 0x80) {
                fixc --;
                s.pop_back();
            }
            pfs->get_size(s.c_str(), w, h);
            if(pt.x >= w)
                break;
        }
    }
    return fixc;
}

int edit::prev_char(int pos)
{
    if(pos < 0 || pos > _textbuf.length())
        pos = _textbuf.length();
    if(pos > 0)
        pos -= (_textbuf.at(pos-1)&0x80) ? 2 : 1;
    if(pos < 0)
        pos = 0;
    return pos;
}

int edit::next_char(int pos)
{
    if(pos < 0 || pos > _textbuf.length())
        pos = _textbuf.length();
    if(pos < _textbuf.length())
        pos += (_textbuf.at(pos)&0x80) ? 2 : 1;
    if(pos > _textbuf.length())
        pos = _textbuf.length();
    return pos;
}

static int get_logic_field(uint c)
{
    static const string sfield1 = _t(" \t\v\a\b\f");
    if(!(c & 0x8080)) {
        assert(c < 0x80);
        if(sfield1.find((char)c) != string::npos)
            return 0;
        if(c == _t('\r') || c == _t('\n'))
            return 1;
        if((c >= _t('!') && c <= _t('/')) || 
            (c >= _t(':') && c <= _t('@')) || 
            (c >= _t('[') && c <= _t('`')) ||
            c >= _t('{')
            )
            return 2;
        if(c >= _t('0') && c <= _t('9'))
            return 3;
        if(c >= _t('A') && c <= _t('z'))
            return 4;
    }
    else if((c & 0x8080) == 0x8080)
        return 5;
    return -1;
}

int edit::prev_logic_char(int pos)
{
    if(!pos || !(pos = prev_char(pos)))
        return 0;
    uint c;
    get_next_char(_textbuf.c_str()+pos, c);
    int f = get_logic_field(c);
    for(;;) {
        int np = prev_char(pos);
        if(np == 0) { pos = 0; break; }
        get_next_char(_textbuf.c_str()+np, c);
        if(get_logic_field(c) != f)
            break;
        pos = np;
    }
    return pos;
}

int edit::next_logic_char(int pos)
{
    if(pos < 0 || pos >= _textbuf.length())
        return _textbuf.length();
    uint c;
    get_next_char(_textbuf.c_str()+pos, c);
    int f = get_logic_field(c);
    for(pos = next_char(pos); pos <= _textbuf.length(); pos = next_char(pos)) {
        get_next_char(_textbuf.c_str()+pos, c);
        if(get_logic_field(c) != f)
            break;
    }
    return pos;
}

scroller::scroller(wsys_manager* m): button(m)
{
    _rangemin = 0;
    _rangemax = 0;
    _scrpos = 0;
    _vtscroll = false;
}

void scroller::set_scroller(int r1, int r2, bool vts, const image* img, bool as)
{
    assert(r1 <= r2 && img);
    set_image(img, as);
    _vtscroll = vts;
    r2 -= vts ? get_height() : get_width();
    _rangemin = r1;
    _rangemax = r2 < r1 ? r1 : r2;
    point pt(_pos.left, _pos.top);
    if(vts) {
        if(pt.y < _rangemin) pt.y = _rangemin;
        else if(pt.y > _rangemax) pt.y = _rangemax;
    }
    else {
        if(pt.x < _rangemin) pt.x = _rangemin;
        else if(pt.x > _rangemax) pt.y = _rangemax;
    }
    move(pt);
}

void scroller::set_scroll(real32 s)
{
    assert(s >= 0.f && s <= 1.f);
    int n = (int)(s * (_rangemax - _rangemin));
    n += _rangemin;
    move(_vtscroll ? point(_pos.left, n) : point(n, _pos.top));
}

bool scroller::create(widget* ptr, const gchar* name, const rect& rc, uint style)
{
    style |= (sm_movable|sm_hitable);
    return superref::create(ptr, name, rc, style);
}

void scroller::on_hover(uint um, const point& pt)
{
    if((_style & sm_movable) && _htpos.x >= 0 && _htpos.y >= 0) {
        point mt = pt;
        mt.offset(-_htpos.x, -_htpos.y);
        mt.offset(_pos.left, _pos.top);
        if(_vtscroll) {
            mt.x = _pos.left;
            if(mt.y < _rangemin) mt.y = _rangemin;
            else if(mt.y > _rangemax) mt.y = _rangemax;
            _scrpos = (real32)(mt.x - _rangemin);
            _scrpos /= _rangemax - _rangemin;
        }
        else {
            mt.y = _pos.top;
            if(mt.x < _rangemin) mt.x = _rangemin;
            else if(mt.x > _rangemax) mt.x = _rangemax;
            _scrpos = (real32)(mt.y - _rangemin);
            _scrpos /= _rangemax - _rangemin;
        }
        move(mt);
        on_scroll(pt, _scrpos, _vtscroll);
    }
}

wsys_manager::wsys_manager()
{
    _driver = nullptr;
    _fontsys = nullptr;
    _capture = _focus = _hover = _root = nullptr;
    _width = 0;
    _height = 0;
    _painter = nullptr;
    _next_tid = tid_refresh;
}

wsys_manager::~wsys_manager()
{
}

void wsys_manager::set_wsysdrv(wsys_driver* drv)
{
    assert(!_driver);
    _driver = drv;
}

void wsys_manager::set_fontsys(fontsys* fsys)
{
    assert(!_fontsys);
    _fontsys = fsys;
    fsys->initialize();
}

void wsys_manager::set_painter(painter* paint)
{
    _painter = paint;
}

void wsys_manager::initialize(const rect& rc)
{
    assert(_driver && _fontsys);
    system_context ctx;
    ctx.mask = system_context::sct_notify |
        system_context::sct_painter |
        system_context::sct_rectangle;
    ctx.painter = _painter;
    ctx.notify = static_cast<wsys_notify*>(this);
    ctx.rectangle = rc;
    _driver->initialize(ctx);
    _driver->setup();
    /* timer to refresh */
    assert(_next_tid == tid_refresh);
    set_timer(get_timer_id(0), 15);
    assert(_next_tid == tid_caret);
    set_timer(get_timer_id(0), 600);
    /* ime support */
    set_ime(0, point(0,0), font(_t("ËÎÌו"), 14));
}

void wsys_manager::set_dimension(int w, int h)
{
    if(_width != w || _height != h) {
        _width = w;
        _height = h;
        _dirty.set_dimension(w, h);
        /* deal with the canvas */
        image* img = gs_new(image);
        //img->create(w, h);
        //if(image* oldimg = _painter->select(img))
        //    gs_del(image, oldimg);
    }
}

void wsys_manager::refresh(const rect& rc, bool imm)
{
    _dirty.add(rc);
    if(imm != false)
        update();
}

void wsys_manager::update()
{
    if(!_root || !_dirty.is_whole() && !_dirty.size())
        return;
    _painter->on_draw_begin();
    update(_root);
    _painter->on_draw_end();
    assert(_driver);
    _driver->update();
    _dirty.clear();
}

void wsys_manager::update(widget* w)
{
    assert(w);
    rect rc = w->get_rect();
    rc.move_to(0, 0);
    w->top_level(rc);
    if(!w->is_visible() || !_dirty.is_dirty(rc))
        return;
    //_painter->lock(rc);
    w->draw(_painter);
    //_painter->unlock();
    if(widget* c = w->_child) {
        while(c->_next)
            c = c->_next;
        for( ; c; c = c->_last)
            update(c);
    }
}

widget* wsys_manager::hit_test(widget* ptr, point pt)
{
    if(ptr == 0 || !ptr->is_visible() || !ptr->is_enable())
        return nullptr;
    const rect& rc = ptr->get_rect();
    if(!rc.in_rect(pt) || !ptr->hit_test(pt))
        return nullptr;
    point p = pt;
    p.offset(-rc.left, -rc.top);
    for(widget* c = ptr->_child; c; c = c->_next) {
        if(widget* r = hit_test(c, p))
            return r;
    }
    return ptr;
}

widget* wsys_manager::hit_proc(const point& pt, point& pt1)
{
    widget* ptr = _capture;
    if(!ptr && !(ptr = hit_test(pt)))
        return 0;
    assert(!pt1.x && !pt1.y);
    ptr->top_level(pt1);
    pt1.x = pt.x - pt1.x;
    pt1.y = pt.y - pt1.y;
    return ptr;
}

widget* wsys_manager::set_capture(widget* ptr, bool b)
{
    widget* r = _capture;
    if(b == true)
        _capture = ptr;
    else if(_capture == ptr)
        _capture = nullptr;
    return r;
}

widget* wsys_manager::set_focus(widget* ptr)
{
    widget* p = _focus;
    _focus = ptr;
    if(ptr) ptr->on_focus(true);
    if(p)   p->on_focus(false);
    return p;
}

void wsys_manager::on_show(bool b)
{
}

void wsys_manager::on_create(wsys_driver* ptr, const rect& rc)
{
    _driver = ptr;
    _painter->set_dirty(&_dirty);
    set_dimension(rc.width(), rc.height());
}

void wsys_manager::on_close()
{
    _focus = 0;
    _capture = 0;
    if(_root) remove_widget(_root);
    //if(image* oldimg = _painter->select(0))
    //    gs_del(image, oldimg);
}

void wsys_manager::on_resize(const rect& rc)
{
    /* unsupported */
}

void wsys_manager::on_paint(const rect& rc)
{
    /*
     * The whole system would be refreshed by the timer,
     * so simply add the rect to the dirty list here
     */
    refresh(rc);
}

void wsys_manager::on_halt()
{
}

void wsys_manager::on_resume()
{
}

bool wsys_manager::on_mouse_down(uint um, unikey uk, const point& pt)
{
    point pt1;
    if(widget* ptr = hit_proc(pt, pt1)) {
        ptr->on_press(um, uk, pt1);
        return true;
    }
    return false;
}

bool wsys_manager::on_mouse_up(uint um, unikey uk, const point& pt)
{
    point pt1;
    if(widget* ptr = hit_proc(pt, pt1)) {
        ptr->on_click(um, uk, pt1);
        return true;
    }
    return false;
}

bool wsys_manager::on_mouse_move(uint um, const point& pt)
{
    point pt1;
    widget* ptr = hit_proc(pt, pt1);
    if(_hover && _hover != ptr) {
        point pt2;
        _hover->top_level(pt2);
        pt2.x = pt.x - pt2.x;
        pt2.y = pt.y - pt2.y;
        _hover->on_leave(um, pt2);
    }
    if(_hover = ptr)
        ptr->on_hover(um, pt1);
    return ptr != nullptr;
}

bool wsys_manager::on_key_down(uint um, unikey uk)
{
    if(_focus && _focus->is_enable() && _focus->is_visible())
        _focus->on_keydown(um, uk);
    return true;
}

bool wsys_manager::on_key_up(uint um, unikey uk)
{
    if(_focus && _focus->is_enable() && _focus->is_visible())
        _focus->on_keyup(um, uk);
    return true;
}

bool wsys_manager::on_char(uint um, uint ch)
{
    if(_focus && _focus->is_enable() && _focus->is_visible())
        _focus->on_char(um, ch);
    return true;
}

void wsys_manager::on_timer(uint tid)
{
    assert(tid < _next_tid);
    if(tid == tid_refresh)
        update();
    else if(tid == tid_caret) {
        if(_focus && _focus->is_enable() && _focus->is_visible())
            _focus->on_caret();
    }
    else {
        assert(tid < (uint)_timer_map.size());
        if(widget* ptr = _timer_map[tid-tid_refresh])
            ptr->on_timer(tid);
    }
}

widget* wsys_manager::find_widget(const string& name)
{
    widget_map::iterator i = _widget_map.find(name);
    return i == _widget_map.end() ? 0 : (i->second);
}

bool wsys_manager::remove_widget(const string& name)
{
    widget_map::iterator i = _widget_map.find(name);
    if(i != _widget_map.end())
        return remove_widget(i);
    return false;
}

bool wsys_manager::remove_widget(widget* ptr)
{
    assert(ptr);
    /* remove all its children */
    for(widget* c = ptr->_child; c;) {
        widget* p = c;
        c = c->_next;
        if(!remove_widget(p))
            return false;
    }
    /* notification */
    ptr->close();
    /* clean up */
    for(uint j = 0; j < (uint)_timer_map.size(); j ++) {
        if(_timer_map[j] == ptr) {
            kill_timer(j);
            _timer_map[j] = 0;
        }
    }
    gs_del(widget, ptr);
    if(_root == ptr)
        _root = nullptr;
    if(_capture == ptr)
        _capture = nullptr;
    if(_focus == ptr)
        _focus = nullptr;
    return true;
}

bool wsys_manager::remove_widget(widget_map::iterator i)
{
    widget* ptr = i->second;
    assert(ptr);
    /* remove all its children */
    for(widget* c = ptr->_child; c;) {
        widget* p = c;
        c = c->_next;
        if(!remove_widget(p))
            return false;
    }
    /* notification */
    ptr->close();
    /* clean up */
    for(uint j = 0; j < (uint)_timer_map.size(); j ++) {
        if(_timer_map[j] == ptr) {
            kill_timer(j);
            _timer_map[j] = 0;
        }
    }
    _widget_map.erase(i);
    gs_del(widget, ptr);
    if(_root == ptr)
        _root = nullptr;
    if(_capture == ptr)
        _capture = nullptr;
    if(_focus == ptr)
        _focus = nullptr;
    return true;
}

uint wsys_manager::get_timer_id(widget* w)
{
    _timer_map.push_back(w);
    return _next_tid ++;
}

void wsys_manager::clear_timer()
{
    if(!_driver)
        return;
    for(uint i = tid_refresh + 1; i < _next_tid; i ++)
        _driver->kill_timer(i);
}

void wsys_manager::set_ime(widget* ptr, point pt, const font& ft)
{
    if(ptr) ptr->top_level(pt);
    assert(_driver);
    _driver->set_ime(pt, ft);
}

void wsys_manager::set_clipboard(const gchar* fmt, const void* ptr, int size)
{
    assert(_driver);
    //_driver->set_clipboard(fmt, ptr, size);
}

int wsys_manager::get_clipboard(const gchar* fmt, const void*& ptr)
{
    assert(_driver);
    //return _driver->get_clipboard(fmt, ptr);
    return 0;
}

int wsys_manager::get_clipboard(clipboard_list& cl, int c)
{
    assert(_driver);
    //return _driver->get_clipboard(cl, c);
    return 0;
}

__pink_end__
