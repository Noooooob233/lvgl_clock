#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"

#include "ui/ui.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <termios.h>
#include <linux/vt.h>

uint32_t my_tick(void);
void signal_handler(int);
void vt_init(void);
void vt_shutdown(void);

int main(int argc, char **argv)
{
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTERM, signal_handler);

    lv_init();

    fbdev_init();
    vt_init();

    static lv_color_t buf[240 * 60];
    static lv_disp_draw_buf_t disp_buf;

    lv_disp_draw_buf_init(&disp_buf, buf, NULL, 240 * 60);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 135;
    lv_disp_drv_register(&disp_drv);

    ui_init();

    while (1)
    {
        lv_task_handler();
        usleep(5000);
    }

    return 0;
}

static int fd0 = -1;
static int fdcon = -1;
static int prev_vt = 0;
static int vt_num = 0;

void vt_init()
{
    struct vt_stat vs;
    char ttystr[32];

    fd0 = open("/dev/tty0", O_RDONLY | O_NOCTTY);

    if (fd0 < 0)
    {
        printf("/dev/tty0 open failed: %s\r\n", strerror(errno));
        return;
    }

    if (ioctl(fd0, VT_GETSTATE, &vs) < 0)
    {
        printf("VT_GETSTATE failed: %s\r\n", strerror(errno));
        close(fd0);
        return;
    }

    prev_vt = vs.v_active;
    printf("vs.v_active:%d\r\n", vs.v_active);

    if (ioctl(fd0, VT_OPENQRY, &vt_num) < 0)
    {
        printf("cannot allocate vt\r\n");
        close(fd0);
        return;
    }

    printf("vt_num:%d\r\n", vt_num);

    snprintf(ttystr, 32, "/dev/tty%d", vt_num);
    fdcon = open(ttystr, O_RDWR | O_NOCTTY);
    if (fdcon < 0)
    {
        printf("%s open failed: %s\r\n", ttystr, strerror(errno));
        return;
    }

    if (ioctl(fdcon, VT_ACTIVATE, vt_num) < 0)
    {
        printf("VT_ACTIVATE failed: %s\n", strerror(errno));
        close(fdcon);
        return;
    }

    if (ioctl(fdcon, VT_WAITACTIVE, vt_num) < 0)
    {
        printf("VT_WAITACTIVE failed: %s\n", strerror(errno));
        close(fdcon);
        return;
    }

    if (ioctl(fdcon, KDSETMODE, KD_GRAPHICS) < 0)
    {
        printf("KDSETMODE failed: %s\n", strerror(errno));
        close(fdcon);
        return;
    }
}

void vt_shutdown()
{
    if (fdcon >= 0)
    {
        if (ioctl(fdcon, KDSETMODE, KD_TEXT) < 0)
        {
            printf("KD_TEXT failed :%s\r\n", strerror(errno));
        }

        close(fdcon);
    }

    if (fd0 >= 0)
    {
        if (ioctl(fd0, VT_ACTIVATE, prev_vt) < 0)
        {
            printf("VT_ACTIVATE failed :%s\r\n", strerror(errno));
        }

        if (ioctl(fd0, VT_WAITACTIVE, prev_vt) < 0)
        {
            printf("VT_WAITACTIVE failed :%s\r\n", strerror(errno));
        }

        close(fd0);
    }
}

void signal_handler(int s)
{
    fbdev_exit();
    vt_shutdown();

    exit(0);
}

uint32_t my_tick(void)
{
    static uint64_t start_ms = 0;
    if (start_ms == 0)
    {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
