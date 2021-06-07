#include "io.h"
#include "basis.h"
#include "uic.h"
#include <gslib/file.h>
#include <ariel/scene.h>
#include <ariel/applicationwin32.h>

using namespace gs;
using namespace gs::ariel;

class uie_application:
    public application
{
public:
    virtual bool setup(const app_config& cfg, const app_env& env) override
    {
        if(!__super::setup(cfg, env))
            return false;

        xmltree src;
        src.load(_t("testui.txt"));

        ui_editor::ui_tree uitree;
        ui_editor::ui_editor_context uiectx;
        ui_editor::ui_convert_source_to_core(uiectx, uitree, src);

        wsys_manager* wsys = scene::get_singleton_ptr()->get_ui_system();
        assert(wsys);
        painter* ptex = wsys->get_painter();
        ptex->set_hints(painter::hint_antialias, true);
        ui_editor::ui_create_widgets(wsys, nullptr, uitree);

        int r = framesys::get_framesys()->run();

        xmltree savesrc;
        ui_editor::ui_convert_core_to_source(savesrc, uiectx, uitree);
        savesrc.set_encode(xmltree::encode_utf16);
        savesrc.save(_t("testui.txt"));

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
    uie_application app;
    app.simple_setup(cfg);
    return 0;
}
