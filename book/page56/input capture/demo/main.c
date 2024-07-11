#include "functions.h"

uint16_t cnt = 0;

int main(void) 
{
    avr_uart_init();
    DDRB &= ~_BV(PB0);
    PORTB |= _BV(PB0);
    TCCR1B |= _BV(CS12);
    TCCR1B &= ~_BV(ICES1);
    TIMSK1 |= _BV(ICIE1);
    TIMSK1 |= _BV(TOIE1);
    sei();
    while(1);
    return 0;
}

ISR(TIMER1_OVF_vect)
{
    cnt++;
}

ISR(TIMER1_CAPT_vect)
{
    blink(0,0);
    xprintf("%d\t%u\n",cnt,ICR1);
}
