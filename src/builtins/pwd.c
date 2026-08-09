#include <stdint.h>
#include <zos_errors.h>
#include <zos_video.h>
#include <core.h>

#include "common.h"

uint8_t cmd_pwd(char* args)
{
    (void)args;
    put_s(cwd.path);
    put_c(CH_NEWLINE);
    return ERR_SUCCESS;
}
