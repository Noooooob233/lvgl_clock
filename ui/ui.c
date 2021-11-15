#include "ui.h"
#include "lvgl/lvgl.h"

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>

#include "config/config.h"

#include "system/net.h"
#include "system/cpu.h"
#include "system/mem.h"

#include "weather/weather.h"

#define MAX_WEATHER_CODE 38

LV_IMG_DECLARE(img_sunny);
LV_IMG_DECLARE(img_clear);
LV_IMG_DECLARE(img_fair_day);
LV_IMG_DECLARE(img_fair_night);
LV_IMG_DECLARE(img_cloudy);
LV_IMG_DECLARE(img_partly_cloudy_day);
LV_IMG_DECLARE(img_partly_cloudy_night);
LV_IMG_DECLARE(img_mostly_cloudy_day);
LV_IMG_DECLARE(img_mostly_cloudy_night);
LV_IMG_DECLARE(img_overcast);
LV_IMG_DECLARE(img_shower);
LV_IMG_DECLARE(img_thundershower);
LV_IMG_DECLARE(img_thundershower_with_hail);
LV_IMG_DECLARE(img_light_rain);
LV_IMG_DECLARE(img_moderate_rain);
LV_IMG_DECLARE(img_heavy_rain);
LV_IMG_DECLARE(img_storm);
LV_IMG_DECLARE(img_heavy_storm);
LV_IMG_DECLARE(img_severe_storm);
LV_IMG_DECLARE(img_ice_rain);
LV_IMG_DECLARE(img_sleet);
LV_IMG_DECLARE(img_snow_flurry);
LV_IMG_DECLARE(img_light_snow);
LV_IMG_DECLARE(img_moderate_snow);
LV_IMG_DECLARE(img_heavy_snow);
LV_IMG_DECLARE(img_snowstorm);
LV_IMG_DECLARE(img_dust);
LV_IMG_DECLARE(img_sand);
LV_IMG_DECLARE(img_duststorm);
LV_IMG_DECLARE(img_sandstorm);
LV_IMG_DECLARE(img_foggy);
LV_IMG_DECLARE(img_haze);
LV_IMG_DECLARE(img_windy);
LV_IMG_DECLARE(img_blustery);
LV_IMG_DECLARE(img_hurricance);
LV_IMG_DECLARE(img_tropical_storm);
LV_IMG_DECLARE(img_tornado);
LV_IMG_DECLARE(img_cold);
LV_IMG_DECLARE(img_hot);
LV_IMG_DECLARE(img_unknown);

static void *weather_table[] = {
    &img_sunny,                   // 0 晴（国内城市白天晴）
    &img_clear,                   // 1 晴（国内城市夜晚晴）
    &img_fair_day,                // 2 晴（国外城市白天晴）
    &img_fair_night,              // 3 晴（国外城市夜晚晴）
    &img_cloudy,                  // 4 多云
    &img_partly_cloudy_day,       // 5 晴间多云
    &img_partly_cloudy_night,     // 6 晴间多云
    &img_mostly_cloudy_day,       // 7 大部多云
    &img_mostly_cloudy_night,     // 8 大部多云
    &img_overcast,                // 9 阴
    &img_shower,                  // 10 阵雨
    &img_thundershower,           // 11 雷阵雨
    &img_thundershower_with_hail, // 12 雷阵雨伴有冰雹
    &img_light_rain,              // 13 小雨
    &img_moderate_rain,           // 14 中雨
    &img_heavy_rain,              // 15 大雨
    &img_storm,                   // 16 暴雨
    &img_heavy_storm,             // 17 大暴雨
    &img_severe_storm,            // 18 特大暴雨
    &img_ice_rain,                // 19 冻雨
    &img_sleet,                   // 20 雨夹雪
    &img_snow_flurry,             // 21 阵雪
    &img_light_snow,              // 22 小雪
    &img_moderate_snow,           // 23 中雪
    &img_heavy_snow,              // 24 大雪
    &img_snowstorm,               // 25 暴雪
    &img_dust,                    // 26 浮尘
    &img_sand,                    // 27 扬沙
    &img_duststorm,               // 28 沙尘暴
    &img_sandstorm,               // 29 强沙尘暴
    &img_foggy,                   // 30 雾
    &img_haze,                    // 31 霾
    &img_windy,                   // 32 风
    &img_blustery,                // 33 大风
    &img_hurricance,              // 34 飓风
    &img_tropical_storm,          // 35 热带风暴
    &img_tornado,                 // 36 龙卷风
    &img_cold,                    // 37 冷
    &img_hot,                     // 38 热
    &img_unknown,                 // 99 N/A
};

static const lv_font_t *font_normal;

static lv_obj_t *tv;

static lv_obj_t *label_hour;
static lv_obj_t *label_min;
static lv_obj_t *label_sec;
static lv_obj_t *label_dot;

static lv_obj_t *label_localtion;
static lv_obj_t *img_weather;
static lv_obj_t *label_temperature;

static lv_obj_t *cpu_temp;
static lv_obj_t *mem_used;

static lv_obj_t *upload;
static lv_obj_t *download;

static lv_obj_t *page1;
static lv_obj_t *page2;
static lv_obj_t *page_act;

static pthread_t thread;
static pthread_attr_t attr;

static void ui_update(lv_timer_t *t);
static void ui_clock_create(lv_obj_t *parent);
static void ui_weather_create(lv_obj_t *parent);
void *ui_weather_update(void *param);
static void ui_net_monitor_create(void);
static void ui_cpu_mem_monitor_create(void);
static void ui_net_speed_update(void);
static void ui_tabview_switch_cb(lv_timer_t *t);
static void ui_anim_y_cb(void *var, int32_t v);

void ui_init()
{
    config_t *config = NULL;

    config = get_config();
    weather_init(config);

    font_normal = &lv_font_montserrat_16;

    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, font_normal);

    lv_obj_set_style_text_font(lv_scr_act(), font_normal, 0);

    tv = lv_tabview_create(lv_scr_act(), LV_DIR_RIGHT, 0);
    lv_obj_t *t1 = lv_tabview_add_tab(tv, " ");
    lv_obj_t *t2 = lv_tabview_add_tab(tv, " ");

    lv_obj_set_style_bg_color(tv, lv_palette_lighten(LV_PALETTE_GREY, 2), 0);

    ui_clock_create(t1);
    ui_weather_create(t2);

    page1 = lv_obj_create(lv_scr_act());
    lv_obj_align(page1, LV_ALIGN_RIGHT_MID, -5, -10);
    lv_obj_set_size(page1, 10, 10);
    lv_obj_set_style_bg_color(page1, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_radius(page1, LV_RADIUS_CIRCLE, 0);

    page2 = lv_obj_create(lv_scr_act());
    lv_obj_align(page2, LV_ALIGN_RIGHT_MID, -5, 10);
    lv_obj_set_size(page2, 10, 10);
    lv_obj_set_style_bg_color(page2, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_radius(page2, LV_RADIUS_CIRCLE, 0);

    page_act = lv_obj_create(lv_scr_act());
    lv_obj_align(page_act, LV_ALIGN_RIGHT_MID, -5, -10);
    lv_obj_set_size(page_act, 10, 10);
    lv_obj_set_style_bg_color(page_act, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_radius(page_act, LV_RADIUS_CIRCLE, 0);

    lv_timer_create(ui_tabview_switch_cb, 5000, NULL);

    ui_net_monitor_create();
    ui_cpu_mem_monitor_create();

    lv_timer_create(ui_update, 50, NULL);
}

static void ui_update(lv_timer_t *t)
{
    static struct tm last_date;
    time_t now_time;
    struct tm *now_date;

    static uint8_t toggle;

    time(&now_time);

    now_date = localtime(&now_time);

    if (now_date->tm_hour != last_date.tm_hour)
    {
        lv_label_set_text_fmt(label_hour, "%2d", now_date->tm_hour);
    }

    if (now_date->tm_min != last_date.tm_min)
    {
        lv_label_set_text_fmt(label_min, "%02d", now_date->tm_min);
    }

    if (now_date->tm_sec != last_date.tm_sec)
    {
        lv_label_set_text_fmt(label_sec, "%02d", now_date->tm_sec);

        if (toggle)
        {
            lv_obj_add_flag(label_dot, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_clear_flag(label_dot, LV_OBJ_FLAG_HIDDEN);
        }

        toggle = !toggle;

        lv_label_set_text_fmt(cpu_temp, "%.1f °C", (double)get_cpu_temp());
        lv_label_set_text_fmt(mem_used, "%.1f %%", (double)get_mem_used());
        ui_net_speed_update();
    }

    last_date = *now_date;
}

static void ui_clock_create(lv_obj_t *parent)
{
    time_t now_time;
    struct tm *now_date;

    time(&now_time);
    now_date = localtime(&now_time);

    label_hour = lv_label_create(parent);
    lv_obj_set_style_text_font(label_hour, &lv_font_montserrat_48, 0);
    lv_obj_align(label_hour, LV_ALIGN_CENTER, -70, 0);
    lv_label_set_text_fmt(label_hour, "%2d", now_date->tm_hour);

    label_dot = lv_label_create(parent);
    lv_obj_set_style_text_font(label_dot, &lv_font_montserrat_48, 0);
    lv_obj_align(label_dot, LV_ALIGN_CENTER, -25, -3);
    lv_label_set_text(label_dot, ":");

    label_min = lv_label_create(parent);
    lv_obj_set_style_text_font(label_min, &lv_font_montserrat_48, 0);
    lv_obj_align(label_min, LV_ALIGN_CENTER, 20, 0);
    lv_label_set_text_fmt(label_min, "%02d", now_date->tm_min);

    label_sec = lv_label_create(parent);
    lv_obj_set_style_text_font(label_sec, &lv_font_montserrat_24, 0);
    lv_obj_align(label_sec, LV_ALIGN_CENTER, 75, 8);
    lv_label_set_text_fmt(label_sec, "%02d", now_date->tm_sec);
}

static void ui_net_speed_update()
{
    float upload_speed = get_net_tx_speed();
    float download_speed = get_net_rx_speed();

    if (upload_speed >= 1000)
    {
        upload_speed /= (float)1024.0;
        lv_label_set_text_fmt(upload, "%0.1fM/s", (double)upload_speed);
    }
    else
    {
        lv_label_set_text_fmt(upload, "%0.1fK/s", (double)upload_speed);
    }

    if (download_speed >= 1000)
    {
        download_speed /= (float)1024.0;
        lv_label_set_text_fmt(download, "%0.1fM/s", (double)download_speed);
    }
    else
    {
        lv_label_set_text_fmt(download, "%0.1fK/s", (double)download_speed);
    }
}

static void ui_net_monitor_create()
{
    net_monitor_init();

    LV_IMG_DECLARE(img_speed);
    lv_obj_t *img_net_speed = lv_img_create(lv_scr_act());
    lv_img_set_src(img_net_speed, &img_speed);
    lv_obj_align(img_net_speed, LV_ALIGN_BOTTOM_LEFT, 5, 0);
    lv_obj_set_size(img_net_speed, 24, 24);

    download = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(download, &lv_font_montserrat_20, 0);
    lv_label_set_text_fmt(download, "0.0K/s");
    lv_obj_align(download, LV_ALIGN_BOTTOM_LEFT, 40, 0);

    upload = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(upload, &lv_font_montserrat_20, 0);
    lv_label_set_text_fmt(upload, "0.0K/s");
    lv_obj_align(upload, LV_ALIGN_BOTTOM_RIGHT, -10, 0);
}

static void ui_cpu_mem_monitor_create()
{
    LV_IMG_DECLARE(img_cpu_temp);
    lv_obj_t *img_cpu = lv_img_create(lv_scr_act());
    lv_img_set_src(img_cpu, &img_cpu_temp);
    lv_obj_align(img_cpu, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_size(img_cpu, 24, 24);

    cpu_temp = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(cpu_temp, &lv_font_montserrat_20, 0);
    lv_label_set_text_fmt(cpu_temp, "%.1f °C", (double)get_cpu_temp());
    lv_obj_align_to(cpu_temp, img_cpu, LV_ALIGN_CENTER, 55, 0);

    LV_IMG_DECLARE(img_mem);
    lv_obj_t *img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_mem);
    lv_obj_align_to(img, img_cpu, LV_ALIGN_CENTER, 125, 0);
    lv_obj_set_size(img, 24, 24);

    mem_used = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(mem_used, &lv_font_montserrat_20, 0);
    lv_label_set_text_fmt(mem_used, "%.1f %%", (double)get_mem_used());
    lv_obj_align_to(mem_used, img_cpu, LV_ALIGN_CENTER, 180, 0);
}

static void ui_weather_create(lv_obj_t *parent)
{
    img_weather = lv_img_create(parent);
    lv_obj_align(img_weather, LV_ALIGN_CENTER, -60, 0);
    lv_img_set_src(img_weather, weather_table[39]);

    LV_FONT_DECLARE(font_deng);
    label_localtion = lv_label_create(parent);
    lv_obj_align(label_localtion, LV_ALIGN_CENTER, 35, -16);
    lv_obj_set_style_text_font(label_localtion, &font_deng, 0);
    lv_label_set_text_fmt(label_localtion, "N/A");

    label_temperature = lv_label_create(parent);
    lv_obj_align(label_temperature, LV_ALIGN_CENTER, 35, 16);
    lv_obj_set_style_text_font(label_temperature, &lv_font_montserrat_24, 0);
    lv_label_set_text_fmt(label_temperature, "N/A °C");

    pthread_attr_init(&attr);
    pthread_create(&thread, &attr, ui_weather_update, 0);
}

void *ui_weather_update(void *param)
{
    weather_data_t *data = NULL;

    while (1)
    {
        data = weather_data_upate();

        if (data != NULL)
        {
            if (data->code >= 0 && data->code <= MAX_WEATHER_CODE)
            {
                lv_img_set_src(img_weather, weather_table[data->code]);
            }
            else
            {
                lv_img_set_src(img_weather, weather_table[MAX_WEATHER_CODE + 1]);
            }

            lv_label_set_text_fmt(label_localtion, "%s", data->location);
            lv_label_set_text_fmt(label_temperature, "%d °C", data->temp);

            printf("next update:after 1hour\r\n");
            sleep(60 * 60);
        }
        else
        {
            printf("next update:after 1min\r\n");
            sleep(60);
        }
    }

    return NULL;
}

static void ui_anim_y_cb(void *var, int32_t v)
{
    lv_obj_set_y(var, v);
}

static void ui_tabview_switch_cb(lv_timer_t *t)
{
    static uint16_t next_tab_idx = 1;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, page_act);

    switch (next_tab_idx)
    {
    case 0:
        lv_anim_set_values(&a, 10, -10);
        break;

    case 1:
        lv_anim_set_values(&a, -10, 10);
        break;

    default:
        break;
    }

    lv_anim_set_time(&a, 400);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);

    lv_anim_set_exec_cb(&a, ui_anim_y_cb);
    lv_anim_start(&a);

    lv_tabview_set_act(tv, next_tab_idx, true);

    next_tab_idx = (next_tab_idx + 1) % 2;
}
