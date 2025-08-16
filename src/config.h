#ifndef CONFIG_H
#define CONFIG_H

const bool dark_mode  = true;
const bool monochrome = false; /* no second accent, no s, v checkin */

static const float max_lightness  = 0.83;
static const float min_lightness  = 0.40;
static const float min_saturation = 0.20;
static const float max_saturation = 0.88;

static const float second_color_hue_diff = 0.083;

static const float bg_color_value          = 0.13;
static const float bg_color_value_alt_diff = 0.10;
static const float bg_min_value_diff       = 0.38;

static const float color_hue_range = 0.04;

#endif /* CONFIG_H */
