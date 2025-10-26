#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <zos_errors.h>

#include "paths.h"
#include "history.h"

extern dir_t cwd;
extern History history;
extern HistoryNode *history_node;

int fflush_stdout(void); // defined in ZOS

int __exit(zos_err_t err);
void print_error(uint8_t code);
void handle_error(zos_err_t err, char *msg, uint8_t fatal);

#endif