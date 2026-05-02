/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

struct zmk_widget_output_status {
  sys_snode_t node;
  lv_obj_t *obj;
  lv_obj_t *usb_label;
  lv_obj_t *ble_label;
  lv_obj_t *marker_label;
  lv_obj_t *endpoint_underline_label;
  bool status_initialized;
  bool last_usb_present;
  int16_t last_ble_x;
  int16_t last_marker_x;
  int16_t last_underline_x;
  int16_t last_underline_y;
};

int zmk_widget_output_status_init(struct zmk_widget_output_status *widget,
                                  lv_obj_t *parent);
lv_obj_t *
zmk_widget_output_status_obj(struct zmk_widget_output_status *widget);
