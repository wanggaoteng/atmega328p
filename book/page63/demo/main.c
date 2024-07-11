#include "avr/wdt.h"
#include "functions.h"

int main(void) 
{
    avr_uart_init();
    DDRB |= _BV(PB5);
    PORTB &= ~_BV(PB5);
    wdt_enable(WDTO_1S);
    wdt_reset();
    delay_ms(500);
    PORTB |= _BV(PB5);
    xprintf("watchdog\n");
    delay_ms(5000);
    while(1)
    {
        xprintf("test\n");
        delay_ms(1000);
    }
    return 0;
}

