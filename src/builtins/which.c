#include <stddef.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_vfs.h>
#include <core.h>

#include "builtin.h"
#include "common.h"
#include "paths.h"
#include "process.h"

uint8_t cmd_which(char* args)
{
    uint8_t shallow = 0;
    char* search_name = args;

    if (str_len(args) > 2 && args[0] == '.' && args[1] == PATH_SEP) {
        shallow = 1;
        search_name = &args[2];
    }

    if (!shallow) {
        for (uint8_t i = 0; builtins[i].handler != NULL; i++) {
            if (str_cmp(search_name, builtins[i].name) == 0) {
                put_s("built-in: ");
                put_s(search_name);
                put_c(CH_NEWLINE);
                return ERR_SUCCESS;
            }
        }
    }

    char cmd[PATH_MAX];
    uint16_t search_len = str_len(search_name);
    if (search_len >= PATH_MAX)
        return ERR_PATH_TOO_LONG;
    str_cpyn(cmd, search_name, search_len);
    cmd[search_len] = CH_NULL;

    zos_err_t err = find_exec(cmd, shallow);
    if (err)
        return ERR_NO_SUCH_ENTRY;
    put_s(cmd);
    put_c(CH_NEWLINE);
    return ERR_SUCCESS;
}
