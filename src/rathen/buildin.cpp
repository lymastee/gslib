#include <assert.h>
#include <gslib\string.h>
#include <rathen\buildin.h>

__rathen_begin__

const oprnode opr_manager::opr_list[] =
{
    { 0, _t("("), false },      { 0, _t(")"), false },
    { 1, _t("++"), true },      { 1, _t("--"), true },
    { 2, _t("-"), true },       { 2, _t("!"), true },       { 2, _t("~"), true },
    { 3, _t("+="), false },     { 3, _t("-="), false },     { 3, _t("*="), false },     { 3, _t("/="), false },     { 3, _t("%="), false },
    { 4, _t("*"), false },      { 4, _t("/"), false },      { 4, _t("%"), false },
    { 5, _t("+"), false },      { 5, _t("-"), false },
    { 6, _t("<<"), false },     { 6, _t(">>"), false },
    { 7, _t("<="), false },     { 7, _t(">="), false },     { 7, _t("=="), false },     { 7, _t("!="), false },     { 7, _t("<"), false },      { 7, _t(">"), false },
    { 8, _t("&&"), false },     { 8, _t("||"), false },     { 8, _t("&"), false },      { 8, _t("|"), false },      { 8, _t("^"), false },
    { 9, _t("="), false }
};
const key_node key_manager::key_list[] =
{
    { pat_com, _t("if") },      { pat_com, _t("elif") },    { pat_seg, _t("else") },
    { pat_com, _t("loop") },    { pat_raw, _t("go") },      { pat_raw, _t("stop") },
    { pat_arg, _t("exit") }
};

#define fixsizeof(ent, s)   (sizeof(ent)/sizeof(s))
#define fixcompare(src, cmp) (strtool::compare(src, cmp, strtool::length(cmp) ) == 0)

const oprnode* opr_manager::get_unary_operator(const gchar* src) const
{
    assert(_cstrcmp(opr_list[2].opr, _t("-")) &&
        _cstrcmp(opr_list[3].opr, _t("!")) &&
        _cstrcmp(opr_list[4].opr, _t("~"))
        );
    for(int i = 2; i < 5; i ++) {
        if(fixcompare(src, opr_list[i].opr))
            return opr_list+i;
    }
    return 0;
}

const oprnode* opr_manager::get_binary_operator(const gchar* src) const
{
    for(int i = 5; i < fixsizeof(opr_list, oprnode); i ++) {
        if(fixcompare(src, opr_list[i].opr))
            return opr_list+i;
    }
    return 0;
}

const key_node* key_manager::get_key(const gchar* src) const
{
    for(int i = 0; i < fixsizeof(key_list, key_node); i ++) {
        if(fixcompare(src, key_list[i].key))
            return key_list+i;
    }
    return 0;
}

int key_manager::get_key_pattern(const gchar* src) const
{
    assert(src);
    const key_node* knode = get_key(src);
    assert(knode && "find key failed.");
    return knode ? knode->pattern : pat_err;
}

__rathen_end__
