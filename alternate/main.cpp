#define MAGICIAN_IMPLEMENTATION

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <math.h>
#include <unordered_map>
#include <vector>
#include "helper.h"
#include "argparser.h"
#include "magician.h"
#include "stb_image.h"

struct ColorData {
    hsv_t hsv;
    uint32_t frequency;
};

bool is_hue_different_enough(hsv_t new_hsv, hsv_t* selected_hsvs, int selected_count) {
    for (int i = 0; i < selected_count; i++) {
        float hue_diff = fabsf(new_hsv.h - selected_hsvs[i].h);
        if (hue_diff > 180.0f) { hue_diff = 360.0f - hue_diff; }
        if (hue_diff < 40.0f) { return false; }
    }
    return true;
}

void process_image(uint8_t* image, int w, int h, int n, Args *args, rgb_t *palette, hsv_t *accent) {
    static const int target_sel_count = 6;
    rgb_t most_used_colors[target_sel_count] = {};
    // uint32_t most_used_freqs[target_sel_count] = {};

    // float darkest_value  = 1.0f;
    // float bright_value   = 0.0f;
    float value_avg      = 0.0f;
    float saturation_avg = 0.0f;

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

                value_avg += hsv.v;
                saturation_avg += hsv.s;
                // if (darkest_value > hsv.v) darkest_value = hsv.v;
                // if (bright_value < hsv.v) bright_value = hsv.v;

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

    if (!color_vec.empty()) {
        value_avg      /= color_vec.size();
        saturation_avg /= color_vec.size();
    } else {
        value_avg = 0.0f;
        saturation_avg = 0.0f;
    }

    hsv_t selected_hsvs[target_sel_count] = {};
    int selected_count = 0;
    bool found_accent = false;

    hsv_t most_used_bg = {0};
    bool most_used_bg_found = false;

    hsv_t most_used_fg = {0};
    bool most_used_fg_found = false;

    for (const auto& pair : color_vec) {
        if (selected_count >= target_sel_count) break;

        rgb_t color = pair.first;
        ColorData data = pair.second;

        static const float max_lightness  = 0.91;
        static const float min_lightness  = 0.52;
        static const float min_saturation = 0.20;
        static const float max_saturation = 0.87;

        if ((data.hsv.v > min_lightness && data.hsv.v < max_lightness &&
                data.hsv.s > min_saturation && data.hsv.s < max_saturation) && !found_accent) {
            *accent = data.hsv;
            found_accent = true;
        }

        if (data.hsv.v <= 0.2 && data.hsv.s < 0.2 && !most_used_bg_found) {
            most_used_bg = data.hsv;
            most_used_bg_found = true;
        }

        if (data.hsv.v >= 0.8 && data.hsv.s < 0.14 && !most_used_fg_found) {
            most_used_fg = data.hsv;
            most_used_fg_found = true;
        }

        bool requirement =
            (data.hsv.v >= std::max(value_avg, 0.3f) && data.hsv.v <= 0.90) &&
            (data.hsv.s >= std::max(saturation_avg, 0.2f) && data.hsv.s <= 0.78);
        // if (args->colorful_mode) requirement = (data.hsv.v >= (value_avg / 1.2f) && data.hsv.s >= (saturation_avg / 1.7f));

        if (is_hue_different_enough(data.hsv, selected_hsvs, selected_count) && requirement)
        {
            most_used_colors[selected_count] = color;
            // most_used_freqs[selected_count] = data.frequency;
            selected_hsvs[selected_count] = data.hsv;
            selected_count++;
        }
    }

    if (args->colorful_mode) {
        value_avg      += 0.1;
        saturation_avg += 0.1;
    }

    bool monochrome_mode = (selected_count <= 1 && !args->colorful_mode) ? true : false;
    if (selected_count < target_sel_count) {
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
                hue = fmodf(selected_hsvs[std::max(i - 1, 0)].h + (360.0f / target_sel_count) * i, 360.0f);
            } else {
                if (selected_count <= 0) saturation_avg = 0.1;
                else saturation_avg = selected_hsvs[0].s;

                hue = selected_hsvs[0].h + i * 60;
                safe_s = std::max(saturation_avg, 0.1f);
                safe_v = std::max(value_avg, min_value);

                if (value_avg >= 1.0 - min_value) mult = -1;
                else mult = 1;

                value_avg += 0.08 * mult;
            }

            selected_hsvs[i] = hsv_t {
                .h = fmodf(hue, 360 + 1),
                .s = safe_s,
                .v = safe_v,
            };

            int counter = 0;
            int max_try = 10;
            while (!is_hue_different_enough(selected_hsvs[i], selected_hsvs, selected_count) && !monochrome_mode) {
                hue = fmodf(selected_hsvs[std::max(counter - 1, 0)].h + (360.0f / target_sel_count) * counter, 360.0f);
                selected_hsvs[i] = hsv_t {
                    .h = fmodf(hue, 360 + 1),
                    .s = safe_s,
                    .v = safe_v,
                };
                if (max_try <= counter++) break;
            }

            most_used_colors[i] = hsv_to_rgb(selected_hsvs[i]);
            // most_used_freqs[i]  = 0;
            selected_count++;
        }
    }

    for (int i = 0; i < target_sel_count; i++) {
        color_e color = tell_color(rgb_to_hsv(most_used_colors[i]));
        uint8_t bright, dark;
        color_enum_to_mapping(color, &bright, &dark);

        // saturate more for light mode [[ new ]]
        hsv_t hsv_c2 = rgb_to_hsv(most_used_colors[i]);
        if (args->light_mode) {
            hsv_c2.s = std::min(hsv_c2.s + 0.1d, 0.9);
            hsv_c2.v = std::min(hsv_c2.v - 0.15d, 0.6);
        }
        rgb_t c2 = hsv_to_rgb(hsv_c2);

        hsv_t hsv = rgb_to_hsv(c2);
        hsv.v = std::clamp(hsv.v + 0.1f, 0.1f, 0.9f);
        rgb_t c = hsv_to_rgb(hsv);

        if (palette[dark] <= 0) palette[dark] = c2;
        if (palette[bright] <= 0) palette[bright] = c;
    }

    // gen bg
    for (int i = 0; i < 3; i++) {
        hsv_t hsv = found_accent ? *accent : rgb_to_hsv(most_used_colors[0]);
        hsv.s = 0.1f;
        if (most_used_bg_found) { hsv = most_used_bg; }

        if (args->light_mode) {
            hsv.v = std::max(0.9f - i * 0.1f, 0.7f);
            hsv.s = std::max(0.08, (double)hsv.s - 0.1);
        }
        else hsv.v = std::min(0.1f + i * 0.1f, 0.3f);
        // hsv.v = std::min(darkest_value + i * 0.08f, 0.38f);
        rgb_t c = hsv_to_rgb(hsv);

        switch (i) {
            case 0:
                palette[0] = c;
                break;
            case 1:
                palette[8] = c;
                break;
            case 2:
                palette[16] = c;
                break;
            default:
                break;
        }
    }

    // gen fg
    for (int i = 0; i < 3; i++) {
        hsv_t hsv = found_accent ? *accent : rgb_to_hsv(most_used_colors[0]);
        hsv.s = monochrome_mode ? 0.0f : 0.1f;

        if (most_used_fg_found) { hsv = most_used_fg; }
        if (args->light_mode) {
            hsv.v = std::min(0.1f + i * 0.1f, 0.2f);
            hsv.s = std::max(hsv.s + 0.05d, 0.4);
        }
        else hsv.v = std::max(0.82f - i * 0.1f, 0.5f);
        // hsv.v = std::max(bright_value - i * 0.08f, 0.72f);
        rgb_t c = hsv_to_rgb(hsv);

        switch (i) {
            case 0:
                palette[15] = c;
                break;
            case 1:
                palette[7] = c;
                break;
            case 2:
                palette[17] = c;
                break;
            default:
                break;
        }
    }

    static float golden_ratio_conj = 0.6180339887f;
    srand(time(NULL));
    int jitter_seed = rand();

    for (int i = 0; i < 16; i++) {
        if (palette[i] > 0x0) continue;

        color_e color = mapping_to_color_enum(i);
        uint8_t bright, dark;
        float base = get_base_hue(color);
        color_enum_to_mapping(color, &bright, &dark);

        float offset = fmodf((i + jitter_seed) * golden_ratio_conj, 1.0f);
        float hue_jitter = offset * 30.0f - 15.0f;

        float h1 = fmodf(base + hue_jitter, 360.0f);
        float h2 = fmodf(base + hue_jitter + 10.0f, 360.0f);

        palette[dark] = hsv_to_rgb(hsv_t { h1, saturation_avg, value_avg });
        palette[bright] = hsv_to_rgb(hsv_t { h2, saturation_avg, std::clamp(value_avg + 0.1f, 0.1f, 0.9f) });
    }

    hsv_t seven = rgb_to_hsv(palette[7]);
    if (!found_accent && !monochrome_mode) *accent = hsv_t {seven.h, seven.s + 0.1f, seven.v};
    else if (!found_accent) *accent = hsv_t {seven.h, seven.s, seven.v};

    int printed = 0;
    for (int i = 0; i < 18; i++) {
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
    rgb_t accent_rgb = hsv_to_rgb(*accent);
    uint8_t r = (accent_rgb >> 16) & 0xFF;
    uint8_t g = (accent_rgb >> 8)  & 0xFF;
    uint8_t b =  accent_rgb        & 0xFF;
    printf("\033[48;2;%d;%d;%dm   \033[0m", r, g, b);
    printf("\n");
    // for (int i = 0; i < 18; i++) {
    //     printf("%d. %.6x\n",i , palette[i]);
    // }
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

    hsv_t accent = {};
    rgb_t *palette = (rgb_t *)calloc(18, sizeof(rgb_t));
    process_image(image, w, h, 4, &a, palette, &accent);

    FILE *output = fopen(a.outfile, "w");
    if (!output) {
        fprintf(stderr, "ERROR: Failed to open the file: %s\n", strerror(errno));
        return 1;
    }
    fprintf(output, "return {\n");
    for(int i=0; i<18; i++) {
        fprintf(output, "\tcolor%.2d = 0x%x,\n", i, palette[i]);
    }
    fprintf(output, "\taccent1 = 0x%x,\n", hsv_to_rgb(accent));
    fprintf(output, "\taccent2 = 0x%x\n", palette[7]);
    fprintf(output, "}\n");

    fclose(output);
    stbi_image_free(image);
    deinit_args(&a);
    return 0;
}
