#include <algorithm>
#include <math.h>

#include "helper.h"

hsv_t rgb_to_hsv(rgb_t rgb) {
    float r = ((rgb >> 16) & 0xFF) / 255.0f;
    float g = ((rgb >> 8)  & 0xFF) / 255.0f;
    float b = ( rgb        & 0xFF) / 255.0f;

    float max = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
    float min = (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);
    float delta = max - min;

    hsv_t out;
    out.v = max;

    // Saturation
    out.s = (max == 0.0f) ? 0.0f : (delta / max);

    // Hue
    if (delta == 0.0f) {
        out.h = 0.0f; // undefined hue
    } else if (max == r) {
        out.h = 60.0f * ((g - b) / delta);
    } else if (max == g) {
        out.h = 60.0f * ((b - r) / delta + 2.0f);
    } else { // max == b
        out.h = 60.0f * ((r - g) / delta + 4.0f);
    }

    // Ensure h is in range 0..360 with automatic wrapping
    out.h = fmodf(out.h, 360.0f);
    if (out.h < 0.0f) out.h += 360.0f;

    // Ensure s and v are in range 0..1 with automatic wrapping
    out.s = std::clamp(out.s, 0.0f, 1.0f);
    out.v = std::clamp(out.v, 0.0f, 1.0f);

    return out;
}

rgb_t hsv_to_rgb(hsv_t hsv) {
    // Ensure h is in range 0..360 with automatic wrapping
    float h = fmodf(hsv.h, 360.0f);
    if (h < 0.0f) h += 360.0f;

    // Ensure s and v are in range 0..1 with automatic wrapping
    float s = std::clamp(hsv.s, 0.0f, 1.0f);
    float v = std::clamp(hsv.v, 0.0f, 1.0f);

    float r, g, b;

    if (s <= 0.0f) {
        // Achromatic (grey)
        r = g = b = v;
    } else {
        float hh = h / 60.0f;  // sector 0 to 5
        int i = (int)hh;
        float f = hh - i;      // fractional part
        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));

        switch (i % 6) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            case 5: r = v; g = p; b = q; break;
            default: r = g = b = 0.0f; break; // Should never happen
        }
    }

    uint8_t R = (uint8_t)(r * 255.0f + 0.5f);
    uint8_t G = (uint8_t)(g * 255.0f + 0.5f);
    uint8_t B = (uint8_t)(b * 255.0f + 0.5f);

    return (R << 16) | (G << 8) | B;
}

color_e tell_color(hsv_t hsv) {
    float h = hsv.h;
    if (h >= 15.0f && h < 75.0f) {
        return ORANGE;
    } else if (h >= 75.0f && h < 150.0f) {
        return GREEN;
    } else if (h >= 150.0f && h < 210.0f) {
        return CYAN;
    } else if (h >= 210.0f && h < 270.0f) {
        return BLUE;
    } else if (h >= 270.0f && h < 330.0f) {
        return MAGENTA;
    } else {
        return RED;
    }
}

void color_enum_to_mapping(color_e color, uint8_t *a, uint8_t *b) {
    switch (color) {
        case RED:
            *a = 12;
            *b = 4;
            return;
        case GREEN:
            *a = 10;
            *b = 2;
            return;
        case BLUE:
            *a = 9;
            *b = 1;
            return;
        case CYAN:
            *a = 11;
            *b = 3;
            return;
        case MAGENTA:
            *a = 13;
            *b = 5;
            return;
        case ORANGE:
            *a = 14;
            *b = 6;
            return;
        default:
            return;
    }
}

color_e mapping_to_color_enum(uint8_t a) {
    switch (a) {
        case 12:
        case 4:
            return RED;

        case 10:
        case 2:
            return GREEN;

        case 9:
        case 1:
            return BLUE;

        case 11:
        case 3:
            return CYAN;

        case 13:
        case 5:
            return MAGENTA;

        case 14:
        case 6:
            return ORANGE;

        default:
            return SHADE;
    }
}

float get_base_hue(color_e color) {
    switch (color) {
        case RED:
            return 0.0f;
        case ORANGE:
            return 52.5f;
        case GREEN:
            return 120.0f;
        case CYAN:
            return 180.0f;
        case BLUE:
            return 240.0f;
        case MAGENTA:
            return 300.0f;
        default:
            return 0.0f;
    }
}
