#include <gslib/error.h>
#include <gslib/rtree.h>
#include <windows.h>

using namespace gs;

typedef rtree_entity<int> myentity;
typedef rtree_node<myentity> mynode;
typedef _tree_allocator<mynode> myalloc;
typedef tree<myentity, mynode, myalloc> mytree;
typedef rtree<myentity, quadratic_split_alg<5, 2, mytree>, mynode, myalloc> myrtree;
typedef deque<myentity> myentlist;

static void make_rand_rect(rectf& rc, int width, int height)
{
    int x = rand() % width;
    int y = rand() % height;
    int maxhalfw = gs_min(x, width - x);
    int maxhalfh = gs_min(y, height - y);
    float ratio = 0.5f;
    int fixedhalfw = (int)(ratio * maxhalfw);
    int fixedhalfh = (int)(ratio * maxhalfh);
    int halfw, halfh;
    halfw = (fixedhalfw == 0) ? 0 : (rand() % fixedhalfw);
    halfh = (fixedhalfh == 0) ? 0 : (rand() % fixedhalfh);
    rc.left = (float)(x - halfw);
    rc.right = (float)(x + halfw);
    rc.top = (float)(y - halfh);
    rc.bottom = (float)(y + halfh);
}

static void make_entity_list(myentlist& entlist, int cap, int w, int h)
{
    for(int i = 0; i < cap; i ++) {
        rectf rc;
        make_rand_rect(rc, w, h);
        entlist.push_back(myentity(rand(), rc));
    }
}

int main()
{
    const int test_area_width = 640;
    const int test_area_height = 480;
    const int test_times = 50000;

    srand(GetTickCount());

    myrtree rt;
    myentlist entlist;
    make_entity_list(entlist, test_times, test_area_width, test_area_height);

    auto t1 = GetTickCount();

    rt.insert(entlist);

    /*
    std::for_each(entlist.begin(), entlist.end(), [&rt](const myentity& ent) {
        rt.insert(ent.get_bind_arg(), ent.const_rect());
        //rt.tracing();
    });
    */

    auto t2 = GetTickCount();
    auto elapse = t2 - t1;
    gs::string info;
    info.format(_t("time elapse: %i"), (int)elapse);
    MessageBox(0, info.c_str(), _t("note"), MB_OK);

    trace(_t("trace while insert finished.\n"));
    rt.tracing();

    while(!entlist.empty()) {
        int i = rand() % (int)entlist.size();
        const auto& p = entlist.at(i);
        rt.remove(p.get_bind_arg(), p.const_rect());
        rt.tracing();
        entlist.erase(entlist.begin() + i);
    }

    return 0;
}
