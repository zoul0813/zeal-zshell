#include <stdio.h>
#include <string.h>
#include <zos_errors.h>
#include "paths.h"

void path_set(dir_t *dir, char *path) {
    dir->truncated = 0;
    strncpy(dir->path, path, PATH_MAX);
    char *p = dir->path;

    while(*p++ != ':');
    strncpy(dir->drive, dir->path, p - dir->path);

    uint8_t len = strlen(dir->path);
    p = &dir->path[len - 2];
    while(p >= dir->path && *--p != '/');
    strncpy(dir->folder, p, PATH_MAX);
    if(p > dir->path + 3) dir->truncated = 1;
}

zos_err_t path_set_cwd(dir_t *dir) {
    char cwd[PATH_MAX];
    zos_err_t err = curdir(cwd);
    if(err) return err;
    path_set(dir, cwd);
    return ERR_SUCCESS;
}