#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <zos_errors.h>
#include "config.h"
#include "paths.h"

#define CH_NULL    '\0'
#define CH_NEWLINE '\n'
#define CH_SPACE   ' '

// put a 4K buffer at the top of page 3
char __at(0xB000) aligned_buffer[1024 * 4];

extern dir_t cwd;
extern char paths[MAX_PATHS][PATH_MAX];

int fflush_stdout(void); // defined in ZOS

int __exit(zos_err_t err);
void print_error(uint8_t code);
void handle_error(zos_err_t err, char* msg, uint8_t fatal);

#if CONFIG_COLOR_SUPPORT
void setcolor(uint8_t fg, uint8_t bg);
#else
#define setcolor(fg, bg)
#endif

#if HISTORY_ENABLED
#include "history.h"
extern History history;
extern HistoryNode* history_node;
#endif

#endif
