// #include "avr/io.h"
// #include "util/delay.h"
// #include "functions.h"

// int main(void) 
// {
//     DDRB |= _BV(PB1);
//     DDRB |= _BV(PB2);
//     TCCR1A |= _BV(COM1A0);
//     TCCR1A |= _BV(COM1B0);
//     TCCR1B |= _BV(WGM12);
//     TCCR1B |= _BV(CS10);
//     TCCR1B |= _BV(CS11);
//     OCR1A = 62499;
//     OCR1B = 31249;
//     while(1);
//     return 0;
// }




// #include "avr/io.h"
// #include "util/delay.h"
// #include "functions.h"
// #include "avr/interrupt.h"

// int main(void) 
// {
//     DDRB |= _BV(PB1);
//     DDRB |= _BV(PB2);
//     TCCR1A |= _BV(COM1A0);
//     TCCR1A |= _BV(COM1B0);
//     TCCR1B |= _BV(WGM12);
//     TCCR1B |= _BV(CS10);
//     TCCR1B |= _BV(CS11);
//     OCR1A = 31249;
//     TIMSK1 |= _BV(OCIE1A);
//     sei();
//     while(1);
//     return 0;
// }

// ISR(TIMER1_COMPA_vect)
// {
//     blink(0,0);
// }


#include "functions.h"

uint16_t timer1Value[2];
uint8_t timer1Flag = 0;
uint8_t timer1CaptureFlag = 0;
uint8_t timer1Counter = 0;

ISR(TIMER1_OVF_vect)
{
    if(timer1Flag)
        timer1Counter++;
}

ISR(TIMER1_CAPT_vect)
{
    static uint8_t cnt = 0;
    if(cnt == 0)
    {
        timer1Value[0] = ICR1;
        cnt++;
        timer1Flag = 1;
    }
    else 
    {
        cnt = 0;
        timer1Value[1] = ICR1;
        TIMSK1 &= ~_BV(ICIE1);
        timer1CaptureFlag = 1;
    }
}

int main(void) 
{
    avr_uart_init();
    long t1;
    cli();
    DDRD |= _BV(PD6);//OC0A
    DDRD |= _BV(PD5);//OC0B
    PORTB |= _BV(PD6);
    PORTB |= _BV(PD5);
    DDRB &= ~_BV(PB0);//ICP1
    PORTB |= _BV(PB0);
    TCCR0A |= _BV(COM0A0);
    TCCR0A |= _BV(COM0B0);
    TCCR0A |= _BV(WGM01);
    TCCR0B |= _BV(CS00);
    TCCR0B |= _BV(CS01);
    OCR0A = 249;//250hz 
    OCR0B = 124;//250hz, phase shift 90 deg
    TCCR1B |= _BV(CS10);
    TCCR1B &= ~_BV(ICES1);
    TIMSK1 |= _BV(TOIE1);
    sei();
    while(1)
    {
        timer1Counter = 0;
        timer1Flag = 0;
        TIMSK1 |= _BV(ICIE1);
        while(timer1CaptureFlag == 0)
            delay_ms(1);
        timer1CaptureFlag = 0;
        if(timer1Counter == 0)
        {
            t1 = timer1Value[1] - timer1Value[0];
        }
        else 
        {
            t1 = 0xffff*(timer1Counter - 1);
            t1 += (0xffff - timer1Value[0]);
            t1 += timer1Value[1];
        }
        t1 = 8000000/t1;
        xprintf("ICP1: %u hz\n",t1);
        delay_ms(3000);
    }
    return 0;
}
