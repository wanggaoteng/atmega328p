#include "functions.h"

int main(void) 
{
    avr_uart_init();

	blink(0,1000);

    return 0;
}


