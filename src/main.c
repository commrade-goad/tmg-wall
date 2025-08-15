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

    /* -- Work -- */
    Pair most_used = {0};

    uint16_t *freq = calloc(UINT16_MAX, sizeof(uint16_t));

    static const uint16_t max_value = 210;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * n;

            rgb_t pixel = pack_color_555(
                    image[index + 0],
                    image[index + 1],
                    image[index + 2]
            );

            int count = ++freq[pixel];
            rgbh_t color = {0};

            color.r = ((pixel >> 10) & 0x1F) << 3;
            color.g = ((pixel >> 5)  & 0x1F) << 3;
            color.b = ( pixel        & 0x1F) << 3;

            if (count > most_used.second ||
                (color.r > max_value || color.g > max_value || color.b > max_value))
            {
                most_used.first = pixel;
                most_used.second = count;
            }
        }
    }
    printf("The most used color are : %x", unpack_color_555(most_used.first));

    /* -- Closing -- */
    free(freq);
    stbi_image_free(image);
    fclose(in_file);
    return 0;
}
