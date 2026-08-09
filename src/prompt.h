#ifndef PROMPT_H
#define PROMPT_H

#include <stdint.h>
#include <zos_errors.h>

zos_err_t prompt_init(void);
void prompt_show(void);
void prompt_clear(void);
void prompt_set_command(const char *command);
void prompt_reset(void);
void prompt_normalize(void);
char *prompt_command(void);
uint16_t prompt_length(void);
void prompt_move_left(void);
void prompt_move_right(void);
void prompt_move_home(void);
void prompt_move_end(void);
void prompt_backspace(void);
void prompt_delete(void);
void prompt_insert(unsigned char c);

#endif
