#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>

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
        printf("ERROR[%02x]: could not open %s\n", -f, path);
        return -f;
    }

    char *buffer = aligned_buffer;
    uint16_t size = sizeof(aligned_buffer);

    zos_err_t err = read(f, buffer, &size);
    close(f);
    if(err) {
        printf("ERROR[%02x]: could not read %s\n", err, path);
        return err;
    }
    if(size < 1) {
        printf("ERROR: %d bytes read\n", size);
        return ERR_SUCCESS;
    }

    // TODO: add option to "set quiet=1" for options |= BATCH_QUIET

    uint8_t *p = &buffer[0];
    uint16_t pos = 0;
    uint8_t cond_block = 0;  // keep track of whether we're in a conditional block

    while(size > 0) {
        char c = *p;
        if(c != '\n') {
            line[pos] = c;
            pos++;
        }

        if(c == '\n' || size == 1) {
            line[pos] = '\0';
            if(strlen(line) > 0) {
                if(line[0] == ';') goto next_line;

                char *cmd = line;
                uint8_t do_run = 1;

                if(cmd[0] == '?' || cmd[0] == ':') {
                    if(cmd[0] == '?' && err) do_run = 0;
                    if(cmd[0] == ':' && !err) do_run = 0;
                    cmd++;
                    while(*cmd == ' ') cmd++; // skip spaces
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
                    printf("> %s\n", line);
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