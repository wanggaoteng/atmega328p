#include "functions.h"

ISR(ANALOG_COMP_vect)
{
    if(ACSR & _BV(ACO))
    {
        PORTB |= _BV(PB3);
        PORTB &= ~(_BV(PB4));
    }
    else
    {
        PORTB &= ~(_BV(PB3));
        PORTB |= _BV(PB4);
    }
}

int main(void) 
{
    DDRB |= (_BV(PB3) | _BV(PB4));
    PORTB &= ~(_BV(PB3) | _BV(PB4));
    DDRD &= ~(_BV(PD6) | _BV(PD7));
    PORTD &= ~(_BV(PD6) | _BV(PD7));
    ACSR |= _BV(ACIE);
    sei();
    if(ACSR & _BV(ACO))
    {
        PORTB |= _BV(PB3);
        PORTB &= ~(_BV(PB4));
    }
    else
    {
        PORTB &= ~(_BV(PB3));
        PORTB |= _BV(PB4);
    }
    while(1);
    return 0;
}

