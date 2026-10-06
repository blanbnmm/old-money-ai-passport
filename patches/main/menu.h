// main/menu.h —— 老钱 AI 主菜单 + 4 个子功能调度
#pragma once

#include "bsp_button.h"

typedef enum {
    MENU_ITEM_STOCK = 0,    // 📈 实时盯盘
    MENU_ITEM_QUOTE,        // 💬 每日一句
    MENU_ITEM_DIARY,        // 📖 老钱日记
    MENU_ITEM_GREETING,     // 💌 问候模式
    MENU_ITEM_COUNT,
} menu_item_t;

void menu_enter(void);
void menu_exit(void);
void menu_key(bsp_btn_t btn, bsp_btn_ev_t ev);

typedef struct {
    const char *name;
    void (*enter)(void);
    void (*exit)(void);
    void (*key)(bsp_btn_t btn, bsp_btn_ev_t ev);
} feature_entry_t;

void menu_register_features(void);
const feature_entry_t *menu_get_features(int *count);
bool menu_check_long_press_ok(int hold_ms);