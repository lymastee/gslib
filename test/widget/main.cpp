#pragma once

#include <windows.h>
#include <ariel/widget.h>
#include <ariel/scene.h>
#include <gslib/error.h>
#include <ariel/imageop.h>
#include <ariel/textureop.h>
#include <ariel/utility.h>

#include <ariel/classicstyle.h>

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
        image img;
        img.create(image::fmt_rgba, rc.width(), rc.height());
        img.enable_alpha_channel(true);
        img.clear(color(0, 0, 0, 60));
        auto* rsys = scene::get_singleton_ptr()->get_rendersys();
        assert(rsys);
        auto* tex = rsys->create_texture2d(img, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0);
        assert(tex);
        _bkgnd.attach(tex);
        set_bkground(tex);
        return true;
    }

protected:
    com_ptr<texture2d>  _bkgnd;
};

/* float text */
class wflttext:
    public widget
{
public:
    wflttext(wsys_manager* m):
        widget(m), _animtimer(m)
    {
        connect_notify(&_animtimer, timer::on_timer, this, on_animation, 4);
    }
    virtual ~wflttext()
    {
    }
    virtual void draw(painter* cvs)
    {
        cvs->draw_image(_cpytext.get(), 0, 0);
    }
    void on_animation(uint)
    {
        _alpha -= _fadeby;
        if(_alpha <= 0) {
            _manager->remove_widget(this);
        }
        else {
            auto* rsys = scene::get_singleton_ptr()->get_rendersys();
            assert(rsys);
            textureop(rsys).set_fade(_cpytext.get(), _text.get(), _alpha);
            point pt(get_rect().left, get_rect().top);
            pt.y -= 2;
            move(pt);
        }
    }
    void set_text(const gchar* str)
    {
        fontsys* pfs = scene::get_singleton_ptr()->get_fontsys();
        assert(pfs);
        font ft(_t("¿¬Ìå"), 12);
        pfs->set_font(ft);
        int w, h;
        pfs->get_size(str, w, h);
        pfs->create_text_texture(&_text, str, 0, 0, color(200,20,20));
        rendersys* rsys = scene::get_singleton_ptr()->get_rendersys();
        assert(rsys);
        _cpytext.attach(rsys->create_texture2d(tw, th, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, 0));
        textureop(rsys).copy_rect(_cpytext.get(), _text.get(), 0, 0, 0, 0, tw, th);
        _alpha = 1.0f;
        _fadeby = 0.02f;
        rect rc = get_rect();
        rc.right = rc.left + w;
        rc.bottom = rc.top + h;
        move(rc);
    }
    void start()
    {
        _animtimer.start(30);
    }
    static wflttext* create_inst(widget* parent, point pt, const gchar* str)
    {
        wsys_manager* mgr = scene::get_singleton_ptr()->get_ui_system();
        assert(mgr);
        wflttext* ptr = mgr->add_widget<wflttext>(
            parent, 0, rect(pt.x, pt.y, 0, 0), sm_hitable|sm_visible
            );
        ptr->set_text(str);
        ptr->start();
        return ptr;
    }

protected:
    com_ptr<texture2d>  _text;
    com_ptr<texture2d>  _cpytext;
    real32              _alpha;
    real32              _fadeby;
    timer               _animtimer;
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

        image bkgnd;
        if(!bkgnd.load(_t("bkground.png")))
            return false;
        rendersys* rsys = scene::get_singleton_ptr()->get_rendersys();
        assert(rsys);
        auto* p1 = rsys->create_texture2d(bkgnd, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0);
        assert(p1);
        _bkgnd.attach(p1);

        /* create sub-widgets */
       _editptr = _manager->add_widget<weditctl>(this, _t("edit"), rect(36,20,150,18), sm_hitable|sm_visible);
       assert(_editptr);
       _editptr->set_text(_t("ÇëÊäÈë..."));
       
       _btn = _manager->add_widget<button>(this, _t("clickme"), rect(195,12,0,0), sm_hitable|sm_visible);
       assert(_btn);
       image btn_image;
       btn_image.load(_t("button.png"));
       _btn_image.attach(rsys->create_texture2d(btn_image, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0));
       
       _btn->set_image(_btn_image.get());
       
       connect_notify(_btn, button::on_click, this, on_btn_clicked, 12);

        return true;
    }
    virtual void draw(painter* cvs)
    {
        cvs->draw_image(_bkgnd.get(), 0, 0);
    }
    void on_btn_clicked(uint um, unikey uk, const point& pt)
    {
        /* must be button clicked */
        int w = get_width(), h = get_height();
        point pt1;
        pt1.x = rand() % w;
        pt1.y = rand() % h;
        wflttext::create_inst(this, pt1, _editptr->get_text());
    }

protected:
    com_ptr<texture2d>  _bkgnd;
    weditctl*           _editptr;
    button*             _btn;
    com_ptr<texture2d>  _btn_image;
};

static void trace_quad(const vec2 cp[3])
{
    trace(_t("@!\n"));
    trace(_t("@moveTo %f, %f;\n"), cp[0].x, cp[0].y);
    trace(_t("@quadTo %f, %f, %f, %f;\n"), cp[1].x, cp[1].y, cp[2].x, cp[2].y);
    trace(_t("@@\n"));
}

static void trace_cubic(const vec2 cp[4])
{
    trace(_t("@!\n"));
    trace(_t("@moveTo %f, %f;\n"), cp[0].x, cp[0].y);
    trace(_t("@cubicTo %f, %f, %f, %f, %f, %f;\n"), cp[1].x, cp[1].y, cp[2].x, cp[2].y, cp[3].x, cp[3].y);
    trace(_t("@@\n"));
}

static void trace_quad_ctls(const vector<vec2>& quadctls)
{
    trace_quad_strip(&quadctls.front(), (int)quadctls.size());
}

static float stupid_cubic_length(const vec2 cp[4], int steps)
{
    vector<vec2> intsps;
    intsps.resize(steps);
    ariel::cubic_interpolate(&intsps.front(), cp[0], cp[1], cp[2], cp[3], steps);
    float len = 0.f;
    for(int i = 1; i < steps; i ++)
        len += vec2().sub(intsps.at(i - 1), intsps.at(i)).length();
    return len;
}


// int gs_main()
// {
//     //vec2 cp[4] = { vec2(172.f, 271.f), vec2(58.f, 158.f), vec2(268.f, 18.f), vec2(302.f, 235.f) };
//     //vec2 cp[4] = { vec2(29.f, 303.f), vec2(242.f, 60.f), vec2(152.f, 65.f), vec2(359.f, 299.f) };
//     //vec2 cp[4] = { vec2(21.f, 289.f), vec2(131.f, 109.f), vec2(282.f, 105.f), vec2(441.f, 272.f) };
//     //vec2 cp[4] = { vec2(28.f, 244.f), vec2(116.f, 69.f), vec2(305.f, 240.f), vec2(452.f, 63.f) };
//     //trace_cubic(cp);
//     //vector<vec2> qq;
//     //float tol = 5.f;
//     //ariel::cubic_to_quad_bezier(qq, cp, tol);
//     //trace_quad_ctls(qq);
// 
//     //float len1 = stupid_cubic_length(cp, 100);
//     //float len2 = cubic_bezier_length(cp, tol);
// 
//     wsys_manager* wsys = scene::get_singleton_ptr()->get_ui_system();
//     painter* ptex = wsys->get_painter();
//     assert(ptex);
//     ptex->set_hints(painter::hint_anti_alias, true);
//     wbkground* root = wsys->add_widget<wbkground>(
//         0, _t("wbkground"), rect(0, 0, wsys->get_width(), wsys->get_height()),
//         sm_hitable|sm_visible
//         );
//     framesys::get_framesys()->refresh();
//     return framesys::get_framesys()->run();
// }

int gs_main()
{
   wsys_manager* wsys = scene::get_singleton_ptr()->get_ui_system();
   painter* ptex = wsys->get_painter();
   ptex->set_hints(painter::hint_anti_alias, true);

   auto* bk = wsys->add_widget<classic_style::background>(
       nullptr, _t("background"), rect(0, 0, wsys->get_width(), wsys->get_height()),
       sm_hitable|sm_visible
       );
   bk->flush_style();

   auto* btn1 = wsys->add_widget<classic_style::button>(
       bk, _t("button1"), rect(10, 10, 100, 20), sm_hitable|sm_visible
       );
   btn1->set_value(_t("caption"), _t("button1"));
   btn1->flush_style();

   auto* edit1 = wsys->add_widget<classic_style::edit>(
       bk, _t("edit1"), rect(10, 40, 100, 20), sm_hitable|sm_visible
       );
   edit1->flush_style();
   
   auto* menu1 = wsys->add_widget<classic_style::menu>(
       bk, _t("menu1"), rect(10, 70, 1, 1), sm_hitable|sm_visible
       );
   assert(menu1);
   menu1->flush_style();
   classic_style::create_menu_from_script(menu1,
       _t("Menu Test 1,Ctrl+A:@MenuTestCmd1;")
       _t("[separator];")
       _t("Menu Test 2, Ctrl+B:{")
       _t("Menu Test 2-1,:@MenuTestCmd2;")
       _t("}")
       );
   menu1->startup();

   framesys::get_framesys()->refresh();
   return framesys::get_framesys()->run();
}
