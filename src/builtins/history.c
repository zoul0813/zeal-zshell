#include <stdint.h>
#include <zos_errors.h>
#include <zos_video.h>
#include <core.h>

#include "history.h"

uint8_t cmd_history(char* args)
{
    (void)args;
    HistoryNode* node = history.tail;
    while (node) {
        put_s("  ");
        put_s(node->str);
        put_c('\n');
        node = node->prev;
    }
    return ERR_SUCCESS;
}
