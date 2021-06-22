#include "mem.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

float get_mem_used()
{
    float mem_used = 0.0;

    uint64_t availableMem = 0;
    uint64_t freeMem = 0;
    uint64_t totalMem = 0;
    uint64_t buffersMem = 0;
    uint64_t cachedMem = 0;
    uint64_t sharedMem = 0;
    uint64_t swapTotalMem = 0;
    uint64_t swapCacheMem = 0;
    uint64_t swapFreeMem = 0;
    uint64_t sreclaimableMem = 0;

    FILE *file = fopen("/proc/meminfo", "r");
    if (!file)
        return 0.0;

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), file))
    {

#define tryRead(label, variable)                                      \
    if (strncmp(buffer, label, strlen(label)) == 0)                   \
    {                                                                 \
        uint64_t parsed_;                                             \
        if (sscanf(buffer + strlen(label), "%llu kB", &parsed_) == 1) \
        {                                                             \
            (variable) = parsed_;                                     \
        }                                                             \
        break;                                                        \
    }

        switch (buffer[0])
        {
        case 'M':
            tryRead("MemAvailable:", availableMem);
            tryRead("MemFree:", freeMem);
            tryRead("MemTotal:", totalMem);
            break;
        case 'B':
            tryRead("Buffers:", buffersMem);
            break;
        case 'C':
            tryRead("Cached:", cachedMem);
            break;
        case 'S':
            switch (buffer[1])
            {
            case 'h':
                tryRead("Shmem:", sharedMem);
                break;
            case 'w':
                tryRead("SwapTotal:", swapTotalMem);
                tryRead("SwapCached:", swapCacheMem);
                tryRead("SwapFree:", swapFreeMem);
                break;
            case 'R':
                tryRead("SReclaimable:", sreclaimableMem);
                break;
            }
            break;
        }

#undef tryRead
    }

    fclose(file);

    const uint64_t usedDiff = freeMem + cachedMem + sreclaimableMem + buffersMem;
    uint64_t usedMem = (totalMem >= usedDiff) ? totalMem - usedDiff : totalMem - freeMem;

    mem_used = usedMem * (float)100.0 / (float)totalMem;

    return mem_used;
}
