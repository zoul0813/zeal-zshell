#include <stdint.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zvb_hardware.h>
#include <core.h>

#include "config.h"
#include "common.h"
#include "paths.h"
#include "prompt.h"

static char command_buffer[COMMAND_MAX];
static uint16_t command_length = 0;
static uint16_t cursor_pos = 0;
static zos_text_area_t text_area;

static void cursor_left(uint16_t count) {
    uint8_t x = zvb_peri_text_curs_x;
    uint8_t y = zvb_peri_text_curs_y;

    while(count--) {
        if(x == 0) {
            x = text_area.width - 1;
            if(y > 0) y--;
        } else {
            x--;
        }
    }

    zvb_peri_text_curs_y = y;
    zvb_peri_text_curs_x = x;
}

static void cursor_right(uint16_t count) {
    uint8_t x = zvb_peri_text_curs_x;
    uint8_t y = zvb_peri_text_curs_y;

    while(count--) {
        x++;
        if(x >= text_area.width) {
            x = 0;
            if(y < text_area.height - 1) y++;
        }
    }

    zvb_peri_text_curs_y = y;
    zvb_peri_text_curs_x = x;
}

zos_err_t prompt_init(void) {
    command_buffer[0] = CH_NULL;
    command_length = 0;
    cursor_pos = 0;
    return ioctl(DEV_STDOUT, CMD_GET_AREA, &text_area);
}

void prompt_show(void) {
    setcolor(TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_BLACK);
    put_c(CH_RETURN);
    put_s(cwd.drive);
    if(cwd.truncated) put_s("/...");
    put_s(cwd.folder);
    put_c('>');
    fflush_stdout();
    setcolor(TEXT_COLOR_WHITE, TEXT_COLOR_BLACK);
}

void prompt_clear(void) {
    uint16_t i;

    cursor_right(command_length - cursor_pos);
    cursor_left(command_length);
    for(i = 0; i < command_length; i++) {
        put_c(CH_SPACE);
    }
    cursor_left(command_length);
    prompt_reset();
    fflush_stdout();
}

void prompt_set_command(const char *command) {
    prompt_clear();
    str_cpyn(command_buffer, command, COMMAND_MAX - 1);
    command_buffer[COMMAND_MAX - 1] = CH_NULL;
    command_length = str_len(command_buffer);
    cursor_pos = command_length;
    put_s(command_buffer);
    fflush_stdout();
}

void prompt_reset(void) {
    command_buffer[0] = CH_NULL;
    command_length = 0;
    cursor_pos = 0;
}

void prompt_normalize(void) {
    normalize_spaces(command_buffer);
    command_length = str_len(command_buffer);
    cursor_pos = command_length;
}

char *prompt_command(void) {
    return command_buffer;
}

uint16_t prompt_length(void) {
    return command_length;
}

void prompt_move_left(void) {
    if(cursor_pos == 0) return;
    cursor_left(1);
    cursor_pos--;
}

void prompt_move_right(void) {
    if(cursor_pos >= command_length) return;
    cursor_right(1);
    cursor_pos++;
}

void prompt_move_home(void) {
    cursor_left(cursor_pos);
    cursor_pos = 0;
}

void prompt_move_end(void) {
    cursor_right(command_length - cursor_pos);
    cursor_pos = command_length;
}

void prompt_backspace(void) {
    uint16_t i;
    uint16_t repaint_length;

    if(cursor_pos == 0) return;
    cursor_left(1);
    cursor_pos--;
    for(i = cursor_pos; i < command_length; i++) {
        command_buffer[i] = command_buffer[i + 1];
    }
    command_length--;
    repaint_length = command_length - cursor_pos;
    put_s(command_buffer + cursor_pos);
    put_c(CH_SPACE);
    cursor_left(repaint_length + 1);
    fflush_stdout();
}

void prompt_delete(void) {
    uint16_t i;
    uint16_t repaint_length;

    if(cursor_pos >= command_length) return;
    for(i = cursor_pos; i < command_length; i++) {
        command_buffer[i] = command_buffer[i + 1];
    }
    command_length--;
    repaint_length = command_length - cursor_pos;
    put_s(command_buffer + cursor_pos);
    put_c(CH_SPACE);
    cursor_left(repaint_length + 1);
    fflush_stdout();
}

void prompt_insert(unsigned char c) {
    uint16_t i;
    uint16_t repaint_length;

    if(c < 0x20 || c > 0x7D) return;
    if(command_length >= COMMAND_MAX - 1) return;
    for(i = command_length; i > cursor_pos; i--) {
        command_buffer[i] = command_buffer[i - 1];
    }
    command_buffer[cursor_pos] = c;
    command_length++;
    command_buffer[command_length] = CH_NULL;
    repaint_length = command_length - cursor_pos - 1;
    put_s(command_buffer + cursor_pos);
    cursor_left(repaint_length);
    cursor_pos++;
    fflush_stdout();
}
