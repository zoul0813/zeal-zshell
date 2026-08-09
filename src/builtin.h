#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdint.h>
#include <zos_vfs.h>

typedef struct {
    char name[FILENAME_LEN_MAX];
    uint8_t (*handler)(char* args);
} builtin_t;

extern const builtin_t builtins[];

uint8_t builtin(char* cmd, char* args);

uint8_t cmd_hash(char* args);
uint8_t cmd_cd(char* args);
uint8_t cmd_pwd(char* args);
uint8_t cmd_exit(char* args);
uint8_t cmd_help(char* args);
uint8_t cmd_exec(char* args);
uint8_t cmd_history(char* args);
uint8_t cmd_clear(char* args);
uint8_t cmd_set(char* args);
uint8_t cmd_which(char* args);
uint8_t cmd_true(char* args);
uint8_t cmd_false(char* args);
uint8_t cmd_ver(char* args);
uint8_t cmd_reset(char* args);

#endif
