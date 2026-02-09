/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "caps_word_indicator.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/events/caps_word_state_changed.h>
#include <zmk/event_manager.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct caps_word_indicator_state {
    bool active;
};

static void caps_word_indicator_set_active(lv_obj_t *container, struct caps_word_indicator_state state) {
    if (state.active) {
        // Show bright "CAPS" when active
        lv_obj_clear_flag(container, LV_OBJ_FLAG_HIDDEN);
    } else {
        // Hide when inactive
        lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN);
    }
}

static void caps_word_indicator_update_cb(struct caps_word_indicator_state state) {
    struct zmk_widget_caps_word_indicator *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        caps_word_indicator_set_active(widget->obj, state);
    }
}

static struct caps_word_indicator_state caps_word_indicator_get_state(const zmk_event_t *eh) {
    const struct zmk_caps_word_state_changed *ev = as_zmk_caps_word_state_changed(eh);
    return (struct caps_word_indicator_state){
        .active = ev->active,
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_caps_word_indicator, struct caps_word_indicator_state,
                            caps_word_indicator_update_cb, caps_word_indicator_get_state)
ZMK_SUBSCRIPTION(widget_caps_word_indicator, zmk_caps_word_state_changed);

int zmk_widget_caps_word_indicator_init(struct zmk_widget_caps_word_indicator *widget,
                                        lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    
    // Create label for CAPS text
    lv_obj_t *label = lv_label_create(widget->obj);
    lv_label_set_text(label, "CAPS");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    
    // Add underline indicator
    lv_obj_t *line = lv_obj_create(widget->obj);
    lv_obj_set_size(line, 20, 2);
    lv_obj_set_style_bg_color(line, lv_color_white(), 0);
    lv_obj_align_to(line, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 1);
    
    // Start hidden
    lv_obj_add_flag(widget->obj, LV_OBJ_FLAG_HIDDEN);

    sys_slist_append(&widgets, &widget->node);

    widget_caps_word_indicator_init();
    return 0;
}

lv_obj_t *zmk_widget_caps_word_indicator_obj(struct zmk_widget_caps_word_indicator *widget) {
    return widget->obj;
}
