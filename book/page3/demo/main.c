#include "functions.h"

int main(void)
{
    uint8_t led = 0;
    set_bits(DDRB, _BV(PB5));
    while(1)
    {
        if(led)
            set_bits(PORTB, _BV(PB5));
        else
            clear_bits(PORTB, _BV(PB5));
        led = !led;
        delay_ms(1000);
    }
	return 0;
}

