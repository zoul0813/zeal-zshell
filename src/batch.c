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


static char line[COMMAND_MAX];

zos_err_t batch_process(const char* path, batch_options_e options) {
    (void*)path;
    (void*)options;

    zos_dev_t f = open(path, O_RDONLY);
    if(f < 0) {
        // printf("ERROR[%02x]: could not open %s\n", -f, path);
        put_s("ERROR[");
        put_hex(-f);
        put_s("]: could not open ");
        put_s(path);
        put_c(CH_NEWLINE);
        return -f;
    }

    char *buffer = aligned_buffer;
    uint16_t size = sizeof(aligned_buffer);

    zos_err_t err = read(f, buffer, &size);
    close(f);
    if(err) {
        // printf("ERROR[%02x]: could not read %s\n", err, path);
        put_s("ERROR[");
        put_hex(err);
        put_s("]: could not read ");
        put_s(path);
        put_c(CH_NEWLINE);
        return err;
    }
    if(size < 1) {
        // printf("ERROR: %d bytes read\n", size);
        put_s("ERROR: ");
        put_u16(size);
        put_s(" bytes read\n");
        return ERR_SUCCESS;
    }

    // Copy to local buffer, so batch_process can be called multiple times
    // (ie; autoexec.zs execs h:/test.zs)
    // TODO: limits apply, we'll run out of stack space if we allow too much nesting
    char local[sizeof(aligned_buffer)];
    mem_cpy(local, buffer, sizeof(aligned_buffer));
    buffer = local;

    // TODO: add option to "set quiet=1" for options |= BATCH_QUIET

    uint8_t *p = &buffer[0];
    uint16_t pos = 0;
    uint8_t cond_block = 0;  // keep track of whether we're in a conditional block

    while(size > 0) {
        char c = *p;
        if(c != CH_NEWLINE) {
            line[pos] = c;
            pos++;
        }

        if(c == CH_NEWLINE || size == 1) {
            line[pos] = CH_NULL;
            if(str_len(line) > 0) {
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
                        return err;
                    }
                }

                if(!(options && BATCH_QUIET)) {
                    setcolor(TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_BLACK);
                    // printf("> %s\n", line);
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
        }
        p++;
        size--;
    }

    return err;
}