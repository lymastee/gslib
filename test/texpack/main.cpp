#include <gslib/mtrand.h>
#include <ariel/texbatch.h>
#include <gslib/uuid.h>
#include <ariel/fsyswin32.h>

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

static fsys_win32 fsyswin32;
static fontsys* fsys = &fsyswin32;
static string currentfolderpath;

static void make_random_image(image& img)
{
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

int wmain()
{
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
    fsys->initialize();
    tex_batcher batcher;
    image_cache cache(generate_counts);
    for(image& img : cache) {
        make_random_image(img);
        batcher.add_image(&img);
    }
    batcher.arrange();
    batcher.tracing();
    image atlas;
    batcher.create_packed_image(atlas);
    string fname;
    fname.format(_t("%s/unpacked/packed.png"), szfolder);
    atlas.save(fname.c_str());
    return 0;
}