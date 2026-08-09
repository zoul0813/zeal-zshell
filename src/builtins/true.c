#include <stdint.h>
#include <zos_errors.h>

zos_err_t cmd_true(char* args)
{
    (void)args;
    return ERR_SUCCESS;
}
