#include <gslib/avl.h>
#include <gslib/rbtree.h>
#include <gslib/bintree.h>
#include <gslib/mtrand.h>
#include <gslib/error.h>
#include <map>
#include <windows.h>

#pragma warning(disable : 4996)

using namespace gs;

struct test_tree_node
{
    int num;
    string str;
    void operator =(const test_tree_node& n) { num = n.num; }
    const string& to_string()
    {
        str.format(_t("%d"), num);
        return str;
    }
    bool operator==(const test_tree_node& that) const { return num == that.num; }
    bool operator<(const test_tree_node& that) const { return num < that.num; }
};

typedef vector<int> test_value_set;
typedef rbtree<test_tree_node> test_tree;       /* could be used to test red black tree or avl tree */
typedef std::set<test_tree_node> test_tree2;

static test_value_set __test_values;

static void flush_tree(test_tree& tr, int i)
{
    test_tree_node n;
    n.num = i;
    trace(_t("%d\n"), i);
    tr.insert(n);
    __test_values.push_back(i);
}

static void init_test(test_tree& tr)
{
    tr.clear();
    __test_values.clear();
    printf("create new test ok.\n");
}

static void run_test_append(test_tree& tr)
{
    uint n = mtrand() % 31001;
    int i = (int)n - 15500;
    flush_tree(tr, i);
    _print_bintree<test_tree> pt(tr);
    printf("%ls", pt.print());
    //tr.debug_check(tr.get_root());
}

static void run_test_delete(test_tree& tr)
{
    int n = 0;
    char c;
    scanf("%c", &c);
    if(c == 'd') {
        int i = mtrand() % (int)__test_values.size();
        n = __test_values.at(i);
        __test_values.erase(__test_values.begin() + i);
    }
    else {
        scanf("%d", &n);
        auto f = std::find(__test_values.begin(), __test_values.end(), n);
        if(f == __test_values.end())
            return;
        __test_values.erase(f);
    }
    test_tree_node node;
    node.num = n;
    tr.erase(node);
    _print_bintree<test_tree> pt(tr);
    printf("%ls", pt.print());
    //tr.debug_check(tr.get_root());
}

static void run_specific_test(test_tree& tr, int n[], int c)
{
    assert(n);
    for(int i = 0; i < c; i ++)
        flush_tree(tr, n[i]);
}

/*
void main()
{
    mtsrand(mtgoodseed());

    // loop times
    const int c = 10000000;

    test_tree tree1;
    DWORD t1 = GetTickCount();
    for(int i = 0; i < c; i ++)
        rand_append_once(tree1);
    DWORD t2 = GetTickCount();
    printf("time elapse for AVL: %d\n", t2 - t1);

    test_tree2 tree2;
    DWORD t3 = GetTickCount();
    for(int i = 0; i < c; i ++)
        rand_append_once(tree2);
    DWORD t4 = GetTickCount();
    printf("time elapse for RB: %d\n", t4 - t3);

    system("pause");
}
*/

void main()
{
    test_tree tr;
    mtsrand(mtgoodseed());
    printf("random test for AVL tree.\n"
        "  c: create new test.\n"
        "  a: random insert.\n"
        "  d [number]: delete specific element.\n"
        "your option? "
        );
    for(;;) {
        char c;
        scanf("%c", &c);
        switch(c)
        {
        case 'c':
            init_test(tr);
            break;
        case 'a':
            run_test_append(tr);
            break;
        case 'd':
            run_test_delete(tr);
            break;
        case '\n':
        case '\r':
            break;
        default:
            printf("unknown comment, again: ");
            break;
        }
    }
}
