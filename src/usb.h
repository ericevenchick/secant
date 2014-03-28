#ifndef _USB_H_
#define _USB_H_

// input buffer sizes
#define CMD_BUFFER_SIZE 100
#define CMD_MAX_ARGS 12
#define CMD_MAX_ARG_SIZE 20

// maximum size of a response string
#define MAX_RESP_SIZE 50


void usb_init(void (*)(int, char argv[CMD_MAX_ARGS][CMD_MAX_ARG_SIZE]));
void usb_send_str(char* str);

#endif
