#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>

typedef uint32_t rgb_t;

typedef struct  {
    float h, s, v;
} hsv_t;

hsv_t rgb_to_hsv(rgb_t rgb);
rgb_t hsv_to_rgb(hsv_t hsv);

#endif /* HELPER_H */
