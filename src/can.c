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

#include "can.h"

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

void can_enable(uint32_t bus) {
    CANEnable(get_base(bus));
}

void can_disable(uint32_t bus) {
    CANDisable(get_base(bus));
}

void can_set_rate(uint32_t bus, uint32_t rate) {
    CANBitRateSet(get_base(bus), SysCtlClockGet(), rate);
}

void can_send(uint32_t bus, tCANMsgObject *msg_ptr) {
    CANMessageSet(get_base(bus), 1, msg_ptr, MSG_OBJ_TYPE_TX);
}
