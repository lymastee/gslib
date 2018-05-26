#include <gslib/type.h>
#include <gslib/error.h>
#include <gslib/entrywin32.h>
#include <ariel/framesys.h>
#include <ariel/config.h>
#include <ariel/rose.h>
#include <ariel/raster.h>
#include <ariel/scene.h>
#include <ariel/clip.h>
#include <gslib/rtree.h>
#include <ariel/delaunay.h>
#include <ariel/loopblinn.h>
#include <gslib/dvt.h>

// #pragma comment(lib, "d3d11.lib")
// #pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "d3dx11d.lib")

using namespace gs;
using namespace gs::ariel;


image img;

class sample_widget:
    public widget
{
public:
    sample_widget(wsys_manager* m): widget(m) {}
    virtual void draw(painter* paint) override
    {
        painter_path path;
//         path.move_to(89.968872f, 117.434502f);
//         path.cubic_to(83.773109f, 71.326660f, 103.929977f, 53.643314f, 134.158203f, 46.822594f);
//         path.close_path();

        path.move_to(100.32f, 159.14f);
        path.cubic_to(41.51f, -15.28f, 224.86f, 71.50f, 287.14f, 27.68f);
        path.line_to(432.43f, 19.89f);
        path.line_to(329.51f, 224.f);
        path.line_to(163.46f, 0.f);
        path.line_to(0.f, 261.19f);
        path.cubic_to(206.70f, 87.93f, 299.24f, 163.75f, 448.86f, 115.03f);
        path.cubic_to(313.66f, 68.32f, 296.94f, 217.08f, 100.32f, 159.14f);
        path.tracing();

        painter_path simplified_path;
        path.simplify(simplified_path);
        simplified_path.tracing();

//         path.move_to(10.f, 10.f);
//         path.line_to(230.f, 210.f);

        painter_brush brush;

        img.load(_t("666.jpg"));
        painter_extra_data ext;
        ext.reset(gs_new(painter_picture_data, &img), [](painter_picture_data* p) { gs_del(painter_picture_data, p); });
        brush.set_tag(painter_brush::picture);
        brush.set_extra(ext);

        //brush.set_tag(painter_brush::solid);
        //brush.set_color(color(255,0,0));
        painter_pen pen;
        pen.set_tag(painter_pen::solid);
        pen.set_color(color(0,255,0));
        paint->save();
        paint->set_brush(brush);
        paint->set_pen(pen);
        paint->draw_path(simplified_path);
        paint->restore();
    }
};

class test666
{
public:
    test666()
    {
        m_a = 111;
        m_b = 787;
    }
    virtual void func1()
    {
        trace(_t("func1()_%d\n"), m_a);
    }
    virtual void func2(int n)
    {
        trace(_t("func2(%d)_%d\n"), n, m_b);
    }

protected:
    int         m_a, m_b;
};

static uint old_func1;
static uint old_func2;

#pragma runtime_checks("scu", off)

static void __declspec(naked) replaced_func1()
{
    __asm call old_func1;
    trace(_t("replaced_func1\n"));
    __asm ret;
}

static void __declspec(naked) replaced_func2()
{
    __asm push dword ptr[esp + 4];
    __asm call old_func2;
    trace(_t("replaced_func2\n"));
    __asm ret 4;
}

#pragma runtime_checks("scu", restore)

#define reflect_child_notify(target, trigger, host, action) { \
    static byte replaced_func[] = { \
        0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:old_func */ \
        0xff, 0xd0,                             /* call eax */ \
        0x8d, 0x0d, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea ecx, 0xcccccccc:host */ \
        0x8d, 0x05, 0xcc, 0xcc, 0xcc, 0xcc,     /* lea eax, 0xcccccccc:action */ \
        0xff, 0xd0,                             /* call eax */ \
        0xc3                                    /* ret */ \
    }; \
    vtable_ops<test666> vo; \
    uint old_func = vo.replace_vtable_method(target, vo.get_virtual_method_index(method_address(trigger)), replaced_func); \
    *(uint*)(&replaced_func[2]) = old_func; \
    *(uint*)(&replaced_func[10]) = (uint)host; \
    uint addr = method_address(action); \
    memcpy(&replaced_func[16], &addr, 4); \
    DWORD oldpro; \
    VirtualProtect(replaced_func, sizeof(replaced_func), PAGE_EXECUTE_READ, &oldpro); \
}

class test666father
{
public:
    test666father(test666* p1, test666* p2):
        son1(p1), son2(p2)
    {
        vtable_ops<test666> vo;
        vo.create_per_instance_vtable(son1);
        reflect_child_notify(son1, test666::func1, this, test666father::on_son1_func1);
    }
    void on_son1_func1()
    {
        trace(_t("on_son1_func1()\n"));
    }

private:
    test666*        son1;
    test666*        son2;
};

int gs_main()
{
    //test666 *t6 = new test666();
    //test666 *t7 = new test666();
    //
    ////test666father father(t6, t7);
    //
    //vtable_ops<test666> vo;
    //
    //t6->func1();
    //t6->func2(666);
    //
    //void* ovt = vo.create_per_instance_vtable(t6);
    //
    //t6->func1();
    //t6->func2(777);
    //
    //old_func1 = vo.replace_vtable_method(t6, vo.get_virtual_method_index(method_address(test666::func1)), replaced_func1);
    //old_func2 = vo.replace_vtable_method(t6, vo.get_virtual_method_index(method_address(test666::func2)), replaced_func2);
    //
    //t6->func1();
    //t6->func2(888);
    //
    //t7->func1();
    //t7->func2(999);
    //
    //vo.destroy_per_instance_vtable(t6, ovt);
    //t6->func1();
    //
    //delete t6;
    //delete t7;

    framesys* sys = framesys::get_framesys();
    scene* scn = scene::get_singleton_ptr();
    wsys_manager* wsys = scn->get_ui_system();
    painter* ptex = wsys->get_painter();
    assert(ptex);
    ptex->set_hints(painter::hint_anti_alias, true);
    wsys->add_widget<sample_widget>(0, _t(""), rect(0,0,640,480), sm_visible | sm_hitable);
    sys->refresh();
    return framesys::get_framesys()->run();
}
