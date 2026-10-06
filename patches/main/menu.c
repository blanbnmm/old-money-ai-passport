// 老钱 AI 主菜单
#include <stdio.h>
#include <stddef.h>
#include "menu.h"
#include "bsp_display.h"
#include "ui_pixel.h"
#include "lvgl.h"

// 引入所有功能的入口
extern void stock_monitor_enter(void);
extern void stock_monitor_key(int btn, int ev);
extern void feature_quote_enter(void);
extern void feature_quote_key(int btn, int ev);
extern void feature_diary_enter(void);
extern void feature_diary_key(int btn, int ev);
extern void feature_greeting_enter(void);
extern void feature_greeting_key(int btn, int ev);

typedef struct {
    const char *icon;
    const char *name;
    void (*enter)(void);
    void (*key)(int btn, int ev);
} entry_t;

static const entry_t FEATURES[] = {
    { "📈", "实时盯盘", stock_monitor_enter, stock_monitor_key },
    { "💬", "每日一句", feature_quote_enter, feature_quote_key },
    { "📖", "老钱日记", feature_diary_enter, feature_diary_key },
    { "💌", "问候模式", feature_greeting_enter, feature_greeting_key },
};
#define FEATURE_COUNT (sizeof(FEATURES) / sizeof(FEATURES[0]))

static int s_selected = 0;
static lv_obj_t *s_scr = NULL;
static lv_obj_t *s_item_icons[MENU_TOTAL];
static lv_obj_t *s_item_names[MENU_TOTAL];
static lv_obj_t *s_hint = NULL;

const feature_entry_t *menu_get_features(int *count) {
    if (count) *count = FEATURE_COUNT;
    return (const feature_entry_t *)FEATURES;
}

int menu_check_long_press_ok(int hold_ms) {
    (void)hold_ms;
    return 0;
}

static void draw(void) {
    s_scr = ui_pixel_screen_create("老钱 AI");
    for (int i = 0; i < FEATURE_COUNT; i++) {
        int y = 40 + i * 60;
        if (i == s_selected) {
            lv_obj_t *bg = lv_obj_create(s_scr);
            lv_obj_set_size(bg, 220, 50);
            lv_obj_set_pos(bg, 10, y);
            lv_obj_set_style_bg_color(bg, lv_color_hex(0x1689E8), 0);
            lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(bg, 8, 0);
        }
        s_item_icons[i] = lv_label_create(s_scr);
        lv_label_set_text(s_item_icons[i], FEATURES[i].icon);
        lv_obj_set_style_text_font(s_item_icons[i], &laoqian_font_20, 0);
        lv_obj_set_pos(s_item_icons[i], 20, y + 12);
        lv_obj_set_style_text_color(s_item_icons[i],
            lv_color_hex(i == s_selected ? 0xF4F4EA : 0x17202A), 0);

        s_item_names[i] = lv_label_create(s_scr);
        lv_label_set_text(s_item_names[i], FEATURES[i].name);
        lv_obj_set_style_text_font(s_item_names[i], &laoqian_font_16, 0);
        lv_obj_set_pos(s_item_names[i], 60, y + 16);
        lv_obj_set_style_text_color(s_item_names[i],
            lv_color_hex(i == s_selected ? 0xF4F4EA : 0x17202A), 0);
    }
    s_hint = lv_label_create(s_scr);
    lv_label_set_text(s_hint, "OK: enter  UP/DOWN: nav");
    lv_obj_set_style_text_font(s_hint, &laoqian_font_16, 0);
    lv_obj_set_pos(s_hint, 8, 304);
    lv_obj_set_style_text_color(s_hint, lv_color_hex(0x0872C9), 0);
}

void menu_enter(void) {
    s_selected = 0;
    if (!bsp_lvgl_lock(500)) return;
    draw();
    bsp_lvgl_unlock();
}

void menu_exit(void) {
    if (!bsp_lvgl_lock(500)) return;
    if (s_scr) {
        lv_obj_del(s_scr);
        s_scr = NULL;
    }
    bsp_lvgl_unlock();
}

void menu_key(int btn, int ev) {
    (void)ev;
    if (!bsp_lvgl_lock(250)) return;
    if (btn == 1) {  // UP
        s_selected = (s_selected + FEATURE_COUNT - 1) % FEATURE_COUNT;
        draw();
    } else if (btn == 2) {  // DOWN
        s_selected = (s_selected + 1) % FEATURE_COUNT;
        draw();
    } else if (btn == 3) {  // OK
        if (s_scr) {
            lv_obj_del(s_scr);
            s_scr = NULL;
        }
        FEATURES[s_selected].enter();
    }
    bsp_lvgl_unlock();
}