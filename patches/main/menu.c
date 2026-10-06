// main/menu.c —— 老钱 AI 主菜单
#include "menu.h"
#include "bsp_display.h"
#include "ui_pixel.h"
#include "lvgl.h"

#include <stdio.h>

typedef struct {
    const char *icon;
    const char *name;
    void (*enter)(void);
    void (*key)(bsp_btn_t btn, bsp_btn_ev_t ev);
} entry_t;

extern void feature_quote_enter(void);
extern void feature_quote_key(bsp_btn_t btn, bsp_btn_ev_t ev);
extern void feature_diary_enter(void);
extern void feature_diary_key(bsp_btn_t btn, bsp_btn_ev_t ev);
extern void feature_greeting_enter(void);
extern void feature_greeting_key(bsp_btn_t btn, bsp_btn_ev_t ev);
extern void stock_monitor_enter(void);
extern void stock_monitor_key(bsp_btn_t btn, bsp_btn_ev_t ev);
extern void stock_monitor_exit(void);

static const entry_t FEATURES[] = {
    { "📈", "实时盯盘", stock_monitor_enter, stock_monitor_key },
    { "💬", "每日一句", feature_quote_enter, feature_quote_key },
    { "📖", "老钱日记", feature_diary_enter, feature_diary_key },
    { "💌", "问候模式", feature_greeting_enter, feature_greeting_key },
};
#define FEATURE_COUNT (sizeof(FEATURES) / sizeof(FEATURES[0]))

static int s_selected = 0;
static lv_obj_t *s_scr;
static lv_obj_t *s_title;
static lv_obj_t *s_item_icons[FEATURE_COUNT];
static lv_obj_t *s_item_names[FEATURE_COUNT];
static lv_obj_t *s_item_panels[FEATURE_COUNT];
static lv_obj_t *s_hint;

extern void ui_pixel_set_selected(lv_obj_t *panel, bool selected, bool enabled);

void menu_refresh(void) {
    if (!bsp_lvgl_lock(250)) return;
    for (int i = 0; i < FEATURE_COUNT; i++) {
        bool sel = (i == s_selected);
        ui_pixel_set_selected(s_item_panels[i], sel, true);
    }
    bsp_lvgl_unlock();
}

void menu_enter(void) {
    s_scr = ui_pixel_screen_create("老钱 AI");

    s_title = ui_pixel_label(s_scr, "老钱 AI", &lv_font_montserrat_14, UI_INK);
    lv_obj_align(s_title, LV_ALIGN_TOP_LEFT, 12, 8);

    for (int i = 0; i < FEATURE_COUNT; i++) {
        int y = 38 + i * 60;
        s_item_panels[i] = ui_pixel_panel_create(s_scr, 14, y, 212, 50, UI_PAPER);

        s_item_icons[i] = lv_label_create(s_item_panels[i]);
        lv_label_set_text(s_item_icons[i], FEATURES[i].icon);
        lv_obj_set_style_text_font(s_item_icons[i], &lv_font_montserrat_20, 0);
        lv_obj_align(s_item_icons[i], LV_ALIGN_LEFT_MID, 8, 0);

        s_item_names[i] = lv_label_create(s_item_panels[i]);
        lv_label_set_text(s_item_names[i], FEATURES[i].name);
        lv_obj_set_style_text_font(s_item_names[i], &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(s_item_names[i], lv_color_hex(UI_INK), 0);
        lv_obj_align(s_item_names[i], LV_ALIGN_LEFT_MID, 50, 0);
    }

    s_hint = ui_pixel_label(s_scr, "OK 进入 | 长按 OK 问候", &lv_font_montserrat_14, UI_SKY_DARK);
    lv_obj_align(s_hint, LV_ALIGN_BOTTOM_LEFT, 12, -10);

    lv_screen_load(s_scr);
    menu_refresh();
}

void menu_exit(void) {
    if (s_scr) { lv_obj_delete(s_scr); s_scr = NULL; }
    s_title = s_hint = NULL;
    for (int i = 0; i < FEATURE_COUNT; i++) {
        s_item_icons[i] = s_item_names[i] = s_item_panels[i] = NULL;
    }
}

void menu_key(bsp_btn_t btn, bsp_btn_ev_t ev) {
    if (ev != BSP_BTN_CLICK && ev != BSP_BTN_LONG) return;

    if (btn == BSP_BTN_OK && ev == BSP_BTN_LONG) {
        menu_exit();
        feature_greeting_enter();
        return;
    }

    if (!bsp_lvgl_lock(250)) return;

    if (btn == BSP_BTN_UP && ev == BSP_BTN_CLICK) {
        s_selected = (s_selected - 1 + FEATURE_COUNT) % FEATURE_COUNT;
    } else if (btn == BSP_BTN_DOWN && ev == BSP_BTN_CLICK) {
        s_selected = (s_selected + 1) % FEATURE_COUNT;
    } else if (btn == BSP_BTN_OK && ev == BSP_BTN_CLICK) {
        int sel = s_selected;
        bsp_lvgl_unlock();
        menu_exit();
        FEATURES[sel].enter();
        return;
    } else {
        bsp_lvgl_unlock();
        return;
    }

    menu_refresh();
    bsp_lvgl_unlock();
}

bool menu_check_long_press_ok(int hold_ms) {
    return false;
}

const feature_entry_t *menu_get_features(int *count) {
    static feature_entry_t fe[FEATURE_COUNT];
    for (int i = 0; i < FEATURE_COUNT; i++) {
        fe[i].name = FEATURES[i].name;
        fe[i].enter = NULL;
        fe[i].exit = NULL;
        fe[i].key = NULL;
    }
    if (count) *count = FEATURE_COUNT;
    return fe;
}

void menu_register_features(void) {
    // no-op
}