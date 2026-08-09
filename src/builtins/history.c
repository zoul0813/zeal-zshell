#include <stdint.h>
#include <zos_errors.h>
#include <zos_video.h>
#include <core.h>

#include "common.h"
#include "history.h"

zos_err_t cmd_history(char* args)
{
    if(args[0] == CH_NULL) {
        history_print();
        return ERR_SUCCESS;
    }

    if(str_cmp(args, "clear") == 0) {
        history_clear();
        return ERR_SUCCESS;
    }

    put_s("usage: history [clear]\n");
    return ERR_INVALID_PARAMETER;
}
