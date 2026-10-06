// stock_monitor.c —— 用 socket 直连 qt.gtimg.cn (HTTP, 不用 esp_http_client)

#include "stock_monitor.h"
#include "demo.h"
#include "ui_pixel.h"
#include "bsp_display.h"
#include "bsp_button.h"
#include "bsp_pins.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>


typedef struct {
    const char *ticker;
    const char *name;
    const char *code;
} stock_cfg_t;

static const stock_cfg_t STOCKS[] = {
    { "sh601088", "神华", "601088" },
    { "sz000977", "浪潮", "000977" },
    { "sz002428", "锗业", "002428" },
};
#define STOCK_COUNT 3

#define REFRESH_INTERVAL_MS 60000
#define QT_HOST  "qt.gtimg.cn"
#define QT_PORT  80

typedef struct {
    float price;
    float prev_close;
    float open;
    float high;
    float low;
    float change_pct;
    bool  valid;
} stock_quote_t;

static stock_quote_t s_quotes[STOCK_COUNT];
static int s_selected = 0;
static lv_obj_t *s_scr;
static lv_obj_t *s_title;
static lv_obj_t *s_wifi_icon;
static lv_obj_t *s_name_lbl;
static lv_obj_t *s_code_lbl;
static lv_obj_t *s_price_lbl;
static lv_obj_t *s_change_lbl;
static lv_obj_t *s_kline_lbl;
static lv_obj_t *s_update_lbl;
static lv_obj_t *s_status_lbl;
static lv_timer_t *s_refresh_timer;
static lv_timer_t *s_clock_timer;
static volatile bool s_wifi_connected;
static volatile bool s_fetch_inflight;

static uint32_t change_color(float pct) {
    if (pct > 0) return 0xE43B2F;
    if (pct < 0) return 0x82BE2D;
    return 0x17202A;
}
static const char *change_arrow(float pct) {
    if (pct > 0) return "▲";
    if (pct < 0) return "▼";
    return "—";
}

static bool parse_quote(const char *resp, int resp_len, stock_quote_t *out) {
    if (!resp || resp_len <= 0) return false;
    const char *eq = strchr(resp, '=');
    if (!eq) return false;
    const char *q1 = strchr(eq, '"');
    if (!q1) return false;
    const char *q2 = strrchr(q1 + 1, '"');
    if (!q2) return false;
    const char *start = q1 + 1;
    int len = (int)(q2 - start);
    if (len <= 0 || len > 2048) return false;
    char buf[2048];
    if (len >= (int)sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, start, len);
    buf[len] = 0;

    char *save = NULL;
    char *fields[10] = {0};
    int n = 0;
    char *tok = strtok_r(buf, "~", &save);
    while (tok && n < 10) {
        fields[n++] = tok;
        tok = strtok_r(NULL, "~", &save);
    }
    if (n < 9) return false;

    out->price = strtof(fields[3], NULL);
    out->prev_close = strtof(fields[4], NULL);
    out->open = strtof(fields[5], NULL);
    out->high = strtof(fields[7], NULL);
    out->low = strtof(fields[8], NULL);
    if (out->prev_close > 0) {
        out->change_pct = (out->price - out->prev_close) / out->prev_close * 100.0f;
    } else {
        out->change_pct = 0;
    }
    out->valid = (out->price > 0);
    return out->valid;
}

static esp_err_t fetch_one(const char *ticker, stock_quote_t *out) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return ESP_FAIL;

    struct timeval timeout = { .tv_sec = 5, .tv_usec = 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    // DNS 解析
    struct hostent *he = gethostbyname(QT_HOST);
    if (!he) {
        close(sock);
        return ESP_FAIL;
    }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(QT_PORT);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return ESP_FAIL;
    }

    char req[256];
    int req_len = snprintf(req, sizeof(req),
        "GET /q=%s HTTP/1.1\r\nHost: qt.gtimg.cn\r\nConnection: close\r\n\r\n", ticker);
    if (send(sock, req, req_len, 0) < 0) {
        close(sock);
        return ESP_FAIL;
    }

    char buf[2048] = {0};
    int total = 0;
    int r;
    while ((r = recv(sock, buf + total, sizeof(buf) - 1 - total, 0)) > 0) {
        total += r;
    }
    close(sock);

    if (total <= 0) return ESP_FAIL;
    return parse_quote(buf, total, out) ? ESP_OK : ESP_FAIL;
}

static void fetch_all(void) {
    if (s_fetch_inflight || !s_wifi_connected) return;
    s_fetch_inflight = true;
    for (int i = 0; i < STOCK_COUNT; i++) {
        fetch_one(STOCKS[i].ticker, &s_quotes[i]);
    }
    s_fetch_inflight = false;
}

static esp_netif_t *s_sta_netif;
static volatile bool s_wifi_init_done;

static void on_wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        s_wifi_connected = false;
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        s_wifi_connected = true;
        fetch_all();
    }
}

static esp_err_t wifi_init_if_needed(void) {
    if (s_wifi_init_done) return ESP_OK;
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) return err;
    err = esp_netif_init();
    if (err != ESP_OK) return err;
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;
    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_WIFI_STA();
    s_sta_netif = esp_netif_new(&netif_cfg);
    if (!s_sta_netif) return ESP_ERR_NO_MEM;
    err = esp_wifi_init(&((wifi_init_config_t){}));
    if (err != ESP_OK) return err;
    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi_event, NULL);
    if (err != ESP_OK) return err;
    err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_wifi_event, NULL);
    if (err != ESP_OK) return err;
    s_wifi_init_done = true;
    return ESP_OK;
}

static void update_ui(void) {
    if (!bsp_lvgl_lock(200)) return;
    const stock_cfg_t *cfg = &STOCKS[s_selected];
    const stock_quote_t *q = &s_quotes[s_selected];
    lv_label_set_text_fmt(s_title, "STOCKS %d/%d", s_selected + 1, (int)STOCK_COUNT);
    lv_label_set_text(s_name_lbl, cfg->name);
    lv_label_set_text_fmt(s_code_lbl, "%s  %s", cfg->code, cfg->ticker);
    if (q->valid) {
        lv_label_set_text_fmt(s_price_lbl, "%.2f", q->price);
        lv_label_set_text_fmt(s_change_lbl, "%s %+.2f%%", change_arrow(q->change_pct), q->change_pct);
        lv_obj_set_style_text_color(s_change_lbl, lv_color_hex(change_color(q->change_pct)), 0);
        lv_label_set_text_fmt(s_kline_lbl, "O %.2f  H %.2f", q->open, q->high);
    } else {
        lv_label_set_text(s_price_lbl, "--.--");
        lv_obj_set_style_text_color(s_status_lbl,
            s_wifi_connected ? lv_color_hex(0xFFD928) : lv_color_hex(0xE43B2F), 0);
        lv_label_set_text(s_status_lbl, s_wifi_connected ? "loading..." : "WiFi not connected");
    }
    lv_obj_set_style_bg_color(s_wifi_icon,
        s_wifi_connected ? lv_color_hex(0x82BE2D) : lv_color_hex(0xE43B2F), 0);
    bsp_lvgl_unlock();
}

static void refresh_tick(lv_timer_t *t) {
    (void)t;
    fetch_all();
    update_ui();
}

void stock_monitor_enter(void) {
    s_scr = ui_pixel_screen_create("STOCKS");
    s_title = ui_pixel_label(s_scr, "STOCKS", &lv_font_montserrat_14, 0x17202A);
    lv_obj_align(s_title, LV_ALIGN_TOP_LEFT, 12, 8);
    s_wifi_icon = lv_obj_create(s_scr);
    lv_obj_set_size(s_wifi_icon, 10, 10);
    lv_obj_set_style_radius(s_wifi_icon, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(s_wifi_icon, lv_color_hex(0xE43B2F), 0);
    lv_obj_align(s_wifi_icon, LV_ALIGN_TOP_RIGHT, -14, 12);
    s_name_lbl = ui_pixel_label(s_scr, "...", &lv_font_montserrat_20, 0x17202A);
    lv_obj_align(s_name_lbl, LV_ALIGN_TOP_LEFT, 16, 40);
    s_code_lbl = ui_pixel_label(s_scr, "...", &lv_font_montserrat_14, 0x0872C9);
    lv_obj_align(s_code_lbl, LV_ALIGN_TOP_LEFT, 16, 70);
    s_price_lbl = ui_pixel_label(s_scr, "--.--", &lv_font_montserrat_20, 0x17202A);
    lv_obj_align(s_price_lbl, LV_ALIGN_CENTER, 0, -20);
    s_change_lbl = ui_pixel_label(s_scr, "—", &lv_font_montserrat_20, 0xD9E7EC);
    lv_obj_align(s_change_lbl, LV_ALIGN_CENTER, 0, 30);
    s_kline_lbl = ui_pixel_label(s_scr, "—", &lv_font_montserrat_14, 0x17202A);
    lv_obj_align(s_kline_lbl, LV_ALIGN_BOTTOM_LEFT, 16, -50);
    s_update_lbl = ui_pixel_label(s_scr, "", &lv_font_montserrat_14, 0x0872C9);
    lv_obj_align(s_update_lbl, LV_ALIGN_BOTTOM_LEFT, 16, -28);
    s_status_lbl = ui_pixel_label(s_scr, "WiFi not connected", &lv_font_montserrat_14, 0xE43B2F);
    lv_obj_align(s_status_lbl, LV_ALIGN_BOTTOM_LEFT, 16, -10);
    s_refresh_timer = lv_timer_create(refresh_tick, REFRESH_INTERVAL_MS, NULL);
    s_clock_timer = lv_timer_create((lv_timer_cb_t)update_ui, 1000, NULL);
    lv_screen_load(s_scr);
    update_ui();
}

void stock_monitor_exit(void) {
    if (s_refresh_timer) lv_timer_delete(s_refresh_timer);
    if (s_clock_timer) lv_timer_delete(s_clock_timer);
    if (s_scr) lv_obj_delete(s_scr);
}

esp_err_t stock_monitor_start(void) {
    const char *ssid = "iTo-Tsin"; // TODO: 用户改成自家 WiFi
    if (strlen(ssid) > 0) {
        esp_err_t err = wifi_init_if_needed();
        if (err != ESP_OK) return err;
        wifi_config_t cfg = {0};
        strncpy((char *)cfg.sta.ssid, ssid, sizeof(cfg.sta.ssid) - 1);
        // WiFi password - 用户在源码改
                cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;
        cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_set_config(WIFI_IF_STA, &cfg);
        esp_wifi_start();
    }
    return ESP_OK;
}

esp_err_t stock_monitor_stop(void) { return ESP_OK; }

void stock_monitor_key(bsp_btn_t btn, bsp_btn_ev_t ev) {
    if (ev != BSP_BTN_CLICK && ev != BSP_BTN_LONG) return;
    if (btn == BSP_BTN_OK && ev == BSP_BTN_LONG) {
        stock_monitor_exit();
        extern void menu_enter(void);
        menu_enter();
        return;
    }
    if (!bsp_lvgl_lock(250)) return;
    if (btn == BSP_BTN_UP && ev == BSP_BTN_CLICK) {
        s_selected = (s_selected + STOCK_COUNT - 1) % STOCK_COUNT;
    } else if (btn == BSP_BTN_DOWN && ev == BSP_BTN_CLICK) {
        s_selected = (s_selected + 1) % STOCK_COUNT;
    } else if (btn == BSP_BTN_OK && ev == BSP_BTN_CLICK) {
        fetch_one(STOCKS[s_selected].ticker, &s_quotes[s_selected]);
    }
    update_ui();
    bsp_lvgl_unlock();
}