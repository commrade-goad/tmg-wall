#include "argparser.h"
#include "version.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
                    a.exit = true;
                    break;
                case 'c':
                    a.colorful_mode = true;
                    break;
                default:
                    fprintf(stderr, "ERROR: Unknown flags!");
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
