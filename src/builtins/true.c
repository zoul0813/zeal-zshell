#include <stdint.h>
#include <zos_errors.h>

uint8_t cmd_true(char* args)
{
    (void)args;
    return ERR_SUCCESS;
}
