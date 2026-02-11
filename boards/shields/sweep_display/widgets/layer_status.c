/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL); // 使用完整路径

#include "layer_status.h"
#include <lvgl.h>
#include <zmk/display.h>
#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

LV_FONT_DECLARE(font_12_roboto_extra_bold);

struct layer_status_state {
  zmk_keymap_layer_index_t index;
  const char *label;
  struct zmk_keymap_layers_info info;
};

static void set_layer_symbol(lv_obj_t *label, struct layer_status_state state) {

  if (state.label == NULL || strlen(state.label) == 0) {
    char text[10] = {};
    snprintf(text, sizeof(text), "layer : %i", state.index);
    lv_label_set_text(label, text);
  } else {
    char text[20] = {};
    // 判断如果state.label的长度大于15，则只显示前15个字符
    if (strlen(state.label) > 10) {
      snprintf(text, sizeof(text), "layer : %.10s", state.label);
    } else {
      snprintf(text, sizeof(text), "layer : %s", state.label);
    }

    lv_label_set_text(label, text);
  }
}

void debug_print_layer_status_state(const struct layer_status_state *state) {
  LOG_INF("当前层 index: %d, label: %s", state->index,
          state->label ? state->label : "(none)");
  LOG_INF("所有有效层:");
  for (size_t i = 0; i < state->info.effective_layer_count; i++) {
    const char *label = state->info.keymap_layers[i].label
                            ? state->info.keymap_layers[i].label
                            : "(none)";
    zmk_keymap_layer_id_t id = state->info.keymap_layers[i].id;
    LOG_INF("  Layer %d: %s", id, label);
  }
}

static void layer_status_update_cb(struct layer_status_state state) {
  struct zmk_widget_layer_status *widget;
  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    // debug_print_layer_status_state(&state);
    set_layer_symbol(widget->obj, state);
  }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
  zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
  struct zmk_keymap_layers_info info = zmk_keymap_get_all_layers_info();

  return (struct layer_status_state){
      .index = index,
      .label = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index)),
      .info = info};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state,
                            layer_status_update_cb, layer_status_get_state)

ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

int zmk_widget_layer_status_init(struct zmk_widget_layer_status *widget,
                                 lv_obj_t *parent) {
  widget->obj = lv_label_create(parent);
  // 设置字体的大小
  lv_obj_set_style_text_font(widget->obj, &font_12_roboto_extra_bold,
                             LV_PART_MAIN | LV_STATE_DEFAULT);

  sys_slist_append(&widgets, &widget->node);

  widget_layer_status_init();
  return 0;
}

lv_obj_t *zmk_widget_layer_status_obj(struct zmk_widget_layer_status *widget) {
  return widget->obj;
}
