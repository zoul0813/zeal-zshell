#include <stddef.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_video.h>
#include <core.h>

#include "builtin.h"

zos_err_t cmd_help(char* args)
{
    (void)args;
    for (uint8_t i = 0; builtins[i].handler != NULL; i++) {
        put_s(builtins[i].name);
        put_c('\n');
    }
    return ERR_SUCCESS;
}
