// #include <stdio.h>
// #include <string.h>
#include <stddef.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zos_vfs.h>
#include <zvb_hardware.h>
#include <core.h>
#include <keyboard.h>

#include "config.h"
#include "common.h"
#include "history.h"
#include "paths.h"
#include "batch.h"
#include "process.h"

static unsigned char buffer[COMMAND_MAX];
static uint8_t pos = 0;
static uint16_t size;
static zos_err_t err;
static zos_stat_t zos_stat;

void prompt(char *cmd) {
    setcolor(TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_BLACK);
    put_c(CH_RETURN);
    put_s(cwd.drive);
    if(cwd.truncated) put_s("/...");
    put_s(cwd.folder);
    put_c('>');
    if(cmd != NULL) {
        put_s(cmd);
    }
    fflush_stdout();
    setcolor(TEXT_COLOR_WHITE, TEXT_COLOR_BLACK);
}

void clear_command(void) {
    put_c(CH_RETURN);
    for(uint8_t i = 0; i < pos+4; i++) {
        put_c(CH_SPACE);
    }
    buffer[0] = CH_NULL;
    pos = 0;
    fflush_stdout();
}

void use_history(HistoryNode *node) {
    if(!node) return;
    clear_command();
    prompt(node->str);
    fflush_stdout();
    str_cpyn(buffer, node->str, COMMAND_MAX - 1);
    buffer[COMMAND_MAX - 1] = CH_NULL;
    pos = str_len(buffer);
}

void usage(void) {
    put_s("usage: zshell [-options] [path]\n");
    put_s("   q - quiet mode\n");
}

batch_options_e parse_args(char **argv, char *path) {
    char* params = argv[0];
    batch_options_e options = BATCH_NONE;

    if(*params == '-') {
        params++;
        while(params) {
            switch(*params) {
                case 'q': {
                    options |= BATCH_QUIET;
                } break;
                case 'h': {
                    usage();
                    exit(ERR_SUCCESS);
                    return options;
                } break;
                case CH_NULL:
                case CH_SPACE:
                    goto parsed;
                default: {
                    put_s("Invalid option: ");
                    put_s(params);
                    put_c(CH_NEWLINE);
                    usage();
                    return ERR_INVALID_PARAMETER;
                } break;
            }
            params++;
        }
    }
parsed:
    while(*params == CH_SPACE) params++;
    if(*params != 0) {
        str_cpy(path, params);
    }
    return options;
}

int main(int argc, char **argv) {
    err = path_set_cwd(&cwd);
    handle_error(err, "path_set_cwd", 1);

    for(uint8_t i = 0; i < MAX_PATHS; i++) {
        paths[i][0] = CH_NULL;
    }
    str_cpy(paths[0], "A:/");

    if(argc == 1) {
        char path[PATH_MAX];
        batch_options_e options = parse_args(argv, path);
        err = batch_process(path, options);
        return err;
    }

    run("ver"); put_c(CH_NEWLINE);

#if AUTOEXEC_ENABLED
    err = stat(AUTOEXEC_FILENAME, &zos_stat);
    if(!err) {
        batch_process(AUTOEXEC_FILENAME, BATCH_QUIET);
    } else {
        put_s("Could not load ");
        put_s(AUTOEXEC_FILENAME);
        put_c(CH_NEWLINE);
    }
#endif

#if HISTORY_ENABLED
    history_init(&history);
    history_node = NULL;
#endif

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

#if HISTORY_ENABLED
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
#endif

                case KB_KEY_ENTER: {
                    put_c(CH_NEWLINE);
                    if(pos < 1) goto end_outer_loop;
                    buffer[pos] = CH_NULL;
#if HISTORY_ENABLED
                    history_add(&history, buffer);
                    history_node = history.tail;
#endif

                    err = run(buffer);
                    if(err) print_error(err);

                    buffer[0] = CH_NULL;
                    pos = 0;
                } goto end_outer_loop;
                case KB_KEY_BACKSPACE: {
                    if(pos == 0) break;
                    pos--;
                    buffer[pos] = CH_NULL;
                    uint8_t x                = zvb_peri_text_curs_x - 1;
                    zvb_peri_text_curs_x     = x;
                    zvb_peri_text_print_char = CH_NULL;
                    zvb_peri_text_curs_x     = x;
                } break;
                default: {
                    // if(pos > COMMAND_MAX - 1) break;
                    unsigned char c = getch(key);
                    if(c < 0x20 || c > 0x7D) break; // unprintable
                    buffer[pos++] = c;
                    put_c(c);
                    fflush_stdout();
                } break;
            }
        }
end_outer_loop:
    }
quit:

    err = kb_mode_default();
    handle_error(err, "reset keyboard", 1);
    put_c(CH_NEWLINE);
    return 0;
}