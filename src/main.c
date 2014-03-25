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
#include "commands.h"

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
    CANInit(CAN0_BASE);

    // TODO: configure CAN1 pins and peripherial

    // enable systick
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / SYSTICKS_PER_SECOND);
    ROM_SysTickIntEnable();
    ROM_SysTickEnable();
}

// callback for when a command is received over USB
// decide which command should be executed, then run it
void cmd_handler(int argc, char argv[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE]) {
    uint32_t status = CMD_ERROR_UNKNOWN_CMD;
    char resp[MAX_RESP_SIZE];

    // command: bus
    if (ustrcasecmp("bus", argv[0]) == 0) {
        status = cmd_bus(argc, argv);
    }
    // command: tx
    if (ustrcasecmp("tx", argv[0]) == 0) {
        status = cmd_tx(argc, argv);
    }
    // command: reset
    if (ustrcasecmp("reset", argv[0]) == 0) {
        status = cmd_reset(argc, argv);
        // never returns due to sw reset
    }

    // handle errors
    if (status == CMD_ERROR_UNKNOWN_CMD) {
        // no commands matched
        usnprintf(resp, MAX_RESP_SIZE, "error: unknown command\r\n");
    } else if (status == CMD_ERROR_INVALID_ARG) {
        usnprintf(resp, MAX_RESP_SIZE, "error: invalid args\r\n");
    } else {
        // no error
        usnprintf(resp, MAX_RESP_SIZE, "ok: %s\r\n", argv[0]);
    }
    // send response
    usb_send_str(resp);
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
