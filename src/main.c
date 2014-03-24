#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_uart.h"
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
#include "driverlib/rom.h"

#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"

#include "utils/ustdlib.h"
#include "utils/uartstdio.h"

#include "usb_serial_structs.h"
#include "usb.h"

// define the systick period
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
    //
    // Update our system time.
    //
    /*
    char resp[30];
    int len;

    len = usprintf(resp, "%d\r\n", g_ui32SysTickCount);
    usb_send_str(resp);
    */

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

    // enable systick
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / SYSTICKS_PER_SECOND);
    ROM_SysTickIntEnable();
    ROM_SysTickEnable();

}

// callback for when a command is received over USB
void cmd_handler(int argc, char argv[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE]) {
    char resp[30];

    usprintf(resp, "%d, %s\r\n", argc, argv[0]);
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
