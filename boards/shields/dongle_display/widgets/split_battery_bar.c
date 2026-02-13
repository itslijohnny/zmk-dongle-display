/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "split_battery_bar.h"

#include <zephyr/bluetooth/services/bas.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/event_manager.h>

#ifndef ZMK_SPLIT_BLE_PERIPHERAL_COUNT
#  define ZMK_SPLIT_BLE_PERIPHERAL_COUNT 2
#endif

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct peripheral_battery {
    lv_obj_t *label;
    lv_obj_t *bar;
    lv_obj_t *bar_bg;
};

// Static allocation for peripheral battery objects
static struct peripheral_battery peripherals[ZMK_SPLIT_BLE_PERIPHERAL_COUNT];

static struct battery_state {
    uint8_t levels[ZMK_SPLIT_BLE_PERIPHERAL_COUNT];
    bool connected[ZMK_SPLIT_BLE_PERIPHERAL_COUNT];
} current_state = {0};

static void set_battery_bar_value(uint8_t source, uint8_t level) {
    if (source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT) {
        return;
    }

    char text[8];
    // Show charging indicator (+) when at 100%
    if (level >= 100) {
        snprintf(text, sizeof(text), "+%u%%", level);
    } else {
        snprintf(text, sizeof(text), "%u%%", level);
    }
    lv_label_set_text(peripherals[source].label, text);
    
    // Manually draw battery level as a filled rectangle
    lv_obj_t *bar = peripherals[source].bar;
    int32_t bar_width = (level * 28) / 100; // 28 is the total width
    lv_obj_set_width(bar, bar_width > 0 ? bar_width : 1);
}

static void set_battery_connection(uint8_t source, bool connected) {
    if (source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT) {
        return;
    }

    if (connected) {
        lv_obj_clear_flag(peripherals[source].label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(peripherals[source].bar_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(peripherals[source].bar, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text(peripherals[source].label, "--");
        lv_obj_add_flag(peripherals[source].bar_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(peripherals[source].bar, LV_OBJ_FLAG_HIDDEN);
    }
}

static void battery_bar_update_cb(struct battery_state state) {
    for (int i = 0; i < ZMK_SPLIT_BLE_PERIPHERAL_COUNT; i++) {
        set_battery_bar_value(i, state.levels[i]);
        set_battery_connection(i, state.connected[i]);
    }
}

static struct battery_state battery_bar_get_state(const zmk_event_t *eh) {
    const struct zmk_peripheral_battery_state_changed *ev = as_zmk_peripheral_battery_state_changed(eh);
    if (ev != NULL && ev->source < ZMK_SPLIT_BLE_PERIPHERAL_COUNT) {
        current_state.levels[ev->source] = ev->state_of_charge;
        current_state.connected[ev->source] = true;
    }
    return current_state;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_split_battery_bar, struct battery_state,
                            battery_bar_update_cb, battery_bar_get_state)

ZMK_SUBSCRIPTION(widget_split_battery_bar, zmk_peripheral_battery_state_changed);

int zmk_widget_split_battery_bar_init(struct zmk_widget_split_battery_bar *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 70, 20);

    for (int i = 0; i < ZMK_SPLIT_BLE_PERIPHERAL_COUNT; i++) {
        // Label showing percentage (small text, with % symbol)
        lv_obj_t *label = lv_label_create(widget->obj);
        lv_label_set_text(label, "--");
        lv_obj_set_pos(label, i * 33 + 2, 0);
        lv_obj_set_style_text_font(label, &lv_font_unscii_8, 0);
        
        // Simple bar as a filled rectangle (no lv_bar widget)
        lv_obj_t *bar_bg = lv_obj_create(widget->obj);
        lv_obj_set_size(bar_bg, 28, 6);
        lv_obj_set_pos(bar_bg, i * 33, 10);
        lv_obj_set_style_bg_color(bar_bg, lv_color_black(), 0);
        lv_obj_set_style_border_width(bar_bg, 1, 0);
        lv_obj_set_style_border_color(bar_bg, lv_color_white(), 0);
        
        // Bar indicator (filled part)
        lv_obj_t *bar = lv_obj_create(bar_bg);
        lv_obj_set_size(bar, 1, 6);
        lv_obj_set_style_bg_color(bar, lv_color_white(), 0);
        lv_obj_set_style_border_width(bar, 0, 0);
        lv_obj_align(bar, LV_ALIGN_LEFT_MID, 0, 0);

        peripherals[i].label = label;
        peripherals[i].bar = bar;
        peripherals[i].bar_bg = bar_bg;

        current_state.levels[i] = 0;
        current_state.connected[i] = false;
    }

    sys_slist_append(&widgets, &widget->node);

    widget_split_battery_bar_init();

    return 0;
}

lv_obj_t *zmk_widget_split_battery_bar_obj(struct zmk_widget_split_battery_bar *widget) {
    return widget->obj;
}
