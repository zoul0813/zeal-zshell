#ifndef AUTOEXEC_H
#define AUTOEXEC_H

typedef enum {
    AUTOEXEC_NONE = 0,
    AUTOEXEC_QUIET = 1,
} autoexec_options_e;

zos_err_t autoexec_process(const char* path, autoexec_options_e quiet);

#endif