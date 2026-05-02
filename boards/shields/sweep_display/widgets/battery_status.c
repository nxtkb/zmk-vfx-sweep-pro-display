/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "battery_status.h"
#include <zmk/battery.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/split/central.h>
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/split/transport/central.h>
#include <zmk/split/transport/types.h>
#endif
#include <zmk/usb.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
extern const struct zmk_split_transport_central *active_transport;
#endif

static inline const char *get_battery_symbol(uint8_t level) {
  if (level > 95) {
    return LV_SYMBOL_BATTERY_FULL;
  } else if (level > 65) {
    return LV_SYMBOL_BATTERY_3;
  } else if (level > 35) {
    return LV_SYMBOL_BATTERY_2;
  } else if (level > 5) {
    return LV_SYMBOL_BATTERY_1;
  } else {
    return LV_SYMBOL_BATTERY_EMPTY;
  }
}

struct battery_status_state {
  bool is_peripheral; // 是否是外设电池状态
  uint8_t source;
  uint8_t level;
  bool valid;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
  bool usb_present;
#endif
};

#define ZMK_SPLIT_CENTRAL_COUNT 1

struct battery_status_state battery_objects[ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT +
                                            ZMK_SPLIT_CENTRAL_COUNT] = {
    [0] = {.is_peripheral = false, .source = 0, .level = 0, .valid = false},
    [1] = {.is_peripheral = true, .source = 0, .level = 0, .valid = false}};

static bool battery_status_peripherals_connected(void) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  if (active_transport == NULL || active_transport->api == NULL ||
      active_transport->api->get_status == NULL) {
    return false;
  }

  struct zmk_split_transport_status status =
      active_transport->api->get_status();
  return status.enabled &&
         status.connections !=
             ZMK_SPLIT_TRANSPORT_CONNECTIONS_STATUS_DISCONNECTED;
#else
  return false;
#endif
}

static void battery_status_refresh_peripherals(void) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
  if (!battery_status_peripherals_connected()) {
    for (uint8_t source = 0; source < ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT;
         source++) {
      battery_objects[source + ZMK_SPLIT_CENTRAL_COUNT] =
          (struct battery_status_state){
              .is_peripheral = true,
              .source = source,
              .level = 0,
              .valid = false,
          };
    }
    return;
  }

  for (uint8_t source = 0; source < ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT;
       source++) {
    uint8_t level;

    if (zmk_split_central_get_peripheral_battery_level(source, &level) == 0 &&
        level > 0) {
      battery_objects[source + ZMK_SPLIT_CENTRAL_COUNT] =
          (struct battery_status_state){
              .is_peripheral = true,
              .source = source,
              .level = level,
              .valid = true,
          };
    }
  }
#endif
}

static void append_battery_text(char *text, size_t size, size_t *len,
                                const char *fmt, ...) {
  if (*len >= size) {
    return;
  }

  va_list args;
  va_start(args, fmt);
  int written = vsnprintf(text + *len, size - *len, fmt, args);
  va_end(args);

  if (written < 0) {
    return;
  }

  *len += MIN((size_t)written, size - *len - 1);
}

static void set_battery_symbol(lv_obj_t *label,
                               struct battery_status_state state) {
  char text[64] = {};
  size_t len = 0;
  bool wrote_item = false;

  for (int i = 0;
       i < ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT + ZMK_SPLIT_CENTRAL_COUNT; i++) {
    state = battery_objects[i];
    if (!state.valid) {
      continue;
    }

    uint8_t level = state.level;

    if (wrote_item) {
      append_battery_text(text, sizeof(text), &len, "\n");
    }
    wrote_item = true;

    append_battery_text(text, sizeof(text), &len, "%s %u%%",
                        get_battery_symbol(level), level);

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    if (!state.is_peripheral && state.usb_present && level > 0) {
      append_battery_text(text, sizeof(text), &len, " %s", LV_SYMBOL_CHARGE);
    }
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
  }

  lv_label_set_text(label, text);
}

void battery_status_update_cb(struct battery_status_state state) {
  int idx = state.is_peripheral ? state.source + ZMK_SPLIT_CENTRAL_COUNT : 0;
  if (idx < 0 ||
      idx >= ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT + ZMK_SPLIT_CENTRAL_COUNT) {
    return;
  }

  battery_objects[idx] = state; // 存储最新状态

  struct zmk_widget_battery_status *widget;
  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    set_battery_symbol(widget->obj, state);
  }
}

static struct battery_status_state
peripheral_battery_status_get_state(const zmk_event_t *eh) {
  const struct zmk_peripheral_battery_state_changed *ev =
      as_zmk_peripheral_battery_state_changed(eh);

  return (struct battery_status_state){
      .is_peripheral = true,
      .source = ev->source,
      .level = ev->state_of_charge,
      .valid = true,
  };
}

static struct battery_status_state
central_battery_status_get_state(const zmk_event_t *eh) {
  const struct zmk_battery_state_changed *ev =
      eh == NULL ? NULL : as_zmk_battery_state_changed(eh);
  uint8_t level =
      (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge();
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
  bool usb_present = zmk_usb_is_powered();
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

  return (struct battery_status_state){
      .is_peripheral = false,
      .source = 0,
      .level = level,
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
      .valid = (ev != NULL) || level > 0,
      .usb_present = usb_present,
#else
      .valid = (ev != NULL) || level > 0,
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
  };
}

static struct battery_status_state
battery_status_get_state(const zmk_event_t *eh) {
  if (eh != NULL && as_zmk_peripheral_battery_state_changed(eh) != NULL) {
    return peripheral_battery_status_get_state(eh);
  } else {
    battery_status_refresh_peripherals();
    return central_battery_status_get_state(eh);
  }
}

static void battery_status_refresh_widgets(void) {
  battery_status_refresh_peripherals();
  battery_objects[0] = central_battery_status_get_state(NULL);

  struct zmk_widget_battery_status *widget;
  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    set_battery_symbol(widget->obj, battery_objects[0]);
  }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);

ZMK_SUBSCRIPTION(widget_battery_status, zmk_peripheral_battery_state_changed);

ZMK_SUBSCRIPTION(widget_battery_status, zmk_split_peripheral_status_changed);

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

int zmk_widget_battery_status_init(struct zmk_widget_battery_status *widget,
                                   lv_obj_t *parent) {
  widget->obj = lv_label_create(parent);
  lv_obj_set_style_text_font(widget->obj, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_set_style_text_line_space(widget->obj, -1, LV_PART_MAIN);
  sys_slist_append(&widgets, &widget->node);
  widget_battery_status_init();
  battery_status_refresh_widgets();
  return 0;
}

lv_obj_t *
zmk_widget_battery_status_obj(struct zmk_widget_battery_status *widget) {
  return widget->obj;
}
