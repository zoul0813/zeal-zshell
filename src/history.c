#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "history.h"

HistoryNode* alloc_node(History *list) {
    for(uint8_t i = 0; i < HISTORY_MAX; i++) {
        if(!list->used[i]) {
            list->used[i] = 1;
            HistoryNode *n = &list->nodes[i];
            n->str[0] = '\0';
            n->next = NULL;
            n->prev = NULL;
            return n;
        }
    }
    return NULL;
}

void free_node(History *list, HistoryNode *node) {
    uint8_t i = (uint8_t)(node - list->nodes);
    if(i < HISTORY_MAX) {
        list->used[i] = 0;
    }
}

void history_init(History *list) {
    for(uint8_t i = 0; i < HISTORY_MAX; i++) {
        list->used[i] = 0;
    }
    list->head = NULL;
    list->tail = NULL;
}

void history_remove(History *list, HistoryNode *node) {
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

void history_push(History *list, HistoryNode *node) {
    node->next = NULL;
    node->prev = list->tail;
    if(list->tail) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
}

HistoryNode* history_find(History *list, char* str) {
    HistoryNode *node = list->head;
    while(node) {
        if(strcmp(node->str, str) == 0) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

int8_t history_add(History *list, char* str) {
    if(!str || str[0] == '\0') {
        return 0; /// no command
    }

    HistoryNode *existing = history_find(list, str);
    if(existing) {
        history_remove(list, existing);
        history_push(list, existing);
        return 0;
    }

    HistoryNode *node = alloc_node(list);
    if(!node) {
        if(list->head) {
            node = list->head;
            history_remove(list, node);
            node->str[0] = '\0';
        }
        // else {
        //     return -1; // no nodes available, list is empty ... how'd we get here?
        // }
    }

    strncpy(node->str, str, COMMAND_MAX - 1);
    node->str[COMMAND_MAX-1] = '\0';
    history_push(list, node);
    return 0;
}