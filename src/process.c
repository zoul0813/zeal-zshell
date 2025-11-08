#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>

#include "config.h"
#include "common.h"
#include "process.h"
#include "paths.h"
#include "history.h"

static zos_err_t retval;

zos_err_t find_exec(unsigned char *name) {
    zos_stat_t zos_stat;

    unsigned char path[PATH_MAX];
    strncpy(path, name, PATH_MAX);

    zos_err_t err = stat(path, &zos_stat);
    if(!err) {
        return ERR_SUCCESS;
    }

    for(uint8_t i = 0; i < MAX_PATHS; i++) {
        if(paths[i][0] == '\0') break;

        strncpy(path, paths[i], PATH_MAX);
        strncat(path, name, PATH_MAX - 3);
        // printf("checking path: %s\n", path);
        err = stat(path, &zos_stat);
        if(!err) {
            strncpy(name, path, PATH_MAX);
            return ERR_SUCCESS;
        }
    }

    return ERR_NO_SUCH_ENTRY;
}

zos_err_t set_path(char *path, char* str, size_t len) {
    char value[PATH_MAX] = "";
    if(len >= PATH_MAX) {
        printf("ERROR: Path too long: %s\n", str);
        return ERR_INVALID_PATH;
    }
    strncpy(value, str, len);
    if(value[len-1] != '/') {
        value[len] = '/';
        len++;
    }
    value[len] = '\0';
    strncpy(path, value, PATH_MAX);
    return ERR_SUCCESS;
}

zos_err_t set(char* arg) {
    char name[FILENAME_LEN_MAX] = "";
    zos_err_t err = ERR_SUCCESS;

    char *equals = strchr(arg, '=');
    if(!equals) {
        if(strcmp(arg, "PATH") == 0) {
            for(uint8_t i = 0; i < MAX_PATHS; i++) {
                if(paths[i][0] == '\0') break;
                printf("%d: %s\n", i, paths[i]);
            }
            return ERR_SUCCESS;
        } else {
            printf("ERROR: Unknown variable: %s\n", arg);
            return ERR_INVALID_PARAMETER;
        }
    }

    char *p = arg;
    char *s = arg;
    uint8_t i = 0;
    while(*p != '\0') {
        switch(*p) {
            case '=': {
                // will hit this condition if we encounter a second "="
                if(name[0] != '\0') {
                    printf("ERROR: Invalid set: %s\n", arg);
                    return ERR_INVALID_PARAMETER;
                }
                size_t l = p - s;
                if(l >= FILENAME_LEN_MAX) {
                    printf("ERROR: Invalid variable length: %d, max %d\n", l, FILENAME_LEN_MAX);
                    return ERR_INVALID_PARAMETER;
                }
                strncpy(name, s, l);
                name[l] = '\0';

                if(strncmp(name, "PATH", FILENAME_LEN_MAX) != 0) {
                    printf("ERROR: Invalid variable name: %s\n", name);
                    return ERR_INVALID_PARAMETER;
                }

                for(uint8_t j = 0; j < MAX_PATHS; j++) {
                    paths[j][0] = '\0';
                }

                s = p + 1;
            } break;
            case ',': {
                // encountered the comma before setting the var name
                if(name[0] == '\0') {
                    printf("ERROR: Invalid set: %s\n", arg);
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

zos_err_t run(unsigned char *b) {
    zos_err_t err;

    uint16_t l = strlen(b);

    unsigned char cmd[PATH_MAX];
    strncpy(cmd, b, PATH_MAX);
    unsigned char args[PATH_MAX];
    args[0] = '\0'; // init to empty string

    if(l > 2 && cmd[0] == '.' && cmd[1] == '/') {
        strncpy(cmd, &b[2], PATH_MAX);
    } else {
        strncpy(cmd, b, PATH_MAX);
    }

    unsigned char *p = cmd;
    while(*p++ != '\0') {
        if(*p == ' ') {
            *p++ = '\0';
            if(*p != '\0') {
                strncpy(args, p, PATH_MAX);
            }
            break;
        }
    }

    if(strcmp(cmd, "#") == 0) {
        printf("%d\n", retval);
        return ERR_SUCCESS;
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
        return ERR_SUCCESS;
    }
    if(strcmp(cmd, "clear") == 0) {
        return ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    }
    if(strcmp(cmd, "set") == 0) {
        return set(args);
    }
    if(strcmp(cmd, "which") == 0) {
        err = find_exec(args);
        if(err) return ERR_SUCCESS; // do nothing
        printf("%s\n", args);
        return ERR_SUCCESS;
    }

    err = find_exec(cmd);
    if(err) goto do_return;

    unsigned char *argv = args;

    err = exec(EXEC_PRESERVE_PROGRAM, cmd, &argv, &retval);
    if(retval) return retval;
    // printf("\n");
    // if(!err && retval > 0) {
    //     err = retval;
    //     printf("Returned ");
    //     print_error(retval);
    // }

do_return:
    return err;
}
