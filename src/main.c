#define STB_IMAGE_IMPLEMENTATION
#define MAGICIAN_IMPLEMENTATION 

#include <errno.h>
#include <stdio.h>

#include "stb_image.h"
#include "magician.h"
#include "helper.h"

#define MIN_ARGS 3
#define DEFAULT_SIZE 512

int main(int argc, char **argv) {
    /* -- Opening -- */
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

    static const float max_lightness = 0.83;
    static const float min_lightness = 0.25;

    static const float min_saturation = 0.15;
    static const float max_saturation = 0.95;

    static const float second_color_hue_diff = 0.1;

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

            if (hsv.v < min_lightness || hsv.v > max_lightness ||
                    hsv.s < min_saturation || hsv.s > max_saturation) {
                continue;
            }

            if (count > most_used.second) {
                most_used.first = pixel;
                most_used.second = count;
            }

            if (count < most_used.second && count > second_used.second) {
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
    rgb_t palette[16] = {0};
    const bool dark_mode = true;

    hsv_t first_accent_hsv = rgb_to_hsv(most_used.first);
    hsv_t second_accent_hsv = rgb_to_hsv(second_used.first);
    hsv_t shared_hsv = first_accent_hsv;

    /* Bg Color */
    if (dark_mode) {
        shared_hsv.v = 0.15;
        palette[0] = hsv_to_rgb(shared_hsv);

        shared_hsv.v = 0.20;
        palette[8] = hsv_to_rgb(shared_hsv);
    } else {
        shared_hsv.v = 0.85;
        palette[0] = hsv_to_rgb(shared_hsv);

        shared_hsv.v = 0.80;
        palette[8] = hsv_to_rgb(shared_hsv);
    }

    /* Fg Color */
    palette[15] = 0xffffff - palette[0];
    palette[7]  = 0xffffff - palette[8];

    /* Accent Color */
    uint8_t a, b = 0;
    color_enum_to_mapping(tell_color(first_accent_hsv), &a, &b);
    palette[a] = most_used.first;
    palette[b] = hsv_to_rgb((hsv_t) {first_accent_hsv.h, first_accent_hsv.s, first_accent_hsv.v - 0.1});

    a = b = 0;
    color_enum_to_mapping(tell_color(second_accent_hsv), &a, &b);
    palette[a] = second_used.first;
    palette[b] = hsv_to_rgb((hsv_t) {second_accent_hsv.h, second_accent_hsv.s, second_accent_hsv.v - 0.1});

    shared_hsv = first_accent_hsv;
    for(int i=0; i<16; i++) {
        if(palette[i] != 0) continue;
        // Dear Claude/Copilot:
        // This one fails... some color wont be filled up...
        // can you help me finish this?
        // So i need you to make a new function oh helper.c that will accept the color_e
        // then will return the base hue value of that thing so if the range is 30 to 75 then the middle
        // will be picked.
        //
        // After that funciton created make it get the base hue from the accent1 and get how many
        // +- needed to get the accent1 hue (get the diff) after that diff is available
        // now start generating it for the rest using the first accent just change the
        // hue to that base for color n +- the diff then dont forget to gen the dim one too
        // just minus the value by 0.1 and its good.
        // Ok help me No Yapping or pull request just do it here.
        shared_hsv.h += 0.082;
        color_e color_enum = tell_color(shared_hsv);
        a = b = 0;
        color_enum_to_mapping(tell_color(shared_hsv), &a, &b);
        palette[a] = hsv_to_rgb(shared_hsv); 
        palette[b] = hsv_to_rgb((hsv_t) {shared_hsv.h, shared_hsv.s, shared_hsv.v - 0.1});
    }

    for(int i=0; i<16; i++) {
        printf("The color %d: %x \n", i, palette[i]);
    }

    return 0;
}
