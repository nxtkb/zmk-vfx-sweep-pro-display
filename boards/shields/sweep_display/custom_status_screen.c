/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "custom_status_screen.h"

#include <zephyr/logging/log.h>

#include "widgets/battery_status.h"
#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_AUX_STATUS_LAYOUT)
#include "widgets/aux_status_layout.h"
#endif
#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_CODEX_STATUS)
#include "widgets/codex_status.h"
#endif
#include "widgets/hid_indicators_status.h"
#include "widgets/kbd_name.h"
#include "widgets/layer_status.h"
#include "widgets/logo.h"
#include "widgets/output_status.h"
#include "widgets/peripheral_status.h"
#include "widgets/trackpad_status.h"
#include "widgets/wpm_status.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

LV_FONT_DECLARE(lv_custom_symbol);

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_BATTERY_STATUS)
static struct zmk_widget_battery_status battery_status_widget;
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_OUTPUT_STATUS)
static struct zmk_widget_output_status output_status_widget;
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_PERIPHERAL_STATUS)
static struct zmk_widget_peripheral_status peripheral_status_widget;
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_LAYER_STATUS)
static struct zmk_widget_layer_status layer_status_widget;
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_CODEX_STATUS)
static struct zmk_widget_codex_status codex_status_widget;
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_TRACKPAD_STATUS)
static struct zmk_widget_trackpad_status trackpad_status_widget;
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_WPM_STATUS)
static struct zmk_widget_wpm_status wpm_status_widget;
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_INDICATORS_STATUS)
static struct zmk_widget_hid_indicators_status indicators_status_widget;
#endif

static struct zmk_widget_logo logo_widget;
static struct zmk_widget_kbd_name kbd_name_widget;
#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_AUX_STATUS_LAYOUT)
static struct zmk_widget_aux_status_layout aux_status_layout;
#endif

static void set_epaper_screen_style(lv_obj_t *obj) {
  lv_obj_set_style_bg_color(obj, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_text_color(obj, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(obj, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
}

static bool is_epaper_fill_bar(lv_obj_t *obj) {
  return lv_obj_get_child_count(obj) == 0 && lv_obj_get_width(obj) >= 8 &&
         lv_obj_get_height(obj) <= 2;
}

static void set_epaper_widget_style(lv_obj_t *obj) {
  if (is_epaper_fill_bar(obj)) {
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
  } else {
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
  }
  lv_obj_set_style_text_color(obj, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(obj, 0, LV_PART_MAIN);

  uint32_t child_count = lv_obj_get_child_count(obj);
  for (uint32_t i = 0; i < child_count; i++) {
    set_epaper_widget_style(lv_obj_get_child(obj, i));
  }
}

lv_obj_t *zmk_display_status_screen() {
  lv_obj_t *screen;
  screen = lv_obj_create(NULL);

  set_epaper_screen_style(screen);
#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_AUX_STATUS_LAYOUT)
  zmk_widget_aux_status_layout_init(&aux_status_layout);
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_OUTPUT_STATUS)
  zmk_widget_output_status_init(&output_status_widget, screen);
  set_epaper_widget_style(zmk_widget_output_status_obj(&output_status_widget));
  lv_obj_set_style_text_font(
      zmk_widget_output_status_obj(&output_status_widget), &lv_custom_symbol,
      LV_PART_MAIN);
  lv_obj_align(zmk_widget_output_status_obj(&output_status_widget),
               LV_ALIGN_TOP_LEFT, 0, 0);
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_BATTERY_STATUS)
  zmk_widget_battery_status_init(&battery_status_widget, screen);
  set_epaper_widget_style(
      zmk_widget_battery_status_obj(&battery_status_widget));
  lv_obj_align(zmk_widget_battery_status_obj(&battery_status_widget),
               LV_ALIGN_TOP_RIGHT, 0, 0);
#endif

  zmk_widget_logo_init(&logo_widget, screen);
  set_epaper_widget_style(zmk_widget_logo_obj(&logo_widget));
  lv_obj_align(zmk_widget_logo_obj(&logo_widget), LV_ALIGN_TOP_MID, 0, 34);

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_PERIPHERAL_STATUS)
  zmk_widget_peripheral_status_init(&peripheral_status_widget, screen);
  set_epaper_widget_style(
      zmk_widget_peripheral_status_obj(&peripheral_status_widget));
  lv_obj_set_style_text_font(
      zmk_widget_peripheral_status_obj(&peripheral_status_widget),
      &lv_custom_symbol, LV_PART_MAIN);
  lv_obj_align(zmk_widget_peripheral_status_obj(&peripheral_status_widget),
               LV_ALIGN_TOP_MID, 0, 70);
#endif

  zmk_widget_kbd_name_init(&kbd_name_widget, screen);
  set_epaper_widget_style(zmk_widget_kbd_name_obj(&kbd_name_widget));
  lv_obj_align(zmk_widget_kbd_name_obj(&kbd_name_widget), LV_ALIGN_TOP_MID, 0,
               96);

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_TRACKPAD_STATUS)
  zmk_widget_trackpad_status_init(&trackpad_status_widget, screen);
  set_epaper_widget_style(
      zmk_widget_trackpad_status_obj(&trackpad_status_widget));
  zmk_widget_aux_status_layout_register(
      &aux_status_layout, ZMK_AUX_STATUS_COMPONENT_TRACKPAD,
      zmk_widget_trackpad_status_obj(&trackpad_status_widget));
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_CODEX_STATUS)
  zmk_widget_codex_status_init(&codex_status_widget, screen);
  zmk_widget_aux_status_layout_register(
      &aux_status_layout, ZMK_AUX_STATUS_COMPONENT_CODEX,
      zmk_widget_codex_status_obj(&codex_status_widget));
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_AUX_STATUS_LAYOUT)
  zmk_widget_aux_status_layout_refresh(&aux_status_layout);
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_LAYER_STATUS)
  zmk_widget_layer_status_init(&layer_status_widget, screen);
  set_epaper_widget_style(zmk_widget_layer_status_obj(&layer_status_widget));
  lv_obj_align(zmk_widget_layer_status_obj(&layer_status_widget),
               LV_ALIGN_BOTTOM_LEFT, 0, 0);
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_WPM_STATUS)
  zmk_widget_wpm_status_init(&wpm_status_widget, screen);
  set_epaper_widget_style(zmk_widget_wpm_status_obj(&wpm_status_widget));
  lv_obj_align(zmk_widget_wpm_status_obj(&wpm_status_widget),
               LV_ALIGN_BOTTOM_LEFT, 0,
               IS_ENABLED(CONFIG_CUSTOM_WIDGET_LAYER_STATUS) ? -14 : 0);
#endif

#if IS_ENABLED(CONFIG_CUSTOM_WIDGET_INDICATORS_STATUS)
  zmk_widget_hid_indicators_status_init(&indicators_status_widget, screen);
  set_epaper_widget_style(
      zmk_widget_hid_indicators_status_obj(&indicators_status_widget));
  lv_obj_align(zmk_widget_hid_indicators_status_obj(&indicators_status_widget),
               LV_ALIGN_BOTTOM_RIGHT, 0, 0);
#endif

  return screen;
}
