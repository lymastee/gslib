#include <gslib/mtrand.h>
#include <ariel/texbatch.h>
#include <gslib/uuid.h>
#include <ariel/fsyswin32.h>
#include <ariel/textureop.h>
#include <ariel/rendersysd3d11.h>
#include <ariel/widget.h>
#include <ariel/scene.h>
#include <gslib/error.h>
#include <ariel/imageop.h>
#include <windows.h>

using namespace gs;
using namespace gs::ariel;

#define generate_counts     50
#define max_font_size       30
#define min_font_size       10
#define max_string_size     6
#define min_string_size     1
#define max_chs_code        0x6000
#define min_chs_code        0x4e00

typedef std::vector<image> image_cache;

static string currentfolderpath;

static void make_random_image(fontsys* fsys, image& img)
{
    assert(fsys);
    /* rand font */
    int fontsize = mtrand() % (max_font_size - min_font_size) + min_font_size;
    font f(_t("ËÎÌו"), fontsize);
    fsys->set_font(f);
    /* rand string */
    int stringsize = mtrand() % (max_string_size - min_string_size) + min_string_size;
    string s;
    for(int i = 0; i < stringsize; i ++) {
        int chs = mtrand() % (max_chs_code - min_chs_code) + min_chs_code;
        s.push_back((gchar)chs);
    }
    /* rand color */
    color fontcr(mtrand() % 255, mtrand() % 255, mtrand() % 255),
        bkcr(mtrand() % 255, mtrand() % 255, mtrand() % 255);
    /* create image */
    int w, h;
    fsys->get_size(s.c_str(), w, h);
    img.create(image::fmt_rgba, w, h);
    img.enable_alpha_channel(false);
    img.clear(bkcr);
    fsys->draw(img, s.c_str(), 0, 0, fontcr);
    /* rand name */
    string uidstr;
    uuid uid(uuid_v1);
    uid.to_string(uidstr);
    /* save image */
    string fname;
    fname.format(_t("%s/unpacked/%s.png"), currentfolderpath.c_str(), uid.to_string(uidstr));
    img.save(fname.c_str());
}

int gs_main()
{
    rendersys* rsys = scene::get_singleton_ptr()->get_rendersys();
    fontsys* fsys = scene::get_singleton_ptr()->get_fontsys();
    assert(rsys && fsys);
    /* current folder */
    gchar szfolder[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, szfolder);
    currentfolderpath.assign(szfolder);
    /* create folder */
    string cmd;
    cmd.format(_t("del \"%s/unpacked\" /f /q"), currentfolderpath.c_str());
    _wsystem(cmd.c_str());
    cmd.format(_t("mkdir \"%s/unpacked\""), currentfolderpath.c_str());
    _wsystem(cmd.c_str());
    tex_batcher batcher;
    image_cache cache(generate_counts);
    for(image& img : cache) {
        make_random_image(fsys, img);
#ifdef _GS_BATCH_IMAGE
        batcher.add_image(&img);
#else
        auto* tex = rsys->create_texture2d(img, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0);
        assert(tex);
        batcher.add_texture(tex);
#endif
    }
    batcher.arrange();
    batcher.tracing();
    image atlas;
#ifdef _GS_BATCH_IMAGE
    batcher.create_packed_image(atlas);
#else
    auto* texatlas = batcher.create_texture(rsys);
    assert(texatlas);
    textureop::convert_to_image(atlas, texatlas);
#endif
    string fname;
    fname.format(_t("%s/unpacked/packed.png"), szfolder);
    atlas.save(fname.c_str());
    _wsystem(_t("pause"));
    return 0;
}
