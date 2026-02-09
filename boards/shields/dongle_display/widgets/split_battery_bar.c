/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "split_battery_bar.h"

#include <zephyr/kernel.h>
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
};

static struct battery_state {
    uint8_t levels[ZMK_SPLIT_BLE_PERIPHERAL_COUNT];
    bool connected[ZMK_SPLIT_BLE_PERIPHERAL_COUNT];
};

static struct battery_state current_state = {0};

static void set_battery_bar_value(lv_obj_t *widget, uint8_t source, uint8_t level) {
    if (source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT) {
        return;
    }

    struct peripheral_battery *peripherals = lv_obj_get_user_data(widget);
    if (peripherals == NULL) {
        return;
    }

    lv_label_set_text_fmt(peripherals[source].label, "%u%%", level);
    lv_bar_set_value(peripherals[source].bar, level, LV_ANIM_OFF);
}

static void set_battery_connection(lv_obj_t *widget, uint8_t source, bool connected) {
    if (source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT) {
        return;
    }

    struct peripheral_battery *peripherals = lv_obj_get_user_data(widget);
    if (peripherals == NULL) {
        return;
    }

    if (connected) {
        lv_obj_clear_flag(peripherals[source].label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(peripherals[source].bar, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text(peripherals[source].label, "--");
    }
}

static void battery_bar_update_cb(struct battery_state state) {
    struct zmk_widget_split_battery_bar *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        for (int i = 0; i < ZMK_SPLIT_BLE_PERIPHERAL_COUNT; i++) {
            set_battery_bar_value(widget->obj, i, state.levels[i]);
            set_battery_connection(widget->obj, i, state.connected[i]);
        }
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
    
    // Allocate peripheral battery objects
    struct peripheral_battery *peripherals = k_malloc(sizeof(struct peripheral_battery) * ZMK_SPLIT_BLE_PERIPHERAL_COUNT);
    lv_obj_set_user_data(widget->obj, peripherals);

    lv_obj_set_size(widget->obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(widget->obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(widget->obj, 8, 0);

    for (int i = 0; i < ZMK_SPLIT_BLE_PERIPHERAL_COUNT; i++) {
        lv_obj_t *container = lv_obj_create(widget->obj);
        lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

        lv_obj_t *label = lv_label_create(container);
        lv_label_set_text(label, "--");
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

        lv_obj_t *bar = lv_bar_create(container);
        lv_obj_set_size(bar, 30, 6);
        lv_bar_set_range(bar, 0, 100);
        lv_bar_set_value(bar, 0, LV_ANIM_OFF);
        lv_obj_align_to(bar, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

        // For OLED, use white foreground on black background
        lv_obj_set_style_bg_color(bar, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_bg_color(bar, lv_color_white(), LV_PART_INDICATOR);

        peripherals[i].label = label;
        peripherals[i].bar = bar;

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
