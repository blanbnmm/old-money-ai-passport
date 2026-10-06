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

typedef struct {
    const char *name;
    void (*enter)(void);
    void (*key)(int btn, int ev);
} feature_entry_t;

void menu_enter(void);
void menu_exit(void);
void menu_key(int btn, int ev);

// 检查长按 OK（返回 1=长按了，0=没有）
int menu_check_long_press_ok(int hold_ms);