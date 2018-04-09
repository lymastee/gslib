#include <random>
#include <gslib/error.h>
#include <ariel/rectpack.h>

// for timeGetTime
#pragma comment(lib, "winmm.lib")

static float min_width = 2.f;
static float max_width = 100.f;
static float min_height = 2.f;
static float max_height = 100.f;

using namespace gs;

void make_random_rect(float& w, float& h)
{
    std::random_device dev;
    std::uniform_real_distribution<float> w_dist(min_width, max_width),
        h_dist(min_height, max_height);
    std::default_random_engine gen(dev());
    w = w_dist(gen);
    h = h_dist(gen);
}

static void get_serial_rect(float& w, float& h)
{
    static const vec2 p[] =
    {
        vec2(26.1383934f, 38.0416107f),
        vec2(43.9968185f, 58.4261742f),
        vec2(36.1055679f, 22.3914337f),
        vec2(40.2648659f, 87.1881027f),
        vec2(46.1324806f, 6.21282387f),
        vec2(10.0626898f, 22.6631641f),
        vec2(16.1211586f, 37.5101776f),
        vec2(3.32914090f, 98.1467209f),
        vec2(81.1805573f, 59.4216690f),
        vec2(55.9062729f, 35.8143082f)
    };
    static int i = 0;
    w = p[i].x;
    h = p[i].y;
    if(i < _countof(p) - 1)
        i ++;
}

int wmain(int argc, gchar* argv[])
{
    printf("this is a benchmark test of the binary packing algorithm.\n\n");
    gs::string str;

    // take the dimensions of the random rects from arguments if existed
    if(argc >= 5) {
        str.assign(argv[1]);    min_width = (float)str.to_real();
        str.assign(argv[2]);    max_width = (float)str.to_real();
        str.assign(argv[3]);    min_height = (float)str.to_real();
        str.assign(argv[4]);    max_height = (float)str.to_real();
        if(min_width <= 0.f)
            min_width = 1.f;
        if(min_height <= 0.f)
            min_height = 1.f;
        if(max_width < min_width)
            max_width = min_width;
        if(max_height < min_height)
            max_height = min_height;
    }

    // printf dimensions
    str.format(_t("dimensions of the random rects for test was below,\n")
        "width range: %f ~ %f\n"
        "height range: %f ~ %f\n\n",
        min_width, max_width, min_height, max_height
        );
    wprintf(str.c_str());

    int input_count = 10;
    printf("now input how many random rects would you like to generate for test:\n");
    if(scanf_s("%d", &input_count) != 1) {
        printf("bad value, now quit.\n");
        system("pause");
        return -1;
    }

    gs::ariel::rect_packer rp;
    gs::ariel::rp_input_list quad_to_pack;
    for(int i = 0; i < input_count; i ++) {
        gs::ariel::rp_input input;
        make_random_rect(input.width, input.height);
        input.binding = (void*)0xffffffff;
        quad_to_pack.push_back(input);
    }

    printf("test set generated, start packing...\n\n");

    uint t1 = timeGetTime();
    rp.pack_automatically(quad_to_pack);
    uint t2 = timeGetTime();
    printf("time elapse: %dms\n\n", t2 - t1);

    // summary
    printf("summary:\n");
    str.format(_t("pack times: %d\n"), rp.get_pack_times());
    wprintf(str.c_str());
    auto& rnode = rp.get_root_node();
    str.format(_t("pack size: %f x %f\n"), rnode.width, rnode.height);
    wprintf(str.c_str());
    str.format(_t("pack area: %f\n"), rnode.width * rnode.height);
    wprintf(str.c_str());

    // sum of input areas
    float sum_area = 0.f;
    for(auto& i : quad_to_pack) {
        float a = i.width * i.height;
        sum_area += a;
    }
    str.format(_t("sum of areas: %f\n"), sum_area);
    wprintf(str.c_str());

    // area used ratio
    float ratio = sum_area / (rnode.width * rnode.height);
    str.format(_t("area used ratio: %f\n\n"), ratio);
    wprintf(str.c_str());

    rp.tracing();
    trace_to_clipboard();
    system("pause");
    return 0;
}
