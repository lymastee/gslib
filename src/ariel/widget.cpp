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
#include <ariel/widget.h>
#include <ariel/scene.h>
#include <ariel/imageop.h>
#include <ariel/textureop.h>

__ariel_begin__

/* in GPU draw, no immediate refresh */
#define refresh_immediately false

const uuid uuid_widget(_t("a18b997a-aa05-4d8b-b5b1-f6e98b9ff426"));

widget::widget(wsys_manager* m)
{
    _manager = m;
    _prev = _next = _child = _last_child = _parent = nullptr;
    _style = 0;
    _visible = false;
    _enabled = true;
}

widget::~widget()
{
}

void* widget::query_interface(const uuid& uid)
{
    if(uid == uuid_widget)
        return this;
    return nullptr;
}

bool widget::create(widget* ptr, const gchar* name, const rect& rc, uint style)
{
    if(_parent = ptr) {
        if(!ptr->_child) {
            ptr->_child = this;
            assert(!ptr->_last_child);
            ptr->_last_child = this;
        }
        else {
            widget* c = ptr->_last_child;
            c->_next = this;
            _prev = c;
            ptr->_last_child = this;
        }
    }
    assert(name);
    _name.assign(name);
    _pos = rc;
    _style = style;
    if(style & sm_visible)
        _visible = true;
    refresh(false);
    return true;
}

void widget::close()
{
    _visible = false;
    _enabled = false;
    capture(false);
    assert(!_child);
    if(_parent && _parent->_child == this)
        _parent->_child = _next;
    if(_parent && _parent->_last_child == this)
        _parent->_last_child = _prev;
    if(_prev)
        _prev->_next = _next;
    if(_next)
        _next->_prev = _prev;
}

void widget::show(bool b)
{
    if(_visible != b) {
        _visible = b;
        refresh(false);
    }
    if(!b)
        capture(false);
}

void widget::enable(bool b)
{
    if(_enabled != b) {
        _enabled = b;
        refresh(false);
    }
}

void widget::move(const rect& rc)
{
    if(rc.right < rc.left || rc.bottom < rc.top)
        return;
    refresh(false);
    _pos = rc;
    refresh(refresh_immediately);
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

void widget::lay(widget* ptr, laytag t)
{
    /* currently unsupported */
    assert(0);
}

void widget::on_press(uint um, unikey uk, const point& pt)
{
    if(uk == mk_left)
        capture(true);
}

void widget::on_click(uint um, unikey uk, const point& pt)
{
    if(uk == mk_left) {
        capture(false);
        /* focus! */
        focus();
    }
}

void widget::on_hover(uint um, const point& pt)
{
}

void widget::on_leave(uint um, const point& pt)
{
}

bool widget::is_focused() const
{
    return _manager ? (_manager->get_focus() == this) : false;
}

point& widget::to_global(point& pt) const
{
    pt.offset(_pos.left, _pos.top);
    if(!_parent)
        return pt;
    return _parent->to_global(pt);
}

rect& widget::to_global(rect& rc) const
{
    rc.offset(_pos.left, _pos.top);
    if(!_parent)
        return rc;
    return _parent->to_global(rc);
}

point& widget::to_local(point& pt) const
{
    point org(0, 0);
    to_global(org);
    pt.offset(-org.x, -org.y);
    return pt;
}

rect& widget::to_local(rect& rc) const
{
    point org(0, 0);
    to_global(org);
    rc.offset(-org.x, -org.y);
    return rc;
}

void widget::move(const point& pt)
{
    rect rc = _pos;
    rc.move_to(pt);
    move(rc);
}

void widget::resize(int w, int h)
{
    rect rc = _pos;
    rc.set_rect(rc.left, rc.top, w, h);
    move(rc);
}

void widget::refresh(bool imm)
{
    refresh(rect(0, 0, get_width(), get_height()), imm);
}

bool widget::register_accelerator(unikey key, uint mask)
{
    assert(_manager);
    return _manager->register_accelerator(this, key, mask);
}

widget* widget::unregister_accelerator(unikey key, uint mask)
{
    assert(_manager);
    return _manager->unregister_accelerator(key, mask);
}

button::button(wsys_manager* m): widget(m)
{
    _source = nullptr;
    _bkground = nullptr;
    _4states = false;
    _btnstate = bs_none;
}

button::~button()
{
    if(_bkground) {
        release_texture2d(_bkground);
        _bkground = nullptr;
    }
}

void button::draw(painter* paint)
{
    if(_bkground)
        paint->draw_image(_bkground, 0, 0);
}

void button::enable(bool b)
{
    bool dif = _enabled != b;
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

void button::set_press()
{
    if(!_source)
        return;
    auto* rsys = scene::get_singleton_ptr()->get_rendersys();
    assert(rsys);
    _4states ?
        textureop(rsys).copy_rect(_bkground, _source, 0, 0, get_width() * 2, 0, get_width(), get_height()) :
        textureop(rsys).set_brightness(_bkground, _source, 0.7f);
    refresh(refresh_immediately);
}

void button::set_normal()
{
    if(!_source)
        return;
    auto* rsys = scene::get_singleton_ptr()->get_rendersys();
    assert(rsys);
    textureop(rsys).copy_rect(_bkground, _source, 0, 0);
    refresh(refresh_immediately);
}

void button::set_hover()
{
    if(!_source)
        return;
    auto* rsys = scene::get_singleton_ptr()->get_rendersys();
    assert(rsys);
    _4states ?
        textureop(rsys).copy_rect(_bkground, _source, 0, 0, get_width(), 0, get_width(), get_height()) :
        textureop(rsys).set_brightness(_bkground, _source, 1.3f);
    refresh(refresh_immediately);
}

void button::set_gray()
{
    if(!_source)
        return;
    auto* rsys = scene::get_singleton_ptr()->get_rendersys();
    assert(rsys);
    _4states ?
        textureop(rsys).copy_rect(_bkground, _source, 0, 0, get_width() * 3, 0, get_width(), get_height()) :
        textureop(rsys).set_gray(_bkground, _source);
    refresh(refresh_immediately);
}

void button::set_image(texture2d* img, bool fs)
{
    assert(img);
    assert(!_source);
    if(_source != nullptr)
        return;
    _source = img;
    _4states = fs;
    refresh(false);
    int w, h;
    textureop::get_texture_dimension(img, w, h);
    if(fs) w >>= 2;
    _pos.right = _pos.left + w;
    _pos.bottom = _pos.top + h;
    auto* rsys = scene::get_singleton_ptr()->get_rendersys();
    assert(rsys);
    _bkground = rsys->create_texture2d(w, h, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0, 0);
    _enabled ? set_normal() : set_gray();
}

edit_line::edit_line(wsys_manager* m): widget(m)
{
    _caret_on = false;
    _caretpos = 0;
    _bkground = nullptr;
    _sel_start = -1;
    _txtcolor = color(0, 0, 0);
    _selcolor = color(0, 125, 255);
    _crtcolor = color(0, 0, 0);
    _font = font(_t("simsun"), 10);
}

void edit_line::draw(painter* paint)
{
    draw_background(paint);
    draw_normal_text(paint);
    draw_select_text(paint);
    draw_caret(paint);
}

void edit_line::on_press(uint um, unikey uk, const point& pt)
{
    if(uk == mk_left) {
        int pos = hit_char(pt);
        set_select(pos, pos);
        refresh(refresh_immediately);
    }
    superref::on_press(um, uk, pt);
}

void edit_line::on_click(uint um, unikey uk, const point& pt)
{
    if(uk == mk_left) {
        int pos = hit_char(pt);
        set_select(_sel_start, pos);
        refresh(refresh_immediately);
    }
    superref::on_click(um, uk, pt);
}

void edit_line::on_hover(uint um, const point& pt)
{
    if(um & um_lmouse) {
        int pos = hit_char(pt);
        set_select(_sel_start, pos);
        refresh(refresh_immediately);
    }
    superref::on_hover(um, pt);
}

void edit_line::on_char(uint um, uint ch)
{
    /* ASCII codes */
    if((ch >= 0x20 && ch < 0x7f) || ch == 0x09 || ch == 0x0a) {
        del_select();
        _textbuf.insert((size_t)_caretpos, 1, (char)ch);
        set_caret(_caretpos + 1);
        trim_if_overrun(true);
        return;
    }
#ifndef _UNICODE
    /* MBCS codes */
    if(ch & 0x8080) {
        del_select();
        _textbuf.insert((size_t)_caretpos, 1, (char)((ch & 0xff00) >> 8));
        _textbuf.insert((size_t)_caretpos + 1, 1, (char)(ch & 0xff));
        set_caret(_caretpos + 2);
        trim_if_overrun(true);
        return;
    }
#else
    if(ch > 0x80) {
        del_select();
        _textbuf.insert((size_t)_caretpos, 1, (wchar_t)ch);
        set_caret(_caretpos + 1);
        trim_if_overrun(true);
        return;
    }
#endif
}

void edit_line::on_keydown(uint um, unikey uk)
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
            _manager->set_clipboard(cf_text, _textbuf.c_str() + start, size);
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

void edit_line::on_caret()
{
    _caret_on = !_caret_on;
    refresh(false);
}

void edit_line::on_focus(bool b)
{
    if(!b) {
        _caret_on = false;
        refresh(false);
    }
}

void edit_line::set_text(const gchar* str)
{
    if(!str) {
        _textbuf.clear();
        set_caret(0);
    }
    else {
        _textbuf.assign(str);
        if(_caretpos > _textbuf.length())
            set_caret(_textbuf.length());
        trim_if_overrun(true);
    }
    set_select(-1, 0);
}

void edit_line::set_caret(int n)
{
    _caretpos = n < 0 || n > _textbuf.length() ? 
        _textbuf.length() : n;
    string s;
    s.assign(_textbuf.c_str(), _caretpos);
    fontsys* pfs = scene::get_singleton_ptr()->get_fontsys();
    assert(pfs);
    int w, h;
    pfs->set_font(_font);
    pfs->query_size(s.c_str(), w, h, s.length());
    _manager->set_ime(this, point(w, 0), _font);
    refresh(false);
}

void edit_line::set_select(int start, int end)
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

void edit_line::replace_select(const gchar* str)
{
    del_select();
    if(str) {
        _textbuf.insert(_caretpos, str);
        set_caret(_caretpos + strtool::length(str));
        trim_if_overrun(true);
    }
}

void edit_line::draw_background(painter* paint)
{
    assert(paint);
    if(_bkground)
        paint->draw_image(_bkground, 0, 0);
}

void edit_line::draw_normal_text(painter* paint)
{
    assert(paint);
    if(_sel_start < 0 || _sel_start == _sel_end) {
        paint->set_font(_font);
        paint->draw_text(_textbuf.c_str(), 0, 0, _txtcolor, _textbuf.length());
    }
    else {
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
        if(start <= 0 && end >= _textbuf.length())      /* all selected */
            return;
        paint->set_font(_font);
        if(start > 0)
            paint->draw_text(_textbuf.c_str(), 0.f, 0.f, _txtcolor, start);
        if(end < _textbuf.length()) {
            int w, h;
            paint->get_text_dimension(_textbuf.c_str(), w, h, end);
            paint->draw_text(_textbuf.c_str() + end, (float)w, 0.f, _txtcolor, _textbuf.length() - end);
        }
    }
}

void edit_line::draw_select_text(painter* paint)
{
    assert(paint);
    if(_sel_start < 0 || _sel_start == _sel_end)    /* no selection */
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
    paint->set_font(_font);
    int bias = 0;
    if(start > 0) {
        int cast;
        paint->get_text_dimension(_textbuf.c_str(), bias, cast, start);
    }
    int w, h;
    paint->get_text_dimension(_textbuf.c_str() + start, w, h, end - start);
    paint->draw_rect(rectf((float)bias, 0.f, (float)w, (float)h), _selcolor);
    paint->draw_text(_textbuf.c_str() + start, (float)bias, 0.f, color(255, 255, 255), end - start);
}

void edit_line::draw_caret(painter* paint)
{
    assert(paint);
    if(_caret_on) {
        string s;
        s.assign(_textbuf.c_str(), _caretpos);
        int w, h;
        paint->get_text_dimension(s.c_str(), w, h, s.length());
        paint->draw_line(pointf((float)w, 0.f), pointf((float)w, (float)get_height()), _crtcolor);
    }
}

void edit_line::del_select()
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

int edit_line::hit_char(point pt)
{
    if(pt.x <= 0 || !_textbuf.length())
        return 0;
    /* treat as mono font and predict the width */
    fontsys* pfs = scene::get_singleton_ptr()->get_fontsys();
    assert(pfs);
    int w, h, fixc;
    pfs->set_font(_font);
    pfs->query_size(_t("a"), w, h);
    fixc = pt.x / w;
    if(fixc > _textbuf.length())
        fixc = _textbuf.length();
#ifndef _UNICODE
    if(is_mbcs_half(_textbuf.c_str() + fixc)) {
        assert(fixc);  /* broken MBCS? */
        fixc --;
    }
#endif
    string s;
    s.assign(_textbuf.c_str(), fixc);
    pfs->query_size(s.c_str(), w, h, s.length());
    if(pt.x > w) {
        for(fixc ++; fixc <= _textbuf.length(); fixc ++) {
            s.push_back(_textbuf.at(fixc - 1));
#ifndef _UNICODE
            if(_textbuf.at(fixc - 1) & 0x80)
                s.push_back(_textbuf.at(fixc ++));
#endif
            pfs->query_size(s.c_str(), w, h, s.length());
            if(pt.x < w)
                break;
        }
        if(fixc > _textbuf.length())
            return _textbuf.length();
#ifndef _UNICODE
        fixc -= (s.back() & 0x80) ? 2 : 1;
#else
        fixc --;
#endif
    }
    else if (pt.x < w) {
        if(fixc == 0)
            return 0;
        for(fixc --; fixc > 0; fixc --) {
#ifndef _UNICODE
            if(s.pop_back() & 0x80) {
                fixc --;
                s.pop_back();
            }
#else
            s.pop_back();
#endif
            pfs->query_size(s.c_str(), w, h, s.length());
            if(pt.x >= w)
                break;
        }
    }
    return fixc;
}

int edit_line::prev_char(int pos)
{
    if(pos < 0 || pos > _textbuf.length())
        pos = _textbuf.length();
    if(pos > 0)
#ifndef _UNICODE
        pos -= (_textbuf.at(pos - 1) & 0x80) ? 2 : 1;
#else
        pos --;
#endif
    if(pos < 0)
        pos = 0;
    return pos;
}

int edit_line::next_char(int pos)
{
    if(pos < 0 || pos > _textbuf.length())
        pos = _textbuf.length();
    if(pos < _textbuf.length())
#ifndef _UNICODE
        pos += (_textbuf.at(pos) & 0x80) ? 2 : 1;
#else
        pos ++;
#endif
    if(pos > _textbuf.length())
        pos = _textbuf.length();
    return pos;
}

static int get_logic_field(uint c)
{
    static const string sfield1 = _t(" \t\v\a\b\f");
#ifndef _UNICODE
    if(!(c & 0x8080)) {
#else
    if(c < 0x80) {
#endif
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
#ifndef _UNICODE
    else if((c & 0x8080) == 0x8080)
#else
    else
#endif
        return 5;
    return -1;
}

int edit_line::prev_logic_char(int pos)
{
    if(!pos || !(pos = prev_char(pos)))
        return 0;
    uint c;
    get_next_char(_textbuf.c_str() + pos, c);
    int f = get_logic_field(c);
    while(pos > 0) {
        int np = prev_char(pos);
        get_next_char(_textbuf.c_str() + np, c);
        if(get_logic_field(c) != f)
            break;
        pos = np;
    }
    return pos;
}

int edit_line::next_logic_char(int pos)
{
    if(pos < 0 || pos >= _textbuf.length())
        return _textbuf.length();
    uint c;
    get_next_char(_textbuf.c_str() + pos, c);
    int f = get_logic_field(c);
    for(pos = next_char(pos); pos <= _textbuf.length(); pos = next_char(pos)) {
        get_next_char(_textbuf.c_str() + pos, c);
        if(get_logic_field(c) != f)
            break;
    }
    return pos;
}

int edit_line::trim_if_overrun(bool alarm)
{
    if(_textbuf.empty())
        return 0;
    fontsys* fsys = scene::get_singleton_ptr()->get_fontsys();
    assert(fsys);
    fsys->set_font(_font);
    int len = _textbuf.length();
    int c = len;
    for( ; c > 0; c --) {
        int w, h;
        fsys->query_size(_textbuf.c_str(), w, h, c);
        if(w <= get_width())
            break;
    }
    if(!c) {
        _textbuf.clear();
        set_caret(0);
    }
    else if(c != len) {
        _textbuf.resize(c);
        if(_caretpos > c)
            set_caret(c);
    }
    int alarm_count = len - c;
    static const int max_alarm_count = 10;
    if(alarm) {
        int alarms = gs_min(alarm_count, max_alarm_count);
        for(int i = 0; i < alarms; i ++)
            sound_alarm();
    }
    return alarm_count;
}

timer::timer(wsys_manager* mgr)
{
    if(mgr)
        initialize(mgr->get_driver());
    else {
        /* only used by dvt ops */
        _driver = nullptr;
        _userdata = 0;
        _elapse = 0;
        _single = false;
    }
}

timer::~timer()
{
    if(_driver && _elapse) {    /* which means the timer was set */
        _elapse = 0;
        _driver->kill_timer((uint)this);
    }
}

void timer::initialize(wsys_driver* drv)
{
    assert(drv);
    _driver = drv;
    _userdata = 0;
    _elapse = 0;
    _single = false;
}

void timer::start(int elapse)
{
    _single = false;
    assert(_driver);
    _elapse = elapse;
    _driver->set_timer((uint)this, elapse);
}

void timer::start_single(int elapse)
{
    _single = true;
    assert(_driver);
    _elapse = elapse;
    _driver->set_timer((uint)this, elapse);
}

void timer::on_notified()
{
    if(_single) {
        _single = false;        /* ensure delete once */
        delete this;
    }
}

bool accel_key::from_string(const string& str)
{
    if(str.empty())
        return false;
    const gchar* s = str.c_str();
    assert(s);
    int len = str.length();
    while(len > 0) {
        if(len >= _cststrlen(_t("Ctrl+")) &&
            strtool::compare_cl(s, _t("Ctrl+"), _cststrlen(_t("Ctrl+"))) == 0) {
            mask |= um_control;
            s += _cststrlen(_t("Ctrl+"));
            len -= _cststrlen(_t("Ctrl+"));
        }
        else if(len >= _cststrlen(_t("Shift+")) &&
            strtool::compare_cl(s, _t("Shift+"), _cststrlen(_t("Shift+"))) == 0) {
            mask |= um_shift;
            s += _cststrlen(_t("Shift+"));
            len -= _cststrlen(_t("Shift+"));
        }
        else if(len >= _cststrlen(_t("Alt+")) &&
            strtool::compare_cl(s, _t("Alt+"), _cststrlen(_t("Alt+"))) == 0) {
            mask |= um_alter;
            s += _cststrlen(_t("Alt+"));
            len -= _cststrlen(_t("Alt+"));
        }
        else break;
    }
    if(len <= 0) {
        assert(!"no accel key.");
        return false;
    }
    else if(len == 1) {
        if((s[0] >= 0x20 && s[0] <= 0x60) ||
            (s[0] >= 0x7b && s[0] <= 0x7f)) {
            key = (unikey)s[0];
            return true;
        }
        else if(s[0] >= 0x61 && s[0] <= 0x7a) {
            key = (unikey)(s[0] - 0x20);        /* turn to upper case */
            return true;
        }
        return false;
    }
    else {
        if(strtool::compare_cl(s, _t("Backspace"), _cststrlen(_t("Backspace"))) == 0) { key = uk_bs; return true; }
        else if(strtool::compare_cl(s, _t("Tab"), _cststrlen(_t("Tab"))) == 0) { key = uk_tab; return true; }
        else if(strtool::compare_cl(s, _t("Enter"), _cststrlen(_t("Enter"))) == 0) { key = uk_cr; return true; }
        else if(strtool::compare_cl(s, _t("Esc"), _cststrlen(_t("Esc"))) == 0) { key = uk_esc; return true; }
        else if(strtool::compare_cl(s, _t("Space"), _cststrlen(_t("Space"))) == 0) { key = uk_sp; return true; }
        else if(strtool::compare_cl(s, _t("Del"), _cststrlen(_t("Del"))) == 0) { key = uk_del; return true; }
        else if(strtool::compare_cl(s, _t("Insert"), _cststrlen(_t("Insert"))) == 0) { key = vk_insert; return true; }
        else if(strtool::compare_cl(s, _t("Caps"), _cststrlen(_t("Caps"))) == 0) { key = vk_caps; return true; }
        else if(strtool::compare_cl(s, _t("Print Screen"), _cststrlen(_t("Print Screen"))) == 0) { key = vk_pscr; return true; }
        else if(strtool::compare_cl(s, _t("Num Lock"), _cststrlen(_t("Num Lock"))) == 0) { key = vk_numlock; return true; }
        else if(strtool::compare_cl(s, _t("Home"), _cststrlen(_t("Home"))) == 0) { key = vk_home; return true; }
        else if(strtool::compare_cl(s, _t("End"), _cststrlen(_t("End"))) == 0) { key = vk_end; return true; }
        else if(strtool::compare_cl(s, _t("Page Up"), _cststrlen(_t("Page Up"))) == 0) { key = vk_pageup; return true; }
        else if(strtool::compare_cl(s, _t("Page Down"), _cststrlen(_t("Page Down"))) == 0) { key = vk_pagedown; return true; }
        else if(strtool::compare_cl(s, _t("Left Arrow"), _cststrlen(_t("Left Arrow"))) == 0) { key = vk_left; return true; }
        else if(strtool::compare_cl(s, _t("Up Arrow"), _cststrlen(_t("Up Arrow"))) == 0) { key = vk_up; return true; }
        else if(strtool::compare_cl(s, _t("Right Arrow"), _cststrlen(_t("Right Arrow"))) == 0) { key = vk_right; return true; }
        else if(strtool::compare_cl(s, _t("Down Arrow"), _cststrlen(_t("Down Arrow"))) == 0) { key = vk_down; return true; }
        else if(strtool::compare_cl(s, _t("F10"), _cststrlen(_t("F10"))) == 0) { key = vk_f10; return true; }
        else if(strtool::compare_cl(s, _t("F11"), _cststrlen(_t("F11"))) == 0) { key = vk_f11; return true; }
        else if(strtool::compare_cl(s, _t("F12"), _cststrlen(_t("F12"))) == 0) { key = vk_f12; return true; }
        else if(strtool::compare_cl(s, _t("F1"), _cststrlen(_t("F1"))) == 0) { key = vk_f1; return true; }
        else if(strtool::compare_cl(s, _t("F2"), _cststrlen(_t("F2"))) == 0) { key = vk_f2; return true; }
        else if(strtool::compare_cl(s, _t("F3"), _cststrlen(_t("F3"))) == 0) { key = vk_f3; return true; }
        else if(strtool::compare_cl(s, _t("F4"), _cststrlen(_t("F4"))) == 0) { key = vk_f4; return true; }
        else if(strtool::compare_cl(s, _t("F5"), _cststrlen(_t("F5"))) == 0) { key = vk_f5; return true; }
        else if(strtool::compare_cl(s, _t("F6"), _cststrlen(_t("F6"))) == 0) { key = vk_f6; return true; }
        else if(strtool::compare_cl(s, _t("F7"), _cststrlen(_t("F7"))) == 0) { key = vk_f7; return true; }
        else if(strtool::compare_cl(s, _t("F8"), _cststrlen(_t("F8"))) == 0) { key = vk_f8; return true; }
        else if(strtool::compare_cl(s, _t("F9"), _cststrlen(_t("F9"))) == 0) { key = vk_f9; return true; }
        return false;
    }
}

const string& accel_key::to_string(string& str) const
{
    str.clear();
    if((mask & um_control) || (mask & um_scontrol))
        str.append(_t("Ctrl+"));
    if((mask & um_shift) || (mask & um_sshift))
        str.append(_t("Shift+"));
    if((mask & um_alter) || (mask & um_salter))
        str.append(_t("Alt+"));
    if(key >= 0x20 && key <= 0x7e)
        str.push_back((gchar)key);
    else {
        switch(key)
        {
        case uk_bs:
            str.append(_t("Bkspce"));
            break;
        case uk_tab:
            str.append(_t("Tab"));
            break;
        case uk_cr:
            str.append(_t("Enter"));
            break;
        case uk_esc:
            str.append(_t("Esc"));
            break;
        case uk_sp:
            str.append(_t("Space"));
            break;
        case uk_del:
            str.append(_t("Del"));
            break;
        case vk_insert:
            str.append(_t("Ins"));
            break;
        case vk_caps:
            str.append(_t("Caps"));
            break;
        case vk_pscr:
            str.append(_t("PrScrn"));
            break;
        case vk_numlock:
            str.append(_t("NumLock"));
            break;
        case vk_home:
            str.append(_t("Home"));
            break;
        case vk_end:
            str.append(_t("End"));
            break;
        case vk_pageup:
            str.append(_t("PgUp"));
            break;
        case vk_pagedown:
            str.append(_t("PgDn"));
            break;
        case vk_left:
            str.push_back(0x2190);      /* left arrow */
            break;
        case vk_up:
            str.push_back(0x2191);      /* up arrow */
            break;
        case vk_right:
            str.push_back(0x2192);      /* right arrow */
            break;
        case vk_down:
            str.push_back(0x2193);      /* down arrow */
            break;
        case vk_f1:
            str.append(_t("F1"));
            break;
        case vk_f2:
            str.append(_t("F2"));
            break;
        case vk_f3:
            str.append(_t("F3"));
            break;
        case vk_f4:
            str.append(_t("F4"));
            break;
        case vk_f5:
            str.append(_t("F5"));
            break;
        case vk_f6:
            str.append(_t("F6"));
            break;
        case vk_f7:
            str.append(_t("F7"));
            break;
        case vk_f8:
            str.append(_t("F8"));
            break;
        case vk_f9:
            str.append(_t("F9"));
            break;
        case vk_f10:
            str.append(_t("F10"));
            break;
        case vk_f11:
            str.append(_t("F11"));
            break;
        case vk_f12:
            str.append(_t("F12"));
            break;
        }
    }
    return str;
}

wsys_manager::wsys_manager()
{
    _driver = nullptr;
    _capture = _focus = _hover = _root = nullptr;
    _width = 0;
    _height = 0;
    _painter = nullptr;
    _caret = nullptr;
}

wsys_manager::~wsys_manager()
{
    if(_caret) {
        delete _caret;
        _caret = nullptr;
    }
}

void wsys_manager::set_wsysdrv(wsys_driver* drv)
{
    assert(!_driver);
    _driver = drv;
}

void wsys_manager::set_painter(painter* paint)
{
    _painter = paint;
}

void wsys_manager::initialize(const rect& rc)
{
    /* dimensions */
    set_dimension(rc.width(), rc.height());
    /* timer */
    assert(!_caret && _driver);
    _caret = new timer(this);
    assert(_caret);
    connect_notify(_caret, &timer::on_timer, this, &wsys_manager::on_caret, 4);
    _caret->start(600);
    /* IME support */
    set_ime(0, point(0, 0), font(_t("simsun"), 14));
}

void wsys_manager::set_dimension(int w, int h)
{
    if(_width != w || _height != h) {
        _width = w;
        _height = h;
        _dirty.set_dimension(w, h);
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
    float x = (float)rc.left, y = (float)rc.top;
    rc.move_to(0, 0);
    w->to_global(rc);
    if(!w->is_visible() || !_dirty.is_dirty(rc))
        return;
    mat3 m;
    m.translation(x, y);
    _painter->save();
    _painter->set_tranform(m);
    w->draw(_painter);
    if(widget* c = w->_child) {
        for( ; c; c = c->_next)
            update(c);
    }
    _painter->restore();
}

widget* wsys_manager::hit_test(widget* ptr, const point& pt)
{
    if(!ptr || !ptr->is_visible() || !ptr->is_enabled())
        return nullptr;
    const rect& rc = ptr->get_rect();
    if(!rc.in_rect(pt) || !ptr->hit_test(pt))
        return nullptr;
    point p = pt;
    p.offset(-rc.left, -rc.top);
    for(widget* c = ptr->_last_child; c; c = c->_prev) {
        if(widget* r = hit_test(c, p))
            return r;
    }
    return ptr;
}

widget* wsys_manager::hit_widget(const point& pt, point& pt1)
{
    widget* ptr = _capture;
    if(!ptr && !(ptr = hit_test(pt)))
        return nullptr;
    assert(!pt1.x && !pt1.y);
    ptr->to_global(pt1);
    pt1.x = pt.x - pt1.x;
    pt1.y = pt.y - pt1.y;
    return ptr;
}

widget* wsys_manager::set_capture(widget* ptr, bool b)
{
    widget* r = _capture;
    if(b == true) {
        _capture = ptr;
        ptr->on_capture(true);
    }
    else if(_capture == ptr) {
        _capture = nullptr;
        ptr->on_capture(false);
    }
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

void wsys_manager::on_caret(uint)
{
    if(_focus && _focus->is_enabled() && _focus->is_visible())
        _focus->on_caret();
}

void wsys_manager::set_cursor(cursor_type curty)
{
    if(_driver)
        _driver->set_cursor(curty);
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
    _focus = nullptr;
    _capture = nullptr;
    _hover = nullptr;
    if(_root)
        remove_widget(_root);
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
    if(widget* ptr = hit_widget(pt, pt1)) {
        ptr->on_press(um, uk, pt1);
        return true;
    }
    return false;
}

bool wsys_manager::on_mouse_up(uint um, unikey uk, const point& pt)
{
    point pt1;
    if(widget* ptr = hit_widget(pt, pt1)) {
        ptr->on_click(um, uk, pt1);
        return true;
    }
    return false;
}

bool wsys_manager::on_mouse_move(uint um, const point& pt)
{
    if(_capture) {
        point pt1;
        _capture->to_global(pt1);
        pt1.x = pt.x - pt1.x;
        pt1.y = pt.y - pt1.y;
        _capture->on_hover(um, pt1);
        return true;
    }
    point pt1;
    widget* ptr = hit_widget(pt, pt1);
    if(_hover && _hover != ptr) {
        point pt2;
        _hover->to_global(pt2);
        pt2.x = pt.x - pt2.x;
        pt2.y = pt.y - pt2.y;
        _hover->on_leave(um, pt2);
    }
    if(_hover = ptr)
        ptr->on_hover(um, pt1);
    return ptr != nullptr;
}

static bool try_proceed_accelerator(const wsys_manager::accel_map& m, unikey uk, uint um)
{
    auto f = m.find(accel_key(uk, um));
    if(f == m.end())
        return false;
    assert(f->second);
    f->second->on_accelerator(uk, um);
    return true;
}

bool wsys_manager::on_key_down(uint um, unikey uk)
{
    if(try_proceed_accelerator(_accel_map, uk, um))
        return true;
    if(_focus && _focus->is_enabled() && _focus->is_visible())
        _focus->on_keydown(um, uk);
    return true;
}

bool wsys_manager::on_key_up(uint um, unikey uk)
{
    if(_focus && _focus->is_enabled() && _focus->is_visible())
        _focus->on_keyup(um, uk);
    return true;
}

bool wsys_manager::on_char(uint um, uint ch)
{
    if(_focus && _focus->is_enabled() && _focus->is_visible())
        _focus->on_char(um, ch);
    return true;
}

void wsys_manager::on_timer(uint tid)
{
    auto* t = reinterpret_cast<timer*>(tid);
    assert(t);
    t->on_timer(tid);
    t->on_notified();
}

widget* wsys_manager::find_widget(const string& name)
{
    widget_map::iterator i = _widget_map.find(name);
    return i == _widget_map.end() ? 0 : (i->second);
}

bool wsys_manager::remove_widget(const string& name)
{
    auto f = _widget_map.find(name);
    if(f == _widget_map.end())
        return false;
    auto* w = f->second;
    assert(w);
    _widget_map.erase(f);
    return remove_widget_internal(w);
}

bool wsys_manager::remove_widget(widget* ptr)
{
    assert(ptr);
    if(ptr->is_delete_later())
        return false;
    if(!ptr->get_name().empty()) {
        auto f = _widget_map.find(ptr->get_name());
        if(f != _widget_map.end()) {
            if(f->second != ptr)
                return false;
            _widget_map.erase(f);
        }
    }
    return remove_widget_internal(ptr);
}

bool wsys_manager::remove_widget_internal(widget* ptr)
{
    assert(ptr);
    if(ptr->is_delete_later())
        return false;
    /* remove all its children */
    for(widget* c = ptr->_child; c;) {
        widget* p = c;
        c = c->_next;
        if(!remove_widget(p))
            return false;
    }
    /* notification */
    ptr->close();
    dvt_collector::get_singleton_ptr()->set_delete_later(ptr);
    if(_root == ptr)
        _root = nullptr;
    if(_capture == ptr)
        _capture = nullptr;
    if(_focus == ptr)
        _focus = nullptr;
    if(_hover == ptr)
        _hover = nullptr;
    return true;
}

void wsys_manager::set_ime(widget* ptr, point pt, const font& ft)
{
    if(ptr) ptr->to_global(pt);
    assert(_driver);
    _driver->set_ime(pt, ft);
}

void wsys_manager::set_clipboard(clipfmt fmt, const void* ptr, int size)
{
    assert(_driver);
    //_driver->set_clipboard(fmt, ptr, size);
}

int wsys_manager::get_clipboard(clipfmt fmt, const void*& ptr)
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

bool wsys_manager::register_accelerator(widget* w, unikey key, uint mask)
{
    assert(w);
    return _accel_map.try_emplace(accel_key(key, mask), w).second;
}

widget* wsys_manager::unregister_accelerator(unikey key, uint mask)
{
    auto f = _accel_map.find(accel_key(key, mask));
    if(f == _accel_map.end())
        return nullptr;
    auto* w = f->second;
    _accel_map.erase(f);
    return w;
}

__ariel_end__
