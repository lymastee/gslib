#include <ariel/delaunay.h>
#include <gslib/mtrand.h>
#include <gslib/error.h>

#pragma warning(disable: 4996)

using namespace gs;
using namespace gs::ariel;

static vec2 create_rand_point(float u, float v)
{
    float x = mtrandf() * u;
    float y = mtrandf() * v;
    return vec2(x, y);
}

typedef std::vector<vec2> dt_test_seq;

void main()
{
    mtgoodseed();
    delaunay_triangulation dt;
    dt_input_joints inputs;
    const int c = 50;
    const float max_u = 500.f;
    const float max_v = 300.f;
    for(int i = 0; i < c; i ++) {
        auto randp = create_rand_point(max_u, max_v);
        dt_joint j;
        j.set_point(randp);
        inputs.push_back(j);
    }

    // pack test input sequence
    dt_test_seq tseq;
    tseq.resize(inputs.size());
    auto i = inputs.begin(), end = inputs.end();
    for(int j = 0; i != end; ++ i, ++ j)
        tseq.at(j) = i->get_point();

    // trace inputs
    trace(_t("#tracing inputs:\n"));
    for(int j = 0; j < (int)tseq.size(); j ++) {
        auto& p = tseq.at(j);
        trace(_t("@dot %f, %f;\n"), p.x, p.y);
        trace(_t("@tip %f, %f, %i;\n"), p.x, p.y, j);
    }

    dt.initialize(inputs);
    dt.run();
    dt.tracing();

    // for trim
    dt_edge_list edges;

    // debug constraints
    for(;;) {
        int n1, n2;
        printf("input the index of the two points that define the constraint like\n\"[num][space][num]\" "
            "or\n\"0 0\" to start a trim process:"
            );
        scanf("%d %d", &n1, &n2);
        if(!n1 && !n2)
            break;
        if(n1 < 0 || n1 >= (int)tseq.size() || n2 < 0 || n2 >= (int)tseq.size() || n1 == n2) {
            printf("invalid index, quit.\n");
            return;
        }
        auto& p1 = tseq.at(n1);
        auto& p2 = tseq.at(n2);
        trace(_t("#add constraint:\n"));
        trace(_t("@moveTo %f, %f; @lineTo %f, %f;\n"), p1.x, p1.y, p2.x, p2.y);
        auto* e = dt.add_constraint(p1, p2);
        assert(e);
        edges.push_back(e);
        trace(_t("#add constraint ok.\n"));
        dt.tracing();
    }

    // debug trim left
    if(!edges.empty()) {
        trace(_t("#start trimming.\n"));
        dt.trim(edges);
        trace(_t("#trim result.\n"));
        dt.tracing();
    }

    __asm nop;
}

