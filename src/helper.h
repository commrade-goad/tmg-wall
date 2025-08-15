#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>

typedef uint32_t rgb_t;

typedef struct  {
    uint8_t r, g, b;
    uint8_t padding; /* Not used */
} rgbh_t;

typedef struct  {
    float h, s, v;
} hsv_t;

typedef struct {
    uint32_t first;
    uint32_t second;
} pair_t;

typedef enum {
    RED,
    GREEN,
    BLUE,
    CYAN,
    MAGENTA,
    ORANGE,
    SHADE
} color_e;

/*
inline uint32_t pack_color_555(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
}

inline rgb_t unpack_color_555(uint32_t idx) {
    uint8_t r = ((idx >> 10) & 0x1F) << 3;
    uint8_t g = ((idx >> 5)  & 0x1F) << 3;
    uint8_t b = ((idx >> 0)  & 0x1F) << 3;
    return (r << 16) | (g << 8) | b;
}
*/
hsv_t rgb_to_hsv(rgb_t rgb);
rgb_t hsv_to_rgb(hsv_t hsv);

float clamp(float value, float min, float max);
color_e tell_color(hsv_t hsv);
const char *color_enum_to_str(color_e color);
void color_enum_to_mapping(color_e color, uint8_t *a, uint8_t *b);
color_e mapping_to_color_enum(uint8_t a);
float get_base_hue(color_e color);

#endif /* HELPER_H */
