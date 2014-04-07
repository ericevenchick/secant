#ifndef _CAN_H_
#define _CAN_H_

#include "driverlib/can.h"

enum {
    CAN_BUS_1 = 1,
    CAN_BUS_2
};

void can_init(void (*)(uint32_t, tCANMsgObject*));
void can_enable(uint32_t);
void can_disable(uint32_t);
void can_set_rate(uint32_t, uint32_t);
void can_set_filter(uint32_t, uint32_t, uint32_t);
void can_send(uint32_t, tCANMsgObject*);

#endif
