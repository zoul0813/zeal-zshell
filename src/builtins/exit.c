#include <stdint.h>
#include <zos_errors.h>

#include "common.h"

uint8_t cmd_exit(char* args)
{
    (void)args;
    return __exit(ERR_SUCCESS);
}
