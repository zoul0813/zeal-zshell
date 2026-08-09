#include <stdint.h>
#include <zos_errors.h>

uint8_t cmd_reset(char* args)
{
    (void)args;
    __asm__("rst 0\n");
    return ERR_SUCCESS;
}
