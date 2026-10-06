// main/stock_monitor.h —— A 股实时盯盘 mini 屏
#pragma once

#include "bsp_button.h"
#include "esp_err.h"

void stock_monitor_enter(void);
void stock_monitor_exit(void);
void stock_monitor_key(bsp_btn_t btn, bsp_btn_ev_t ev);
esp_err_t stock_monitor_start(void);
esp_err_t stock_monitor_stop(void);