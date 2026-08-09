// #include <stdio.h>
// #include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>
#include <core.h>

#include "config.h"
#include "common.h"
#include "keyboard.h"
#include "builtin.h"
#include "process.h"
#include "paths.h"
#include "history.h"
#include "batch.h"

static zos_err_t copy_checked(unsigned char* dst, const unsigned char* src, uint16_t capacity)
{
    uint16_t len = str_len(src);
    if (len >= capacity)
        return ERR_PATH_TOO_LONG;

    str_cpyn(dst, src, len);
    dst[len] = CH_NULL;
    return ERR_SUCCESS;
}

static zos_err_t return_status(zos_err_t status)
{
    last_status = status;
    return status;
}

static zos_err_t find_with_extension(unsigned char* name, const char* extension, uint8_t shallow,
                                     unsigned char* result_path)
{
    zos_stat_t zos_stat;
    unsigned char path[PATH_MAX];
    zos_err_t err;
    uint16_t name_len = str_len(name);
    uint16_t ext_len = extension ? str_len(extension) : 0;

    // Try in current location first
    if (shallow) {
        if (name_len + ext_len >= PATH_MAX)
            return ERR_PATH_TOO_LONG;
        str_cpyn(path, name, name_len);
        str_cpyn(path + name_len, extension, ext_len);
        path[name_len + ext_len] = CH_NULL;
        err = stat(path, &zos_stat);
        if (err)
            return err;
        if (D_ISFILE(zos_stat.s_flags))
            return copy_checked(result_path, path, PATH_MAX);
        return ERR_NO_SUCH_ENTRY;
    }

    // Try in all paths
    for (uint8_t i = 0; i < path_count; i++) {
        uint16_t base_len = str_len(paths[i]);
        if (base_len + name_len + ext_len >= PATH_MAX)
            continue;
        str_cpyn(path, paths[i], base_len);
        str_cpyn(path + base_len, name, name_len);
        str_cpyn(path + base_len + name_len, extension, ext_len);
        path[base_len + name_len + ext_len] = CH_NULL;
        err = stat(path, &zos_stat);
        if (err == ERR_NO_SUCH_ENTRY)
            continue;
        if (err)
            return err;
        if (D_ISFILE(zos_stat.s_flags))
            return copy_checked(result_path, path, PATH_MAX);
    }

    return ERR_NO_SUCH_ENTRY;
}

zos_err_t find_exec(unsigned char* name, uint8_t shallow)
{
    unsigned char path[PATH_MAX];
    zos_err_t err;

    // Check if the name already has an extension
    unsigned char* dot   = str_chrr(name, '.');
    unsigned char* slash = str_chrr(name, PATH_SEP);

    // If there's a dot after the last slash (or no slash), it has an extension
    int has_extension = (dot != NULL && (slash == NULL || dot > slash));

    // Try without extension
    err = find_with_extension(name, "", shallow, path);
    if (!err) {
        return copy_checked(name, path, PATH_MAX);
    }
    if (err != ERR_NO_SUCH_ENTRY)
        return err;

    // Only try with extensions if not shallow and the original name doesn't have one
    if (!shallow && !has_extension) {
        // Try with .bin extension
        err = find_with_extension(name, ".bin", shallow, path);
        if (!err) {
            return copy_checked(name, path, PATH_MAX);
        }
        if (err != ERR_NO_SUCH_ENTRY)
            return err;

        // Try with .zs extension
        err = find_with_extension(name, ".zs", shallow, path);
        if (!err) {
            return copy_checked(name, path, PATH_MAX);
        }
        if (err != ERR_NO_SUCH_ENTRY)
            return err;
    }

    return ERR_NO_SUCH_ENTRY;
}

zos_err_t run(const char* arg)
{
    zos_err_t err;

    unsigned char cmd[PATH_MAX];
    unsigned char args[PATH_MAX];
    const unsigned char* command_start = (const unsigned char*)arg;
    const unsigned char* separator;
    uint16_t command_len;
    uint16_t args_len;
    uint16_t l = str_len(arg);
    args[0] = CH_NULL;

    uint8_t shallow = 0;

    if (l > 2 && arg[0] == '.' && arg[1] == PATH_SEP) {
        shallow = 1;
        command_start += 2;
    } else {
        if (l > 3 && arg[1] == ':' && arg[2] == PATH_SEP) {
            shallow = 1;
        }
    }

    separator = command_start;
    while (*separator != CH_NULL && *separator != CH_SPACE)
        separator++;

    command_len = separator - command_start;
    if (command_len == 0 || command_len >= PATH_MAX)
        return return_status(ERR_PATH_TOO_LONG);
    str_cpyn(cmd, command_start, command_len);
    cmd[command_len] = CH_NULL;

    if (*separator == CH_SPACE) {
        separator++;
        args_len = str_len(separator);
        if (args_len >= PATH_MAX)
            return return_status(ERR_PATH_TOO_LONG);
        str_cpyn(args, separator, args_len);
        args[args_len] = CH_NULL;
    }

    if (!shallow) {
        if (builtin(cmd, args, &err) == BUILTIN_MATCHED)
            return return_status(err);
    }

    err = find_exec(cmd, shallow);
    if (err)
        goto do_return;

    // Check if file is .zs and use batch_process() instead of exec()
    unsigned char* dot = str_chrr(cmd, '.');
    if (dot != NULL && str_cmp(dot, ".zs") == 0) {
        if (args[0] != CH_NULL)
            return return_status(ERR_INVALID_PARAMETER);
        err = batch_process(cmd, BATCH_QUIET);
        return return_status(err);
    }

    unsigned char* argv = args;
    zos_err_t child_status = ERR_SUCCESS;
    zos_err_t exec_err = exec(EXEC_PRESERVE_PROGRAM, cmd, &argv, &child_status);
    zos_err_t keyboard_err = kb_mode_non_block_raw();

    if (exec_err)
        return return_status(exec_err);
    if (keyboard_err)
        return return_status(keyboard_err);
    return return_status(child_status);

do_return:
    return return_status(err);
}
