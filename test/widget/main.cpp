#pragma once
#include <ariel/widget.h>
#include <ariel/scene.h>

using namespace gs;
using namespace gs::ariel;

class weditctl:
    public edit
{
public:
    weditctl(wsys_manager* m):
        edit(m)
    {
    }
    virtual bool create(widget* ptr, const gchar* name, const rect& rc, uint style)
    {
        if(!__super::create(ptr, name, rc, style))
            return false;
        _bkgnd.create(image::fmt_rgba, rc.width(), rc.height());
        _bkgnd.enable_alpha_channel(true);
        _bkgnd.clear(color(0, 0, 0, 60));
        set_bkground(&_bkgnd);
        return true;
    }

protected:
    image       _bkgnd;
};

/* float text */
class wflttext:
    public widget
{
public:
    wflttext(wsys_manager* m):
        widget(m)
    {
        _timerid = m->get_timer_id(this);
    }
    virtual ~wflttext()
    {
        _manager->kill_timer(_timerid);
    }
    virtual void draw(painter* cvs)
    {
        cvs->draw_image(&_cpytext, 0, 0);
    }
    virtual void on_timer(uint tid)
    {
        assert(tid == _timerid);
        _alpha -= _fadeby;
        if(_alpha <= 0) {
            _manager->remove_widget(this);
        }
        else {
            //_cpytext.copy(_text);
            //_cpytext.fade_to(_alpha);
            point pt(get_rect().left, get_rect().top);
            pt.y -= 2;
            move(pt);
        }
    }
    void set_text(const gchar* str)
    {
        fontsys* pfs = scene::get_singleton_ptr()->get_fontsys();
        assert(pfs);
        font ft(_t("����"), 16);
        pfs->set_font(ft);
        int w, h;
        pfs->get_size(str, w, h);
        _text.create(image::fmt_rgba, w, h);
        _text.enable_alpha_channel(true);
        assert(_text.is_valid());
        pfs->create_text_image(_text, str, 0, 0, color(200,20,20));
        _cpytext.create(image::fmt_rgba, w, h);
        _cpytext.enable_alpha_channel(true);
        _cpytext.copy(_text);
        _alpha = 1.0f;
        _fadeby = 0.02f;
        rect rc = get_rect();
        rc.right = rc.left + w;
        rc.bottom = rc.top + h;
        move(rc);
    }
    void start()
    {
        _manager->set_timer(_timerid, 30);
    }
    static wflttext* create_inst(widget* parent, point pt, const gchar* str)
    {
        wsys_manager* mgr = scene::get_singleton_ptr()->get_ui_system();
        assert(mgr);
        wflttext* ptr = mgr->add_widget<wflttext>(
            parent, 0, rect(pt.x,pt.y,0,0), sm_hitable|sm_visible
            );
        ptr->set_text(str);
        ptr->start();
        return ptr;
    }

protected:
    image       _text;
    image       _cpytext;
    uint        _timerid;
    real32      _alpha;
    real32      _fadeby;
};

/* background */
class wbkground:
    public widget
{
public:
    wbkground(wsys_manager* m):
        widget(m)
    {
    }
    virtual bool create(widget* ptr, const gchar* name, rect const& rc, uint style)
    {
        if(!__super::create(ptr, name, rc, style))
            return false;
        if(!_bkgnd.load(_t("bkground.png")))
            return false;

        /* create sub-widgets */
        _editptr = _manager->add_widget<weditctl>(this, _t("edit"), rect(36,20,150,18), sm_hitable|sm_visible);
        assert(_editptr);
        _editptr->set_text(_t("������..."));

        _btn = _manager->add_widget<button>(this, _t("clickme"), rect(195,12,0,0), sm_hitable|sm_visible);
        assert(_btn);
        _btn_image.load(_t("button.png"));
        _btn->set_image(&_btn_image);

        /* reflection */
        //reflect(_btn, hid_click);

        return true;
    }
    virtual void draw(painter* cvs)
    {
        cvs->draw_image(&_bkgnd, 0, 0);
    }
    virtual void on_reflect(widget* ptr, int msgid, va_list vlst)
    {
        /* must be button clicked */
        int w = get_width(), h = get_height();
        point pt;
        pt.x = rand() % w;
        pt.y = rand() % h;
        wflttext::create_inst(this, pt, _editptr->get_text());
    }

protected:
    image       _bkgnd;
    weditctl*   _editptr;
    button*     _btn;
    image       _btn_image;
};

// int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
// {
//     wsysdrv_win32 wsd(hInstance, lpCmdLine);
//     fsys_win32 fs;
//     wsys_manager* mgr = wsys_manager::get_singleton_ptr();
//     mgr->set_wsysdrv(&wsd);
//     mgr->set_fontsys(&fs);
//     mgr->initialize(rect(100,100,800,500));
// 
//     widget* root = mgr->add_widget<wbkground>(
//         0, _t("wbkground"), rect(0,0,mgr->get_width(),mgr->get_height()),
//         sm_hitable|sm_visible
//         );
// 
//     return wsd.run();
// }

int gs_main()
{
    wsys_manager* wsys = scene::get_singleton_ptr()->get_ui_system();
    painter* ptex = wsys->get_painter();
    assert(ptex);
    ptex->set_hints(painter::hint_anti_alias, true);
    widget* root = wsys->add_widget<wbkground>(
        0, _t("wbkground"), rect(0,0, wsys->get_width(), wsys->get_height()),
        sm_hitable|sm_visible
        );
    framesys::get_framesys()->refresh();
    return framesys::get_framesys()->run();
}