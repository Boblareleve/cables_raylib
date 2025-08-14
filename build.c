#define STRING_IMPLEMENTATION
#define BUILD_IMPLEMENTATION
#define PRINT_IMPLEMENTATION
#include "build.h"
#include "sa.h"

#include <string.h>
#include <stdlib.h>


#define SRC_DIR "./src/"
#define ENGIN_DIR "./engin/"
#define DOT_O_DIR "./object"


SA_TYPEDEF_ARRAY(Strv);
typedef struct Arg_Shell_List
{
    enum Target {
        none = 0,
        debug,
        gdb,
        release,
    } target;
    enum Scoop {
        game,
        engin
    } scoop;
    enum Mode {
        all,
        inc,
        once,
    } mode;
    enum Platform {
        platform_windows,
        platform_linux,
    } platform;

    da_Strv comp_flags;
    da_Strv link_flags;
    da_Strv source_files;

    Strv compiler;
    Strv bin_name;

    int64_t proc_max;
} Arg_Shell_List;
Arg_Shell_List arg_parse(const sa_Strv *args)
{
    Arg_Shell_List res = {
        .mode =   once,
        .target = debug,
        .scoop =  game,
        .platform = platform_linux,
        .compiler = Strv_cstr("cc"),
        .bin_name = Strv_cstr("cables")
    };
    for (int i = 1; i < args->size; i++)
    {
        Strv arg = sa_get(args, i);
        if (Str_start_with_cstr(arg, "-j"))
        {
            if (arg.size < 3)
                printf("[WARNING] missing number of proc max fallback to mono proc\n");
            else res.proc_max = Str_atoll(Strv_stride(arg, 2), NULL);
        }
        else if (Str_start_with_cstr(arg, "-t")
              || Str_start_with_cstr(arg, "-target")
        ) {
            if (args->size <= ++i)
            {
                printf("[ERROR] missing target name\n");
                exit(1);   
            }
            arg = args->arr[i];
            
            if (Str_start_with_cstr(arg, "debug"))
                res.target = debug;
            else if (Str_start_with_cstr(arg, "release"))
                res.target = release;
            else if (Str_start_with_cstr(arg, "gdb"))
                res.target = gdb;
            else printf("[WARNING] unreconize target fallback to default\n");
        }
        else if (Str_start_with_cstr(arg, "-m")
              || Str_start_with_cstr(arg, "-mode")
        ) {
            if (args->size <= ++i)
            {
                printf("[ERROR] missing mode\n");
                exit(1);   
            }
            arg = args->arr[i];
            
            if (Str_start_with_cstr(arg, "inc"))
                res.mode = inc;
            else if (Str_start_with_cstr(arg, "all"))
                res.mode = all;
            else if (Str_start_with_cstr(arg, "once"))
                res.mode = once;
            else printf("[WARNING] unreconize target fallback to default\n");
        }
        else if (Str_start_with_cstr(arg, "-s")
              || Str_start_with_cstr(arg, "-scoop")
        ) {
            if (args->size <= ++i)
            {
                printf("[ERROR] expected project scoop\n");
                exit(1);   
            }
            arg = args->arr[i];
            
            if (Str_start_with_cstr(arg, "game"))
                res.scoop = game;
            else if (Str_start_with_cstr(arg, "engin"))
                res.scoop = engin;
            else printf("[WARNING] unreconized scoop fallback to default\n");
        }
        else if (Str_start_with_cstr(arg, "-p")
              || Str_start_with_cstr(arg, "-platform")
        ) {
            if (args->size <= ++i)
            {
                printf("[ERROR] expected platform name\n");
                exit(1);
            }
            arg = args->arr[i];

            if (Str_start_with_cstr(arg, "linux"))
                res.platform = platform_linux;
            else if (Str_start_with_cstr(arg, "windows"))
                res.platform = platform_windows;
            else printf("[WARNING] unreconized platform fallback to default\n");
        }
        else if (Str_start_with_cstr(arg, "-D"))
            da_push(&res.comp_flags, arg);
        else if (Str_start_with_cstr(arg, "-o"))
        {
            if (args->size <= ++i)
            {
                printf("[ERROR] missing binary name\n");
                exit(1);
            }
            res.bin_name = args->arr[i];
        }
        else if (Str_equal(arg, Strv_cstr("-help"))
              || Str_equal(arg, Strv_cstr("-h")))
        {
            printf("arg list\n");
            printf("    -j<number>                   how many thread to use (a positive number 0 fall back to 1)\n");
            printf("    -o <name>                    name of the binary (name: a string)\n");
            printf("    (-m)|(-mode) <mode>          how to build (\"inc\", \"once\", \"all\")\n");
            printf("    (-t)|(-target) <target>      what config to use (\"debug\", \"release\", \"gdb\")\n");
            printf("    (-s)|(-scoop) <scoop>        what part of the project to build (\"engin\", \"game\")\n");
            printf("    (-p)|(-platform) <platform>  what platform to build for (\"linux\", \"windows\")\n");
        }
        else
        {
            printf("[ERROR] unreconize argument -> \"");
            Str_print(args->arr[i]);
            printf("\"\n");
            exit(1);
        }
    }

    { // general flags
        Strb build_path = Strb_cstr("-I");
        Strb_cat(&build_path, getenv("HOME"));
        Strb_cat(&build_path, "/C/my_lib");
        Strb_cat_null(&build_path);
        
        da_push_many(&res.comp_flags,
            Strv_cstr("-I./includes"),
            build_path.self,
            Strv_cstr("-Wall"),
            Strv_cstr("-Wextra"),
            Strv_cstr("-Wno-missing-braces")
        );
        
        // da_print(&res.comp_flags, Str_print);
        // printf("|\n\n");
        
        da_push_many(&res.link_flags, 
            Strv_cstr("-L./lib"), 
            Strv_cstr("-lm")
        );
    }
    { // mode
        if (res.mode == all)
            da_push(&res.comp_flags, Strv_cstr("-c"));
        else if (res.mode == inc)
            TODO("impl inc ");
        else if (res.mode == once) {}
        else UNREACHABLE("invalid mode");
    }
    { // target
        if (res.target == debug)
        {
            da_push_many(&res.comp_flags,
                Strv_cstr("-fsanitize=address,"/* undefined, */"leak"), 
                Strv_cstr("-g3"),
                Strv_cstr("-gdwarf-2"),
                Strv_cstr("-DDEBUG")
            );
        }
        else if (res.target == release)
        {
            da_push_many(&res.comp_flags,
                Strv_cstr("-O2"), 
                Strv_cstr("-DNDEBUG")
            );
            if (res.mode == once)
                da_push(&res.link_flags, Strv_cstr("-flto"));
        }
        else if (res.target == gdb)
        {
            da_push_many(&res.comp_flags,
                Strv_cstr("-ggdb"),
                Strv_cstr("-DDEBUG")
            );
        }
        else UNREACHABLE("invalid target");
    }
    { // scoop
        if (res.scoop == game) 
        {
            da_push_many(&res.source_files,
                Strv_cstr(ENGIN_DIR"cell_auto.c"),
                Strv_cstr(ENGIN_DIR"editor.c"),
                Strv_cstr(SRC_DIR"main.c"),
                Strv_cstr(SRC_DIR"lib_impl.c"),
                Strv_cstr(SRC_DIR"draw.c"),
                Strv_cstr(SRC_DIR"button.c"),
                Strv_cstr(SRC_DIR"ui_input.c")
            );

            da_push(&res.link_flags, 
                (res.target == debug || res.target == gdb)
                    ? Strv_cstr("-l:libraylib_debug.a")
                    : Strv_cstr("-l:libraylib.a")
            );
        }
        else if (res.scoop == engin) 
        {
            da_push(&res.source_files, Strv_cstr(ENGIN_DIR"cell_auto.c" ));
            da_push(&res.source_files, Strv_cstr(ENGIN_DIR"editor.c"    ));
            da_push(&res.source_files, Strv_cstr(ENGIN_DIR"test_main.c" ));
        }
        else UNREACHABLE("invalid scoop");
    }
    { // platform
        if (res.platform == platform_linux) {}
        else if (res.platform == platform_windows)
            TODO("windows platform");
        else UNREACHABLE("invalid platform");
    }
    return res;
} 


sa_Strv *argcv_strv(int argc, char **argv)
{
    sa_Strv *res = sa_make(res, argc);
    for (int i = 0; i < argc; i++)
        res->arr[i] = Strv_cstr(argv[i]);

    return res;
}
int main(int argc, char **argv)
{
    { // go_rebuild urself
        da_Strb rebuild_flags = {0};
        da_push(&rebuild_flags, (Strb){0});
        if (!Strb_catf(&da_top(&rebuild_flags), "-I%s/C/my_lib", getenv("HOME"))) return 1;
        da_push(&rebuild_flags, (Strb){0});
        if (!Strb_cat(&da_top(&rebuild_flags), "-Wall"))         return 1;
        da_push(&rebuild_flags, (Strb){0});
        if (!Strb_cat(&da_top(&rebuild_flags), "-Wextra"))       return 1;
        da_push(&rebuild_flags, (Strb){0});
        if (!Strb_cat(&da_top(&rebuild_flags), "-g3"))           return 1;
        if (!BUILD_GO_REBUILD_URSELF(argc, argv, rebuild_flags)) return 1;
        da_free_func(&rebuild_flags, Strb_free);
    }

    // dddddddddddddddddddddddddddddd

    sa_Strv *args = argcv_strv(argc, argv);
    if (!args) return 1;

    Arg_Shell_List parameters = arg_parse(args);
    
    // da_print(&parameters.comp_flags, Str_print);

    if (parameters.mode == once)
    {
        Cmd cmd = {0};
        da_push(&cmd, parameters.compiler);
        
        foreach_ptr (Strv, file, &parameters.source_files)
            da_push(&cmd, *file);
        
        da_push(&cmd, Strv_cstr("-o"));
        da_push(&cmd, parameters.bin_name);

        foreach_ptr (Strv, flag, &parameters.comp_flags)
            da_push(&cmd, *flag);
        foreach_ptr (Strv, flag, &parameters.link_flags)
            da_push(&cmd, *flag);

        if (!Cmd_run_wait_reset(&cmd)) return 1;
    }
    else if (parameters.mode == inc)
    {
        TODO("do inc");
    }
    else if (parameters.mode == all)
    {
        TODO("do all");
    }
    else UNREACHABLE("parse ?");

    sa_free(args);
    return 0;
}
