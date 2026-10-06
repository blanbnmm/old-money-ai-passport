// main/feature_diary.c —— 老钱日记功能
#include <esp_random.h>
#include "feature_diary.h"
#include "old_money_diary.h"
#include "bsp_display.h"
#include "ui_pixel.h"
#include "lvgl.h"

#include <stdio.h>

static lv_obj_t *s_scr;
static lv_obj_t *s_title;
static lv_obj_t *s_body;
static lv_obj_t *s_page_lbl;
static int s_idx;

void feature_diary_enter(void) {
    s_idx = 0;
    s_scr = ui_pixel_screen_create("老钱日记");

    s_title = ui_pixel_label(s_scr, "📖 老钱日记", &laoqian_font_16, UI_INK);
    lv_obj_align(s_title, LV_ALIGN_TOP_LEFT, 12, 8);

    s_body = lv_label_create(s_scr);
    lv_obj_set_style_text_font(s_body, &laoqian_font_16, 0);
    lv_obj_set_style_text_color(s_body, lv_color_hex(UI_INK), 0);
    lv_obj_set_width(s_body, 200);
    lv_obj_set_style_text_align(s_body, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(s_body, LV_ALIGN_CENTER, 0, -10);

    s_page_lbl = ui_pixel_label(s_scr, "1/30", &laoqian_font_16, UI_SKY_DARK);
    lv_obj_align(s_page_lbl, LV_ALIGN_BOTTOM_LEFT, 12, -10);

    lv_label_set_text(s_body, diary_get(s_idx));
    lv_label_set_text_fmt(s_page_lbl, "%d/%d", s_idx + 1, diary_count());

    lv_screen_load(s_scr);
}

void feature_diary_exit(void) {
    if (s_scr) { lv_obj_delete(s_scr); s_scr = NULL; }
    s_title = s_body = s_page_lbl = NULL;
}

void feature_diary_key(bsp_btn_t btn, bsp_btn_ev_t ev) {
    if (ev != BSP_BTN_CLICK && ev != BSP_BTN_LONG) return;
    if (!bsp_lvgl_lock(250)) return;

    int n = diary_count();
    if (btn == BSP_BTN_UP && ev == BSP_BTN_CLICK) {
        s_idx = (s_idx - 1 + n) % n;
    } else if (btn == BSP_BTN_DOWN && ev == BSP_BTN_CLICK) {
        s_idx = (s_idx + 1) % n;
    } else if (btn == BSP_BTN_OK && ev == BSP_BTN_CLICK) {
        s_idx = (s_idx + (esp_random() % 7) + 1) % n;
    } else if (btn == BSP_BTN_OK && ev == BSP_BTN_LONG) {
        feature_diary_exit();
        bsp_lvgl_unlock();
        extern void menu_enter(void);
        menu_enter();
        return;
    } else {
        bsp_lvgl_unlock();
        return;
    }

    lv_label_set_text(s_body, diary_get(s_idx));
    lv_label_set_text_fmt(s_page_lbl, "%d/%d", s_idx + 1, n);

    bsp_lvgl_unlock();
}