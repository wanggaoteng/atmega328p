#include "functions.h"

int main(void)
{
    uint8_t i = 0;
    avr_uart_init();
    while(1)
    {
        blink(0, 0);
        xprintf("%d\n", i++);
        delay_ms(1000);
    }
	return 0;
}
