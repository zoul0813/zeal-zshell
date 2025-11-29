#ifndef HISTORY_H
#define HISTORY_H

#include <stdint.h>
#include "config.h"

typedef struct HistoryNode {
    char str[COMMAND_MAX];
    struct HistoryNode *prev;
    struct HistoryNode *next;
} HistoryNode;

typedef struct {
    HistoryNode nodes[HISTORY_MAX];
    uint8_t used[HISTORY_MAX];
    HistoryNode *head;
    HistoryNode *tail;
} History;

void history_init(History *list);
HistoryNode* alloc_node(History *list);
void free_node(History *list, HistoryNode *node);
int8_t history_add(History *list, char* str);

#endif