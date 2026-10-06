#pragma once

#include <stdbool.h>

// 主菜单 4 个功能
enum {
    MENU_STOCK = 0,
    MENU_QUOTE,
    MENU_DIARY,
    MENU_GREETING,
    MENU_TOTAL
};

// 按键事件（用 bsp_button.h 的类型）
typedef int bsp_btn_t;
typedef int bsp_btn_ev_t;

// 菜单项条目
typedef struct {
    const char *icon;
    const char *name;
    void (*enter)(void);
    void (*key)(int btn, int ev);
} feature_entry_t;

void menu_enter(void);
void menu_exit(void);
void menu_key(int btn, int ev);

const feature_entry_t *menu_get_features(int *count);
int menu_check_long_press_ok(int hold_ms);