/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "output_status.h"
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/usb.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

LV_FONT_DECLARE(lv_custom_symbol);

struct output_status_state {
  struct zmk_endpoint_instance selected_endpoint;
  bool usb_present;
  bool active_profile_connected;
  bool active_profile_bonded;
};

static struct output_status_state get_state(const zmk_event_t *_eh) {
  return (struct output_status_state){
      .selected_endpoint = zmk_endpoint_get_selected(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
      .usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
      .active_profile_connected = zmk_ble_active_profile_is_connected(),
      .active_profile_bonded = !zmk_ble_active_profile_is_open()};
}

static void set_status_symbol(struct zmk_widget_output_status *widget,
                              struct output_status_state state) {
  char marker_text[8] = {};
  int idx = zmk_ble_active_profile_index();
  uint8_t profile = (uint8_t)(idx + 1);
  int16_t ble_x = state.usb_present ? 19 : 0;
  int16_t marker_x = ble_x + 12;
  bool usb_selected = state.selected_endpoint.transport == ZMK_TRANSPORT_USB;
  int16_t underline_x = usb_selected ? 2 : ble_x + 2;
  int16_t underline_y = usb_selected ? 0 : 3;

  if (!state.active_profile_bonded) {
    snprintf(marker_text, sizeof(marker_text), "-\n%u", profile);
  } else {
    snprintf(marker_text, sizeof(marker_text), "%s\n%u",
             state.active_profile_connected ? LV_SYMBOL_OK : LV_SYMBOL_CLOSE,
             profile);
  }

  if (!widget->status_initialized ||
      state.usb_present != widget->last_usb_present) {
    if (state.usb_present) {
      lv_obj_clear_flag(widget->usb_label, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(widget->usb_label, LV_OBJ_FLAG_HIDDEN);
    }
    widget->last_usb_present = state.usb_present;
  }

  if (!widget->status_initialized || ble_x != widget->last_ble_x) {
    lv_obj_align(widget->ble_label, LV_ALIGN_LEFT_MID, ble_x, 0);
    widget->last_ble_x = ble_x;
  }

  if (!widget->status_initialized || marker_x != widget->last_marker_x) {
    lv_obj_align(widget->marker_label, LV_ALIGN_TOP_LEFT, marker_x, -1);
    widget->last_marker_x = marker_x;
  }

  if (!widget->status_initialized || underline_x != widget->last_underline_x ||
      underline_y != widget->last_underline_y) {
    lv_obj_align(widget->endpoint_underline_label, LV_ALIGN_BOTTOM_LEFT,
                 underline_x, underline_y);
    widget->last_underline_x = underline_x;
    widget->last_underline_y = underline_y;
  }

  if (strcmp(lv_label_get_text(widget->marker_label), marker_text) != 0) {
    lv_label_set_text(widget->marker_label, marker_text);
  }

  widget->status_initialized = true;
}

static void output_status_update_cb(struct output_status_state state) {
  struct zmk_widget_output_status *widget;
  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    set_status_symbol(widget, state);
  }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
// We don't get an endpoint changed event when the active profile
// connects/disconnects but there wasn't another endpoint to switch from/to, so
// update on BLE events too.
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

int zmk_widget_output_status_init(struct zmk_widget_output_status *widget,
                                  lv_obj_t *parent) {
  widget->obj = lv_obj_create(parent);
  lv_obj_remove_style_all(widget->obj);
  lv_obj_set_size(widget->obj, 42, 24);
  lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);

  widget->usb_label = lv_label_create(widget->obj);
  lv_label_set_text(widget->usb_label, LV_SYMBOL_USB);
  lv_obj_set_style_text_font(widget->usb_label, &lv_custom_symbol,
                             LV_PART_MAIN);
  lv_obj_align(widget->usb_label, LV_ALIGN_LEFT_MID, 0, 0);

  widget->ble_label = lv_label_create(widget->obj);
  lv_label_set_text(widget->ble_label, LV_SYMBOL_BLUETOOTH);
  lv_obj_set_style_text_font(widget->ble_label, &lv_custom_symbol,
                             LV_PART_MAIN);
  lv_obj_align(widget->ble_label, LV_ALIGN_LEFT_MID, 0, 0);

  widget->marker_label = lv_label_create(widget->obj);
  lv_obj_set_style_text_font(widget->marker_label, &lv_font_montserrat_10,
                             LV_PART_MAIN);
  lv_obj_set_style_text_line_space(widget->marker_label, 0, LV_PART_MAIN);
  lv_obj_align(widget->marker_label, LV_ALIGN_TOP_LEFT, 10, -1);

  widget->endpoint_underline_label = lv_label_create(widget->obj);
  lv_label_set_text(widget->endpoint_underline_label, LV_SYMBOL_MINUS);
  lv_obj_set_style_text_font(widget->endpoint_underline_label,
                             &lv_font_montserrat_14, LV_PART_MAIN);
  lv_label_set_long_mode(widget->endpoint_underline_label, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(widget->endpoint_underline_label, 14, 12);
  lv_obj_align(widget->endpoint_underline_label, LV_ALIGN_BOTTOM_LEFT, 2, 0);

  sys_slist_append(&widgets, &widget->node);

  widget_output_status_init();
  return 0;
}

lv_obj_t *
zmk_widget_output_status_obj(struct zmk_widget_output_status *widget) {
  return widget->obj;
}
