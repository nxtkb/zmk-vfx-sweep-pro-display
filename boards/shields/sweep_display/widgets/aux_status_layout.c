/* SPDX-License-Identifier: MIT */

#include <errno.h>

#include <zephyr/kernel.h>

#include "aux_status_layout.h"
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

#define AUX_STATUS_SINGLE_Y 116
#define AUX_STATUS_FIRST_Y 112
#define AUX_STATUS_SECOND_Y 124
#define AUX_STATUS_LAYER_UPDATE_DELAY_MS 50

static struct zmk_widget_aux_status_layout *active_layout;

static void set_component_visible(struct zmk_aux_status_component *component,
                                  bool visible, int16_t y) {
  if (component->obj == NULL) {
    return;
  }

  if (visible) {
    lv_obj_align(component->obj, LV_ALIGN_TOP_MID, 0, y);
    lv_obj_clear_flag(component->obj, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(component->obj, LV_OBJ_FLAG_HIDDEN);
  }
}

static void default_visibility(bool *show_codex, bool *show_trackpad) {
  *show_codex = IS_ENABLED(CONFIG_CUSTOM_AUX_STATUS_DEFAULT_CODEX) ||
                IS_ENABLED(CONFIG_CUSTOM_AUX_STATUS_DEFAULT_BOTH);
  *show_trackpad = IS_ENABLED(CONFIG_CUSTOM_AUX_STATUS_DEFAULT_TRACKPAD) ||
                   IS_ENABLED(CONFIG_CUSTOM_AUX_STATUS_DEFAULT_BOTH);
}

void zmk_widget_aux_status_layout_refresh(
    struct zmk_widget_aux_status_layout *layout) {
  if (layout == NULL) {
    return;
  }

  zmk_keymap_layer_index_t layer = zmk_keymap_highest_layer_active();
  bool show_codex = false;
  bool show_trackpad = false;

  if (layer == CONFIG_CUSTOM_AUX_STATUS_CODEX_LAYER_INDEX) {
    show_codex = true;
  } else if (layer == CONFIG_CUSTOM_AUX_STATUS_MOUSE_LAYER_INDEX) {
    show_trackpad = true;
  } else if (layer <= CONFIG_CUSTOM_AUX_STATUS_BASE_LAYER_MAX) {
    default_visibility(&show_codex, &show_trackpad);
  }

  show_codex = show_codex &&
               layout->components[ZMK_AUX_STATUS_COMPONENT_CODEX].obj != NULL;
  show_trackpad =
      show_trackpad &&
      layout->components[ZMK_AUX_STATUS_COMPONENT_TRACKPAD].obj != NULL;

  if (show_codex && show_trackpad) {
    set_component_visible(
        &layout->components[ZMK_AUX_STATUS_COMPONENT_CODEX], true,
        AUX_STATUS_FIRST_Y);
    set_component_visible(
        &layout->components[ZMK_AUX_STATUS_COMPONENT_TRACKPAD], true,
        AUX_STATUS_SECOND_Y);
  } else {
    set_component_visible(
        &layout->components[ZMK_AUX_STATUS_COMPONENT_CODEX], show_codex,
        AUX_STATUS_SINGLE_Y);
    set_component_visible(
        &layout->components[ZMK_AUX_STATUS_COMPONENT_TRACKPAD], show_trackpad,
        AUX_STATUS_SINGLE_Y);
  }
}

static void aux_status_layout_work_cb(struct k_work *work) {
  ARG_UNUSED(work);
  zmk_widget_aux_status_layout_refresh(active_layout);
}

K_WORK_DELAYABLE_DEFINE(aux_status_layout_work, aux_status_layout_work_cb);

static int aux_status_layout_event_cb(const zmk_event_t *eh) {
  ARG_UNUSED(eh);
  if (active_layout != NULL && zmk_display_is_initialized()) {
    k_work_reschedule_for_queue(zmk_display_work_q(), &aux_status_layout_work,
                                K_MSEC(AUX_STATUS_LAYER_UPDATE_DELAY_MS));
  }
  return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(widget_aux_status_layout, aux_status_layout_event_cb);
ZMK_SUBSCRIPTION(widget_aux_status_layout, zmk_layer_state_changed);

int zmk_widget_aux_status_layout_init(
    struct zmk_widget_aux_status_layout *layout) {
  if (layout == NULL) {
    return -EINVAL;
  }

  active_layout = layout;
  return 0;
}

int zmk_widget_aux_status_layout_register(
    struct zmk_widget_aux_status_layout *layout,
    enum zmk_aux_status_component_id id, lv_obj_t *obj) {
  if (layout == NULL || obj == NULL || id >= ZMK_AUX_STATUS_COMPONENT_COUNT) {
    return -EINVAL;
  }

  layout->components[id].obj = obj;
  lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  return 0;
}
