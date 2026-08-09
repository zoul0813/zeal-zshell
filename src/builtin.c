// #include <stdio.h>
// #include <string.h>
#include <stdint.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>
#include <core.h>

#include "config.h"
#include "common.h"
#include "keyboard.h"
#include "paths.h"
#include "batch.h"
#include "history.h"
#include "process.h"
#include "builtin.h"
#include "version.h"

static zos_err_t retval;

// Individual command functions
static uint8_t cmd_hash(char* args)
{
    (void*) args;
    put_u8(retval);
    put_c(CH_NEWLINE);
    return ERR_SUCCESS;
}

static uint8_t cmd_cd(char* args)
{
    uint16_t l = str_len(args);
    if (l == 0 || (args[l - 1] != PATH_SEP && l >= PATH_MAX - 1)) {
        put_s("cd <path>\n");
        return ERR_INVALID_PARAMETER;
    }
    if (args[l - 1] != PATH_SEP) {
        args[l]     = PATH_SEP;
        args[l + 1] = CH_NULL;
    }

    zos_err_t err = chdir(args);
    handle_error(err, "change dir", 0);
    return path_set_cwd(&cwd);
}

static uint8_t cmd_pwd(char* args)
{
    (void*) args;
    put_s(cwd.path);
    put_c(CH_NEWLINE);
    return ERR_SUCCESS;
}

static uint8_t cmd_exit(char* args)
{
    (void*) args;
    return __exit(ERR_SUCCESS);
}

static uint8_t cmd_history(char* args)
{
    (void*) args;
    HistoryNode* node = history.tail;
    while (node) {
        put_s("  ");
        put_s(node->str);
        put_c(CH_NEWLINE);
        node = node->prev;
    }
    return ERR_SUCCESS;
}

static uint8_t cmd_clear(char* args)
{
    (void*) args;
    return ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
}

zos_err_t set_path(char* path, char* str, size_t len)
{
    char value[PATH_MAX] = "";
    if (len == 0 || len >= PATH_MAX || (str[len - 1] != PATH_SEP && len >= PATH_MAX - 1)) {
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

static uint8_t cmd_set(char* args)
{
    char* equals = str_chr(args, '=');
    if (!equals) {
        if (str_cmp(args, "PATH") == 0) {
            for (uint8_t i = 0; i < MAX_PATHS; i++) {
                if (paths[i][0] == CH_NULL)
                    break;
                put_u8(i);
                put_c(CH_SPACE);
                put_s(paths[i]);
                put_c(CH_NEWLINE);
            }
            return ERR_SUCCESS;
        } else {
            put_s("ERROR: Unknown variable: ");
            put_s(args);
            put_c(CH_NEWLINE);
            return ERR_INVALID_PARAMETER;
        }
    }

    if ((equals - args) != 4 || str_cmpn(args, "PATH", 4) != 0 || equals[1] == CH_NULL) {
        put_s("ERROR: Invalid set: ");
        put_s(args);
        put_c(CH_NEWLINE);
        return ERR_INVALID_PARAMETER;
    }

    // Validate the complete assignment before changing the existing PATH.
    char* start = equals + 1;
    char* p = start;
    uint8_t count = 0;
    while (1) {
        if (*p == '=' ) {
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

    return ERR_SUCCESS;
}

static uint8_t cmd_which(char* args)
{
    uint8_t shallow   = 0;
    char* search_name = args;

    // Check if it starts with ./
    if (str_len(args) > 2 && args[0] == '.' && args[1] == PATH_SEP) {
        shallow     = 1;
        search_name = &args[2]; // Strip the ./ prefix
    }

    // Only check builtins if not using ./
    if (!shallow) {
        for (int i = 0; builtins[i].handler != NULL; i++) {
            if (str_cmp(search_name, builtins[i].name) == 0) {
                put_s("built-in: ");
                put_s(search_name);
                put_c(CH_NEWLINE);
                return ERR_SUCCESS;
            }
        }
    }

    // Need to copy to a mutable buffer since find_exec modifies it
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

static uint8_t cmd_true(char* args)
{
    (void*) args;
    return ERR_SUCCESS;
}

static uint8_t cmd_false(char* args)
{
    (void*) args;
    return ERR_FAILURE;
}

static uint8_t cmd_ver(char* args)
{
    (void*) args;
    put_s(APP_NAME);
    put_c(CH_SPACE);
    put_s(APP_VERSION_STRING);
    put_c('-');
    put_u8(APP_VERSION_BUILD);
    put_c(CH_NEWLINE);
    return ERR_SUCCESS;
}

static uint8_t cmd_reset(char* args)
{
    (void*) args;
    __asm__("rst 0\n");
    return 0;
}

static uint8_t cmd_exec(char* args)
{
    put_s("exec '");
    put_s(args);
    put_s("'\n");
    return run(args);
}

static uint8_t cmd_help(char* args); // declare it

// Lookup table
const builtin_t builtins[] = {
    {      "#",    cmd_hash},
    {     "cd",      cmd_cd},
    {    "pwd",     cmd_pwd},
    {   "exit",    cmd_exit},
    {   "help",    cmd_help},
    {   "exec",    cmd_exec},
    {"history", cmd_history},
    {  "clear",   cmd_clear},
    {    "set",     cmd_set},
    {  "which",   cmd_which},
    {   "true",    cmd_true},
    {  "false",   cmd_false},
    {    "ver",     cmd_ver},
    {  "reset",   cmd_reset},
    {       "",        NULL}  // sentinel
};
const uint8_t builtsin_len = sizeof(builtins) / sizeof(builtin_t);


static uint8_t cmd_help(char* args) {
    (void*)args;
    for(uint8_t i = 0; i < builtsin_len; i++) {
        put_s(builtins[i].name);
        put_c(CH_NEWLINE);
    }
    return 0;
}

uint8_t builtin(char* cmd, char* args)
{
    for (int i = 0; builtins[i].handler != NULL; i++) {
        if (str_cmp(cmd, builtins[i].name) == 0) {
            return builtins[i].handler(args);
        }
    }
    return 0xFF;
}
