idf_component_register(
    SRCS "main.c"
         "demo_navigation.c"
         "ui_pixel.c"
         "ui_pixel_math.c"
         "demo_display.c"
         "demo_button.c"
         "demo_low_power.c"
         "stock_monitor.c"
         "menu.c"
         "feature_quote.c"
         "feature_diary.c"
         "feature_greeting.c"
         "old_money_quotes.c"
         "old_money_diary.c"
    INCLUDE_DIRS "."
    REQUIRES bsp esp_event esp_lcd esp_hw_support esp_netif esp_timer esp_wifi nvs_flash
)

# 老钱 AI 中文 LVGL 字体子集（801 字 / 16px + 20px）
target_sources(${COMPONENT_LIB} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../assets/fonts/laoqian_font_16.c"
    "${CMAKE_CURRENT_LIST_DIR}/../assets/fonts/laoqian_font_20.c"
)
