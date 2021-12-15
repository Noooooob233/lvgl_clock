#ifndef __WEATHER_H
#define __WEATHER_H

#ifdef __cplusplus
extern "C"
{
#endif

#define USE_API_V4 0

#include "config/config.h"

    enum weather_code
    {
        SUNNY = 0,
        CLEAR,
        FAIR_DAY,
        FAIR_NIGHT,
        CLOUDY,
        PARTLY_CLOUDY_DAY,
        PARTLY_CLOUDY_NIGHT,
        MOSTLY_CLOUDY_DAY,
        MOSTLY_CLOUDY_NIGHT,
        OVERCAST,
        SHOWER,
        THUNDERSHOWER,
        THUNDERSHOWER_WITH_HAIL,
        LIGHT_RAIN,
        MODERATE_RAIN,
        HEAVY_RAIN,
        STORM,
        HEAVY_STORM,
        SEVERE_STORM,
        ICE_RAIN,
        SLEET,
        SNOW_FLURRY,
        LIGHT_SNOW,
        MODERATE_SNOW,
        HEAVY_SNOW,
        SNOWSTORM,
        DUST,
        SAND,
        DUSTSTORM,
        SANDSTORM,
        FOGGY,
        HAZE,
        WINDY,
        BLUSTERY,
        HURRICANCE,
        TROPICAL_STORM,
        TORNADO,
        COLD,
        HOT,
        UNKNOWN = 99
    };

    typedef struct
    {
        char location[64];
        char text[64];
        int code;
        int temp;
    } weather_now_t;

    typedef struct
    {
        char location[64];
        char text_day[64];
        char text_night[64];
        int code_day;
        int code_night;
        int temp_high;
        int temp_low;
    } weather_day_t;

    void weather_init(config_t *config);
    weather_now_t *weather_get_now(void);
    weather_day_t *weather_get_daily(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
