#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>

#include "common.h"
#include "process.h"
#include "paths.h"
#include "history.h"


extern dir_t cwd;
extern History history;

zos_err_t find_exec(unsigned char *name) {
    zos_stat_t zos_stat;

    unsigned char path[PATH_MAX];
    strncpy(path, name, PATH_MAX);
    // printf("FIND: %s\n", path);

    zos_err_t err = stat(path, &zos_stat);
    if(!err) {
        // printf("FOUND:0: %s\n", path);
        return ERR_SUCCESS;
    }

    strcpy(path, "B:/");
    strncat(path, name, PATH_MAX - 3);
    // printf("CHECKING: %s\n", path);
    err = stat(path, &zos_stat);
    if(!err) {
        strncpy(name, path, PATH_MAX);
        // printf("FOUND:1: %s\n", name);
        return ERR_SUCCESS;
    }

    strcpy(path, "A:/");
    strncat(path, name, PATH_MAX - 3);
    // printf("CHECKING: %s\n", path);
    err = stat(path, &zos_stat);
    if(!err) {
        strncpy(name, path, PATH_MAX);
        // printf("FOUND:1: %s\n", name);
        return ERR_SUCCESS;
    }

    return ERR_NO_SUCH_ENTRY;
}

zos_err_t run(unsigned char *b) {
    zos_err_t err;

    uint16_t l = strlen(b);

    unsigned char cmd[PATH_MAX];
    strncpy(cmd, b, PATH_MAX);
    unsigned char args[PATH_MAX];
    args[0] = '\0'; // init to empty string

    if(l > 2 && cmd[0] == '.' && cmd[1] == '/') {
        // exec
        strncpy(cmd, &b[2], PATH_MAX);
        // cmd = &cmd[2];
    } else {
        strncpy(cmd, b, PATH_MAX);
    }

    unsigned char *p = cmd;
    while(*p++ != '\0') {
        if(*p == ' ') {
            *p++ = '\0';
            if(*p != '\0') {
                strncpy(args, p, PATH_MAX);
                // argv = p++;
            }
            break;
        }
    }

    if(strcmp(cmd, "cd") == 0) {
        err = chdir(args);
        handle_error(err, "change dir", 0);
        return path_set_cwd(&cwd);
    }
    if(strcmp(cmd, "pwd") == 0) {
        printf("%s\n", cwd.path);
        return ERR_SUCCESS;
    }
    if(strcmp(cmd, "exit") == 0) {
        return __exit(ERR_SUCCESS);
    }
    if(strcmp(cmd, "history") == 0) {
        HistoryNode *node = history.tail;
        while(node) {
            printf("  %s\n", node->str);
            node = node->prev;
        }
        return 0;
    }
    if(strcmp(cmd, "clear") == 0) {
        return ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    }
    if(strcmp(cmd, "set") == 0) {
        printf("%s not implemented\n", cmd);
        return ERR_NOT_IMPLEMENTED;
    }

    // printf("CMD: %s\n", cmd);
    // printf("ARGS: %s\n", args);

    err = find_exec(cmd);
    if(err) goto do_return;

    // printf("EXEC:CMD: '%s'\n", cmd);
    // printf("EXEC:ARGS: '%s'\n", args);

    unsigned char *argv = args;

    uint8_t retval;
    err = exec(EXEC_PRESERVE_PROGRAM, cmd, &argv, &retval);
    if(!err && retval > 0) {
        printf("Returned ");
        print_error(retval);
    }

do_return:
    return err;
}
