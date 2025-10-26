#include <stdio.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include "common.h"

dir_t cwd;
History history;
HistoryNode *history_node;

const char *ERROR_STRINGS[] = {
    "ERR_SUCCESS",
    "ERR_FAILURE",
    "ERR_NOT_IMPLEMENTED",
    "ERR_NOT_SUPPORTED",
    "ERR_NO_SUCH_ENTRY",
    "ERR_INVALID_SYSCALL",
    "ERR_INVALID_PARAMETER",
    "ERR_INVALID_VIRT_PAGE",
    "ERR_INVALID_PHYS_ADDRESS",
    "ERR_INVALID_OFFSET",
    "ERR_INVALID_NAME",
    "ERR_INVALID_PATH",
    "ERR_INVALID_FILESYSTEM",
    "ERR_INVALID_FILEDEV",
    "ERR_PATH_TOO_LONG",
    "ERR_ALREADY_EXIST",
    "ERR_ALREADY_OPENED",
    "ERR_ALREADY_MOUNTED",
    "ERR_READ_ONLY",
    "ERR_BAD_MODE",
    "ERR_CANNOT_REGISTER_MORE",
    "ERR_NO_MORE_ENTRIES",
    "ERR_NO_MORE_MEMORY",
    "ERR_NOT_A_DIR",
    "ERR_NOT_A_FILE",
    "ERR_ENTRY_CORRUPTED",
    "ERR_DIR_NOT_EMPTY",
    "INVALID_ERROR_CODE",
};
const uint8_t ERROR_STRINGS_LEN = sizeof(ERROR_STRINGS) / sizeof(ERROR_STRINGS[0]);

int __exit(zos_err_t err) {
  exit(err);
  return err;
}

void print_error(uint8_t code) {
    if(code >= ERROR_STRINGS_LEN) code = ERROR_STRINGS_LEN-1;
    printf("ERROR($%02x): %s\n", code, ERROR_STRINGS[code]);
}

void handle_error(zos_err_t err, char *msg, uint8_t fatal) {
  if(err != ERR_SUCCESS) {
    if(msg != NULL) printf("failed to %s, ", msg);
    print_error(err);
    if(fatal) __exit(err);
  }
}
