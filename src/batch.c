// #include <stdio.h>
// #include <string.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>
#include <core.h>

#include "common.h"
#include "batch.h"
#include "history.h"
#include "process.h"


#define BATCH_READ_BUFFER_SIZE 128

static char lines[BATCH_MAX_DEPTH][COMMAND_MAX];
static uint8_t batch_depth = 0;

static zos_err_t line_too_long(zos_dev_t f) {
    put_s("<line length>\n");
    close(f);
    batch_depth--;
    return ERR_PATH_TOO_LONG;
}

zos_err_t batch_process(const char* path, batch_options_e options) {
    (void*)path;
    (void*)options;

    if(batch_depth >= BATCH_MAX_DEPTH) {
        put_s("<batch depth>\n");
        return ERR_NO_MORE_MEMORY;
    }

    zos_dev_t f = open(path, O_RDONLY);
    if(f < 0) {
        put_s("ERROR[");
        put_hex(-f);
        put_s("]: could not open ");
        put_s(path);
        put_c(CH_NEWLINE);
        return -f;
    }

    batch_depth++;
    char *line = lines[batch_depth - 1];
    char buffer[BATCH_READ_BUFFER_SIZE];

    // TODO: add option to "set quiet=1" for options |= BATCH_QUIET

    uint16_t pos = 0;
    uint8_t overflow = 0;
    uint8_t cond_block = 0;  // keep track of whether we're in a conditional block
    zos_err_t err = ERR_SUCCESS;
    uint32_t total_read = 0;

    while(1) {
        uint16_t size = sizeof(buffer);
        zos_err_t read_err = read(f, buffer, &size);
        if(read_err) {
            close(f);
            batch_depth--;
            put_s("ERROR[");
            put_hex(read_err);
            put_s("]: could not read ");
            put_s(path);
            put_c(CH_NEWLINE);
            return read_err;
        }
        if(size < 1) {
            break;
        }
        total_read += size;

        for(uint16_t i = 0; i < size; i++) {
            char c = buffer[i];
            if(c != CH_NEWLINE) {
                if(!overflow && pos < (COMMAND_MAX - 1)) {
                    line[pos++] = c;
                } else {
                    overflow = 1;
                }
            }

            if(c == CH_NEWLINE) {
                if(overflow)
                    return line_too_long(f);
                line[pos] = CH_NULL;
                if(!overflow && str_len(line) > 0) {
                    if(line[0] == BATCH_COMMENT) goto next_line;

                    char *cmd = line;
                    uint8_t do_run = 1;

                    if(cmd[0] == TERNARY_TRUE || cmd[0] == TERNARY_FALSE) {
                        if(cmd[0] == TERNARY_TRUE && err) do_run = 0;
                        if(cmd[0] == TERNARY_FALSE && !err) do_run = 0;
                        cmd++;
                        while(*cmd == CH_SPACE) cmd++; // skip spaces
                        cond_block = 1;
                    } else {
                        // Not a conditional - reset error if we were in a conditional block
                        if(cond_block) {
                            cond_block = 0;
                            err = ERR_SUCCESS;
                        }
                        if(err) {
                            close(f);
                            batch_depth--;
                            return err;
                        }
                    }

                    if(!(options && BATCH_QUIET)) {
                        setcolor(TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_BLACK);
                        put_s("> ");
                        put_s(line);
                        put_c(CH_NEWLINE);
                        setcolor(TEXT_COLOR_WHITE, TEXT_COLOR_BLACK);
                    }

                    if(do_run) {
                        err = run(cmd);
                    }
                }
                next_line:
                pos = 0;
                overflow = 0;
            }
        }
    }

    if(total_read < 1) {
        close(f);
        batch_depth--;
        return ERR_SUCCESS;
    }

    if(overflow)
        return line_too_long(f);

    if(pos > 0) {
        line[pos] = CH_NULL;
        if(!overflow && str_len(line) > 0) {
            if(line[0] != BATCH_COMMENT) {
                char *cmd = line;
                uint8_t do_run = 1;

                if(cmd[0] == TERNARY_TRUE || cmd[0] == TERNARY_FALSE) {
                    if(cmd[0] == TERNARY_TRUE && err) do_run = 0;
                    if(cmd[0] == TERNARY_FALSE && !err) do_run = 0;
                    cmd++;
                    while(*cmd == CH_SPACE) cmd++; // skip spaces
                    cond_block = 1;
                } else {
                    if(cond_block) {
                        cond_block = 0;
                        err = ERR_SUCCESS;
                    }
                    if(err) {
                        close(f);
                        batch_depth--;
                        return err;
                    }
                }

                if(!(options && BATCH_QUIET)) {
                    setcolor(TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_BLACK);
                    put_s("> ");
                    put_s(line);
                    put_c(CH_NEWLINE);
                    setcolor(TEXT_COLOR_WHITE, TEXT_COLOR_BLACK);
                }

                if(do_run) {
                    err = run(cmd);
                }
            }
        }
    }

    close(f);
    batch_depth--;
    return err;
}
