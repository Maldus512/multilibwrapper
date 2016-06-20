#define _GNU_SOURCE
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

const char* script_path_installed = "/usr/share/multilibwrapper/Makefile_script";
const char* script_path_dir_installed = "/usr/share/multilibwrapper/";
const char* script_path = "./multilibwrapper/Makefile_script";
const char* script_path_dir = "./multilibwrapper/";
const char* script = "./Makefile_script";

void usage(char *name) {
    printf("%s [OPTION] LIBRARY HEADER [HEADER...]\n", name);
    printf("Options:\n");
    printf("\t-h, --help:\t print help message\n");
    printf("\t-v, --verbose:\t print makefile output\n");
    printf("\t-s, --source-only:\t only generate the source file, without compiling it\n");
    printf("\t-d, --destination:\t specify the destination directory where to put the results\n");
}

int main(int argc, char* argv[], char* envp[]){
    char *libname, *includes, *dir, *prefix, *headers, *dest;
    char **args;
    const char *path, *path_dir;
    size_t i, j;
    int k, nullfd, verbose, optindex, source_only, c;
    extern int optind;
    extern char* optarg;
    struct stat sb;
    prefix = NULL;
    struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"destination", required_argument, 0, 'd'},
        {"source-only", no_argument, 0, 's'},
        {0, 0, 0, 0}
    };
    args = (char**) malloc(sizeof(char*)*8);
    verbose = source_only = 0;
    dest = NULL;

    while ((c = getopt_long(argc, argv, "hvsd:", long_options, &optindex)) != -1) {
        switch (c) {
            case 'h':
                usage(argv[0]);
                exit(0);
                break;
            case 'v':
                printf("verbosity enabled\n");
                verbose = 1;
                break;
            case 's':
                source_only = 1;
                break;
            case 'd':
                asprintf(&dest, "%s", optarg);
                break;
            default:
                break;
        }
    }

    if (argc - optind < 2) {
        usage(argv[0]);
        exit(1);
    }

    if (!verbose) {
        /* redirect output to null*/
        if ((nullfd = open("/dev/null", O_WRONLY)) == -1) {
            perror("Error opening /dev/null:");
            exit(1);
        }
        dup2(nullfd, STDOUT_FILENO);
    }

    if (stat(script_path_dir_installed, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        path = script_path_installed;
        path_dir = script_path_dir_installed;
    }
    else {
        path = script_path;
        path_dir = script_path_dir;
    }

    if (dest && !(stat(dest, &sb) == 0 && S_ISDIR(sb.st_mode))) {
        fprintf(stderr, "Error: invalid destination directory\n");
        exit(1);
    }

    libname = argv[optind++];
    prefix = (dest == NULL ? get_current_dir_name(): dest);
    chdir(path_dir);

    includes = (char*) malloc(sizeof(char));
    includes[0] = 0;
    headers = (char*) malloc(sizeof(char));
    headers[0] = 0;

    for(k = optind; k < argc; k++) {
        asprintf(&headers, "%s %s", argv[k], headers); 
        if (strstr(includes, argv[k]) == NULL)
            asprintf( &includes, "-I%s %s", dirname(argv[k]), includes);
    }
    optind++;
    k = 0;

    headers[strlen(headers)-1] = 0;
    includes[strlen(includes)-1] = 0;

    asprintf(&args[k++], "%s", path);
    asprintf(&args[k++], "PREFIX=%s", prefix);
    asprintf(&args[k++], "HEADERS=\"%s\"", headers);
    asprintf(&args[k++], "DINLIB=%s", libname);
    asprintf(&args[k++], "INCLUDES=\"%s\"", includes);
    if (!source_only) {
        asprintf(&args[k++], "COMPILE=yes");
    }
    if (verbose) {
        asprintf(&args[k++], "--debug");
    }
    args[k] = NULL;

    int err = execve(script, args, envp);
    perror("execve:");
    return err;
}
