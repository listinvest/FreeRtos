#include "header.h"

void help_command() {
    char *msg = "List of supported commands:\
    \n\r\e[32mled on [1-3]\e[0m - enables led x. If no led specified, all leds are enabled.\
    \n\r\e[32mled off [1-3]\e[0m - unables led x. If no led specified, all leds are unabled.\
    \n\r\e[32mled pulse [1-3] [f=x.y(optional; 0 <= x <= 2, 0 <= y <= 9); f=1.0 by default]\e[0m - makes led pulse with given frequency. If no led specified, all leds are pulsing\
    \n\r\e[32mhelp\e[0m - lists all supported commands.\n\r";
    uart_write_bytes(UART_PORT, msg, strlen(msg));
}

