/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

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

LV_FONT_DECLARE(font_12_roboto_extra_bold);

struct trackpad_status_state {
  uint8_t pointer_speed_index;
  uint8_t scroll_speed_index;
  bool cirque_mode_known;
  bool cirque_relative_mode;
};

static void set_trackpad_status(lv_obj_t *label,
                                struct trackpad_status_state state) {
  char text[32] = {};
  const char *mode =
      state.cirque_mode_known ? (state.cirque_relative_mode ? "R" : "A") : "-";

  snprintf(text, sizeof(text), "Trackpad: %s P%u S%u", mode,
           state.pointer_speed_index, state.scroll_speed_index);
  lv_label_set_text(label, text);
}

static void trackpad_status_update_cb(struct trackpad_status_state state) {
  struct zmk_widget_trackpad_status *widget;

  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    set_trackpad_status(widget->obj, state);
  }
}

static struct trackpad_status_state
trackpad_status_get_state(const zmk_event_t *eh) {
  ARG_UNUSED(eh);

  return (struct trackpad_status_state){
      .pointer_speed_index =
          zmk_pointing_speed_get_index(ZMK_POINTING_SPEED_TARGET_POINTER),
      .scroll_speed_index =
          zmk_pointing_speed_get_index(ZMK_POINTING_SPEED_TARGET_SCROLL),
      .cirque_mode_known = zmk_cirque_mode_is_known(),
      .cirque_relative_mode = zmk_cirque_mode_get_relative(),
  };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_trackpad_status,
                            struct trackpad_status_state,
                            trackpad_status_update_cb,
                            trackpad_status_get_state)

ZMK_SUBSCRIPTION(widget_trackpad_status, zmk_trackpad_status_changed);

int zmk_widget_trackpad_status_init(struct zmk_widget_trackpad_status *widget,
                                    lv_obj_t *parent) {
  widget->obj = lv_label_create(parent);
  lv_obj_set_style_text_font(widget->obj, &font_12_roboto_extra_bold,
                             LV_PART_MAIN | LV_STATE_DEFAULT);

  sys_slist_append(&widgets, &widget->node);

  widget_trackpad_status_init();
  return 0;
}

lv_obj_t *
zmk_widget_trackpad_status_obj(struct zmk_widget_trackpad_status *widget) {
  return widget->obj;
}
