#ifndef CONFIG_H
#define CONFIG_H

static const float max_lightness  = 0.80;
static const float min_lightness  = 0.58;
static const float min_saturation = 0.15;
static const float max_saturation = 0.78;

static const float second_color_hue_diff = 0.083;

static const float bg_color_value_alt_diff = 0.16; // changed from 0.12 (maybe broke some stuff that like color gray but you can restore this to be 0.12 if you want
static const float bg_min_value_diff       = 0.38;

static const float color_hue_range = 0.04;

#endif /* CONFIG_H */
