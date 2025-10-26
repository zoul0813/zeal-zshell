#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <zos_errors.h>

zos_err_t run(const char* path);

#endif