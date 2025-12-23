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
    uint8_t l = str_len(args);
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
    if (len >= PATH_MAX) {
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
    str_cpyn(path, value, PATH_MAX);
    return ERR_SUCCESS;
}

static uint8_t cmd_set(char* args)
{
    char name[FILENAME_LEN_MAX] = "";
    zos_err_t err               = ERR_SUCCESS;

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

    char* p   = args;
    char* s   = args;
    uint8_t i = 0;
    while (*p != CH_NULL) {
        switch (*p) {
            case '=': {
                // will hit this condition if we encounter a second "="
                if (name[0] != CH_NULL) {
                    put_s("ERROR: Invalid set: ");
                    put_s(args);
                    put_c(CH_NEWLINE);
                    return ERR_INVALID_PARAMETER;
                }
                size_t l = p - s;
                if (l >= FILENAME_LEN_MAX) {
                    put_s("ERROR: Invalid variable length: ");
                    put_u16(l);
                    put_s(", max ");
                    put_u16(FILENAME_LEN_MAX);
                    put_c(CH_NEWLINE);
                    return ERR_INVALID_PARAMETER;
                }
                str_cpyn(name, s, l);
                name[l] = CH_NULL;

                if (str_cmpn(name, "PATH", FILENAME_LEN_MAX) != 0) {
                    put_s("ERROR: Invalid variable name: ");
                    put_s(name);
                    put_c(CH_NEWLINE);
                    return ERR_INVALID_PARAMETER;
                }

                for (uint8_t j = 0; j < MAX_PATHS; j++) {
                    paths[j][0] = CH_NULL;
                }

                s = p + 1;
            } break;
            case ',': {
                // encountered the comma before setting the var name
                if (name[0] == CH_NULL) {
                    put_s("ERROR: Invalid set: ");
                    put_s(args);
                    put_c(CH_NEWLINE);
                    return ERR_INVALID_PARAMETER;
                }
                size_t l = p - s;
                err      = set_path(paths[i], s, l);
                if (err)
                    return err;
                s = p + 1;
                i++;
            }
        }
        p++;
    }
    if (p > s) {
        size_t l = p - s;
        err      = set_path(paths[i], s, l);
    }

    return err;
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
    str_cpyn(cmd, search_name, PATH_MAX);

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
