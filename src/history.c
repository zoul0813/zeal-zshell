// #include <stdio.h>
// #include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <core.h>

#include "config.h"
#include "common.h"
#include "history.h"

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

static History history;
static HistoryNode *history_node;
static char history_draft[COMMAND_MAX];
static uint8_t history_navigating;

static HistoryNode* alloc_node(History *list) {
    for(uint8_t i = 0; i < HISTORY_MAX; i++) {
        if(!list->used[i]) {
            list->used[i] = 1;
            HistoryNode *n = &list->nodes[i];
            n->str[0] = CH_NULL;
            n->next = NULL;
            n->prev = NULL;
            return n;
        }
    }
    return NULL;
}

void history_init(void) {
    history_clear();
}

void history_clear(void) {
    for(uint8_t i = 0; i < HISTORY_MAX; i++) {
        history.used[i] = 0;
    }
    history.head = NULL;
    history.tail = NULL;
    history_node = NULL;
    history_draft[0] = CH_NULL;
    history_navigating = 0;
}

static void history_remove(History *list, HistoryNode *node) {
    if(node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }

    if(node->next) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }

    node->next = NULL;
    node->prev = NULL;
}

static void history_push(History *list, HistoryNode *node) {
    node->next = NULL;
    node->prev = list->tail;
    if(list->tail) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
}

static HistoryNode* history_find(History *list, const char* str) {
    HistoryNode *node = list->head;
    while(node) {
        if(str_cmp(node->str, str) == 0) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

int8_t history_add(const char* str) {
    History *list = &history;

    if(!str || str[0] == CH_NULL) {
        return 0; /// no command
    }

    HistoryNode *existing = history_find(list, str);
    if(existing) {
        history_remove(list, existing);
        history_push(list, existing);
        history_reset_navigation();
        return 0;
    }

    HistoryNode *node = alloc_node(list);
    if(!node) {
        if(list->head) {
            node = list->head;
            history_remove(list, node);
            node->str[0] = CH_NULL;
        }
        // else {
        //     return -1; // no nodes available, list is empty ... how'd we get here?
        // }
    }

    str_cpyn(node->str, str, COMMAND_MAX - 1);
    node->str[COMMAND_MAX-1] = CH_NULL;
    history_push(list, node);
    history_reset_navigation();
    return 0;
}

static void history_start_navigation(const char *current) {
    str_cpyn(history_draft, current, COMMAND_MAX - 1);
    history_draft[COMMAND_MAX - 1] = CH_NULL;
    history_navigating = 1;
}

const char *history_previous(const char *current) {
    if(!history.tail) return NULL;

    if(!history_navigating) {
        history_start_navigation(current);
        history_node = history.tail;
    } else if(!history_node) {
        history_node = history.tail;
    } else if(history_node->prev) {
        history_node = history_node->prev;
    }
    return history_node->str;
}

const char *history_next(void) {
    if(!history_navigating) return NULL;

    if(history_node && history_node->next) {
        history_node = history_node->next;
        return history_node->str;
    }

    history_node = NULL;
    return history_draft;
}

void history_reset_navigation(void) {
    history_node = NULL;
    history_navigating = 0;
}

void history_print(void) {
    HistoryNode *node = history.head;
    uint8_t number = 1;

    while(node) {
        put_u8(number++);
        put_s("  ");
        put_s(node->str);
        put_c(CH_NEWLINE);
        node = node->next;
    }
}
