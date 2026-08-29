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

#define ZMK_SPLIT_CENTRAL_COUNT 1
#define BATTERY_STATUS_ITEM_COUNT                                                         \
  (ZMK_SPLIT_CENTRAL_COUNT + ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT)

struct battery_status_item {
  uint8_t level;
  bool valid;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
  bool usb_present;
#endif
};

struct battery_status_state {
  struct battery_status_item items[BATTERY_STATUS_ITEM_COUNT];
};

/*
 * The display listener serializes calls to battery_status_get_state(). Keeping
 * the complete model here means coalesced display work always renders the
 * latest state of both halves instead of losing one of two adjacent events.
 */
static struct battery_status_state pending_state;

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

static bool battery_status_peripherals_connected(void) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  if (active_transport == NULL || active_transport->api == NULL ||
      active_transport->api->get_status == NULL) {
    return false;
  }

  struct zmk_split_transport_status status = active_transport->api->get_status();
  return status.enabled &&
         status.connections != ZMK_SPLIT_TRANSPORT_CONNECTIONS_STATUS_DISCONNECTED;
#else
  return false;
#endif
}

static void battery_status_refresh_peripherals(void) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
  if (!battery_status_peripherals_connected()) {
    for (uint8_t source = 0; source < ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT; source++) {
      pending_state.items[source + ZMK_SPLIT_CENTRAL_COUNT] =
          (struct battery_status_item){};
    }
    return;
  }

  for (uint8_t source = 0; source < ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT; source++) {
    uint8_t level;
    int err = zmk_split_central_get_peripheral_battery_level(source, &level);
    struct battery_status_item *item =
        &pending_state.items[source + ZMK_SPLIT_CENTRAL_COUNT];

    if (err == 0 && level > 0) {
      item->level = level;
      item->valid = true;
    } else {
      item->level = 0;
      item->valid = false;
    }
  }
#endif
}

static void append_battery_text(char *text, size_t text_size, size_t *len,
                                const char *format, ...) {
  if (*len >= text_size) {
    return;
  }

  va_list args;
  va_start(args, format);
  int written = vsnprintf(text + *len, text_size - *len, format, args);
  va_end(args);

  if (written > 0) {
    *len += MIN((size_t)written, text_size - *len - 1);
  }
}

static void set_battery_symbol(lv_obj_t *label, struct battery_status_state state) {
  char text[64] = {};
  size_t len = 0;
  bool wrote_item = false;

  for (int i = 0; i < BATTERY_STATUS_ITEM_COUNT; i++) {
    struct battery_status_item item = state.items[i];
    if (!item.valid) {
      continue;
    }

    if (wrote_item) {
      append_battery_text(text, sizeof(text), &len, "\n");
    }
    wrote_item = true;
    append_battery_text(text, sizeof(text), &len, "%s %u%%",
                        get_battery_symbol(item.level), item.level);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    if (item.usb_present) {
      append_battery_text(text, sizeof(text), &len, " %s", LV_SYMBOL_CHARGE);
    }
#endif
  }

  lv_label_set_text(label, text);
}

static void battery_status_update_cb(struct battery_status_state state) {
  struct zmk_widget_battery_status *widget;
  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    set_battery_symbol(widget->obj, state);
  }
}

static void battery_status_update_central(const zmk_event_t *eh) {
  const struct zmk_battery_state_changed *battery_ev =
      eh == NULL ? NULL : as_zmk_battery_state_changed(eh);
  uint8_t level = battery_ev != NULL ? battery_ev->state_of_charge
                                     : zmk_battery_state_of_charge();
  struct battery_status_item *item = &pending_state.items[0];

  item->level = level;
  item->valid = level > 0;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
  item->usb_present = zmk_usb_is_powered();
#endif
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
  const struct zmk_peripheral_battery_state_changed *peripheral_battery_ev =
      eh == NULL ? NULL : as_zmk_peripheral_battery_state_changed(eh);

  if (peripheral_battery_ev != NULL &&
      peripheral_battery_ev->source < ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT) {
    struct battery_status_item *item =
        &pending_state.items[peripheral_battery_ev->source + ZMK_SPLIT_CENTRAL_COUNT];
    item->level = peripheral_battery_ev->state_of_charge;
    item->valid = peripheral_battery_ev->state_of_charge > 0;
#if IS_ENABLED(CONFIG_ZMK_SPLIT_PERIPHERAL_USB_POWER)
  } else if (eh != NULL && as_zmk_peripheral_usb_conn_state_changed(eh) != NULL) {
    const struct zmk_peripheral_usb_conn_state_changed *usb_ev =
        as_zmk_peripheral_usb_conn_state_changed(eh);
    if (usb_ev->source < ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT) {
      pending_state.items[usb_ev->source + ZMK_SPLIT_CENTRAL_COUNT].usb_present =
          usb_ev->powered;
      LOG_DBG("Display USB power update: source=%u powered=%u", usb_ev->source,
              usb_ev->powered);
    }
#endif
  } else {
    battery_status_refresh_peripherals();
    battery_status_update_central(eh);
  }

  return pending_state;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(widget_battery_status, zmk_peripheral_battery_state_changed);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_PERIPHERAL_USB_POWER)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_peripheral_usb_conn_state_changed);
#endif

ZMK_SUBSCRIPTION(widget_battery_status, zmk_split_peripheral_status_changed);

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif

int zmk_widget_battery_status_init(struct zmk_widget_battery_status *widget,
                                   lv_obj_t *parent) {
  widget->obj = lv_label_create(parent);
  lv_obj_set_style_text_font(widget->obj, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_set_style_text_line_space(widget->obj, -1, LV_PART_MAIN);

  sys_slist_append(&widgets, &widget->node);
  widget_battery_status_init();
  return 0;
}

lv_obj_t *zmk_widget_battery_status_obj(struct zmk_widget_battery_status *widget) {
  return widget->obj;
}
