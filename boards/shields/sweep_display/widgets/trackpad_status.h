/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include <zephyr/sys/slist.h>

struct zmk_widget_trackpad_status {
  sys_snode_t node;
  lv_obj_t *obj;
  lv_obj_t *mode_label;
  lv_obj_t *pointer_speed_label;
  lv_obj_t *scroll_speed_label;
};

int zmk_widget_trackpad_status_init(struct zmk_widget_trackpad_status *widget,
                                    lv_obj_t *parent);
lv_obj_t *
zmk_widget_trackpad_status_obj(struct zmk_widget_trackpad_status *widget);
