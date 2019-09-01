#include <io.h>
#include <basis.h>
#include <uic.h>
#include <gslib/string.h>
#include <gslib/file.h>
#include <gslib/error.h>

using namespace gs;
using namespace gs::ariel;

#if defined(UNICODE) || defined(_UNICODE)
#define uic_printf      wprintf
#define uic_wcstr       "%ls"
#else
#define uic_printf      printf
#define uic_wcstr       "%s"
#endif

static void show_help_info()
{
    printf(
        "UIC Tool v1.0 Command help:\n"
        "  1) uic.exe -help:\n"
        "     uic.exe --help:\n"
        "     uic.exe -?:\n"
        "  => Show help info.\n\n"
        "  2) uic.exe \"src\":\n"
        "  => Generate cpp file from ui source file, dest path was ##as the ui file specified## or\n"
        "     ##the same name with src but with .hpp postfix##.\n\n"
        "  3) uic.exe \"src\" \"dest\":\n"
        "  => Generate cpp file from ui source file, dest path was specified by the command line,\n"
        "     ignore the specification from the ui file.\n"
        );
}

static void print_error()
{
    auto* err = get_last_error();
    if(err) {
        printf("Last error:\n");
        uic_printf(err->desc);
    }
}

#if defined(UNICODE) || defined(_UNICODE)
int wmain(_In_ int argc, _In_reads_(argc) _Pre_z_ wchar_t** argv, _In_z_ wchar_t** envp)
#else
int main(_In_ int argc, _In_reads_(argc) _Pre_z_ char** argv, _In_z_ char** envp)
#endif
{
    if(argc <= 1 ||
        strtool::compare(argv[1], _t("-help")) == 0 ||
        strtool::compare(argv[1], _t("--help")) == 0 ||
        strtool::compare(argv[1], _t("-?")) == 0) {
        show_help_info();
        return 0;
    }

    xmltree src;
    if(!src.load(argv[1])) {
        printf("Compile ui failed: " uic_wcstr "\n", argv[1]);
        print_error();
        return -1;
    }

    printf("Compile ui completed: " uic_wcstr "\n", argv[1]);

    string output_path;
    if(argc >= 3)   /* 1st if cmd line has specified the dest path, override always. */
        output_path.assign(argv[2]);
    else {
        /* If the ui file specified the dest path, then take it. */
        auto f = src.unique_path_locater(_t("output"));
        if(f.is_valid()) {
            auto fv = f.child();
            if(fv && fv->is_value())
                output_path = fv->get_name();
        }
    }
    /* try to get name from src path */
    if(output_path.empty()) {
        /* strip name from src path */
        string src_path(argv[1]);
        size_t strip_begin, strip_end;
        strip_begin = src_path.find_last_of(_t("\\/"));
        if(strip_begin == string::npos)
            strip_begin = 0;
        else {
            strip_begin ++;
            if(strip_begin == src_path.size())  /* src path was finished by /, unexpected. */
                strip_begin = string::npos;
        }
        /* then find the strip end */
        if(strip_begin != string::npos) {
            strip_end = src_path.find_last_of(_t('.'));
            if(strip_end == string::npos || strip_end < strip_begin)
                strip_end = src_path.size();
            if(strip_end != string::npos && strip_begin < strip_end) {
                output_path.assign(src_path, strip_begin, strip_end - strip_begin);
                output_path.append(_t(".hpp"));     /* add postfix */
            }
        }
    }
    /* simply give a default name */
    if(output_path.empty())
        output_path.assign(_t("uic_output.hpp"));

    ui_editor::ui_tree uitree;
    ui_editor::ui_editor_context ctx;
    if(!ui_editor::ui_convert_source_to_core(ctx, uitree, src)) {
        printf("Convert source to core failed.\n");
        print_error();
        return -1;
    }

    string str;
    if(!ui_editor::ui_compile(str, ctx, uitree)) {
        printf("Generate ui code failed.\n");
        print_error();
        return -1;
    }

    {
        file savefile(output_path.c_str(), _t("w,ccs=UNICODE"));
        savefile.write(str);
    }

    printf("Generate ui code completed: " uic_wcstr ".\n", output_path.c_str());

    return 0;
}