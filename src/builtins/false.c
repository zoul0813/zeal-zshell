#include <stdint.h>
#include <zos_errors.h>

uint8_t cmd_false(char* args)
{
    (void)args;
    return ERR_FAILURE;
}
