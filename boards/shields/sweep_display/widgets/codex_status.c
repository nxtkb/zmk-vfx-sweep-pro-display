/* SPDX-License-Identifier: MIT */

#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>

#include "codex_status.h"
#include <nxtkb/codex/events.h>
#include <nxtkb/codex/state.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>

#define CODEX_STATUS_WIDTH 152
#define CODEX_STATUS_HEIGHT 12
#define CODEX_STATUS_UPDATE_DELAY_MS 100

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct codex_status_state {
    struct nxtkb_codex_agent_snapshot agents;
};

static char state_symbol(enum nxtkb_codex_agent_state state)
{
    switch (state) {
    case NXTKB_CODEX_AGENT_OFF:
        return '-';
    case NXTKB_CODEX_AGENT_IDLE:
        return 'I';
    case NXTKB_CODEX_AGENT_WORKING:
        return 'W';
    case NXTKB_CODEX_AGENT_UNREAD:
        return 'N';
    case NXTKB_CODEX_AGENT_AWAITING_INPUT:
        return '?';
    case NXTKB_CODEX_AGENT_ERROR:
        return '!';
    case NXTKB_CODEX_AGENT_UNKNOWN:
    default:
        return 'U';
    }
}

static struct codex_status_state codex_status_get_state(void)
{
    struct codex_status_state state = {};
    nxtkb_codex_state_get(&state.agents);
    return state;
}

static void codex_status_update_cb(struct codex_status_state state)
{
    char text[32];
    int length = snprintf(text, sizeof(text), "A%c  1%c 2%c 3%c 4%c 5%c 6%c",
                          state.agents.active_slot < 0 ? '-' : '1' + state.agents.active_slot,
                          state_symbol(state.agents.slots[0].state),
                          state_symbol(state.agents.slots[1].state),
                          state_symbol(state.agents.slots[2].state),
                          state_symbol(state.agents.slots[3].state),
                          state_symbol(state.agents.slots[4].state),
                          state_symbol(state.agents.slots[5].state));
    if (length <= 0 || length >= sizeof(text)) {
        return;
    }

    struct zmk_widget_codex_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        if (strcmp(lv_label_get_text(widget->label), text) != 0) {
            lv_label_set_text(widget->label, text);
        }
    }
}

static void codex_status_work_cb(struct k_work *work)
{
    ARG_UNUSED(work);
    codex_status_update_cb(codex_status_get_state());
}

K_WORK_DELAYABLE_DEFINE(codex_status_work, codex_status_work_cb);

static int codex_status_event_cb(const zmk_event_t *eh)
{
    ARG_UNUSED(eh);
    if (zmk_display_is_initialized()) {
        k_work_reschedule_for_queue(zmk_display_work_q(), &codex_status_work,
                                    K_MSEC(CODEX_STATUS_UPDATE_DELAY_MS));
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(widget_codex_status, codex_status_event_cb);
ZMK_SUBSCRIPTION(widget_codex_status, nxtkb_codex_state_changed);

int zmk_widget_codex_status_init(struct zmk_widget_codex_status *widget, lv_obj_t *parent)
{
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, CODEX_STATUS_WIDTH, CODEX_STATUS_HEIGHT);
    lv_obj_set_style_bg_color(widget->obj, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);

    widget->label = lv_label_create(widget->obj);
    lv_obj_set_size(widget->label, CODEX_STATUS_WIDTH, CODEX_STATUS_HEIGHT);
    lv_obj_set_pos(widget->label, 0, 0);
    lv_obj_set_style_text_font(widget->label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_opa(widget->label, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(widget->label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->label, LV_OPA_COVER, LV_PART_MAIN);
    lv_label_set_long_mode(widget->label, LV_LABEL_LONG_CLIP);
    lv_label_set_text(widget->label, "A-  1- 2- 3- 4- 5- 6-");

    sys_slist_append(&widgets, &widget->node);
    codex_status_update_cb(codex_status_get_state());
    return 0;
}

lv_obj_t *zmk_widget_codex_status_obj(struct zmk_widget_codex_status *widget)
{
    return widget->obj;
}
