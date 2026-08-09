#include <stdint.h>
#include <zos_errors.h>

zos_err_t cmd_false(char* args)
{
    (void)args;
    return ERR_FAILURE;
}
