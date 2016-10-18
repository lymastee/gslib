#include <gslib/string.h>
#include <gslib/error.h>
#include <gslib/gsm.h>
#include <gslib/mtrand.h>

using namespace gs;

void print_errors()
{
    if(get_last_error()) {
        strtool::ctlprintf(_t("unknown error."));
        return;
    }
    strtool::ctlprintf(_t("print error stack:\n"));
    while(auto* err = get_last_error()) {
        strtool::ctlprintf(_t("%s:%d> %s\n"), err->file, err->line, err->desc);
        pop_error();
    }
}

#ifdef _UNICODE
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    if(argc <= 1) {
        string help;
        gsm_help(help);
        strtool::ctlprintf(help.c_str());
        return 0;
    }
    /* srand */
    mtbestseed();
    static const string cmdarg_debug(_cststr(_t("-debug"))),
        cmdarg_release(_cststr(_t("-release"))),
        cmdarg_compiler(_cststr(_t("-compiler"))),
        cmdarg_src(_cststr(_t("-src"))),
        cmdarg_tar(_cststr(_t("-tar")));
    /* set default arguments. */
    gsm_config cfgs;
    cfgs.is_debug = true;
    /* parse arugments */
    for(int i = 1; i < argc; i ++) {
        if(cmdarg_debug == argv[i])
            cfgs.is_debug = true;
        else if(cmdarg_release == argv[i])
            cfgs.is_debug = false;
        else if(cmdarg_compiler == argv[i]) {
            if(++ i >= argc) {
                strtool::ctlprintf(_t("error: missing argument."));
                return 1;
            }
            cfgs.spec_compiler.assign(argv[i]);
        }
        else if(cmdarg_src == argv[i]) {
            if(++ i >= argc) {
                strtool::ctlprintf(_t("error: missing argument."));
                return 1;
            }
            cfgs.source_dir.assign(argv[i]);
        }
        else if(cmdarg_tar == argv[i]) {
            if(++ i >= argc) {
                strtool::ctlprintf(_t("error: missing argument."));
                return 1;
            }
            cfgs.target_dir.assign(argv[i]);
        }
    }
    gsm_globals globals;
    if(!gsm_setup_globals(globals, cfgs)) {
        print_errors();
        return 1;
    }
    vdir vd;
    gsm_proj_list holdings;
    if(!gsm_prepare_projects(holdings, vd, globals) ||
        !gsm_generate_projects(vd, globals) ||
        !gsm_finalize_projects(vd, holdings, globals)
        ) {
        print_errors();
        return 1;
    }
    vd.save(globals.target_dir.c_str());
    return 0;
}