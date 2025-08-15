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
        out.h = ( (g - b) / delta );
    } else if (max == g) {
        out.h = ( (b - r) / delta ) + 2.0f;
    } else {
        out.h = ( (r - g) / delta ) + 4.0f;
    }

    out.h /= 6.0f;
    if (out.h < 0.0f) out.h += 1.0f;

    return out;
}

rgb_t hsv_to_rgb(hsv_t hsv) {
    float h = hsv.h;
    float s = hsv.s;
    float v = hsv.v;

    float r, g, b;

    if (s == 0.0f) {
        // Achromatic (grey)
        r = g = b = v;
    } else {
        h = h * 6.0f;  // sector 0 to 5
        int i = (int)h;
        float f = h - i;
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
        }
    }

    uint8_t R = (uint8_t)(r * 255.0f + 0.5f);
    uint8_t G = (uint8_t)(g * 255.0f + 0.5f);
    uint8_t B = (uint8_t)(b * 255.0f + 0.5f);

    return (R << 16) | (G << 8) | B;
}

float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

color_e tell_color(hsv_t hsv) {
    if (hsv.s < 0.1f || hsv.v < 0.1f) {
        return SHADE;
    }

    float h = hsv.h;

    // The boundary values have been divided by 360.
    // 15/360   = 0.0416f
    // 75/360   = 0.2083f
    // 150/360  = 0.4167f
    // 210/360  = 0.5833f
    // 270/360  = 0.75f
    // 330/360  = 0.9167f
    if (h >= 0.0416f && h < 0.2083f) {
        return ORANGE;
    } else if (h >= 0.2083f && h < 0.4167f) {
        return GREEN;
    } else if (h >= 0.4167f && h < 0.5833f) {
        return CYAN;
    } else if (h >= 0.5833f && h < 0.75f) {
        return BLUE;
    } else if (h >= 0.75f && h < 0.9167f) {
        return MAGENTA;
    } else {
        // This 'else' correctly covers the Red range which wraps around 0.0/1.0
        // (i.e., from 0.9167 to 1.0 AND from 0.0 to 0.0833)
        return RED;
    }
}

const char *color_enum_to_str(color_e color) {
    switch (color) {
        case RED:
            return "red";
        case GREEN:
            return "green";
        case BLUE:
            return "blue";
        case CYAN:
            return "cyan";
        case MAGENTA:
            return "magenta";
        case ORANGE:
            return "orange/yellow";
        case SHADE:
            return "shade of white or black";
        default:
            return "unknown";
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
            *a = 1;
            *b = 9;
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
            return 0.0f;  // midpoint between 330/360 and 15/360, wrapping around 0/360
        case ORANGE:
            return 0.125f;  // midpoint between 15/360 and 75/360
        case GREEN:
            return 0.3125f;  // midpoint between 75/360 and 150/360
        case CYAN:
            return 0.5f;  // midpoint between 150/360 and 210/360
        case BLUE:
            return 0.6667f;  // midpoint between 210/360 and 270/360
        case MAGENTA:
            return 0.8333f;  // midpoint between 270/360 and 330/360
        case SHADE:
            return 0.0f;  // default, but not really relevant for shades
        default:
            return 0.0f;
    }
}
