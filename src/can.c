#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_can.h"
#include "inc/hw_sysctl.h"

#include "driverlib/interrupt.h"
#include "driverlib/can.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"

#include "ustdlib.h"

#include "can.h"
#include "usb.h"

#define CAN_RX_OBJ 1

void (*can_callback)(uint32_t, tCANMsgObject*);

// convert bus number to peripherial base
static uint32_t get_base(uint32_t bus) {
    /* TODO: enable 2nd can bus
    if (bus == CAN_BUS_2) {
        return CAN1_BASE;
    } else {
        return CAN0_BASE;
    }*/
    return CAN0_BASE;
}

void can0_rx_isr() {
    tCANMsgObject received_msg;
    uint8_t data[8];
    uint32_t status;

    IntMasterDisable();

    status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);
    if (status == CAN_INT_INTID_STATUS) {
        // status interrupt, do nothing for now
        IntMasterEnable();
        return;
    }

    received_msg.pui8MsgData = (uint8_t *) &data;
    // get the message and clear the flag
    CANMessageGet(CAN0_BASE, status, &received_msg, true);
    can_callback(CAN_BUS_1, &received_msg);

    IntMasterEnable();
}

void can_init(void (*can_callback_ptr)(uint32_t, tCANMsgObject*)) {
    tCANMsgObject can0_rx_msg;
    int can_obj_count ;

    // wait here if the peripherial isn't enabled
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_CAN0));

    // initialize CAN0
    CANInit(CAN0_BASE);

    // accept all messages by default
    can0_rx_msg.ui32MsgID = 0;
    can0_rx_msg.ui32MsgIDMask = 0;

    /* TODO: determine if FIFO actually helps!
    // set up FIFO
    can0_rx_msg.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER |
                            MSG_OBJ_FIFO;
    for (can_obj_count = 1; can_obj_count < CAN_RX_OBJECTS; can_obj_count++) {
        CANMessageSet(CAN0_BASE, can_obj_count, &can0_rx_msg, MSG_OBJ_TYPE_RX);
    }
    */

    // last in FIFO, clear FIFO flag
    can0_rx_msg.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
    CANMessageSet(CAN0_BASE, CAN_RX_OBJ, &can0_rx_msg, MSG_OBJ_TYPE_RX);

    CANIntRegister(CAN0_BASE, can0_rx_isr);

    // setup the callback
    can_callback = can_callback_ptr;
}


void can_enable(uint32_t bus) {
    CANEnable(get_base(bus));
    CANIntEnable(get_base(bus), CAN_INT_MASTER);
}

void can_disable(uint32_t bus) {
    CANDisable(get_base(bus));
}

void can_set_rate(uint32_t bus, uint32_t rate) {
    CANBitRateSet(get_base(bus), SysCtlClockGet(), rate);
}

void can_set_filter(uint32_t bus, uint32_t id, uint32_t mask) {
    tCANMsgObject rx_msg;

    rx_msg.ui32MsgID = id;
    rx_msg.ui32MsgIDMask = mask;
    rx_msg.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;

    CANMessageSet(get_base(bus), CAN_RX_OBJ, &rx_msg, MSG_OBJ_TYPE_RX);
}

void can_send(uint32_t bus, tCANMsgObject *msg_ptr) {
    CANMessageSet(get_base(bus), 32, msg_ptr, MSG_OBJ_TYPE_TX);
}
