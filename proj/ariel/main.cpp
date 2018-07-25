#include <gslib/type.h>
#include <gslib/error.h>
//#include <gslib/entrywin32.h>
#include <ariel/framesys.h>
#include <ariel/config.h>
#include <ariel/rose.h>
#include <ariel/raster.h>
#include <ariel/scene.h>
#include <ariel/clip.h>
#include <gslib/rtree.h>
#include <ariel/delaunay.h>
#include <ariel/loopblinn.h>

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

        static render_texture2d* tex = nullptr;
        if(!tex) {
            img.load(_t("666.jpg"));
            auto* rsys = scene::get_singleton_ptr()->get_rendersys();
            assert(rsys);
            tex = rsys->create_texture2d(img, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0); // todo: release
            assert(tex);
        }
        painter_extra_data ext;
        ext.reset(gs_new(painter_picture_data, tex), [](painter_picture_data* p) { gs_del(painter_picture_data, p); });
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

int gs_main()
{
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
