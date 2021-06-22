#include "config.h"

#include "cJSON/cJSON.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define INIT_LOCATION "beijing"
#define INIT_KEY "YOR_PRIVATE_KEY"

static config_t config;
void create_new_config_file(void);
int get_config_file_path(char *path, int len);

int get_config_file_path(char *path, int len)
{
    int ret = 0;

    ret = readlink("/proc/self/exe", path, len);

    if (ret > 0)
    {
        for (int i = ret; i >= 0; i--)
        {
            if (path[i] == '/')
            {
                strcpy(&path[i] + 1, ".config");
                break;
            }
        }
    }

    printf("config path:%s\r\n", path);

    return ret;
}

void create_new_config_file()
{
    FILE *fd = NULL;

    char path[256] = {'\0'};

    cJSON *root = NULL;
    cJSON *weather = NULL;
    cJSON *location = NULL;
    cJSON *key = NULL;

    char *str = NULL;

    if (get_config_file_path(path, sizeof(path)) != -1)
    {
        if ((fd = fopen(path, "wt+")) == NULL)
        {
            return;
        }

        root = cJSON_CreateObject();

        weather = cJSON_CreateObject();

        cJSON_AddItemToObject(root, "weather", weather);

        location = cJSON_CreateString(INIT_LOCATION);
        key = cJSON_CreateString(INIT_KEY);

        cJSON_AddItemToObject(weather, "location", location);
        cJSON_AddItemToObject(weather, "key", key);

        str = cJSON_Print(root);

        printf("%s\r\n", str);

        fwrite(str, 1, strlen(str), fd);

        cJSON_Delete(root);

        fclose(fd);
    }
}

config_t *get_config()
{
    FILE *fd;
    char path[256] = {'\0'};

    if (get_config_file_path(path, sizeof(path)) != -1)
    {
        fd = fopen(path, "r");

        if (fd != NULL)
        {
            printf("config file open success\r\n");

            char str[1024] = {'\0'};

            cJSON *root = NULL;
            cJSON *weather = NULL;
            cJSON *location = NULL;
            cJSON *key = NULL;

            fread(str, 1, 1024, fd);

            root = cJSON_Parse(str);

            weather = cJSON_GetObjectItem(root, "weather");

            location = cJSON_GetObjectItem(weather, "location");
            key = cJSON_GetObjectItem(weather, "key");

            if (location != NULL && key != NULL)
            {
                strcpy(config.location, cJSON_GetStringValue(location));
                strcpy(config.key, cJSON_GetStringValue(key));
            }
            else
            {
                strcpy(config.location, INIT_LOCATION);
                strcpy(config.key, INIT_KEY);
            }

            cJSON_Delete(root);
            fclose(fd);

            return &config;
        }
        else
        {
            printf("config file open fail\r\n");
            create_new_config_file();
        }
    }

    strcpy(config.location, INIT_LOCATION);
    strcpy(config.key, INIT_KEY);

    printf("config:%s, %s\r\n", config.location, config.key);
    return &config;
}
