#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/usb.h"
#include "driverlib/can.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"

#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"

#include "utils/ustdlib.h"
#include "utils/uartstdio.h"

#include "usb_serial_structs.h"
#include "usb.h"
#include "can.h"

// define the systick period at 1 ms
#define SYSTICKS_PER_SECOND 1000
#define SYSTICK_PERIOD_MS (1000 / SYSTICKS_PER_SECOND)

// globals
//
// global systick counter
volatile uint32_t g_ui32SysTickCount = 0;

// function prototypes
//
// initialize system and peripherials
void hw_init(void);

// called by libaries if error occurs
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while(1)
    {
    }
}
#endif

// systick interrupt handler
void SysTickIntHandler(void)
{
    // update system time
    g_ui32SysTickCount++;
}

void hw_init() {
    // enable lazy stacking for the FPU
    ROM_FPULazyStackingEnable();

    // set system clock to use PLL @ 50 MHz
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    // configure USB pins
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_5 | GPIO_PIN_4);

    // configure LED GPIO pins
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 |
                                GPIO_PIN_1);

    // configure CAN0 pins and peripherial
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    ROM_GPIOPinConfigure(GPIO_PE5_CAN0TX);
    ROM_GPIOPinConfigure(GPIO_PE4_CAN0RX);
    ROM_GPIOPinTypeCAN(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_CAN0));

    // TODO: configure CAN1 pins and peripherial

    // enable systick
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / SYSTICKS_PER_SECOND);
    ROM_SysTickIntEnable();
    ROM_SysTickEnable();
}

// callback for when a command is received over USB
void cmd_handler(int argc, char argv[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE]) {
    tCANMsgObject tx_msg;
    uint8_t tx_data[8];

    // fake commands
    if (ustrcasecmp("go", argv[0]) == 0) {
        can_enable(CAN0_BASE, 500000);
        usb_send_str("going!\r\n");
        return;
    }
    if (ustrcasecmp("send", argv[0]) == 0) {
        tx_msg.ui32MsgID = 0x100;
        tx_msg.ui32MsgLen = 8;
        tx_msg.ui32Flags = 0;
        tx_data[0] = 1;
        tx_data[1] = 2;
        tx_data[2] = 3;
        tx_msg.pui8MsgData = tx_data;

        can_send(CAN0_BASE, &tx_msg);
        return;
    }

    // command: reset
    // immediately reset the device
    if (ustrcasecmp("reset", argv[0]) == 0) {
        SysCtlReset();
        return;
    }
    // no commands matched, send an error
    usb_send_str("unknown command\r\n");
}

int main(void)
{
    hw_init();
    usb_init(cmd_handler);

    // main loop
    while(1)
    {
    }
}
