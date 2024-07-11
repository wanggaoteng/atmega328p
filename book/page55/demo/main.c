#include "functions.h"

uint16_t cnt = 0;
uint8_t i = 0;
bool b = 0;
volatile bool flag = 0;

ISR(TIMER0_OVF_vect)
{
    cnt++;
    if(cnt >= 31250)
    {
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
        cnt = 0;
        flag = 1;
    }
}

int main(void) 
{
    avr_uart_init();

    DDRB |= _BV(PB5);
    PORTB &= ~_BV(PB5);

    TCNT0 = 0;
    TCCR0B |= _BV(CS00);
    TIMSK0 |= _BV(TOIE0);
    sei();

    while(1)
    {
        if(flag)
        {
            xprintf("%d\n",i++);
            flag = 0;
        }
    }

    return 0;
}
