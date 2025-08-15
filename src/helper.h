#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>

typedef uint32_t rgb_t;

typedef struct  {
    uint8_t r, g, b;
} rgbh_t;

typedef struct {
    uint32_t first;
    uint32_t second;
} Pair;

inline uint32_t pack_color_555(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
}

inline rgb_t unpack_color_555(uint32_t idx) {
    uint8_t r = ((idx >> 10) & 0x1F) << 3;
    uint8_t g = ((idx >> 5)  & 0x1F) << 3;
    uint8_t b = ((idx >> 0)  & 0x1F) << 3;
    return (r << 16) | (g << 8) | b;
}

#endif /* HELPER_H */
