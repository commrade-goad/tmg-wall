#define STB_IMAGE_IMPLEMENTATION
#define MAGICIAN_IMPLEMENTATION

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include "helper.h"
#include "argparser.h"
#include "magician.h"
#include "stb_image.h"

#include <unordered_map>
#include <vector>
#include <algorithm>

struct ColorData {
    hsv_t hsv;
    uint32_t frequency;
};

bool is_hue_different_enough(hsv_t new_hsv, hsv_t* selected_hsvs, int selected_count) {
    for (int i = 0; i < selected_count; i++) {
        float hue_diff = fabsf(new_hsv.h - selected_hsvs[i].h);
        if (hue_diff > 180.0f) {
            hue_diff = 360.0f - hue_diff;
        }
        if (hue_diff < 20.0f) {
            return false;
        }
    }
    return true;
}

void process_image(uint8_t* image, int w, int h, int n, Args *args) {
    static const int target_sel_count= 5;
    rgb_t most_used_colors[target_sel_count] = {0};
    uint32_t most_used_freqs[target_sel_count] = {0};

    std::unordered_map<rgb_t, ColorData> color_map;

    for (int y = 0; y < h; y += 2) {
        for (int x = 0; x < w; x+= 2) {
            int index = (y * w + x) * n;

            uint8_t r = image[index + 0];
            uint8_t g = image[index + 1];
            uint8_t b = image[index + 2];
            rgb_t pixel = (r << 16) | (g << 8) | b;

            auto it = color_map.find(pixel);
            if (it != color_map.end()) {
                it->second.frequency++;
            } else {
                hsv_t hsv = rgb_to_hsv(pixel);
                color_map[pixel] = {hsv, 1};
            }
        }
    }

    std::vector<std::pair<rgb_t, ColorData>> color_vec;
    color_vec.reserve(color_map.size());

    for (const auto& pair : color_map) {
        color_vec.push_back(pair);
    }

    std::sort(color_vec.begin(), color_vec.end(),
            [](const auto& a, const auto& b) {
            return a.second.frequency > b.second.frequency;
            });

    hsv_t selected_hsvs[target_sel_count] = {0};
    int selected_count = 0;

    float value_avg      = 0.0f;
    float saturation_avg = 0.0f;

    for (const auto& pair : color_vec) {
        if (selected_count >= target_sel_count) break;

        rgb_t color = pair.first;
        ColorData data = pair.second;

        value_avg += data.hsv.v;
        saturation_avg += data.hsv.s;

        bool requirement = (data.hsv.v > 0.40 && data.hsv.s > 0.20);
        if (args->colorful_mode) requirement = (data.hsv.v > 0.47 && data.hsv.s > 0.27);

        if ((selected_count == 0 || is_hue_different_enough(data.hsv, selected_hsvs, selected_count)) && requirement)
        {
            most_used_colors[selected_count] = color;
            most_used_freqs[selected_count] = data.frequency;
            selected_hsvs[selected_count] = data.hsv;
            selected_count++;
        }
    }

    if (!color_vec.empty()) {
        value_avg      /= color_vec.size();
        saturation_avg /= color_vec.size();
    } else {
        value_avg = 0.0f;
        saturation_avg = 0.0f;
    }

    if (args->colorful_mode) {
        value_avg      += 0.1;
        saturation_avg += 0.1;
    }

    if (selected_count < target_sel_count) {
        bool monochrome_mode = (selected_count <= 1 && !args->colorful_mode) ? true : false;
        float hue = 0.0f;
        int mult = 1;
        float min_value = 0.2f;
        float min_saturation = 0.3f;
        float safe_v = std::max(value_avg, min_value);
        float safe_s = std::max(saturation_avg, min_saturation);

        // Fill remaining slots with synthetic colors
        for (int i = 0; i < target_sel_count; i++) {
            if (selected_hsvs[i].v >= 0.2f) continue;

            if (!monochrome_mode) {
                hue = 0.0f;
                if (selected_count > 0) {
                    hue = fmodf(selected_hsvs[0].h + (360.0f / target_sel_count) * i, 360.0f);
                } else {
                    hue = (360.0f / target_sel_count) * i;
                }
            } else {
                if (selected_count <= 0) saturation_avg = 0.1;
                else saturation_avg = selected_hsvs[0].s;

                hue = selected_hsvs[0].h + i * 60;
                safe_s = saturation_avg;
                safe_v = std::max(value_avg, min_value);

                if (value_avg >= 1.0 - min_value) mult = -1;
                else mult = 1;

                value_avg += 0.08 * mult;
            }

            selected_hsvs[i] = hsv_t {
                .h = hue,
                .s = safe_s,
                .v = safe_v,
            };

            most_used_colors[i] = hsv_to_rgb(selected_hsvs[i]);
            most_used_freqs[i]  = 0;
            selected_count++;
        }
    }

    for (int i = 0; i < target_sel_count; i++) {
        uint8_t r = (most_used_colors[i] >> 16) & 0xFF;
        uint8_t g = (most_used_colors[i] >> 8)  & 0xFF;
        uint8_t b =  most_used_colors[i]        & 0xFF;
        printf("\033[48;2;%d;%d;%dm   \033[0m", r, g, b);
    }
    printf("\n");
    for (int i = 0; i < target_sel_count; i++) {
        hsv_t hsv = rgb_to_hsv(most_used_colors[i]);
        hsv.v = std::clamp(hsv.v + 0.1f, 0.1f, 0.9f);
        rgb_t c = hsv_to_rgb(hsv);
        uint8_t r = (c >> 16) & 0xFF;
        uint8_t g = (c >> 8)  & 0xFF;
        uint8_t b =  c        & 0xFF;
        printf("\033[48;2;%d;%d;%dm   \033[0m", r, g, b);
    }
    printf("\n 1  2  3  4  5 \n");
    for (int i = 0; i < target_sel_count; i++) {
        printf("%d) #%06x: %u\n", i+1, most_used_colors[i], most_used_freqs[i]);
    }
}

int main(int argc, char **argv) {
    Args a = init_args(argc, argv);
    if (a.exit) return 0;
    if (!a.infile) return 1;
    if (!a.outfile) return 1;

    FILE *input = fopen(a.infile, "rb");
    if (!input) {
        fprintf(stderr, "ERROR: Failed to read the file because : %s\n", strerror(errno));
        deinit_args(&a);
        return 1;
    }

    if (!is_png(input) && !is_jpeg(input)) {
        fprintf(stderr, "ERROR: File `%s` is not a png or jpeg file!\n", a.infile);
        fclose(input);
        deinit_args(&a);
        return 1;
    }

    int w, h, n;
    unsigned char *image = stbi_load_from_file(input, &w, &h, &n, 4);
    if (!image) {
        fprintf(stderr, "ERROR: Failed to parse file `%s`: %s\n", a.infile, stbi_failure_reason());
        fclose(input);
        deinit_args(&a);
        return 1;
    }
    fclose(input);

    process_image(image, w, h, 4, &a);

    stbi_image_free(image);
    deinit_args(&a);
    return 0;
}
