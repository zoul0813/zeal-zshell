#include <stdint.h>
#include <zos_errors.h>
#include <zos_vfs.h>
#include <core.h>

#include "common.h"
#include "paths.h"

uint8_t cmd_cd(char* args)
{
    uint16_t len = str_len(args);
    if (len == 0 || (args[len - 1] != PATH_SEP && len >= PATH_MAX - 1)) {
        put_s("cd <path>\n");
        return ERR_INVALID_PARAMETER;
    }
    if (args[len - 1] != PATH_SEP) {
        args[len] = PATH_SEP;
        args[len + 1] = CH_NULL;
    }

    zos_err_t err = chdir(args);
    handle_error(err, "change dir", 0);
    if (err)
        return err;
    return path_set_cwd(&cwd);
}
