#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>

typedef uint32_t rgb_t;

typedef struct  {
    float h, s, v;
} hsv_t;

typedef enum {
    RED,
    GREEN,
    BLUE,
    CYAN,
    MAGENTA,
    ORANGE,
    SHADE
} color_e;

hsv_t rgb_to_hsv(rgb_t rgb);
rgb_t hsv_to_rgb(hsv_t hsv);
color_e tell_color(hsv_t hsv);
void color_enum_to_mapping(color_e color, uint8_t *a, uint8_t *b);
color_e mapping_to_color_enum(uint8_t a);
float get_base_hue(color_e color);

#endif /* HELPER_H */
