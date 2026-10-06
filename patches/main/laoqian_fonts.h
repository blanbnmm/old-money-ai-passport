// 老钱 AI 中文字体声明
#pragma once
#include "lvgl.h"

// 16px / 20px 中文 LVGL 字体子集（801 字：688 中文 + ASCII + 标点）
LV_FONT_DECLARE(laoqian_font_16)
LV_FONT_DECLARE(laoqian_font_20)

// 标题字体（20px）
#define FONT_TITLE  (&laoqian_font_20)
// 正文字体（16px）
#define FONT_BODY   (&laoqian_font_16)
// 小标签字体（16px）
#define FONT_SMALL  (&laoqian_font_16)
