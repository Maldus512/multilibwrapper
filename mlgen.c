#define _GNU_SOURCE
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

const char* script_path_installed = "/usr/share/multilibwrapper/Makefile";
const char* script_path_dir_installed = "/usr/share/multilibwrapper/";
const char* script_path = "./multilibwrapper/Makefile";
const char* script_path_dir = "./multilibwrapper/";
const char* script = "./Makefile";

void usage(char *name) {
    printf("%s LIBRARY HEADER [HEADER...]\n", name);
}

int main(int argc, char* argv[], char* envp[]){
    char *libname, *includes, *dir, *prefix, *headers;
    char **args;
    const char *path, *path_dir;
    size_t i, j;
    int k;
    struct stat sb;
    prefix = NULL;
    args = (char**) malloc(sizeof(char*)*6);

    if (argc < 2) {
        usage(argv[0]);
        exit(1);
    }

    if (stat(script_path_dir_installed, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        path = script_path_installed;
        path_dir = script_path_dir_installed;
    }
    else {
        path = script_path;
        path_dir = script_path_dir;
    }

    libname = argv[1];
    prefix = get_current_dir_name();
    chdir(path_dir);

    includes = (char*) malloc(sizeof(char));
    includes[0] = 0;
    headers = (char*) malloc(sizeof(char));
    headers[0] = 0;

    for(k = 2; k < argc; k++) {
        asprintf(&headers, "%s %s", argv[k], headers); 
        if (strstr(includes, argv[k]) == NULL)
            asprintf( &includes, "-I%s %s", dirname(argv[k]), includes);
    }

    headers[strlen(headers)-1] = 0;
    includes[strlen(includes)-1] = 0;

    asprintf(&args[0], "%s", path);
    asprintf(&args[1], "PREFIX=%s", prefix);
    asprintf(&args[2], "HEADERS=\"%s\"", headers);
    asprintf(&args[3], "DINLIB=%s", libname);
    asprintf(&args[4], "INCLUDES=\"%s\"", includes);
    args[5] = NULL;

    printf("%s --debug %s %s %s %s\n", args[0], args[1], args[2], args[3], args[4]);
    printf("%s\n", get_current_dir_name());
    int err = execve(script, args, envp);
    perror("execve:");
    return err;
}
