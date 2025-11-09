#include <stdint.h>
#include <zos_vfs.h>

typedef struct {
        char name[FILENAME_LEN_MAX];
        uint8_t (*handler)(char* args);
} builtin_t;

extern const builtin_t builtins[];

uint8_t builtin(char *cmd, char* args);
