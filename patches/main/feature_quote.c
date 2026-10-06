// main/feature_quote.c —— 每日一句功能
#include <esp_random.h>
#include "feature_quote.h"
#include <esp_random.h>
#include "old_money_quotes.h"
#include <esp_random.h>
#include "bsp_display.h"
#include <esp_random.h>
#include "ui_pixel.h"
#include <esp_random.h>
#include "lvgl.h"

#include <stdio.h>

static lv_obj_t *s_scr;
static lv_obj_t *s_title;
static lv_obj_t *s_body;
static lv_obj_t *s_page_lbl;
static int s_idx;

void feature_quote_enter(void) {
    s_idx = 0;
    s_scr = ui_pixel_screen_create("每日一句");

    s_title = ui_pixel_label(s_scr, "💬 每日一句", &lv_font_montserrat_14, UI_INK);
    lv_obj_align(s_title, LV_ALIGN_TOP_LEFT, 12, 8);

    s_body = lv_label_create(s_scr);
    lv_obj_set_style_text_font(s_body, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_body, lv_color_hex(UI_INK), 0);
    lv_obj_set_width(s_body, 200);
    lv_obj_set_style_text_align(s_body, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(s_body, LV_ALIGN_CENTER, 0, -10);

    s_page_lbl = ui_pixel_label(s_scr, "1/100", &lv_font_montserrat_14, UI_SKY_DARK);
    lv_obj_align(s_page_lbl, LV_ALIGN_BOTTOM_LEFT, 12, -10);

    lv_label_set_text(s_body, quotes_get(s_idx));
    lv_label_set_text_fmt(s_page_lbl, "%d/%d", s_idx + 1, quotes_count());

    lv_screen_load(s_scr);
}

void feature_quote_exit(void) {
    if (s_scr) { lv_obj_delete(s_scr); s_scr = NULL; }
    s_title = s_body = s_page_lbl = NULL;
}

void feature_quote_key(bsp_btn_t btn, bsp_btn_ev_t ev) {
    if (ev != BSP_BTN_CLICK && ev != BSP_BTN_LONG) return;
    if (!bsp_lvgl_lock(250)) return;

    int n = quotes_count();
    if (btn == BSP_BTN_UP && ev == BSP_BTN_CLICK) {
        s_idx = (s_idx - 1 + n) % n;
    } else if (btn == BSP_BTN_DOWN && ev == BSP_BTN_CLICK) {
        s_idx = (s_idx + 1) % n;
    } else if (btn == BSP_BTN_OK && ev == BSP_BTN_CLICK) {
        s_idx = (s_idx + (esp_random() % 17) + 1) % n;
    } else if (btn == BSP_BTN_OK && ev == BSP_BTN_LONG) {
        feature_quote_exit();
        bsp_lvgl_unlock();
        extern void menu_enter(void);
        menu_enter();
        return;
    } else {
        bsp_lvgl_unlock();
        return;
    }

    lv_label_set_text(s_body, quotes_get(s_idx));
    lv_label_set_text_fmt(s_page_lbl, "%d/%d", s_idx + 1, n);

    bsp_lvgl_unlock();
}