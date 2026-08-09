#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdint.h>
#include <zos_errors.h>
#include <zos_vfs.h>

typedef struct {
    char name[FILENAME_LEN_MAX];
    zos_err_t (*handler)(char* args);
} builtin_t;

typedef enum {
    BUILTIN_NOT_MATCHED = 0,
    BUILTIN_MATCHED,
} builtin_match_t;

extern const builtin_t builtins[];

builtin_match_t builtin(char* cmd, char* args, zos_err_t* status);

zos_err_t cmd_hash(char* args);
zos_err_t cmd_cd(char* args);
zos_err_t cmd_pwd(char* args);
zos_err_t cmd_exit(char* args);
zos_err_t cmd_help(char* args);
zos_err_t cmd_exec(char* args);
zos_err_t cmd_history(char* args);
zos_err_t cmd_clear(char* args);
zos_err_t cmd_set(char* args);
zos_err_t cmd_which(char* args);
zos_err_t cmd_true(char* args);
zos_err_t cmd_false(char* args);
zos_err_t cmd_ver(char* args);
zos_err_t cmd_reset(char* args);

#endif /* BUILTIN_H */
