#include "functions.h"

bool flag = 1;

int main(void) 
{
    PCICR |= _BV(PCIE2);
    PCMSK2 |= _BV(PCINT18);
    DDRD &= ~_BV(PD2);
    PORTD |= _BV(PD2);
    DDRB |= _BV(PB5);
    PORTB &= ~_BV(PB5);
    sei();
    while(1)
    {
        if(flag)
            PORTB |= _BV(PB5);
        else
            PORTB &= ~_BV(PB5);
    }
    return 0;
}

ISR(PCINT2_vect)
{
    flag = !flag;
}
