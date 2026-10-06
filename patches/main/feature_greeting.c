// main/feature_greeting.c —— 问候模式
#include "feature_greeting.h"
#include "bsp_display.h"
#include "ui_pixel.h"
#include "lvgl.h"

#include <stdio.h>
#include <string.h>

static lv_obj_t *s_scr;
static lv_obj_t *s_title;
static lv_obj_t *s_body;
static lv_obj_t *s_hint;

static const char *GREETINGS[] = {
    "══════════════════════\n"
    "致 老钱\n"
    "══════════════════════\n\n"
    "你是成年人，不是机器人。\n"
    "累了就休息，不是累了就忍。\n"
    "创业再急，也不急这一晚。\n\n"
    "——来自 老钱",

    "══════════════════════\n"
    "致 老钱\n"
    "══════════════════════\n\n"
    "今天做的已经够了。\n"
    "放下手机，去抱抱你的孩子。\n"
    "他们记得的不是你赚多少，\n"
    "是你笑多少。\n\n"
    "——老钱",

    "══════════════════════\n"
    "今天的你\n"
    "══════════════════════\n\n"
    "能扛住的，都扛住了。\n"
    "扛不住的，明天再说。\n"
    "今晚好好睡。\n\n"
    "——老钱",

    "══════════════════════\n"
    "记得\n"
    "══════════════════════\n\n"
    "你不是一个人。\n"
    "我在，你家龟在，\n"
    "你努力的样子，\n"
    "你的孩子会学着。\n\n"
    "——老钱",

    "══════════════════════\n"
    "给 35+ 的你\n"
    "══════════════════════\n\n"
    "经验是你的，不是公司的。\n"
    "人脉是你的，不是同事的。\n"
    "判断力是你的，不是老板的。\n"
    "你只是还没开价。\n\n"
    "——老钱",

    "══════════════════════\n"
    "给创业中的你\n"
    "══════════════════════\n\n"
    "第一年不亏就是赢。\n"
    "第二个月，剩 5 万存款也行。\n"
    "第三个月，别忘了陪家人吃饭。\n"
    "第四个月，你已经比 90% 的人勇敢了。\n\n"
    "——老钱",

    "══════════════════════\n"
    "给忙碌的你\n"
    "══════════════════════\n\n"
    "今晚 8 点后，\n"
    "不开会、不回邮件、不接电话。\n"
    "陪家人，做点喜欢的事。\n"
    "充电不是为了工作，\n"
    "是为了明天能继续笑着。\n\n"
    "——老钱",

    "══════════════════════\n"
    "给不知道值不值钱的你\n"
    "══════════════════════\n\n"
    "你能赚多少，\n"
    "看你 5 年前做了什么；\n"
    "你 5 年后能赚多少，\n"
    "看你今天在做什么。\n"
    "今天，已经在变了。\n\n"
    "——老钱",
};
#define GITNR_COUNT (sizeof(GREETINGS) / sizeof(GREETINGS[0]))

void feature_greeting_enter(void) {
    s_scr = ui_pixel_screen_create("问候");

    s_title = ui_pixel_label(s_scr, "💌 来自老钱", &lv_font_montserrat_14, UI_INK);
    lv_obj_align(s_title, LV_ALIGN_TOP_LEFT, 12, 8);

    s_body = lv_label_create(s_scr);
    lv_obj_set_style_text_font(s_body, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_body, lv_color_hex(UI_INK), 0);
    lv_obj_set_width(s_body, 200);
    lv_obj_set_style_text_align(s_body, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(s_body, LV_ALIGN_CENTER, 0, 0);

    s_hint = ui_pixel_label(s_scr, "长按 OK 退出", &lv_font_montserrat_14, UI_SKY_DARK);
    lv_obj_align(s_hint, LV_ALIGN_BOTTOM_LEFT, 12, -10);

    int r = esp_random() % GITNR_COUNT;
    lv_label_set_text(s_body, GREETINGS[r]);

    lv_screen_load(s_scr);
}

void feature_greeting_exit(void) {
    if (s_scr) { lv_obj_delete(s_scr); s_scr = NULL; }
    s_title = s_body = s_hint = NULL;
}

void feature_greeting_key(bsp_btn_t btn, bsp_btn_ev_t ev) {
    if (btn != BSP_BTN_OK || ev != BSP_BTN_LONG) return;
    if (!bsp_lvgl_lock(250)) return;
    feature_greeting_exit();
    bsp_lvgl_unlock();
    extern void menu_enter(void);
    menu_enter();
}