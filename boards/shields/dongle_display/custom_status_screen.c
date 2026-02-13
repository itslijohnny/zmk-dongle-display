/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "custom_status_screen.h"
#include "widgets/battery_status.h"
#include "widgets/modifiers.h"
#include "widgets/bongo_cat.h"
#include "widgets/layer_status.h"
#include "widgets/layer_roller.h"
#include "widgets/output_status.h"
#include "widgets/hid_indicators.h"
#include "widgets/wpm_status.h"
#include "widgets/split_battery_bar.h"
#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_CAPS_WORD_INDICATOR)
#include "widgets/caps_word_indicator.h"
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// New prospector-style widgets
static struct zmk_widget_layer_roller layer_roller_widget;
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
static struct zmk_widget_split_battery_bar split_battery_bar_widget;
#endif
#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_CAPS_WORD_INDICATOR)
static struct zmk_widget_caps_word_indicator caps_word_indicator_widget;
#endif

// Keep existing widgets for compatibility
static struct zmk_widget_output_status output_status_widget;

#if IS_ENABLED(CONFIG_ZMK_BATTERY) && IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_DONGLE_BATTERY)
static struct zmk_widget_dongle_battery_status dongle_battery_status_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_MODIFIERS)
static struct zmk_widget_modifiers modifiers_widget;
#endif

#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_WPM)
static struct zmk_widget_wpm_status wpm_status_widget;
#endif

lv_style_t global_style;

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen;

    screen = lv_obj_create(NULL);

    // Set up black background with white text (OLED style)
    lv_style_init(&global_style);
    lv_style_set_bg_color(&global_style, lv_color_white());
    lv_style_set_bg_opa(&global_style, LV_OPA_COVER);
    lv_style_set_text_color(&global_style, lv_color_black());
    lv_style_set_text_font(&global_style, &lv_font_unscii_8);
    lv_style_set_text_letter_space(&global_style, 1);
    lv_style_set_text_line_space(&global_style, 1);
    lv_obj_add_style(screen, &global_style, LV_PART_MAIN);
    
    // Prospector-style layout for OLED
    
    // Large centered layer name display
    zmk_widget_layer_roller_init(&layer_roller_widget, screen);
    lv_obj_align(zmk_widget_layer_roller_obj(&layer_roller_widget), LV_ALIGN_CENTER, 0, 0);
    
    // Bottom left: Modifiers (with optional caps word indicator)
#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_MODIFIERS)
    zmk_widget_modifiers_init(&modifiers_widget, screen);
    lv_obj_align(zmk_widget_modifiers_obj(&modifiers_widget), LV_ALIGN_BOTTOM_LEFT, 0, 0);
    
#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_CAPS_WORD_INDICATOR)
    zmk_widget_caps_word_indicator_init(&caps_word_indicator_widget, screen);
    lv_obj_align_to(zmk_widget_caps_word_indicator_obj(&caps_word_indicator_widget), 
                    zmk_widget_modifiers_obj(&modifiers_widget), 
                    LV_ALIGN_OUT_LEFT_MID, -3, 0);
#endif
#elif IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_CAPS_WORD_INDICATOR)
    // Just caps word if modifiers disabled
    zmk_widget_caps_word_indicator_init(&caps_word_indicator_widget, screen);
    lv_obj_align(zmk_widget_caps_word_indicator_obj(&caps_word_indicator_widget), LV_ALIGN_TOP_RIGHT, -5, 0);
#endif
    
    // Split keyboard battery bar at top right
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
    zmk_widget_split_battery_bar_init(&split_battery_bar_widget, screen);
    lv_obj_align(zmk_widget_split_battery_bar_obj(&split_battery_bar_widget), LV_ALIGN_TOP_RIGHT, 0, 0);
#endif
    
    // Keep output status at top left for connection info
    zmk_widget_output_status_init(&output_status_widget, screen);
    lv_obj_align(zmk_widget_output_status_obj(&output_status_widget), LV_ALIGN_TOP_LEFT, 0, 0);

    // Optional: Keep existing battery status for dongle itself if enabled
#if IS_ENABLED(CONFIG_ZMK_BATTERY) && IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_DONGLE_BATTERY)
    zmk_widget_dongle_battery_status_init(&dongle_battery_status_widget, screen);
    lv_obj_align(zmk_widget_dongle_battery_status_obj(&dongle_battery_status_widget), LV_ALIGN_TOP_RIGHT, 0, 12);
#endif

    // WPM at bottom right
#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_WPM)
    zmk_widget_wpm_status_init(&wpm_status_widget, screen);
    lv_obj_align(zmk_widget_wpm_status_obj(&wpm_status_widget), LV_ALIGN_BOTTOM_RIGHT, 0, 0);
#endif

    return screen;
}
