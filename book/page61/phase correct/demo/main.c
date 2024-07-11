#include "functions.h"

int main(void) 
{
    TCCR1A |= _BV(COM1A1);
    TCCR1A |= _BV(WGM10);
    TCCR1B |= _BV(CS11);
    TCCR1B |= _BV(CS10);
    TCNT1 = 0;
    DDRB |= _BV(PB1);
    OCR1A = 51;
    while(1);
    return 0;
}

