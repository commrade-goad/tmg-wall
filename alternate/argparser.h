#ifndef ARGPARSER_H
#define ARGPARSER_H

#include "stdbool.h"

typedef struct {
    char *infile;
    char *outfile;
    bool exit;
    bool colorful_mode;
} Args;

Args init_args(int argc, char **argv);
void deinit_args(Args *args);

#endif /* ARGPARSER_H */
