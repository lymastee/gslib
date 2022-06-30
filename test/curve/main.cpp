#include <windows.h>
#include <ariel/framesys.h>
#include <ariel/scene.h>
#include <ariel/applicationwin32.h>
#include "curve_view.h"

using namespace gs;
using namespace gs::ariel;

class my_application:
    public application
{
public:
    virtual bool setup(const app_config& cfg, const app_env& env) override
    {
        if(!__super::setup(cfg, env))
            return false;

        wsys_manager* wsys = scene::get_singleton_ptr()->get_ui_system();
        painter* paint = wsys->get_painter();
        paint->set_hints(painter::hint_antialias, true);

        auto cv = wsys->add_widget<curve_view>(nullptr, _t("root"), rect(0, 0, wsys->get_width(), wsys->get_height()), sm_visible | sm_hitable);

        framesys::get_framesys()->refresh();
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
    cfg.position = rect(50, 50, 1080, 640);
    my_application app;
    app.simple_setup(cfg);
    return app.run();
}