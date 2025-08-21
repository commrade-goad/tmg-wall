#ifndef VERSION_H
#define VERSION_H

#include <stdio.h>

#define NAME "tmg-wall-alt"
#define VERSION_MAYOR 1
#define VERSION_MINOR 0

static inline void print_version() {
    printf("%s %d.%d\n", NAME, VERSION_MAYOR, VERSION_MINOR);
}

#endif /* VERSION_H */
