#include "net.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lvgl/lvgl.h"

static float tx_speed = 0;
static float rx_speed = 0;

static NetworkIOData last_data;
static void net_speed_update(lv_timer_t *t);
static bool getNetworkIO(NetworkIOData *data);

void net_monitor_init()
{
    getNetworkIO(&last_data);

    lv_timer_create(net_speed_update, 1000, NULL);
}

static bool getNetworkIO(NetworkIOData *data)
{
    FILE *fd = fopen("/proc/net/dev", "r");
    if (!fd)
        return false;

    // 清空数据
    memset(data, 0, sizeof(NetworkIOData));

    char lineBuffer[512];

    // 将所有网络数据记录
    while (fgets(lineBuffer, sizeof(lineBuffer), fd))
    {
        char interfaceName[32];
        unsigned long long int bytesReceived, packetsReceived, bytesTransmitted, packetsTransmitted;

        if (sscanf(lineBuffer, "%31s %llu %llu %*u %*u %*u %*u %*u %*u %llu %llu",
                   interfaceName,
                   &bytesReceived,
                   &packetsReceived,
                   &bytesTransmitted,
                   &packetsTransmitted) != 5)
            continue;

        // 跳过lo设备
        if (strcmp(interfaceName, "lo:") == 0)
            continue;

        data->bytesReceived += bytesReceived;
        data->packetsReceived += packetsReceived;
        data->bytesTransmitted += bytesTransmitted;
        data->packetsTransmitted += packetsTransmitted;
    }

    fclose(fd);

    return true;
}

static void net_speed_update(lv_timer_t *t)
{
    NetworkIOData now_data;

    // 获取io数据
    getNetworkIO(&now_data);

    tx_speed = (now_data.bytesTransmitted - last_data.bytesTransmitted) / (float)1024.0;
    tx_speed = (tx_speed * 1000) / (float)1000.0;
    last_data.bytesTransmitted = now_data.bytesTransmitted;

    rx_speed = (now_data.bytesReceived - last_data.bytesReceived) / (float)1024.0;
    rx_speed = (rx_speed * 1000) / (float)1000.0;
    last_data.bytesReceived = now_data.bytesReceived;
}

float get_net_tx_speed()
{
    return tx_speed;
}

float get_net_rx_speed()
{
    return rx_speed;
}