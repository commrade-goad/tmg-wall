#include "argparser.h"
#include "version.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static inline void print_help(const char *name) {
    printf("%s [infile] [outfile] <flags>\n", name);
    printf("<flags> :\n");
    printf("   -c : use more popping color.\n");
    printf("   -h : print this help.\n");
    printf("   -v : print version.\n");
}

Args init_args(int argc, char **argv) {
    Args a = {0};
    bool is_flag = false;
    int i = 1;
    while (i < argc) {
        char *current = argv[i];
        if (*current == '-' && !is_flag) is_flag = true;
        if (is_flag) {
            switch (current[1]) {
                case 'v':
                    print_version();
                    a.exit = true;
                    break;
                case 'h':
                    print_help(argv[0]);
                    a.exit = true;
                    break;
                case 'c':
                    a.colorful_mode = true;
                    break;
                default:
                    fprintf(stderr, "ERROR: Unknown flags `%s`!\n", current);
                    a.exit = true;
                    break;
            }
        } else {
            switch (i) {
                case 1:
                    a.infile = strdup(current);
                    break;
                case 2:
                    a.outfile = strdup(current);
                    break;
                default:
                    break;
            }

        }
        i++;
    }
    return a;
}

void deinit_args(Args *args) {
    if (args->infile) free(args->infile);
    if (args->outfile) free(args->outfile);
}
