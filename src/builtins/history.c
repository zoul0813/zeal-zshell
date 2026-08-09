#include <stdint.h>
#include <zos_errors.h>
#include <zos_video.h>
#include <core.h>

#include "history.h"

zos_err_t cmd_history(char* args)
{
    (void)args;
    history_print();
    return ERR_SUCCESS;
}
