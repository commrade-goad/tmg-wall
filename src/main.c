#define STB_IMAGE_IMPLEMENTATION
#define MAGICIAN_IMPLEMENTATION

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "stb_image.h"
#include "magician.h"
#include "helper.h"
#include "config.h"
#include "version.h"

#define MIN_ARGS 3
#define DEFAULT_SIZE 512

/* * Unified Core Generation Function
 * Shared between the standalone executable and the library target.
 */
int libtmg_wall_generate_color(rgb_t *buffer, bool monochrome, bool dark_mode, const char *path)
{
    /* NOTE: make sure buffer is rgb_t[20]! */
    FILE *in_file = fopen(path, "rb");
    if (!in_file) {
        fprintf(stderr, "ERROR: Failed to open the file: %s\n", strerror(errno));
        return 1;
    }

    if (!is_png(in_file) && !is_jpeg(in_file)) {
        fprintf(stderr, "ERROR: File `%s` is not a png or jpeg file!\n", path);
        fclose(in_file);
        return 1;
    }

    int width, height, n;
    unsigned char *image = stbi_load_from_file(in_file, &width, &height, &n, 4);
    if (!image) {
        fprintf(stderr, "ERROR: Failed to parse file `%s`: %s\n", path, stbi_failure_reason());
        fclose(in_file);
        return 1;
    }

    /* Don't need it anymore goodbye! */
    fclose(in_file);

    pair_t most_used    = {0};
    pair_t second_used  = {0};
    pair_t most_used_of_all_dont_care_criteria = {0};

    bool found = false;
    uint16_t *freq = calloc(0x1000000, sizeof(uint16_t));
    if (!freq) {
        fprintf(stderr, "ERROR: Out of memory allocated for frequency table\n");
        stbi_image_free(image);
        return 1;
    }

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

            if (count > most_used_of_all_dont_care_criteria.second) {
                most_used_of_all_dont_care_criteria.first = pixel;
                most_used_of_all_dont_care_criteria.second = count;
            }

            if (hsv.v < (!dark_mode ? clamp(min_lightness + 0.1f, 0.0, 1.0) : min_lightness) ||
                hsv.v > (!dark_mode ? clamp(max_lightness - 0.1f, 0.0, 1.0) : max_lightness) ||
                hsv.s < (!dark_mode ? clamp(min_saturation + 0.1f, 0.0, 1.0) : min_saturation) ||
                hsv.s > (!dark_mode ? clamp(max_saturation - 0.1f, 0.0, 1.0) : max_saturation)) {
                continue;
            }
            if (count > most_used.second) {
                if (!found) found = true;
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

    if (!found && !monochrome) {
        printf("INFO: There is no matching color for the current criteria, activating monochrome mode automatically!\n");
        monochrome = true;
        most_used = most_used_of_all_dont_care_criteria;
    }

/* List of base16:
    * 0 : Black
    * 1 : Dark Blue
    * ...
    * 17: Same as 7 but lot more dark
    * 18-19: for the accent1 and accent2
    */
    memset(buffer, 0, sizeof(rgb_t) * 18);

    if (monochrome) {
        /* Get base color from the most used color */
        hsv_t base_hsv = rgb_to_hsv(most_used.first);
        hsv_t original = base_hsv;
        if (base_hsv.s >= 0.3) base_hsv.s -= base_hsv.s / 3;
        if (base_hsv.v < 0.4) {
            base_hsv.v = 0.41;
            most_used.first = hsv_to_rgb(base_hsv);
        }
        if (base_hsv.v > 0.85) {
            base_hsv.v = 0.84;
            most_used.first = hsv_to_rgb(base_hsv);
        }

        bool is_black_and_white = (original.s <= 0.1) ? true : false;
        double base_sat = 0.0;
        if (!is_black_and_white) {
            base_sat = 0.08 + (original.s / 7.0);
        }

        /* Bg Color */
        double invert = dark_mode ? 1.0 : -1.0;
        double offset = dark_mode ? 0.0 : 1.0;

        hsv_t bg = {
            .h = base_hsv.h,
            .s = base_sat,
            .v = offset + ((invert * original.v) / 7.0f)
        };
        buffer[0] = hsv_to_rgb(bg);

        hsv_t bg_alt = {
            .h = base_hsv.h,
            .s = base_sat,
            .v = bg.v + (invert * bg_color_value_alt_diff * 1.5)
        };
        buffer[8] = hsv_to_rgb(bg_alt);

        hsv_t bg_alt_2 = {
            .h = base_hsv.h,
            .s = base_sat,
            .v = bg_alt.v + invert * (bg_color_value_alt_diff * 1.5)
        };
        buffer[16] = hsv_to_rgb(bg_alt_2);

        /* Fg Color */
        hsv_t fg = { base_hsv.h, base_sat, 1.0f - bg.v };
        buffer[15] = hsv_to_rgb(fg);

        float shift_by = bg_alt.v / 16.0f;
        shift_by *= (bg_alt.v >= 0.5) ? 1 : -1;

        hsv_t fg_alt = { base_hsv.h, base_sat, 0.5f + shift_by };
        buffer[7] = hsv_to_rgb(fg_alt);

        hsv_t fg_alt_2 = { base_hsv.h, base_sat, 0.5f + (shift_by * 2) };
        buffer[17] = hsv_to_rgb(fg_alt_2);

        /* Generate remaining colors with graduated brightness levels */
        for(int i = 1; i < 16; i++) {
            if (buffer[i] != 0) continue;
            if (i == 0 || i == 8 || i == 15 || i == 7) continue;

            float brightness;
            float saturation;

            if (dark_mode) {
                if (i < 8) {
                    brightness = 0.5 + (0.35 * (i - 1) / 6.0);
                } else {
                    brightness = 0.5 + (0.35 * (i - 8) / 6.0);
                }
            } else {
                if (i < 8) {
                    brightness = 0.35 - (0.25 * (i - 1) / 6.0);
                } else {
                    brightness = 0.35 - (0.25 * (i - 8) / 6.0);
                }
            }

            if (dark_mode) brightness = clamp(brightness, 0.5, 0.95);

            if (!is_black_and_white) {
                saturation = base_hsv.s + ((i % 4) * 5e-4);
            } else {
                saturation = 0.0;
            }

            hsv_t color_hsv = {
                .h = base_hsv.h,
                .s = saturation,
                .v = brightness
            };
            buffer[i] = hsv_to_rgb(color_hsv);
        }
        second_used.first = hsv_to_rgb(fg_alt);
    } else {
        hsv_t first_accent_hsv = rgb_to_hsv(most_used.first);
        if (second_used.first == 0 || second_used.second <= 0) second_used = most_used;
        hsv_t second_accent_hsv = rgb_to_hsv(second_used.first);

        /* Bg Color */
        double invert = dark_mode ? 1.0 : -1.0;
        double offset = dark_mode ? 0.0 : 1.0;

        hsv_t bg = {
            .h = first_accent_hsv.h,
            .s = 0.08 + (first_accent_hsv.s / 7.0),
            .v = offset + (invert * first_accent_hsv.v / 7.0f)
        };
        buffer[0] = hsv_to_rgb(bg);

        hsv_t bg_alt = {
            .h = first_accent_hsv.h,
            .s = first_accent_hsv.s,
            .v = bg.v + (invert * bg_color_value_alt_diff)
        };
        buffer[8] = hsv_to_rgb(bg_alt);

        hsv_t bg_alt_2 = {
            .h = first_accent_hsv.h,
            .s = first_accent_hsv.s,
            .v = bg_alt.v + (invert * bg_color_value_alt_diff)
        };
        buffer[16] = hsv_to_rgb(bg_alt_2);

        /* Fg Color */
        hsv_t fg = { bg.h, bg.s, 1.0f - bg.v};
        buffer[15] = hsv_to_rgb(fg);

        float shift_by = bg_alt.v / 16.0f;
        shift_by *= (bg_alt.v >= 0.5) ? 1 : -1;

        hsv_t fg_alt = { bg_alt.h, 0.25, 0.5f + shift_by};
        buffer[7]  = hsv_to_rgb(fg_alt);

        hsv_t fg_alt_2 = { bg_alt.h, 0.25, 0.5f + (shift_by * 2)};
        buffer[17]  = hsv_to_rgb(fg_alt_2);

        bool swapped = false;
        if (fabs(first_accent_hsv.v - bg.v) <= bg_min_value_diff) {
            hsv_t tmp = first_accent_hsv;
            first_accent_hsv = second_accent_hsv;
            second_accent_hsv = tmp;
            swapped = true;
        }

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
            buffer[0] = hsv_to_rgb(bg);
            buffer[8] = hsv_to_rgb(bg_alt);
        }

        /* Accent Color */
        uint8_t a, b = 0;
        color_enum_to_mapping(tell_color(first_accent_hsv), &a, &b);
        buffer[a] = hsv_to_rgb(first_accent_hsv);
        buffer[b] = hsv_to_rgb((hsv_t) {first_accent_hsv.h, first_accent_hsv.s, first_accent_hsv.v - 0.1f});

        a = b = 0;
        color_enum_to_mapping(tell_color(second_accent_hsv), &a, &b);
        buffer[a] = hsv_to_rgb(second_accent_hsv);
        buffer[b] = hsv_to_rgb((hsv_t) {second_accent_hsv.h, second_accent_hsv.s, second_accent_hsv.v - 0.1f});

        /* Others Color */
        color_e first_accent_color = tell_color(first_accent_hsv);
        float base_first_hue = get_base_hue(first_accent_color);
        float hue_diff = first_accent_hsv.h - base_first_hue;

        if (hue_diff > 0.5f) hue_diff -= 1.0f;
        else if (hue_diff < -0.5f) hue_diff += 1.0f;

        for(int i = 0; i < 16; i++) {
            if(buffer[i] != 0) continue;

            color_e color_enum = mapping_to_color_enum(i);
            if (color_enum == SHADE) continue;

            float base_hue = get_base_hue(color_enum);
            float adjusted_hue = clamp(base_hue + hue_diff, base_hue - (color_hue_range * 0.5f), base_hue + (color_hue_range * 0.5f));
            if (adjusted_hue > 1.0f) adjusted_hue -= 1.0f;
            else if (adjusted_hue < 0.0f) adjusted_hue += 1.0f;

            hsv_t color_hsv = {
                .h = adjusted_hue,
                .s = !dark_mode ? (first_accent_hsv.s + 0.1f) : first_accent_hsv.s,
                .v = !dark_mode ? (first_accent_hsv.v - 0.08f) : first_accent_hsv.v
            };

            uint8_t a_map = 0, b_map = 0;
            color_enum_to_mapping(color_enum, &a_map, &b_map);

            buffer[a_map] = hsv_to_rgb(color_hsv);
            hsv_t darker = {color_hsv.h, color_hsv.s, color_hsv.v - 0.1f};
            buffer[b_map] = hsv_to_rgb(darker);
        }
    }

    buffer[18] = most_used.first;
    buffer[19] = second_used.first;

    return 0;
}

#ifndef AS_LIB
int main(int argc, char **argv)
{
    bool monochrome = false;
    bool dark_mode = true;
    char *target = NULL;
    char *input = NULL;
    bool exit_mode = false;

    for (int i = 1; i < argc; i++) {
        char *current = argv[i];
        if (current[0] == '-') {
            if (strlen(current) < 2) continue;
            switch (current[1]) {
            case 'h':
                printf("%s [infile] [outfile]\n", argv[0]);
                exit_mode = true;
                break;
            case 'v':
                printf("%s %d.%d.%d\n", argv[0], VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
                exit_mode = true;
                break;
            case 'l':
                dark_mode = false;
                break;
            case 'm':
                monochrome = true;
                break;
            default:
                fprintf(stderr, "ERROR: Not a valid argument!\n");
                break;
            }
        } else {
            if (!input) input = current;
            else if (!target) target = current;
        }
    }

    if (exit_mode) return 0;

    if (!input || !target) {
        fprintf(stderr, "ERROR: Not enough arguments!\n");
        return 1;
    }

    /* Run core engine */
    rgb_t palette[20] = {0};
    if (libtmg_wall_generate_color(palette, monochrome, dark_mode, input) != 0) {
        return 1;
    }

    /* -- Output the file -- */
    FILE *out_file = fopen(target, "w");
    if (!out_file) {
        fprintf(stderr, "ERROR: Failed to open the target file: %s\n", strerror(errno));
        return 1;
    }

    fprintf(out_file, "return {\n");
    for(int i = 0; i < 18; i++) {
        fprintf(out_file, "\tcolor%.2d = 0x%x,\n", i, palette[i]);
    }
    /* Note: Extracted original targets for diagnostic table items below */
    fprintf(out_file, "\taccent1 = 0x%x,\n", palette[18]); // Mirroring structural fallback logic if targets not stored
    fprintf(out_file, "\taccent2 = 0x%x\n", palette[19]);
    fprintf(out_file, "}\n");
    fclose(out_file);

    /* Print color previews to stdout */
    int printed = 0;
    for (int i = 0; i < 20; i++) {
        uint8_t r = (palette[i] >> 16) & 0xFF;
        uint8_t g = (palette[i] >> 8)  & 0xFF;
        uint8_t b =  palette[i]        & 0xFF;
        printf("\033[48;2;%d;%d;%dm   \033[0m", r, g, b);
        printed++;
        if (printed >= 8) {
            printed = 0;
            printf("\n");
        }
    }
    printf("\n");

    return 0;
}
#endif
