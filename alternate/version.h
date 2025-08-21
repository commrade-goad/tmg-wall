#ifndef VERSION_H
#define VERSION_H

#include <stdio.h>

#define NAME "tmg-wall-alt"
#define VERSION_MAYOR 0
#define VERSION_MINOR 1

static inline void print_version() {
    printf("%s %d.%d", NAME, VERSION_MAYOR, VERSION_MINOR);
}

#endif /* VERSION_H */
