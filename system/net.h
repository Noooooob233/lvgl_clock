#ifndef __NET_H
#define __NET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

    typedef struct NetworkIOData_
    {
        uint64_t bytesReceived;
        uint64_t packetsReceived;
        uint64_t bytesTransmitted;
        uint64_t packetsTransmitted;
    } NetworkIOData;

    void net_monitor_init(void);

    float get_net_tx_speed(void);

    float get_net_rx_speed(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
