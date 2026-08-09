#include <stddef.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_vfs.h>
#include <core.h>

#include "common.h"
#include "paths.h"

static zos_err_t set_path(char* path, char* str, size_t len)
{
    char value[PATH_MAX] = "";
    if (len == 0 || len >= PATH_MAX ||
        (str[len - 1] != PATH_SEP && len >= PATH_MAX - 1)) {
        put_s("ERROR: Path too long: ");
        put_s(str);
        put_c(CH_NEWLINE);
        return ERR_INVALID_PATH;
    }
    str_cpyn(value, str, len);
    if (value[len - 1] != PATH_SEP) {
        value[len] = PATH_SEP;
        len++;
    }
    value[len] = CH_NULL;
    str_cpyn(path, value, len + 1);
    return ERR_SUCCESS;
}

uint8_t cmd_set(char* args)
{
    char* equals = str_chr(args, '=');
    if (!equals) {
        if (str_cmp(args, "PATH") == 0) {
            for (uint8_t i = 0; i < path_count; i++) {
                put_u8(i);
                put_c(CH_SPACE);
                put_s(paths[i]);
                put_c(CH_NEWLINE);
            }
            return ERR_SUCCESS;
        }
        put_s("ERROR: Unknown variable: ");
        put_s(args);
        put_c(CH_NEWLINE);
        return ERR_INVALID_PARAMETER;
    }

    if ((equals - args) != 4 || str_cmpn(args, "PATH", 4) != 0 ||
        equals[1] == CH_NULL) {
        put_s("ERROR: Invalid set: ");
        put_s(args);
        put_c(CH_NEWLINE);
        return ERR_INVALID_PARAMETER;
    }

    char* start = equals + 1;
    char* p = start;
    uint8_t count = 0;
    while (1) {
        if (*p == '=') {
            put_s("ERROR: Invalid set: ");
            put_s(args);
            put_c(CH_NEWLINE);
            return ERR_INVALID_PARAMETER;
        }
        if (*p == ',' || *p == CH_NULL) {
            size_t len = p - start;
            if (len == 0 || count >= MAX_PATHS || len >= PATH_MAX ||
                (start[len - 1] != PATH_SEP && len >= PATH_MAX - 1)) {
                put_s("ERROR: Invalid PATH: ");
                put_s(args);
                put_c(CH_NEWLINE);
                return ERR_INVALID_PARAMETER;
            }
            count++;
            if (*p == CH_NULL)
                break;
            start = p + 1;
        }
        p++;
    }

    for (uint8_t i = 0; i < MAX_PATHS; i++)
        paths[i][0] = CH_NULL;

    start = equals + 1;
    for (uint8_t i = 0; i < count; i++) {
        p = start;
        while (*p != ',' && *p != CH_NULL)
            p++;
        zos_err_t err = set_path(paths[i], start, p - start);
        if (err)
            return err;
        start = p + 1;
    }

    path_count = count;
    return ERR_SUCCESS;
}
