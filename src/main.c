#define STB_IMAGE_IMPLEMENTATION
#define MAGICIAN_IMPLEMENTATION

#include <errno.h>
#include <stdio.h>

#include "stb_image.h"
#include "magician.h"
#include "helper.h"
#include "config.h"
#include "version.h"

#define MIN_ARGS 3
#define DEFAULT_SIZE 512

int main(int argc, char **argv) {
    /* -- Opening -- */
    if (argc == 2) {
        if (argv[1][0] == '-') {
            switch (argv[1][1]) {
                case 'h':
                    printf("%s [infile] [outfile]\n", argv[0]);
                    break;
                case 'v':
                    printf("%s %d.%d.%d\n", argv[0], VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
                    break;
                default:
                    fprintf(stderr, "ERROR: Not a valid argument!\n");
                    break;
            }
        }
    }
    if (argc < MIN_ARGS) {
        fprintf(stderr, "ERROR: Not enought argument!\n");
        return 1;
    }

    FILE *in_file = fopen(argv[1], "rb");
    if (!in_file) {
        fprintf(stderr, "ERROR: Failed to open the file: %s\n", strerror(errno));
        return 1;
    }

    if (!is_png(in_file) && !is_jpeg(in_file)) {
        fprintf(stderr, "ERROR: File `%s` is not a png or jpeg file!\n", argv[1]);
        return 1;
    }

    int width, height, n;
    unsigned char *image = stbi_load_from_file(in_file, &width, &height, &n, 4);
    if (!image) {
        fprintf(stderr, "ERROR: Failed to parse file `%s`: %s\n", argv[1], stbi_failure_reason());
        return 1;
    }

    /* Don't need it anymore goodbye! */
    fclose(in_file);

    /* -- Work -- */

    pair_t most_used    = {0};
    pair_t second_used  = {0};

    uint16_t *freq = calloc(0x1000000, sizeof(uint16_t));

    size_t color_count = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * n;

            uint8_t r = image[index + 0];
            uint8_t g = image[index + 1];
            uint8_t b = image[index + 2];

            rgb_t pixel = (r << 16) | (g << 8) | b;

            hsv_t hsv = rgb_to_hsv(pixel);

            uint32_t count = ++freq[pixel];
            if (count == 1) ++color_count;

            if (!monochrome) {
                if (hsv.v < min_lightness || hsv.v > max_lightness ||
                        hsv.s < min_saturation || hsv.s > max_saturation) {
                    continue;
                }
            }

            if (count > most_used.second) {
                most_used.first = pixel;
                most_used.second = count;
            }

            if (!monochrome && count < most_used.second && count > second_used.second) {
                hsv_t first_hsv = rgb_to_hsv(most_used.first);

                // Compute circular hue distance
                float hue_dist = fabs(hsv.h - first_hsv.h);
                if (hue_dist > 0.5f) hue_dist = 1.0f - hue_dist;

                if (hue_dist >= second_color_hue_diff) {
                    second_used.first = pixel;
                    second_used.second = count;
                }
            }
        }
    }

    /* Don't need it anymore goodbye! */
    stbi_image_free(image);
    free(freq);

    /* Generate the color */

    /* List of base16:
     * 0 : Black
     * 1 : Dark Blue
     * 2 : Dark Green
     * 3 : Dark Cyan
     * 4 : Dark Red
     * 5 : Dark Magenta
     * 6 : Orange
     * 7 : Light Gray
     * 8 : Dark Gray
     * 9 : Light Blue
     * 10: Light Green
     * 11: Light Cyan
     * 12: Light Red
     * 13: Light Magenta
     * 14: Yellow
     * 15: White
     */

    rgb_t palette[18] = {0}; /* 16 (+2 of slighly more black and white) */

    hsv_t first_accent_hsv = rgb_to_hsv(most_used.first);
    hsv_t second_accent_hsv = rgb_to_hsv(second_used.first);

    /* Bg Color */
    double invert = dark_mode ? 1.0 : -1.0;
    double offset = dark_mode ? 0.0 : 1.0;

    hsv_t bg = {
        .h = first_accent_hsv.h,
        .s = first_accent_hsv.s,
        .v = offset + invert * bg_color_value
    };
    palette[0] = hsv_to_rgb(bg);

    hsv_t bg_alt = {
        .h = first_accent_hsv.h,
        .s = first_accent_hsv.s,
        .v = offset + invert * (bg_color_value + bg_color_value_alt_diff)
    };
    palette[8] = hsv_to_rgb(bg_alt);

    hsv_t bg_alt_2 = {
        .h = first_accent_hsv.h,
        .s = first_accent_hsv.s,
        .v = offset + invert * (bg_color_value + (bg_color_value_alt_diff * 2))
    };
    palette[16] = hsv_to_rgb(bg_alt_2);

    /* Fg Color */
    hsv_t fg = { bg.h,     0.15, 1.0f - bg.v};
    palette[15] = hsv_to_rgb(fg);

    float shift_by = bg_alt.v / 16.0f;
    shift_by *= (bg_alt.v >= 0.5) ? 1 : -1;

    hsv_t fg_alt = { bg_alt.h, 0.25, 0.5f + shift_by};
    palette[7]  = hsv_to_rgb(fg_alt);

    hsv_t fg_alt_2 = { bg_alt.h, 0.25, 0.5f + (shift_by * 2)};
    palette[17]  = hsv_to_rgb(fg_alt_2);

    bool swapped = false;
    if (fabs(first_accent_hsv.v - bg.v) <= bg_min_value_diff) {
        hsv_t tmp = first_accent_hsv;
        first_accent_hsv = second_accent_hsv;
        second_accent_hsv = tmp;
        swapped = true;
    }

    if (!monochrome) {
        if (fabs(first_accent_hsv.v - bg.v) <= bg_min_value_diff) {
            if (swapped) {
                hsv_t tmp = first_accent_hsv;
                first_accent_hsv = second_accent_hsv;
                second_accent_hsv = tmp;
            }
            if (dark_mode) {
                bg.v     = clamp(bg.v     - (bg_min_value_diff / 5.2f), 0.07, 1.0);
                bg_alt.v = clamp(bg_alt.v - (bg_min_value_diff / 5.2f), 0.07, 1.0);
            } else {
                bg.v     = clamp(bg.v     + (bg_min_value_diff / 5.2f), 0.07, 1.0);
                bg_alt.v = clamp(bg_alt.v + (bg_min_value_diff / 5.2f), 0.07, 1.0);
            }
            palette[0] = hsv_to_rgb(bg);
            palette[8] = hsv_to_rgb(bg_alt);
        }

        /* Accent Color */
        uint8_t a, b = 0;
        color_enum_to_mapping(tell_color(first_accent_hsv), &a, &b);
        palette[a] = hsv_to_rgb(first_accent_hsv);
        palette[b] = hsv_to_rgb((hsv_t) {first_accent_hsv.h, first_accent_hsv.s, first_accent_hsv.v - 0.1});

        a = b = 0;
        color_enum_to_mapping(tell_color(second_accent_hsv), &a, &b);
        palette[a] = hsv_to_rgb(second_accent_hsv);
        palette[b] = hsv_to_rgb((hsv_t) {second_accent_hsv.h, second_accent_hsv.s, second_accent_hsv.v - 0.1});

        /* Others Color */
        color_e first_accent_color = tell_color(first_accent_hsv);
        float base_first_hue = get_base_hue(first_accent_color);

        float hue_diff = first_accent_hsv.h - base_first_hue;

        if (hue_diff > 0.5f) hue_diff -= 1.0f;
        else if (hue_diff < -0.5f) hue_diff += 1.0f;

        for(int i=0; i<16; i++) {
            if(palette[i] != 0) continue;

            color_e color_enum = mapping_to_color_enum(i);
            if (color_enum == SHADE) continue;

            float base_hue = get_base_hue(color_enum);
            float adjusted_hue =
                clamp(base_hue + hue_diff, base_hue - (color_hue_range * 0.5f), base_hue + (color_hue_range * 0.5f));
            if (adjusted_hue > 1.0f) adjusted_hue -= 1.0f;
            else if (adjusted_hue < 0.0f) adjusted_hue += 1.0f;

            hsv_t color_hsv = {
                .h = adjusted_hue,
                .s = first_accent_hsv.s,
                .v = first_accent_hsv.v
            };

            uint8_t a = 0, b = 0;
            color_enum_to_mapping(color_enum, &a, &b);

            palette[a] = hsv_to_rgb(color_hsv);

            hsv_t darker = {color_hsv.h, color_hsv.s, color_hsv.v - 0.1f};
            palette[b] = hsv_to_rgb(darker);
        }
    } else {
        /* TODO: Monochrome is not yet implemented! */
        fprintf(stderr, "TODO: Not yet implemented!");
        assert(false);
    }

    /* -- Output the file -- */
    FILE *out_file = fopen(argv[2], "w");
    if (!out_file) {
        fprintf(stderr, "ERROR: Failed to open the file: %s\n", strerror(errno));
        return 1;
    }
    fprintf(out_file, "return {\n");
    for(int i=0; i<18; i++) {
        fprintf(out_file, "\tcolor%.2d = 0x%x,\n", i, palette[i]);
    }
    fprintf(out_file, "\taccent1 = 0x%x,\n", most_used.first);
    fprintf(out_file, "\taccent2 = 0x%x\n", second_used.first);
    fprintf(out_file, "}\n");

    fclose(out_file);
    return 0;
}
