#include <stdio.h>
#include <string.h>
#include <zos_errors.h>

#include "common.h"
#include "paths.h"

void path_set(dir_t* dir, char* path)
{
    dir->truncated = 0;
    strncpy(dir->path, path, PATH_MAX - 1);
    dir->path[PATH_MAX - 1] = CH_NULL;

    // Find drive (up to and including DRIVE_SEP)
    char* p = dir->path;
    while (*p && *p != DRIVE_SEP) p++;
    if (*p == DRIVE_SEP)
        p++;
    strncpy(dir->drive, dir->path, p - dir->path);
    dir->drive[p - dir->path] = CH_NULL;

    // Start from after the drive, skip leading slash
    char* folder_start = p;
    if (*folder_start == PATH_SEP)
        folder_start++;

    // Find end of path
    char* end = folder_start;
    while (*end) end++;
    if (end > folder_start && *(end - 1) == PATH_SEP)
        end--; // Skip trailing slash

    // Count slashes backwards to find last 2 components
    uint8_t slashes  = 0;
    char* truncate_point = end;

    p = end - 1;
    while (p >= folder_start) {
        if (*p == PATH_SEP) {
            slashes++;
            if (slashes == 2) {
                truncate_point = p;
                dir->truncated = 1;
                break;
            }
        }
        p--;
    }

    // Build folder path
    dir->folder[0] = PATH_SEP;
    strcpy(dir->folder + 1, truncate_point == end ? folder_start : truncate_point + 1);
}

zos_err_t path_set_cwd(dir_t* dir)
{
    char cwd[PATH_MAX];
    zos_err_t err = curdir(cwd);
    if (err)
        return err;
    path_set(dir, cwd);
    return ERR_SUCCESS;
}
