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


// put a 1K buffer at the top of page 2
char __at(0xBC00) aligned_buffer[1024];
static char line[COMMAND_MAX];

zos_err_t batch_process(const char* path, batch_options_e options) {
    (void*)path;
    (void*)options;

    zos_dev_t f = open(path, O_RDONLY);
    if(f < 0) {
        put_s("ERROR[");
        put_hex(-f);
        put_s("]: could not open ");
        put_s(path);
        put_c(CH_NEWLINE);
        return -f;
    }

    char *buffer = aligned_buffer;
    char local[sizeof(aligned_buffer)];

    // TODO: add option to "set quiet=1" for options |= BATCH_QUIET

    uint16_t pos = 0;
    uint8_t overflow = 0;
    uint8_t cond_block = 0;  // keep track of whether we're in a conditional block
    zos_err_t err = ERR_SUCCESS;
    uint32_t total_read = 0;

    while(1) {
        uint16_t size = sizeof(aligned_buffer);
        zos_err_t read_err = read(f, buffer, &size);
        if(read_err) {
            close(f);
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

        // Copy to local buffer, so batch_process can be called multiple times
        // (ie; autoexec.zs execs h:/test.zs)
        // TODO: limits apply, we'll run out of stack space if we allow too much nesting
        mem_cpy(local, buffer, size);

        uint8_t *p = (uint8_t *)&local[0];
        for(uint16_t i = 0; i < size; i++) {
            char c = p[i];
            if(c != CH_NEWLINE) {
                if(!overflow && pos < (COMMAND_MAX - 1)) {
                    line[pos++] = c;
                } else {
                    overflow = 1;
                }
            }

            if(c == CH_NEWLINE) {
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
        put_s("ERROR: ");
        put_u16(0);
        put_s(" bytes read\n");
        close(f);
        return ERR_SUCCESS;
    }

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
    return err;
}
