#include "functions.h"

int main(void) 
{
    bool direction = 1;
    uint8_t pwm = 0;
    TCCR1A |= _BV(COM1A1);
    TCCR1A |= _BV(WGM10);
    TCCR1B |= _BV(CS11);
    TCCR1B |= _BV(CS10);
    TCNT1 = 0;
    DDRB |= _BV(PB1);
    while(1)
    {
        if(direction)
        {
            if(++pwm == 255)
                direction = 0;
        }
        else 
        {
            if(--pwm == 0)
                direction = 1;
        }
        OCR1A = pwm;
        delay_ms(20);
    }
    return 0;
}

