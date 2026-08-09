#include <stdint.h>
#include <zos_errors.h>
#include <zos_video.h>
#include <core.h>

#include "common.h"
#include "version.h"

uint8_t cmd_ver(char* args)
{
    (void)args;
    put_s(APP_NAME);
    put_c(CH_SPACE);
    put_s(APP_VERSION_STRING);
    put_c('-');
    put_u8(APP_VERSION_BUILD);
    put_c(CH_NEWLINE);
    return ERR_SUCCESS;
}
