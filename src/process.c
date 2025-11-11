#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>

#include "config.h"
#include "common.h"
#include "builtin.h"
#include "process.h"
#include "paths.h"
#include "history.h"
#include "batch.h"

static zos_err_t retval;

static zos_err_t find_with_extension(unsigned char* name, const char* extension, uint8_t shallow, unsigned char* result_path)
{
    zos_stat_t zos_stat;
    unsigned char path[PATH_MAX];
    zos_err_t err;

    // Try in current location first
    strncpy(path, name, PATH_MAX);
    if (extension && strlen(extension) > 0) {
        strncat(path, extension, PATH_MAX - strlen(path) - 1);
    }
    err = stat(path, &zos_stat);
    if (!err && D_ISFILE(zos_stat.s_flags)) {
        strncpy(result_path, path, PATH_MAX);
        return ERR_SUCCESS;
    }

    if(shallow) return ERR_NO_SUCH_ENTRY;

    // Try in all paths
    for (uint8_t i = 0; i < MAX_PATHS; i++) {
        if (paths[i][0] == CH_NULL)
            break;

        strncpy(path, paths[i], PATH_MAX);
        int ext_len = (extension && strlen(extension) > 0) ? strlen(extension) : 0;
        strncat(path, name, PATH_MAX - strlen(path) - ext_len - 1);
        if (ext_len > 0) {
            strncat(path, extension, PATH_MAX - strlen(path) - 1);
        }
        err = stat(path, &zos_stat);
        if (!err && D_ISFILE(zos_stat.s_flags)) {
            strncpy(result_path, path, PATH_MAX);
            return ERR_SUCCESS;
        }
    }

    return ERR_NO_SUCH_ENTRY;
}

zos_err_t find_exec(unsigned char* name, uint8_t shallow)
{
    unsigned char path[PATH_MAX];
    zos_err_t err;

    // Check if the name already has an extension
    unsigned char* dot   = strrchr(name, '.');
    unsigned char* slash = strrchr(name, PATH_SEP);

    // If there's a dot after the last slash (or no slash), it has an extension
    int has_extension = (dot != NULL && (slash == NULL || dot > slash));

    // Try without extension
    err = find_with_extension(name, "", shallow, path);
    if (!err) {
        strncpy(name, path, PATH_MAX);
        return ERR_SUCCESS;
    }

    // Only try with extensions if not shallow and the original name doesn't have one
    if (!shallow && !has_extension) {
        // Try with .bin extension
        err = find_with_extension(name, ".bin", shallow, path);
        if (!err) {
            strncpy(name, path, PATH_MAX);
            return ERR_SUCCESS;
        }

        // Try with .zs extension
        err = find_with_extension(name, ".zs", shallow, path);
        if (!err) {
            strncpy(name, path, PATH_MAX);
            return ERR_SUCCESS;
        }
    }

    return ERR_NO_SUCH_ENTRY;
}

zos_err_t run(const char* arg)
{
    zos_err_t err;

    uint16_t l = strlen(arg);

    unsigned char cmd[PATH_MAX];
    // strncpy(cmd, arg, PATH_MAX);
    unsigned char args[PATH_MAX];
    args[0] = CH_NULL; // init to empty string

    uint8_t shallow = 0;

    if (l > 2 && arg[0] == '.' && arg[1] == PATH_SEP) {
        shallow = 1;
        strncpy(cmd, &arg[2], PATH_MAX);
    } else {
        strncpy(cmd, arg, PATH_MAX);
    }

    unsigned char* p = cmd;
    while (*p++ != CH_NULL) {
        if (*p == CH_SPACE) {
            *p++ = CH_NULL;
            if (*p != CH_NULL) {
                strncpy(args, p, PATH_MAX);
            }
            break;
        }
    }

    if(!shallow) {
        err = builtin(cmd, args);
        if (err != 0xFF) return err;
    }

    err = find_exec(cmd, shallow);
    if (err)
        goto do_return;

    // Check if file is .zs and use batch_process() instead of exec()
    unsigned char* dot = strrchr(cmd, '.');
    if (dot != NULL && strcmp(dot, ".zs") == 0) {
        err = batch_process(cmd, BATCH_QUIET);
        return err;
    }

    unsigned char* argv = args;

    err = exec(EXEC_PRESERVE_PROGRAM, cmd, &argv, &retval);
    if (retval)
        return retval;

do_return:
    return err;
}
