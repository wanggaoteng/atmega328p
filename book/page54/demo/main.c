#include "functions.h"

int main(void) 
{
    avr_uart_init();

    uint16_t i;
    uint8_t j=0;
    bool b=0;

    DDRB |= _BV(PB5);
    PORTB &= ~_BV(PB5);

    TCNT0 = 0;
    TCCR0B |= _BV(CS00);

    while(1)
    {
        for(i=0;i<31250;i++)
        {
            loop_until_bit_is_set(TIFR0,TOV0);
            TIFR0 |= _BV(TOV0);
        }
        if(b)
        {
            PORTB |= _BV(PB5);
            b=0;
        }
        else
        {
            PORTB &= ~_BV(PB5);
            b=1;
        }
        xprintf("%d\n",j++);
    }

    return 0;
}
