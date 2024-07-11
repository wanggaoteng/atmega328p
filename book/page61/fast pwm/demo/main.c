#include "functions.h"

int main(void) 
{
    TCCR1A |= _BV(COM1A0);
    TCCR1A |= _BV(COM1B1);
    TCCR1A |= _BV(WGM11);
    TCCR1A |= _BV(WGM10);
    TCCR1B |= _BV(WGM13);
    TCCR1B |= _BV(WGM12);
    TCCR1B |= _BV(CS11);
    TCCR1B |= _BV(CS10);
    TCNT1 = 0;
    DDRB |= _BV(PB1);
    DDRB |= _BV(PB2);
    OCR1A = 1249;
    OCR1B = 249;
    while(1);
    return 0;
}

