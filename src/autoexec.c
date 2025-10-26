
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>

#include "autoexec.h"
#include "history.h"
#include "process.h"

char buffer[1024 * 4];
char line[COMMAND_MAX];

zos_err_t autoexec_process(void) {
    zos_dev_t f = open(AUTOEXEC_FILENAME, O_RDONLY);
    if(f < 0) {
        printf("ERROR[%02x]: could not open %s\n", -f, AUTOEXEC_FILENAME);
        return -f;
    }

    uint16_t size = sizeof(buffer);
    zos_err_t err = read(f, buffer, &size);
    close(f);
    if(err) {
        printf("ERROR[%02x]: could not read %s\n", err, AUTOEXEC_FILENAME);
        return err;
    }
    if(size < 1) {
        printf("ERROR: %d bytes read\n", size);
        return ERR_SUCCESS;
    }

    uint8_t *p = &buffer[0];
    uint16_t pos = 0;
    while(size > 0) {
        char c = *p;
        // printf("char: %c\n", c);
        if(c == '\n') {
            line[pos] = '\0';
            if(strlen(line) > 0) {
                run(line);
            }
            pos = 0;
        } else {
            line[pos] = c;
            pos++;
        }
        p++;
        size--;
    }
    // line[pos] = '\0';
    // if(strlen(line) > 0) {
    //     run(line);
    // }

    return ERR_SUCCESS;
}