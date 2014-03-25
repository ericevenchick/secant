#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

#include "driverlib/interrupt.h"
#include "driverlib/can.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"

void can_enable(uint32_t base, uint32_t baud) {
    CANInit(base);
    CANBitRateSet(base, SysCtlClockGet(), baud);
    CANEnable(base);
}

void can_send(uint32_t base, tCANMsgObject *msg_ptr) {
    tCANMsgObject tx_msg;
    uint8_t tx_data[8];

    tx_msg.ui32MsgID = 0x100;
    tx_msg.ui32MsgLen = 3;
    tx_msg.ui32Flags = 0;
    tx_data[0] = 1;
    tx_data[1] = 2;
    tx_data[2] = 3;
    tx_msg.pui8MsgData = tx_data;

    CANMessageSet(base, 1, &tx_msg, MSG_OBJ_TYPE_TX);
}
