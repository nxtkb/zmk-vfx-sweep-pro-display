
/*
 *
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "hid_indicators_status.h"
#include <zmk/display.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>
#include <zmk/hid_indicators.h>
#include <dt-bindings/zmk/modifiers.h>

#define ZMK_LED_NUMLOCK_BIT BIT(0)
#define ZMK_LED_CAPSLOCK_BIT BIT(1)
#define ZMK_LED_SCROLLLOCK_BIT BIT(2)

#define MOD_ALT_MASK (MOD_LALT | MOD_RALT)
#define MOD_CTRL_MASK (MOD_LCTL | MOD_RCTL)
#define MOD_CMD_MASK (MOD_LGUI | MOD_RGUI)
#define MOD_SHIFT_MASK (MOD_LSFT | MOD_RSFT)

#define LV_SYMBOL_MOD_ALT "\xE2\x8C\xA5"
#define LV_SYMBOL_MOD_CTRL "^"
#define LV_SYMBOL_MOD_CMD "\xE2\x8C\x98"
#define LV_SYMBOL_MOD_SHIFT "\xE2\x87\xA7"

enum hid_indicator_cell {
  HID_INDICATOR_ALT,
  HID_INDICATOR_CTRL,
  HID_INDICATOR_CMD,
  HID_INDICATOR_SHIFT,
  HID_INDICATOR_CAPS,
  HID_INDICATOR_COUNT,
};

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);
LV_FONT_DECLARE(lv_mac_modifier_symbol);

struct hid_indicators_status_state {
  zmk_hid_indicators_t flags; // HID Indicator Status Bit Mask
  zmk_mod_flags_t mods;
};

static uint8_t hid_indicators_active_mask(zmk_mod_flags_t mods,
                                          zmk_hid_indicators_t flags) {
  uint8_t active_mask = 0;

  if (mods & MOD_ALT_MASK) {
    active_mask |= BIT(HID_INDICATOR_ALT);
  }
  if (mods & MOD_CTRL_MASK) {
    active_mask |= BIT(HID_INDICATOR_CTRL);
  }
  if (mods & MOD_CMD_MASK) {
    active_mask |= BIT(HID_INDICATOR_CMD);
  }
  if (mods & MOD_SHIFT_MASK) {
    active_mask |= BIT(HID_INDICATOR_SHIFT);
  }
  if (flags & ZMK_LED_CAPSLOCK_BIT) {
    active_mask |= BIT(HID_INDICATOR_CAPS);
  }

  return active_mask;
}

static void set_indicator_cell(lv_obj_t *label, const char *name) {
  if (strcmp(lv_label_get_text(label), name) != 0) {
    lv_label_set_text(label, name);
  }
}

static void set_indicator_pair(lv_obj_t *label, lv_obj_t *shadow,
                               const char *name) {
  set_indicator_cell(label, name);
  set_indicator_cell(shadow, name);
}

static void set_active_line(lv_obj_t *line, bool active) {
  if (active) {
    lv_obj_clear_flag(line, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(line, LV_OBJ_FLAG_HIDDEN);
  }
}

static void set_caps_cell(lv_obj_t *label) {
  if (strcmp(lv_label_get_text(label), "A") != 0) {
    lv_label_set_text(label, "A");
  }
}

static void set_hid_indicators_symbol(
    struct zmk_widget_hid_indicators_status *widget,
    struct hid_indicators_status_state state) {
  static const char *cell_names[HID_INDICATOR_CAPS] = {
      LV_SYMBOL_MOD_ALT, LV_SYMBOL_MOD_CTRL, LV_SYMBOL_MOD_CMD,
      LV_SYMBOL_MOD_SHIFT};
  uint8_t active_mask = hid_indicators_active_mask(state.mods, state.flags);
  bool modifiers_invalidated = false;

  if (!widget->status_initialized ||
      widget->last_active_mask != active_mask) {
    for (uint8_t i = 0; i < HID_INDICATOR_COUNT; i++) {
      bool was_active = widget->last_active_mask & BIT(i);
      bool is_active = active_mask & BIT(i);

      if (!widget->status_initialized || was_active != is_active) {
        if (i < HID_INDICATOR_CAPS) {
          set_indicator_pair(widget->cells[i], widget->cell_shadows[i],
                             cell_names[i]);
          modifiers_invalidated = true;
        } else {
          set_caps_cell(widget->cells[i]);
        }
        set_active_line(widget->active_lines[i], is_active);
      }
    }
    widget->last_active_mask = active_mask;
  }

  if (modifiers_invalidated) {
    lv_obj_invalidate(widget->obj);
  }

  widget->status_initialized = true;
}

static void
hid_indicators_status_update_cb(struct hid_indicators_status_state state) {
  struct zmk_widget_hid_indicators_status *widget;
  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    set_hid_indicators_symbol(widget, state);
  }
}

static zmk_mod_flags_t hid_indicators_current_mods(const zmk_event_t *eh) {
  zmk_mod_flags_t mods = zmk_hid_get_explicit_mods();
  const struct zmk_keycode_state_changed *ev =
      eh == NULL ? NULL : as_zmk_keycode_state_changed(eh);

  if (ev != NULL && is_mod(ev->usage_page, ev->keycode)) {
    zmk_mod_flags_t mod = BIT(ev->keycode - HID_USAGE_KEY_KEYBOARD_LEFTCONTROL);

    if (ev->state) {
      mods |= mod;
    } else {
      mods &= ~mod;
    }
  }

  return mods;
}

static struct hid_indicators_status_state
hid_indicators_status_get_state(const zmk_event_t *eh) {
  return (struct hid_indicators_status_state){
      .flags = zmk_hid_indicators_get_current_profile(),
      .mods = hid_indicators_current_mods(eh)};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_hid_indicators_status,
                            struct hid_indicators_status_state,
                            hid_indicators_status_update_cb,
                            hid_indicators_status_get_state)

ZMK_SUBSCRIPTION(widget_hid_indicators_status, zmk_hid_indicators_changed);
ZMK_SUBSCRIPTION(widget_hid_indicators_status, zmk_keycode_state_changed);

int zmk_widget_hid_indicators_status_init(
    struct zmk_widget_hid_indicators_status *widget, lv_obj_t *parent) {
  static const int16_t cell_x[HID_INDICATOR_COUNT] = {14, 35, 50, 67, 0};
  static const int16_t cell_y[HID_INDICATOR_COUNT] = {8, 8, 8, 8, 7};
  static const int16_t cell_width[HID_INDICATOR_COUNT] = {18, 14, 16, 16, 10};
  static const int16_t line_x[HID_INDICATOR_COUNT] = {16, 36, 52, 69, 1};
  static const int16_t line_width[HID_INDICATOR_COUNT] = {14, 14, 14, 14, 8};

  widget->obj = lv_obj_create(parent);
  lv_obj_remove_style_all(widget->obj);
  lv_obj_set_size(widget->obj, 83, 23);
  lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);

  for (uint8_t i = 0; i < HID_INDICATOR_COUNT; i++) {
    widget->cells[i] = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->cells[i],
                               i == HID_INDICATOR_CAPS
                                   ? &lv_font_montserrat_14
                                   : &lv_mac_modifier_symbol,
                               LV_PART_MAIN);
    lv_label_set_long_mode(widget->cells[i], LV_LABEL_LONG_CLIP);
    lv_obj_set_size(widget->cells[i], cell_width[i], 13);
    lv_obj_align(widget->cells[i], LV_ALIGN_TOP_LEFT, cell_x[i], cell_y[i]);

    if (i < HID_INDICATOR_CAPS) {
      widget->cell_shadows[i] = lv_label_create(widget->obj);
      lv_obj_set_style_text_font(widget->cell_shadows[i],
                                 &lv_mac_modifier_symbol, LV_PART_MAIN);
      lv_label_set_long_mode(widget->cell_shadows[i], LV_LABEL_LONG_CLIP);
      lv_obj_set_size(widget->cell_shadows[i], cell_width[i], 13);
      lv_obj_align(widget->cell_shadows[i], LV_ALIGN_TOP_LEFT, cell_x[i] + 1,
                   cell_y[i]);
    }
  }

  for (uint8_t i = 0; i < HID_INDICATOR_COUNT; i++) {
    widget->active_lines[i] = lv_label_create(widget->obj);
    lv_label_set_text(widget->active_lines[i], LV_SYMBOL_MINUS);
    lv_obj_set_style_text_font(widget->active_lines[i], &lv_font_montserrat_14,
                               LV_PART_MAIN);
    lv_label_set_long_mode(widget->active_lines[i], LV_LABEL_LONG_CLIP);
    lv_obj_set_size(widget->active_lines[i], line_width[i], 12);
    lv_obj_align(widget->active_lines[i], LV_ALIGN_TOP_LEFT, line_x[i], -2);
    lv_obj_add_flag(widget->active_lines[i], LV_OBJ_FLAG_HIDDEN);
  }

  widget->status_initialized = false;
  widget->last_active_mask = 0;

  sys_slist_append(&widgets, &widget->node);
  widget_hid_indicators_status_init();
  return 0;
}

lv_obj_t *zmk_widget_hid_indicators_status_obj(
    struct zmk_widget_hid_indicators_status *widget) {
  return widget->obj;
}
