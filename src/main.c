#include <stdio.h>
#include <string.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zos_vfs.h>
#include <keyboard.h>
#include <ansi.h>

#include "common.h"
#include "history.h"
#include "paths.h"
#include "autoexec.h"
#include "process.h"

// PATH_MAX = 128
// FILENAME_LEN_MAX = 16
static unsigned char buffer[COMMAND_MAX];
static uint8_t pos = 0;
static uint16_t size;
zos_err_t err;

void prompt(char *cmd) {
    printf("\r%s", cwd.drive);
    if(cwd.truncated) printf("...");
    printf("%s> ", cwd.folder);
    if(cmd != NULL) {
        printf("%s", cmd);
    }
    fflush_stdout();
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
    prompt(node->str);
    fflush_stdout();
    strncpy(buffer, node->str, COMMAND_MAX - 1);
    buffer[COMMAND_MAX - 1] = '\0';
    pos = strlen(buffer);
}

int main(int argc, char **argv) {
    (void*)argc;
    (void*)argv;

    err = path_set_cwd(&cwd);
    handle_error(err, "path_set_cwd", 1);

    autoexec_process();

    history_init(&history);
    history_node = NULL;

    err = kb_mode_non_block_raw();
    handle_error(err, "init keyboard", 1);

    for(;;) {
        prompt(NULL);
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
                    prompt(NULL);
                    fflush_stdout();
                } break;

                case KB_KEY_ENTER: {
                    buffer[pos] = '\0';
                    history_add(&history, buffer);
                    history_node = history.tail;

                    printf("\n");
                    err = run(buffer);
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
                    // if(pos > COMMAND_MAX - 1) break;
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