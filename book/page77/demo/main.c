#include "functions.h"

uint8_t displayBuf[16] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};
volatile uint8_t counter = 0, flag = 0;
volatile bool mode = 0;


void serOut(uint8_t dat)
{
    uint8_t i;
    for(i=0;i<8;i++)
    {
        if(dat & 0x80)
            PORTD |= _BV(PD4);
        else
            PORTD &= ~_BV(PD4);
        PORTD |= _BV(PD6);
        PORTD &= ~_BV(PD6);
        dat <<= 1;
    }
}

void displayNumber(uint8_t num,uint8_t hex)
{
    uint8_t buf[2];
    uint8_t temp;
    if(hex)
    {
        buf[0] = displayBuf[num>>4];
        buf[1] = displayBuf[num&0x0f];
    }
    else
    {
        buf[1] = displayBuf[num%10];
        temp = num % 100;
        buf[0] = displayBuf[temp/10];
        temp = num / 100;
        if(temp>0)
            buf[1] |= 0x80;
        if(temp>1)
            buf[0] |= 0x80;
    }
    serOut(buf[0]);
    serOut(buf[1]);
    PORTD |= _BV(PD5);
    PORTD &= ~_BV(PD5);
}



int main(void) 
{
    DDRD |= _BV(PD4) | _BV(PD5) | _BV(PD6);
    PORTD |= (_BV(PD2) | _BV(PD3));
    EICRA |= _BV(ISC01);
    EICRA |= _BV(ISC11);
    EIMSK |= _BV(INT0);
    EIMSK |= _BV(INT1);
    sei();
    displayNumber(counter, mode);
    while(1)
    {
        if(flag == 1)
        {
            displayNumber(++counter, mode);
            flag = 0;
        }
        
    }
    return 0;
}

ISR(INT0_vect)
{
    flag = 1;
}

ISR(INT1_vect)
{
    mode = !mode;
    displayNumber(counter, mode);
}