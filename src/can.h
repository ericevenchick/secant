#ifndef _CAN_H_
#define _CAN_H_

#include "driverlib/can.h"

void can_enable(uint32_t, uint32_t);
void can_send(uint32_t, tCANMsgObject*);

#endif
