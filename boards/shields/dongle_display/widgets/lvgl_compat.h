/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>

#if defined(LVGL_VERSION_MAJOR) && (LVGL_VERSION_MAJOR >= 9)
#if !defined(LV_IMG_CF_INDEXED_1BIT)
#define LV_IMG_CF_INDEXED_1BIT LV_COLOR_FORMAT_I1
#endif
#endif