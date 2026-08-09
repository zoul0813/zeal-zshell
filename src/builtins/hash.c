#include <stdint.h>
#include <zos_errors.h>
#include <zos_video.h>
#include <core.h>

#include "common.h"

zos_err_t cmd_hash(char* args)
{
    (void)args;
    put_u8(last_status);
    put_c(CH_NEWLINE);
    return ERR_SUCCESS;
}
