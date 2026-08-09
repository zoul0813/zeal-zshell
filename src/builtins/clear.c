#include <stddef.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>

zos_err_t cmd_clear(char* args)
{
    (void)args;
    return ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
}
