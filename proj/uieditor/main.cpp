#include "io.h"
#include "basis.h"
#include "uic.h"
#include <gslib/file.h>
#include <ariel/scene.h>

using namespace gs;
using namespace gs::ariel;

int gs_main()
{
    xmltree src;
    src.load(_t("testui.txt"));

    ui_editor::ui_tree uitree;
    ui_editor::ui_editor_context uiectx;
    ui_editor::ui_convert_source_to_core(uiectx, uitree, src);

    string cs;
    ui_editor::ui_compile(cs, uiectx, uitree);

    {
        file ff(_t("ss.h"), _t("w,ccs=UTF-8"));
        ff.write(cs.c_str());
    }

    wsys_manager* wsys = scene::get_singleton_ptr()->get_ui_system();
    assert(wsys);
    painter* ptex = wsys->get_painter();
    ptex->set_hints(painter::hint_anti_alias, true);
    ui_editor::ui_create_widgets(wsys, nullptr, uitree);

    int r = framesys::get_framesys()->run();

    xmltree savesrc;
    ui_editor::ui_convert_core_to_source(savesrc, uiectx, uitree);
    savesrc.set_encode(xmltree::encode_utf16);
    savesrc.save(_t("testui.txt"));

    return r;
}