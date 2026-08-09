#ifndef HISTORY_H
#define HISTORY_H

#include <stdint.h>

void history_init(void);
int8_t history_add(const char *str);
const char *history_previous(void);
const char *history_next(void);
void history_reset_navigation(void);
void history_print(void);

#endif
