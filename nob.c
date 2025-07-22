#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#include <string.h>
#include <stdlib.h>

Cmd cmd = {0};

#define memlitcmp(str, strlit) memcmp(str, strlit, sizeof(""strlit"") - 1)


// #define COMPILER "clang"
#define COMPILER "gcc"



#define SRC_DIR "./src/"
#define ENGIN_DIR "./engin/"
#define DOT_O_DIR "./object"

#define SHARED_FLAGS "-I./includes",\
                     "-I/home/augustin/C/my_lib",\
                     "-Wall",\
                     "-Wextra",\
                     "-std=c2x"
                     //"-pedantic"
#define GENERAL_FLAGS "-Wall", "-Wno-gnu-binary-literal", "-Wno-missing-braces"
#define LINK_FLAGS "-L./lib", "-lm"



const char *source_files[] = {
    ENGIN_DIR"cell_auto.c",
    ENGIN_DIR"editor.c",
    SRC_DIR"main.c",
};
const char *engin_source_files[] = {
    ENGIN_DIR"cell_auto.c",
    ENGIN_DIR"editor.c",
    ENGIN_DIR"test_main.c",
};

char *dir_name_ext(const String_View file_name, const String_View dir, const String_View ext)
{
    assert(ext.data[0] == '.');
    
    char buffer[1024] = {0};
    strncat(buffer, dir.data, dir.count);
    strcat(buffer, "/");
    strncat(buffer, file_name.data, file_name.count);
    strncat(buffer, ext.data, ext.count);

    return strdup(buffer);
}

char *get_name_cxx_to_o(const char *file_cxx)
{
    String_View file_name_c = sv_from_cstr(path_name(file_cxx));
    String_View file_name = sv_chop_by_delim(&file_name_c, '.');
    
    return dir_name_ext(
        file_name,
        sv_from_cstr(DOT_O_DIR), 
        sv_from_cstr(".o")
    );
}


typedef struct Arg_Shell_List
{
    enum Target {
        none = 0,
        debug,
        gdb,
        release,
        engin_debug,
        engin_release,
    } target;
    Cmd additionnal_compilation_flags;
    Cmd additionnal_link_flags;
    int thread_count;
} Arg_Shell_List;
Arg_Shell_List arg_parse(int argc, char **argv)
{
    Arg_Shell_List res = {0};

    for (int i = 1; i < argc; i++)
    {
        if (0 == memcmp(argv[i], "-j", 2))
            res.thread_count = atoi(&argv[i][2]);
        else if (!memlitcmp(argv[i], "debug"))
        {
            cmd_append(&res.additionnal_compilation_flags, "-fsanitize=address");
            cmd_append(&res.additionnal_compilation_flags, "-fsanitize=undefined");
            cmd_append(&res.additionnal_compilation_flags, "-g3");
            cmd_append(&res.additionnal_compilation_flags, "-DDEBUG");
            cmd_append(&res.additionnal_link_flags, "-l:libraylib_debug.a");
            
            res.target = debug;
        }
        else if (!memlitcmp(argv[i], "release"))
        {
            cmd_append(&res.additionnal_compilation_flags, "-O2");
            cmd_append(&res.additionnal_link_flags, "-l:libraylib.a");
            // cmd_append(&res.additionnal_link_flags, "-l:libraylib_debug.a");
            res.target = release;
        }
        else if (!memlitcmp(argv[i], "gdb"))
        {
            cmd_append(&res.additionnal_compilation_flags, "-DDEBUG");
            cmd_append(&res.additionnal_compilation_flags, "-ggdb");
            cmd_append(&res.additionnal_link_flags, "-l:libraylib_debug.a");
            res.target = gdb;
        }
        else if (!memlitcmp(argv[i], "engin_debug"))
        {
            cmd_append(&res.additionnal_compilation_flags, "-fsanitize=address");
            cmd_append(&res.additionnal_compilation_flags, "-fsanitize=undefined");
            cmd_append(&res.additionnal_compilation_flags, "-g3");
            cmd_append(&res.additionnal_compilation_flags, "-DDEBUG");
            
            res.target = engin_debug;
        }
        else if (!memlitcmp(argv[i], "engin_release"))
        {
            cmd_append(&res.additionnal_compilation_flags, "-O2");

            res.target = engin_release;
        }
        else if (strlen(argv[i]) >= 2 && 0 == memcmp(argv[i], "-D", 2))
            cmd_append(&res.additionnal_compilation_flags, argv[i]);
        else
        {
            printf("unreconize argument -> %s\n", argv[i]);
            exit(2);
        }
    }
    if (res.target == none)
        res.target = debug;
    
    return (res);
} 


// build .o
void make_o_files(const char **source_files, int size, const Arg_Shell_List *arg)
{ 
    Procs procs_ids = {0};
    for (int i = 0; i < size; i++)
    {
        // make "object/*.o" string
        char *o_file_path = get_name_cxx_to_o(source_files[i]);

        // rm old one
        {
            Cmd cmd1 = {0};
            cmd_append(&cmd1, "rm", o_file_path);
            cmd_run_sync_and_reset(&cmd1);
        }

        // add async cmd
        cmd_append(&cmd, COMPILER);

        String_Builder path_home = {0};
        /* { // -I$HOME/...
            sb_append_cstr(&path_home, "-I");
            const char *get_path_home = getenv("$HOME"+1);
            if (get_path_home)
                sb_append_cstr(&path_home, get_path_home);
            else assert(!"can't found $HOME env variable"[0]);
            sb_append_cstr(&path_home, ADDITIONAL_HEADER_DIR);
            sb_append_null(&path_home);
        } */
        // cmd_append(&cmd, path_home.items);

        da_foreach (const char*, str_flag, &arg->additionnal_compilation_flags)
            cmd_append(&cmd, *str_flag);
        cmd_append(&cmd, SHARED_FLAGS, GENERAL_FLAGS, "-c", source_files[i], "-o", o_file_path);
        
        Proc tmp = arg->thread_count ? cmd_run_async_and_reset(&cmd) 
                                     : cmd_run_sync_and_reset(&cmd);
        if (tmp == INVALID_PROC)
            exit(1);
        
        if (arg->thread_count)
            procs_append_with_flush(&procs_ids, tmp, arg->thread_count); // wait if more than max_count
    }
    // wait
    if (arg->thread_count) da_foreach (Proc, pc, &procs_ids)
        proc_wait(*pc);
}


void link_o_files(const char **source_files, int size, const Arg_Shell_List *arg, const char *binaray_name)
{ // link
    cmd_append(&cmd, COMPILER);
    for (int i = 0; i < size; i++)
    {
        // make "object/*.o" string
        const char *file_name_c = path_name(source_files[i]);
        char *o_file = malloc(1 + strlen(file_name_c) + strlen(DOT_O_DIR));
        strcpy(o_file, DOT_O_DIR);
        strcat(o_file, "/");
        strcat(o_file, file_name_c);

        
        int o_file_size = strlen(o_file);
        if (o_file[o_file_size-1] == 'p')
        { // .cpp
            o_file[o_file_size-3] = 'o';
            o_file[o_file_size-2] = '\0';
            o_file_size -= 2;
        }
        else
        { // .c
            o_file[o_file_size-1] = 'o';
            o_file[o_file_size] = '\0';
        }

        // printf("c file #%s#\n", o_file);
        cmd_append(&cmd, o_file); // add files before link flags
    }
    da_foreach (const char*, str_flag, &arg->additionnal_compilation_flags)
        cmd_append(&cmd, *str_flag);
    da_foreach (const char*, str_flag, &arg->additionnal_link_flags)
        cmd_append(&cmd, *str_flag);
    cmd_append(&cmd, SHARED_FLAGS, LINK_FLAGS, "-o", binaray_name);
    if (!cmd_run_sync_and_reset(&cmd))
        exit(1);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);


    Arg_Shell_List arg = arg_parse(argc, argv);
    
    
    if (argc >= 2 && (arg.target == debug 
                   || arg.target == release 
                   || arg.target == gdb)
    ) {
        mkdir_if_not_exists(DOT_O_DIR);
        mkdir_if_not_exists(SRC_DIR);
        mkdir_if_not_exists(ENGIN_DIR);

        make_o_files(source_files, ARRAY_LEN(source_files), &arg);
        link_o_files(
            source_files,
            ARRAY_LEN(source_files), 
            &arg,
            arg.target == debug ? "debug"
                        : gdb   ? "gdb"
                        :/* _  ?*/"release"
        );
    }
    else if (argc >= 2 && (arg.target == engin_debug || arg.target == engin_release))
    {
        mkdir_if_not_exists(DOT_O_DIR);
        mkdir_if_not_exists(SRC_DIR);
        mkdir_if_not_exists(ENGIN_DIR);

        make_o_files(engin_source_files, ARRAY_LEN(engin_source_files), &arg);
        link_o_files(
            engin_source_files, 
            ARRAY_LEN(engin_source_files),
            &arg,
            arg.target == engin_debug ? "engin_debug" 
                                      : "engin_release"
        );
    }
    else printf("done nothing");

    printf("\n");
    return 0;
}