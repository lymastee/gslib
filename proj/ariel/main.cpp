#include <gslib/type.h>
#include <gslib/error.h>
#include <ariel/framesys.h>
#include <ariel/config.h>
#include <ariel/rose.h>
#include <ariel/painter.h>
#include <ariel/scene.h>
#include <ariel/clip.h>
#include <gslib/rtree.h>
#include <ariel/delaunay.h>
#include <ariel/loopblinn.h>
#include <ariel/fsysdwrite.h>
#include <ariel/fsyswin32.h>
#include <ariel/textureop.h>
#include <ariel/applicationwin32.h>
//#include <ariel/clip2.h>

// #pragma comment(lib, "d3d11.lib")
// #pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "d3dx11d.lib")

#pragma comment(lib, "d3d10_1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")

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

//         static bool bprinted = false;
//         if(!bprinted) {
//             bprinted = true;
//             clipper cl;
//             cl.add_path(path);
//             cl.tracing();
//         }

        painter_path simplified_path;
        path.simplify(simplified_path);
        simplified_path.tracing();

//         path.move_to(10.f, 10.f);
//         path.line_to(230.f, 210.f);

        painter_brush brush;

        static com_ptr<render_texture2d> tex;
        if(!tex) {
            img.load(_t("666.jpg"));
            auto* rsys = scene::get_singleton_ptr()->get_rendersys();
            assert(rsys);
            tex.attach(rsys->create_texture2d(img, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0));
            assert(tex.get());
        }
        painter_extra_data ext;
        ext.reset(new painter_picture_data(tex.get()), [](painter_picture_data* p) { delete p; });
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

static void proc_cdt(delaunay_triangulation& cdt, dt_input_joints& inputs, const painter_path& path)
{
    cdt.initialize(inputs);
    cdt.run();
    dt_edge_list bound;
    for(int i = 0; i < path.size() - 1; i ++)
        bound.push_back(cdt.add_constraint(path.get_node(i)->get_point(), path.get_node(i + 1)->get_point()));
    bound.push_back(cdt.add_constraint(path.get_node(path.size() - 1)->get_point(), path.get_node(0)->get_point()));
    cdt.trim(bound);
    cdt.set_range_left(bound.front());
}

class my_application:
    public application
{
public:
    virtual bool setup(const app_config& cfg, const app_env& env) override
    {
        if(!__super::setup(cfg, env))
            return false;

        framesys* sys = framesys::get_framesys();
        scene* scn = scene::get_singleton_ptr();
        wsys_manager* wsys = scn->get_ui_system();

    //     painter_path apath;
    //     apath.move_to(580.881493f, 375.054329f);
    //     apath.line_to(580.025711f, 376.019935f);
    //     apath.line_to(571.770802f, 385.337651f);
    //     apath.line_to(563.501057f, 394.655653f);
    //     apath.line_to(555.212673f, 403.958343f);
    //     apath.line_to(546.901844f, 413.230125f);
    //     apath.line_to(538.564767f, 422.455403f);
    //     apath.line_to(534.881550f, 426.489041f);
    //     apath.line_to(533.502162f, 425.268568f);
    //     apath.line_to(523.232004f, 416.440259f);
    //     apath.line_to(512.845346f, 407.801677f);
    //     apath.line_to(502.330682f, 399.376787f);
    //     apath.line_to(491.676505f, 391.189556f);
    //     apath.line_to(480.871306f, 383.263949f);
    //     apath.line_to(469.903580f, 375.623931f);
    //     apath.line_to(458.761818f, 368.293468f);
    //     apath.line_to(447.434513f, 361.296525f);
    //     apath.line_to(435.910158f, 354.657069f);
    //     apath.line_to(424.177246f, 348.399064f);
    //     apath.line_to(412.224270f, 342.546476f);
    //     apath.line_to(400.039722f, 337.123271f);
    //     apath.line_to(387.612095f, 332.153414f);
    //     apath.line_to(374.929882f, 327.660870f);
    //     apath.line_to(361.981575f, 323.669606f);
    //     apath.line_to(376.344659f, 324.899583f);
    //     apath.line_to(390.380594f, 326.399648f);
    //     apath.line_to(404.105739f, 328.156296f);
    //     apath.line_to(417.536449f, 330.156023f);
    //     apath.line_to(430.689083f, 332.385326f);
    //     apath.line_to(443.579998f, 334.830698f);
    //     apath.line_to(456.225552f, 337.478637f);
    //     apath.line_to(468.642102f, 340.315637f);
    //     apath.line_to(480.846004f, 343.328195f);
    //     apath.line_to(492.853618f, 346.502805f);
    //     apath.line_to(504.681300f, 349.825964f);
    //     apath.line_to(516.345408f, 353.284167f);
    //     apath.line_to(527.862298f, 356.863909f);
    //     apath.line_to(539.248330f, 360.551687f);
    //     apath.line_to(550.519859f, 364.333995f);
    //     apath.line_to(561.693243f, 368.197330f);
    //     apath.line_to(572.784841f, 372.128186f);
    //     apath.reverse();
    // 
    //     dt_input_joints inputs;
    //     // only line to
    //     for(int i = 0; i < apath.size(); i ++)
    //         inputs.push_back(dt_joint(apath.get_node(i)->get_point(), (void*)i));
    // 
    //     delaunay_triangulation cdt;
    //     proc_cdt(cdt, inputs, apath);
    //     cdt.tracing();


    //     auto* pdwfsys = new fsys_dwrite();
    //     pdwfsys->initialize();
    // 
    //     pdwfsys->set_font(font(_t("simsun"), 10));
    //     com_ptr<texture2d> textex;
    //     pdwfsys->create_text_texture(&textex, _t("ÇëÊäÈë..."), 1, color(255,0,0));
    // 
    //     image img;
    //     textureop::convert_to_image(img, textex.get());
    //     img.save(_t("d:/tttex.png"));

        painter* pntr = wsys->get_painter();
        assert(pntr);
        pntr->set_hints(painter::hint_antialias, true);
        wsys->add_widget<sample_widget>(0, _t(""), rect(0, 0, 640, 480), sm_visible | sm_hitable);
        sys->refresh();

        return true;
    }
};

#ifdef _UNICODE
int __stdcall wWinMain
#else
int __stdcall WinMain
#endif
    (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
{
    app_config cfg;
    framesys::set_default_config(cfg);
    my_application app;
    app.simple_setup(cfg);
    return app.run();
}

