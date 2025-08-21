#ifndef MAGICIAN_H
#define MAGICIAN_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool is_png(FILE *f);
bool is_jpeg(FILE *f);
bool is_bmp(FILE *f);
bool is_gif(FILE *f);

#ifdef MAGICIAN_IMPLEMENTATION

bool readn_and_match(FILE *f, size_t n, const unsigned char *except){
    if (n > 32) return false;
    unsigned char buffer[32];
    size_t readed = fread(buffer, 1, n, f);
    if (readed != n) return false;

    fseek(f, 0, SEEK_SET);
    return memcmp(buffer, except, n) == 0;
}

bool is_png(FILE *f) {
    const unsigned char magic[] = {137,80,78,71,13,10,26,10};
    return readn_and_match(f, sizeof(magic), magic);
}

bool is_jpeg(FILE *f) {
    const unsigned char magic[] = {255, 216, 255};
    return readn_and_match(f, sizeof(magic), magic);
}

bool is_gif(FILE *f) {
    const unsigned char gif87a[] = {71, 73, 70, 56, 55, 97};
    const unsigned char gif89a[] = {71, 73, 70, 56, 57, 97};
    return readn_and_match(f, sizeof(gif87a), gif87a) ||
           readn_and_match(f, sizeof(gif89a), gif89a);
}

#endif /* MAGICIAN_IMPLEMENTATION */
#endif /* MAGICIAN_H */
