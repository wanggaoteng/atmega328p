// OC2A(PB3)  --->   ICP1(PB0)

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

void IOInit(void)
{
    DDRB |= _BV(PB3);
    PORTB |= _BV(PB3);
    DDRB &= ~_BV(PB0);
    PORTB |= _BV(PB0);
    TCCR1B |= _BV(CS10);
    TCCR1B &= ~_BV(ICES1);
    TIMSK1 |= _BV(TOIE1);
    TCCR2B |= _BV(CS22);
    TCCR2A |= _BV(WGM21);
    TCCR2A |= _BV(COM2A0);
}

int main(void) 
{
    avr_uart_init();
    uint8_t i = 0;
    long t1;
    cli();
    IOInit();
    sei();
    while(1)
    {
        for(i=0;i<250;i+=5)
        {
            OCR2A = i;
            t1 = 8000000/(2*64*(1+i));
            xprintf("T/C2: %u hz\t",t1);
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
    }
    return 0;
}

