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

static zos_err_t find_with_extension(unsigned char* name, const char* extension, unsigned char* result_path)
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

    // Try in all paths
    for (uint8_t i = 0; i < MAX_PATHS; i++) {
        if (paths[i][0] == '\0')
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

zos_err_t find_exec(unsigned char* name)
{
    unsigned char path[PATH_MAX];
    zos_err_t err;

    // Check if the name already has an extension
    unsigned char* dot   = strrchr(name, '.');
    unsigned char* slash = strrchr(name, '/');

    // If there's a dot after the last slash (or no slash), it has an extension
    int has_extension = (dot != NULL && (slash == NULL || dot > slash));

    // Try without extension (including current directory and all paths)
    err = find_with_extension(name, "", path);
    if (!err) {
        strncpy(name, path, PATH_MAX);
        return ERR_SUCCESS;
    }

    // Only try with extensions if the original name doesn't have one
    if (!has_extension) {
        // Try with .bin extension
        err = find_with_extension(name, ".bin", path);
        if (!err) {
            strncpy(name, path, PATH_MAX);
            return ERR_SUCCESS;
        }

        // Try with .zs extension
        err = find_with_extension(name, ".zs", path);
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
    strncpy(cmd, arg, PATH_MAX);
    unsigned char args[PATH_MAX];
    args[0] = '\0'; // init to empty string

    if (l > 2 && cmd[0] == '.' && cmd[1] == '/') {
        strncpy(cmd, &arg[2], PATH_MAX);
    } else {
        strncpy(cmd, arg, PATH_MAX);
    }

    unsigned char* p = cmd;
    while (*p++ != '\0') {
        if (*p == ' ') {
            *p++ = '\0';
            if (*p != '\0') {
                strncpy(args, p, PATH_MAX);
            }
            break;
        }
    }

    err = builtin(cmd, args);
    if (err != 0xFF)
        return err;

    err = find_exec(cmd);
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
