/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>

#if !defined(LV_IMG_CF_INDEXED_1BIT)
#if defined(LV_COLOR_FORMAT_I1)
#define LV_IMG_CF_INDEXED_1BIT LV_COLOR_FORMAT_I1
#elif defined(LV_IMG_CF_INDEXED_1)
#define LV_IMG_CF_INDEXED_1BIT LV_IMG_CF_INDEXED_1
#endif
#endif