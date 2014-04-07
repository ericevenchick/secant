#ifndef _COMMANDS_H_
#define _COMMANDS_H_

enum {
    CMD_ERROR_NONE = 0,
    CMD_ERROR_UNKNOWN_CMD,
    CMD_ERROR_INVALID_ARG
};


uint32_t cmd_reset(int, char[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE]);
uint32_t cmd_bus(int argc, char argv[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE]);
uint32_t cmd_tx(int argc, char argv[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE]);

#endif
