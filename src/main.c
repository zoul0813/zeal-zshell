// #include <stdio.h>
// #include <string.h>
#include <stddef.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <core.h>
#include <keyboard.h>

#include "config.h"
#include "common.h"
#include "history.h"
#include "paths.h"
#include "batch.h"
#include "process.h"
#include "prompt.h"

static zos_err_t err;
#if AUTOEXEC_ENABLED
static zos_stat_t zos_stat;
#endif

void use_history(HistoryNode *node) {
    if(!node) return;
    prompt_set_command(node->str);
}

void usage(void) {
    put_s("usage: zshell [-options] path\n");
    put_s("   q - quiet mode\n");
}

zos_err_t parse_args(char **argv, char *path, batch_options_e *options) {
    char* params = argv[0];
    uint16_t path_len;

    path[0] = CH_NULL;
    *options = BATCH_NONE;

    if(*params == '-') {
        params++;
        while(*params != CH_NULL && *params != CH_SPACE) {
            switch(*params) {
                case 'q': {
                    *options |= BATCH_QUIET;
                } break;
                case 'h': {
                    usage();
                    exit(ERR_SUCCESS);
                    return ERR_SUCCESS;
                } break;
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

    while(*params == CH_SPACE) params++;
    if(*params == CH_NULL) {
        usage();
        return ERR_INVALID_PARAMETER;
    }

    path_len = str_len(params);
    if(path_len >= PATH_MAX) {
        put_s("Path too long\n");
        return ERR_PATH_TOO_LONG;
    }

    str_cpyn(path, params, path_len);
    path[path_len] = CH_NULL;
    return ERR_SUCCESS;
}

int main(int argc, char **argv) {
    err = path_set_cwd(&cwd);
    handle_error(err, "path_set_cwd", 1);

    for(uint8_t i = 0; i < MAX_PATHS; i++) {
        paths[i][0] = CH_NULL;
    }
    str_cpy(paths[0], "A:/");
    path_count = 1;

    if(argc == 1) {
        char path[PATH_MAX];
        batch_options_e options;
        err = parse_args(argv, path, &options);
        if(err)
            return err;
        err = batch_process(path, options);
        return err;
    }

    run("ver"); put_c(CH_NEWLINE);

#if AUTOEXEC_ENABLED
    err = stat(AUTOEXEC_FILENAME, &zos_stat);
    if(!err && D_ISFILE(zos_stat.s_flags)) {
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
    err = prompt_init();
    handle_error(err, "get text area", 1);

    for(;;) {
        prompt_show();
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
                    prompt_clear();
                } break;
#endif

                case KB_KEY_ENTER: {
                    put_c(CH_NEWLINE);
                    if(prompt_length() < 1) goto end_outer_loop;
#if HISTORY_ENABLED
                    history_add(&history, prompt_command());
                    history_node = history.tail;
#endif

                    err = run(prompt_command());
                    if(err) print_error(err);

                    prompt_reset();
                } goto end_outer_loop;
                case KB_LEFT_ARROW: {
                    prompt_move_left();
                } break;
                case KB_RIGHT_ARROW: {
                    prompt_move_right();
                } break;
                case KB_KEY_BACKSPACE: {
                    prompt_backspace();
                } break;
                case KB_DELETE: {
                    prompt_delete();
                } break;
                default: {
                    unsigned char c = getch(key);
                    prompt_insert(c);
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
