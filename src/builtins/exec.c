#include <stdint.h>
#include <zos_errors.h>
#include <core.h>

#include "process.h"

uint8_t cmd_exec(char* args)
{
    put_s("exec '");
    put_s(args);
    put_s("'\n");
    return run(args);
}
