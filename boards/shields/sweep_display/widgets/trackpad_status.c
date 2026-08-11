/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "trackpad_status.h"
#include <zmk/cirque_mode.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/trackpad_status_changed.h>
#include <zmk/pointing_speed.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

LV_FONT_DECLARE(lv_font_montserrat_12);
LV_FONT_DECLARE(lv_custom_symbol);

#define TRACKPAD_STATUS_UPDATE_DELAY_MS 250
#define TRACKPAD_STATUS_WIDTH 152
#define TRACKPAD_STATUS_HEIGHT 12
#define TRACKPAD_STATUS_LABEL_HEIGHT 12
#define TRACKPAD_STATUS_POINTER_VALUE_WIDTH 36
#define TRACKPAD_STATUS_SCROLL_VALUE_WIDTH 44
#define LV_SYMBOL_CUSTOM_MOUSE "\xEF\xA3\x8C"

struct trackpad_status_state {
  uint32_t pointer_speed_multiplier_q16;
  uint32_t scroll_speed_multiplier_q16;
  bool cirque_mode_known;
  bool cirque_relative_mode;
};

K_MUTEX_DEFINE(trackpad_status_state_mutex);
static struct trackpad_status_state trackpad_status_state;

static void format_multiplier(char *text, size_t text_size,
                              uint32_t multiplier_q16) {
  uint32_t hundredths = (multiplier_q16 * 100U + 32768U) / 65536U;

  snprintf(text, text_size, "%u.%02ux", (unsigned int)(hundredths / 100U),
           (unsigned int)(hundredths % 100U));
}

static void set_label_text_if_changed(lv_obj_t *label, const char *text) {
  if (strcmp(lv_label_get_text(label), text) == 0) {
    return;
  }

  lv_label_set_text(label, text);
}

static void set_trackpad_status(struct zmk_widget_trackpad_status *widget,
                                struct trackpad_status_state state) {
  char pointer_text[8] = {};
  char scroll_text[8] = {};
  const char *mode =
      state.cirque_mode_known ? (state.cirque_relative_mode ? "R" : "A") : "-";

  format_multiplier(pointer_text, sizeof(pointer_text),
                    state.pointer_speed_multiplier_q16);
  format_multiplier(scroll_text, sizeof(scroll_text),
                    state.scroll_speed_multiplier_q16);

  set_label_text_if_changed(widget->mode_label, mode);
  set_label_text_if_changed(widget->pointer_speed_label, pointer_text);
  set_label_text_if_changed(widget->scroll_speed_label, scroll_text);
}

static struct trackpad_status_state trackpad_status_get_local_state(void) {
  struct trackpad_status_state state;

  k_mutex_lock(&trackpad_status_state_mutex, K_FOREVER);
  state = trackpad_status_state;
  k_mutex_unlock(&trackpad_status_state_mutex);

  return state;
}

static void trackpad_status_update_cb(struct trackpad_status_state state) {
  struct zmk_widget_trackpad_status *widget;

  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    set_trackpad_status(widget, state);
  }
}

static void trackpad_status_refresh_state(const zmk_event_t *eh) {
  ARG_UNUSED(eh);

  struct trackpad_status_state state = {
      .pointer_speed_multiplier_q16 =
          zmk_pointing_speed_get_multiplier_q16(ZMK_POINTING_SPEED_TARGET_POINTER),
      .scroll_speed_multiplier_q16 =
          zmk_pointing_speed_get_multiplier_q16(ZMK_POINTING_SPEED_TARGET_SCROLL),
      .cirque_mode_known = zmk_cirque_mode_is_known(),
      .cirque_relative_mode = zmk_cirque_mode_get_relative(),
  };

  k_mutex_lock(&trackpad_status_state_mutex, K_FOREVER);
  trackpad_status_state = state;
  k_mutex_unlock(&trackpad_status_state_mutex);
}

static void trackpad_status_work_cb(struct k_work *work) {
  ARG_UNUSED(work);
  trackpad_status_update_cb(trackpad_status_get_local_state());
}

K_WORK_DELAYABLE_DEFINE(trackpad_status_work, trackpad_status_work_cb);

static void trackpad_status_init(void) {
  trackpad_status_refresh_state(NULL);
  trackpad_status_work_cb(NULL);
}

static int trackpad_status_cb(const zmk_event_t *eh) {
  if (zmk_display_is_initialized()) {
    trackpad_status_refresh_state(eh);
    k_work_reschedule_for_queue(zmk_display_work_q(), &trackpad_status_work,
                                K_MSEC(TRACKPAD_STATUS_UPDATE_DELAY_MS));
  }

  return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(widget_trackpad_status, trackpad_status_cb);

ZMK_SUBSCRIPTION(widget_trackpad_status, zmk_trackpad_status_changed);

static lv_obj_t *create_label(lv_obj_t *parent, const char *text, int16_t x,
                              int16_t width, const lv_font_t *font) {
  lv_obj_t *label = lv_label_create(parent);

  lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(label, width, TRACKPAD_STATUS_LABEL_HEIGHT);
  lv_obj_set_pos(label, x, 0);
  lv_obj_set_style_text_font(label, font, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_opa(label, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(label, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(label, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(label, 0, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(label, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(label, 0, LV_PART_MAIN);
  lv_label_set_text(label, text);

  return label;
}

int zmk_widget_trackpad_status_init(struct zmk_widget_trackpad_status *widget,
                                    lv_obj_t *parent) {
  widget->obj = lv_obj_create(parent);
  lv_obj_set_size(widget->obj, TRACKPAD_STATUS_WIDTH, TRACKPAD_STATUS_HEIGHT);
  lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(widget->obj, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);
  lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);

  create_label(widget->obj, LV_SYMBOL_CUSTOM_MOUSE, 0, 14, &lv_custom_symbol);
  widget->mode_label = create_label(widget->obj, "-", 17, 8,
                                    &lv_font_montserrat_10);
  create_label(widget->obj, "P", 30, 8, &lv_font_montserrat_10);
  widget->pointer_speed_label =
      create_label(widget->obj, "1.00x", 39,
                   TRACKPAD_STATUS_POINTER_VALUE_WIDTH,
                   &lv_font_montserrat_10);
  create_label(widget->obj, "S", 78, 8, &lv_font_montserrat_10);
  widget->scroll_speed_label =
      create_label(widget->obj, "1.00x", 87,
                   TRACKPAD_STATUS_SCROLL_VALUE_WIDTH, &lv_font_montserrat_10);

  sys_slist_append(&widgets, &widget->node);

  trackpad_status_init();
  return 0;
}

lv_obj_t *
zmk_widget_trackpad_status_obj(struct zmk_widget_trackpad_status *widget) {
  return widget->obj;
}
