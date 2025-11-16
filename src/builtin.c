#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>

#include "config.h"
#include "common.h"
#include "paths.h"
#include "history.h"
#include "process.h"
#include "builtin.h"
#include "version.h"

static zos_err_t retval;

// Individual command functions
static uint8_t cmd_hash(char* args)
{
    (void*) args;
    printf("%d\n", retval);
    return ERR_SUCCESS;
}

static uint8_t cmd_cd(char* args)
{
    uint8_t l = strlen(args);
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
    printf("%s\n", cwd.path);
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
        printf("  %s\n", node->str);
        node = node->prev;
    }
    return ERR_SUCCESS;
}

static uint8_t cmd_clear(char* args)
{
    (void*) args;
    return ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
}

zos_err_t set_path(char *path, char* str, size_t len) {
    char value[PATH_MAX] = "";
    if(len >= PATH_MAX) {
        printf("ERROR: Path too long: %s\n", str);
        return ERR_INVALID_PATH;
    }
    strncpy(value, str, len);
    if(value[len-1] != PATH_SEP) {
        value[len] = PATH_SEP;
        len++;
    }
    value[len] = CH_NULL;
    strncpy(path, value, PATH_MAX);
    return ERR_SUCCESS;
}

static uint8_t cmd_set(char* args)
{
    char name[FILENAME_LEN_MAX] = "";
    zos_err_t err = ERR_SUCCESS;

    char *equals = strchr(args, '=');
    if(!equals) {
        if(strcmp(args, "PATH") == 0) {
            for(uint8_t i = 0; i < MAX_PATHS; i++) {
                if(paths[i][0] == CH_NULL) break;
                printf("%d: %s\n", i, paths[i]);
            }
            return ERR_SUCCESS;
        } else {
            printf("ERROR: Unknown variable: %s\n", args);
            return ERR_INVALID_PARAMETER;
        }
    }

    char *p = args;
    char *s = args;
    uint8_t i = 0;
    while(*p != CH_NULL) {
        switch(*p) {
            case '=': {
                // will hit this condition if we encounter a second "="
                if(name[0] != CH_NULL) {
                    printf("ERROR: Invalid set: %s\n", args);
                    return ERR_INVALID_PARAMETER;
                }
                size_t l = p - s;
                if(l >= FILENAME_LEN_MAX) {
                    printf("ERROR: Invalid variable length: %d, max %d\n", l, FILENAME_LEN_MAX);
                    return ERR_INVALID_PARAMETER;
                }
                strncpy(name, s, l);
                name[l] = CH_NULL;

                if(strncmp(name, "PATH", FILENAME_LEN_MAX) != 0) {
                    printf("ERROR: Invalid variable name: %s\n", name);
                    return ERR_INVALID_PARAMETER;
                }

                for(uint8_t j = 0; j < MAX_PATHS; j++) {
                    paths[j][0] = CH_NULL;
                }

                s = p + 1;
            } break;
            case ',': {
                // encountered the comma before setting the var name
                if(name[0] == CH_NULL) {
                    printf("ERROR: Invalid set: %s\n", args);
                    return ERR_INVALID_PARAMETER;
                }
                size_t l = p - s;
                err = set_path(paths[i], s, l);
                if(err) return err;
                s = p + 1;
                i++;
            }
        }
        p++;
    }
    if(p > s) {
        size_t l = p - s;
        err = set_path(paths[i], s, l);
    }

    return err;
}

static uint8_t cmd_which(char* args)
{
    for (int i = 0; builtins[i].handler != NULL; i++) {
        if (strcmp(args, builtins[i].name) == 0) {
            printf("built-in: %s\n", args);
            return ERR_SUCCESS;
        }
    }

    zos_err_t err = find_exec(args, 0);
    if (err) return ERR_NO_SUCH_ENTRY;
    printf("%s\n", args);
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
    printf("%s v%s-%d\n", APP_NAME, APP_VERSION_STRING, APP_VERSION_BUILD);
    return ERR_SUCCESS;
}

static uint8_t cmd_reset(char * args) {
    __asm__(
        "rst 0\n"
    );
}

// Lookup table
const builtin_t builtins[] = {
    {      "#",    cmd_hash},
    {     "cd",      cmd_cd},
    {    "pwd",     cmd_pwd},
    {   "exit",    cmd_exit},
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

uint8_t builtin(char* cmd, char* args)
{
    for (int i = 0; builtins[i].handler != NULL; i++) {
        if (strcmp(cmd, builtins[i].name) == 0) {
            return builtins[i].handler(args);
        }
    }
    return 0xFF;
}
