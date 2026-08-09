#ifndef BATCH_H
#define BATCH_H

#include <zos_errors.h>

#define TERNARY_TRUE    '?'
#define TERNARY_FALSE   ':'
#define BATCH_COMMENT   ';'

typedef enum {
    BATCH_NONE = 0,
    BATCH_QUIET = 1,
} batch_options_e;

zos_err_t batch_process(const char* path, batch_options_e quiet);

#endif /* BATCH_H */
