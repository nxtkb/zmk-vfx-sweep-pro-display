/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "widgets/battery_status.h"
#include "widgets/layer_status.h"
#include "widgets/output_status.h"
#include "widgets/peripheral_status.h"
#include "widgets/wpm_status.h"
#include "widgets/hid_indicators_status.h"
#include "widgets/logo.h"
#include "widgets/kbd_name.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// #if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY_STATUS)
// static struct zmk_widget_battery_status battery_status_widget;
// #endif

// #if IS_ENABLED(CONFIG_ZMK_WIDGET_OUTPUT_STATUS)
// static struct zmk_widget_output_status output_status_widget;
// #endif

// #if IS_ENABLED(CONFIG_ZMK_WIDGET_PERIPHERAL_STATUS)
// static struct zmk_widget_peripheral_status peripheral_status_widget;
// #endif

// #if IS_ENABLED(CONFIG_ZMK_WIDGET_LAYER_STATUS)
// static struct zmk_widget_layer_status layer_status_widget;
// #endif
// #if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
// static struct zmk_widget_wpm_status wpm_status_widget;
// #endif

// #if IS_ENABLED(CONFIG_ZMK_WIDGET_INDICATORS_STATUS)
// static struct zmk_widget_hid_indicators_status indicators_status_widget;
// #endif

// static struct zmk_widget_logo logo_widget;
// static struct zmk_widget_kbd_name kbd_name_widget;

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    LOG_ERR("显示状态屏幕");

    // zmk_widget_battery_status_init(&battery_status_widget, screen);

    LOG_ERR("显示状态屏幕2222");
    // lv_obj_align(zmk_widget_battery_status_obj(&battery_status_widget), LV_ALIGN_CENTER, 0, 0);

// #if IS_ENABLED(CONFIG_ZMK_WIDGET_OUTPUT_STATUS)
//     zmk_widget_output_status_init(&output_status_widget, screen);
//     lv_obj_align(zmk_widget_output_status_obj(&output_status_widget), LV_ALIGN_TOP_LEFT, 0, 0);
// #endif

//     // 展示logo
//     zmk_widget_logo_init(&logo_widget, screen);
//     lv_obj_align(zmk_widget_logo_obj(&logo_widget), LV_ALIGN_TOP_MID, 0, 35);

// #if IS_ENABLED(CONFIG_ZMK_WIDGET_PERIPHERAL_STATUS)
//     zmk_widget_peripheral_status_init(&peripheral_status_widget, screen);
//     lv_obj_align(zmk_widget_peripheral_status_obj(&peripheral_status_widget), LV_ALIGN_TOP_MID, 0,
//                  70);
// #endif

//     // 展示name
//     zmk_widget_kbd_name_init(&kbd_name_widget, screen);
//     lv_obj_align(zmk_widget_kbd_name_obj(&kbd_name_widget), LV_ALIGN_TOP_MID, 0, 95);

// #if IS_ENABLED(CONFIG_ZMK_WIDGET_LAYER_STATUS)
//     zmk_widget_layer_status_init(&layer_status_widget, screen);
//     lv_obj_align(zmk_widget_layer_status_obj(&layer_status_widget), LV_ALIGN_TOP_MID, 0, 115);
// #endif

// #if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
//     zmk_widget_wpm_status_init(&wpm_status_widget, screen);
//     lv_obj_align(zmk_widget_wpm_status_obj(&wpm_status_widget), LV_ALIGN_BOTTOM_LEFT, 0, 0);
// #endif

// #if IS_ENABLED(CONFIG_ZMK_WIDGET_INDICATORS_STATUS)
//     zmk_widget_hid_indicators_status_init(&indicators_status_widget, screen);
//     lv_obj_align(zmk_widget_hid_indicators_status_obj(&indicators_status_widget),
//                  LV_ALIGN_BOTTOM_RIGHT, 0, 0);
// #endif

    return screen;
}
