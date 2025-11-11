#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <zos_errors.h>

zos_err_t run(const char* arg);
zos_err_t find_exec(unsigned char *name, uint8_t shallow);

#endif