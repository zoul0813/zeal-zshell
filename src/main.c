#include <stdio.h>
#include <string.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zos_vfs.h>
#include <keyboard.h>
#include <ansi.h>

#include "history.h"

#define STATUS_OK   0
#define STATUS_EXIT 1

// PATH_MAX = 128
// FILENAME_LEN_MAX = 16
static unsigned char buffer[COMMAND_MAX];
static uint8_t pos = 0;
static uint16_t size;
static uint8_t status;

static History history;
static HistoryNode *history_node;

const char *ERROR_STRINGS[] = {
    "ERR_SUCCESS",
    "ERR_FAILURE",
    "ERR_NOT_IMPLEMENTED",
    "ERR_NOT_SUPPORTED",
    "ERR_NO_SUCH_ENTRY",
    "ERR_INVALID_SYSCALL",
    "ERR_INVALID_PARAMETER",
    "ERR_INVALID_VIRT_PAGE",
    "ERR_INVALID_PHYS_ADDRESS",
    "ERR_INVALID_OFFSET",
    "ERR_INVALID_NAME",
    "ERR_INVALID_PATH",
    "ERR_INVALID_FILESYSTEM",
    "ERR_INVALID_FILEDEV",
    "ERR_PATH_TOO_LONG",
    "ERR_ALREADY_EXIST",
    "ERR_ALREADY_OPENED",
    "ERR_ALREADY_MOUNTED",
    "ERR_READ_ONLY",
    "ERR_BAD_MODE",
    "ERR_CANNOT_REGISTER_MORE",
    "ERR_NO_MORE_ENTRIES",
    "ERR_NO_MORE_MEMORY",
    "ERR_NOT_A_DIR",
    "ERR_NOT_A_FILE",
    "ERR_ENTRY_CORRUPTED",
    "ERR_DIR_NOT_EMPTY",
    "INVALID_ERROR_CODE",
};
const uint8_t ERROR_STRINGS_LEN = sizeof(ERROR_STRINGS) / sizeof(ERROR_STRINGS[0]);

int fflush_stdout(void); // defined in ZOS

int __exit(zos_err_t err) {
//   if(err == ERR_SUCCESS) err = ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
  exit(err);
  return err;
}

void print_error(uint8_t code) {
    if(code >= ERROR_STRINGS_LEN) code = ERROR_STRINGS_LEN-1;
    printf("ERROR($%02x): %s\n", code, ERROR_STRINGS[code]);
}

void handle_error(zos_err_t err, char *msg, uint8_t fatal) {
  if(err != ERR_SUCCESS) {
    printf("failed to %s, ", msg);
    print_error(err);
    if(fatal) __exit(err);
  }
}

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

void clear_command(void) {
    printf("\r");
    for(uint8_t i = 0; i < pos+4; i++) {
        printf(" ");
    }
    buffer[0] = '\0';
    pos = 0;
    fflush_stdout();
}

void use_history(HistoryNode *node) {
    if(!node) return;
    clear_command();
    printf("\r> %s", node->str);
    fflush_stdout();
    strncpy(buffer, node->str, COMMAND_MAX - 1);
    buffer[COMMAND_MAX - 1] = '\0';
    pos = strlen(buffer);
}

zos_err_t run(unsigned char *b, uint8_t* s) {
    *s = STATUS_OK;
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
        return chdir(args);
    }
    if(strcmp(cmd, "pwd") == 0) {
        err = curdir(buffer);
        handle_error(err, "Failed", 1);
        printf("%s\n", buffer);
        buffer[0] = '\0';
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

int main(int argc, char **argv) {
    (void*)argc;
    (void*)argv;

    history_init(&history);
    history_node = NULL;

    zos_err_t err = kb_mode_non_block_raw();
    handle_error(err, "init keyboard", 1);

    for(;;) {
        printf("> ");
        fflush_stdout();
        for(;;) {
            kb_keys_t key = getkey();
            if(key == 0) continue;
            switch(key) {
                case KB_END: {
                    goto quit;
                }

                // History navigation
                case KB_UP_ARROW: {
                    if(!history_node) {
                        history_node = history.tail;
                    } else {
                        history_node = history_node->prev;
                        if(!history_node) history_node = history.tail;
                    }
                    use_history(history_node);

                } break;
                case KB_DOWN_ARROW: {
                    if(!history_node) {
                        history_node = history.head;
                    } else {
                        history_node = history_node->next;
                        if(!history_node) history_node = history.head;
                    }
                    use_history(history_node);
                } break;
                case KB_ESC: {
                    history_node = NULL;
                    clear_command();
                    printf("\r> ");
                    fflush_stdout();
                } break;

                case KB_KEY_ENTER: {
                    buffer[pos] = '\0';
                    history_add(&history, buffer);
                    history_node = history.tail;

                    printf("\n");
                    err = run(buffer, &status);
                    if(err) print_error(err);

                    buffer[0] = '\0';
                    pos = 0;
                } goto end_outer_loop;
                case KB_KEY_BACKSPACE: {
                    if(pos == 0) break;
                    pos--;
                    buffer[pos] = '\0';
                    uint8_t x                = zvb_peri_text_curs_x - 1;
                    zvb_peri_text_curs_x     = x;
                    zvb_peri_text_print_char = '\0';
                    zvb_peri_text_curs_x     = x;
                } break;
                default: {
                    if(pos > COMMAND_MAX - 1) break;
                    unsigned char c = getch(key);
                    if(c < 0x20 || c > 0x7D) break; // unprintable
                    buffer[pos++] = c;
                    printf("%c", c);
                    fflush_stdout();
                } break;
            }
        }
end_outer_loop:
    }
quit:

    err = kb_mode_default();
    handle_error(err, "reset keyboard", 1);
    printf("\n");
    return 0;
}