#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

struct zmk_widget_hid_indicators_status {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *cells[5];
    lv_obj_t *cell_shadows[4];
    lv_obj_t *active_lines[5];
    bool status_initialized;
    uint8_t last_active_mask;
};

int zmk_widget_hid_indicators_status_init(struct zmk_widget_hid_indicators_status *widget,
                                          lv_obj_t *parent);
lv_obj_t *zmk_widget_hid_indicators_status_obj(struct zmk_widget_hid_indicators_status *widget);
