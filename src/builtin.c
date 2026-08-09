#include <stddef.h>
#include <stdint.h>
#include <zos_errors.h>
#include <core.h>

#include "config.h"
#include "builtin.h"

const builtin_t builtins[] = {
#if CONFIG_BUILTIN_HASH
    {      "#",    cmd_hash},
#endif
    {     "cd",      cmd_cd},
#if CONFIG_BUILTIN_PWD
    {    "pwd",     cmd_pwd},
#endif
#if CONFIG_BUILTIN_EXIT
    {   "exit",    cmd_exit},
#endif
    {   "help",    cmd_help},
#if CONFIG_BUILTIN_EXEC
    {   "exec",    cmd_exec},
#endif
#if HISTORY_ENABLED
    {"history", cmd_history},
#endif
#if CONFIG_BUILTIN_CLEAR
    {  "clear",   cmd_clear},
#endif
    {    "set",     cmd_set},
#if CONFIG_BUILTIN_WHICH
    {  "which",   cmd_which},
#endif
#if CONFIG_BUILTIN_TRUE
    {   "true",    cmd_true},
#endif
#if CONFIG_BUILTIN_FALSE
    {  "false",   cmd_false},
#endif
#if CONFIG_BUILTIN_VER
    {    "ver",     cmd_ver},
#endif
#if CONFIG_BUILTIN_RESET
    {  "reset",   cmd_reset},
#endif
    {       "",        NULL}
};

uint8_t builtin(char* cmd, char* args)
{
    for (uint8_t i = 0; builtins[i].handler != NULL; i++) {
        if (str_cmp(cmd, builtins[i].name) == 0)
            return builtins[i].handler(args);
    }
    return 0xFF;
}
