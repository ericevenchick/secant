#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_sysctl.h"

#include "driverlib/sysctl.h"
#include "driverlib/can.h"

#include "utils/ustdlib.h"

#include "usb.h"
#include "can.h"
#include "commands.h"

static int32_t get_bus(char *arg) {
    if (ustrcasecmp("1", arg) == 0) {
        return CAN_BUS_1;
    } else if (ustrcasecmp("2", arg) == 0) {
        return CAN_BUS_2;
    }
    return -1;
}

uint32_t cmd_reset(int argc, char argv[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE]) {
        SysCtlReset();
        return CMD_ERROR_NONE;
}

uint32_t cmd_bus(int argc, char argv[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE]) {
    int32_t bus;
    uint32_t bit_rate;

    if (argc < 2) {
        // need at least bus and action args
        return CMD_ERROR_INVALID_ARG;
    }

    // arg 1: select bus
    bus = get_bus(argv[1]);
    if (bus < 0) {
        return CMD_ERROR_INVALID_ARG;
    }

    // arg 2: action
    if (ustrcasecmp("rate", argv[2]) == 0) {
        // rate: set the bit rate to arg 3
        bit_rate = ustrtoul(argv[3], NULL, 0);
        if (bit_rate == 0) {
            // bit rate is invalid
            return CMD_ERROR_INVALID_ARG;
        }
        can_set_rate(bus, bit_rate);
        return CMD_ERROR_NONE;
    } else if (ustrcasecmp("up", argv[2]) == 0) {
        // up: enable the bus
        can_enable(bus);
        return CMD_ERROR_NONE;
    } else if (ustrcasecmp("down", argv[2]) == 0) {
        // down: disable the bus
        can_disable(bus);
        return CMD_ERROR_NONE;
    }

    // no commands were handled, args must be invalid
    return CMD_ERROR_INVALID_ARG;
}

uint32_t cmd_tx(int argc, char argv[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE]) {
    int i;
    int32_t bus;
    uint8_t tx_data[8];
    tCANMsgObject tx_msg;

    if (argc < 3) {
        // need at least bus, msg id, and dlc
        return CMD_ERROR_INVALID_ARG;
    }

    // arg 1: select bus
    bus = get_bus(argv[1]);
    if (bus < 0) {
        return CMD_ERROR_INVALID_ARG;
    }

    // arg 2: message id
    tx_msg.ui32MsgID = ustrtoul(argv[2], NULL, 0);
    // arg 3: message length
    tx_msg.ui32MsgLen = ustrtoul(argv[3], NULL, 0);
    // arg 4 - 12
    // TODO: length checking based on argc, message length, etc...
    for (i = 0; i < 8; i++) {
        tx_data[i] = ustrtoul(argv[i+4], NULL, 0);
    }

    tx_msg.pui8MsgData = tx_data;
    tx_msg.ui32Flags = 0;
    can_send(bus, &tx_msg);
    return CMD_ERROR_NONE;
}
