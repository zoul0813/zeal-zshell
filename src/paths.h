#ifndef PATHS_H
#define PATHS_H

#include <stdint.h>
#include <zos_errors.h>
#include <zos_vfs.h>

#define PATH_SEP    '/'
#define DRIVE_SEP   ':'

typedef struct {
    char path[PATH_MAX];
    char drive[4];
    char folder[(FILENAME_LEN_MAX * 2) + 4]; // up to two dirs, plus the separator
    uint8_t truncated;
} dir_t;

void path_set(dir_t *dir, char *path);
zos_err_t path_set_cwd(dir_t *dir);

#endif