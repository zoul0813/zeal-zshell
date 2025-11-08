#ifndef BATCH_H
#define BATCH_H

typedef enum {
    BATCH_NONE = 0,
    BATCH_QUIET = 1,
} batch_options_e;

zos_err_t batch_process(const char* path, batch_options_e quiet);

#endif