#include "net.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

static float tx_speed = 0;
static float rx_speed = 0;

static NetworkIOData last_data;
static void net_speed_update(int);
static bool getNetworkIO(NetworkIOData *data);

void net_monitor_init()
{
    struct sigevent evp;

    struct itimerspec ts;
    timer_t timer;

    evp.sigev_value.sival_ptr = &timer;
    evp.sigev_notify = SIGEV_SIGNAL;
    evp.sigev_signo = SIGUSR1;

    signal(SIGUSR1, net_speed_update);

    timer_create(CLOCK_REALTIME, &evp, &timer);

    ts.it_interval.tv_sec = 1;
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = 1;
    ts.it_value.tv_nsec = 0;

    timer_settime(timer, 0, &ts, NULL);

    getNetworkIO(&last_data);
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

static void net_speed_update(int s)
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