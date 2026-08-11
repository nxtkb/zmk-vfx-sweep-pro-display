/* SPDX-License-Identifier: MIT */

#pragma once

#include <lvgl.h>

enum zmk_aux_status_component_id {
  ZMK_AUX_STATUS_COMPONENT_CODEX,
  ZMK_AUX_STATUS_COMPONENT_TRACKPAD,
  ZMK_AUX_STATUS_COMPONENT_COUNT,
};

struct zmk_aux_status_component {
  lv_obj_t *obj;
};

struct zmk_widget_aux_status_layout {
  struct zmk_aux_status_component components[ZMK_AUX_STATUS_COMPONENT_COUNT];
};

int zmk_widget_aux_status_layout_init(
    struct zmk_widget_aux_status_layout *layout);
int zmk_widget_aux_status_layout_register(
    struct zmk_widget_aux_status_layout *layout,
    enum zmk_aux_status_component_id id, lv_obj_t *obj);
void zmk_widget_aux_status_layout_refresh(
    struct zmk_widget_aux_status_layout *layout);
