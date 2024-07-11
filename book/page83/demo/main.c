#include "functions.h"

void my_putc(uint8_t c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

uint8_t my_getc(void)
{
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

int main(void) 
{
    UBRR0 = 51;
    set_bits(UCSR0B, _BV(RXEN0) | _BV(TXEN0));
    while(1)
    {
        my_putc(my_getc());
    }
    return 0;
}