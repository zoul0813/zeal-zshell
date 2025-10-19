#include <stdio.h>
#include <string.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zos_vfs.h>
#include <keyboard.h>
#include <ansi.h>

// PATH_MAX = 128
// FILENAME_LEN_MAX = 16
#define BUFFER_SIZE 256
static unsigned char buffer[BUFFER_SIZE];
static uint8_t pos = 0;
static uint16_t size;

int fflush_stdout(void); // defined in ZOS

int __exit(zos_err_t err) {
  if(err == ERR_SUCCESS) err = ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
  exit(err);
  return err;
}

void handle_error(zos_err_t err, char *msg, uint8_t fatal) {
  if(err != ERR_SUCCESS) {
    // cursor_xy(2,20); // why does ZMT move the cursor???
    printf("failed to %s, %d (%02x)\n", msg, err, err);
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

zos_err_t run(unsigned char *b) {
    zos_err_t err;
    b[pos] = '\0';
    // printf("PARSING: %s\n", b);

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
        printf("Returned %02x\n", retval);
    }

do_return:
    return err;
}

int main(int argc, char **argv) {
    (void*)argc;
    (void*)argv;

    zos_err_t err = kb_mode_non_block_raw();
    handle_error(err, "init keyboard", 1);

    for(;;) {
        printf("> ");
        fflush_stdout();
        for(;;) {
            kb_keys_t key = getkey();
            if(key == 0) continue;
            switch(key) {
                case KB_END:
                    goto quit;
                case KB_KEY_ENTER:
                    printf("\n");
                    err = run(buffer);
                    if(err) {
                        printf("ERROR: %02x\n", err);
                    }
                    buffer[0] = '\0';
                    pos = 0;
                    goto end_outer_loop;
                case KB_KEY_BACKSPACE: {
                    buffer[pos] = '\0';
                    pos--;
                    uint8_t x                = zvb_peri_text_curs_x - 1;
                    zvb_peri_text_curs_x     = x;
                    zvb_peri_text_print_char = '\0';
                    zvb_peri_text_curs_x     = x;
                } break;
                default:
                    unsigned char c = getch(key);
                    buffer[pos++] = c;
                    printf("%c", c);
                    fflush_stdout();
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