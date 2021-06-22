#include "cpu.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

float get_cpu_temp()
{
    long unsigned int temp = 0;

    FILE *fd = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

    if (fd)
    {
        fscanf(fd, "%lu", &temp);

        fclose(fd);
    }

    return (float)(temp / 1000.0);
}
