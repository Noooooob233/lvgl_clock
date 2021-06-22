#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        char location[128];
        char key[128];
    } config_t;

    config_t *get_config(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
