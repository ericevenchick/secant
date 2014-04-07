#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_ints.h"

#include "driverlib/interrupt.h"
#include "driverlib/rom.h"

#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"

#include "utils/ustdlib.h"

#include "usb_serial_structs.h"
#include "usb.h"

#define RX_BUFFER_SIZE 100

// globals
//
// indicates if the device is connected
static volatile bool g_bUSBConfigured = false;
// status string for USB device
char *g_pcStatus;
// holds the current command
char cmd_buffer[CMD_BUFFER_SIZE];
// index for the command buffer
unsigned int cmd_buffer_index;
// function pointer to the callback for handling commands
void (*cmd_callback)(int, char argv[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE]);

// initialize the USB buffers, device, and go on bus
void usb_init(void (*cmd_callback_ptr)(int,
              char argv[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE])) {
    cmd_callback = cmd_callback_ptr;

    // initialize USB transmit and receive buffers
    USBBufferInit(&g_sTxBuffer);
    USBBufferInit(&g_sRxBuffer);

    // set USB mode to force device. will always be devices regardless of
    // hardware
    USBStackModeSet(0, eUSBModeForceDevice, 0);

    // initialize the CDC device
    USBDCDCInit(0, &g_sCDCDevice);

    // wait for USB to initialize
    while (!g_bUSBConfigured);
}

// send a string to the USB host
void usb_send_str(char* str) {
    size_t size;

    size = ustrlen(str);
    USBBufferWrite((tUSBBuffer *)&g_sTxBuffer,
                   (uint8_t *)str, size);
}

// parse the command into its arguments
static void parse_cmd(char *cmd) {
    // buffer to store the arguments
    char args[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE];
    int arg_count = 0;
    int arg_index = 0;

    char cur;
    bool overflow = false;

    // split the command into arguments
    while ((cur = *(cmd++)) != '\0') {
        if (cur == ' ') {
            args[arg_count][arg_index] = '\0';
            arg_index = 0;
            arg_count++;
        } else {
            args[arg_count][arg_index] = cur;
            arg_index++;
        }

        // check for buffer overflows
        if (arg_count >= CMD_MAX_ARGS ||
            arg_index >= CMD_MAX_ARG_SIZE) {
            overflow = true;
            break;
        }
    }

    if (overflow) {
        usb_send_str("invalid command\r\n");
    } else {
        // terminate the last argument string
        args[arg_count][arg_index] = '\0';
        // call the command handler
        cmd_callback(arg_count, args);
    }
}

//*****************************************************************************
//
// Handles CDC driver notifications related to control and setup of the device.
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to perform control-related
// operations on behalf of the USB host.  These functions include setting
// and querying the serial communication parameters, setting handshake line
// states and sending break conditions.
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t
ControlHandler(void *pvCBData, uint32_t ui32Event,
               uint32_t ui32MsgValue, void *pvMsgData)
{
    uint32_t ui32IntsOff;

    //
    // Which event are we being asked to process?
    //
    switch(ui32Event)
    {
        //
        // We are connected to a host and communication is now possible.
        //
        case USB_EVENT_CONNECTED:
            //
            // Flush our buffers.
            //
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);

            //
            // Tell the main loop to update the display.
            //
            ui32IntsOff = ROM_IntMasterDisable();
            g_pcStatus = "Connected";
            if(!ui32IntsOff)
            {
                ROM_IntMasterEnable();
            }

            g_bUSBConfigured = true;
            break;

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
            g_bUSBConfigured = false;
            ui32IntsOff = ROM_IntMasterDisable();
            g_pcStatus = "Disconnected";
            if(!ui32IntsOff)
            {
                ROM_IntMasterEnable();
            }
            break;

        //
        // Return the current serial communication parameters.
        //
        case USBD_CDC_EVENT_GET_LINE_CODING:
            break;

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_LINE_CODING:
            break;

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
            break;

        //
        // Send a break condition on the serial line.
        //
        case USBD_CDC_EVENT_SEND_BREAK:
            break;

        //
        // Clear the break condition on the serial line.
        //
        case USBD_CDC_EVENT_CLEAR_BREAK:
            break;

        //
        // Ignore SUSPEND and RESUME for now.
        //
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
            break;

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif

    }

    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the transmit channel (data to
// the USB host).
//
// \param ui32CBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t
TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    //
    // Which event have we been sent?
    //
    switch(ui32Event)
    {
        case USB_EVENT_TX_COMPLETE:
            //
            // Since we are using the USBBuffer, we don't need to do anything
            // here.
            //
            break;

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif

    }
    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the receive channel (data from
// the USB host).
//
// \param ui32CBData is the client-supplied callback data value for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t
RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    uint32_t ui32Count;
    uint8_t rx[RX_BUFFER_SIZE];
    uint32_t rx_size;
    uint32_t i;
    uint32_t j;

    //
    // Which event are we being sent?
    //
    switch(ui32Event)
    {
        //
        // A new packet has been received.
        //
        case USB_EVENT_RX_AVAILABLE:
        {
            //
            // Feed some characters into the UART TX FIFO and enable the
            // interrupt so we are told when there is more space.
            //
            rx_size = USBBufferRead((tUSBBuffer *)&g_sRxBuffer,
                            rx, RX_BUFFER_SIZE);

            for (i = 0; i < rx_size; i++) {
                if (rx[i] == '\r') {
                    // newline sent, parse the command
                    parse_cmd(cmd_buffer);

                    // clear the buffer
                    for (j = 0; j < sizeof(cmd_buffer); j++) {
                        cmd_buffer[j] = '\0';
                    }
                    cmd_buffer_index = 0;
                } else {
                    // add the received character to the buffer
                    cmd_buffer[cmd_buffer_index] = rx[i];
                    cmd_buffer_index++;
                }
            }

            break;
        }

        //
        // We are being asked how much unprocessed data we have still to
        // process. We return 0 if the UART is currently idle or 1 if it is
        // in the process of transmitting something. The actual number of
        // bytes in the UART FIFO is not important here, merely whether or
        // not everything previously sent to us has been transmitted.
        //
        case USB_EVENT_DATA_REMAINING:
        {
            //
            // Get the number of bytes in the buffer and add 1 if some data
            // still has to clear the transmitter.
            //
            //ui32Count = ROM_UARTBusy(USB_UART_BASE) ? 1 : 0;
            return(ui32Count);
        }

        //
        // We are being asked to provide a buffer into which the next packet
        // can be read. We do not support this mode of receiving data so let
        // the driver know by returning 0. The CDC driver should not be sending
        // this message but this is included just for illustration and
        // completeness.
        //
        case USB_EVENT_REQUEST_BUFFER:
        {
            return(0);
        }

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif
    }

    return(0);
}
