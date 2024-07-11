#include "functions.h"

uint16_t adcValue[8];

void adcInit(void)
{
    ADMUX |= _BV(REFS0);
    ADMUX &= ~(_BV(MUX3) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0));
    ADCSRA |= (_BV(ADEN) | _BV(ADPS1) | _BV(ADPS0));
}

uint16_t adcRead(uint8_t channel)
{
    uint8_t i,j;
    uint16_t ret,temp;
    ADMUX |= (channel & 0x0f);
    for(i=0;i<8;i++)
    {
        ADCSRA |= _BV(ADSC);
        while(!(ADCSRA & (1 << ADIF)));
        ADCSRA |= (1 << ADIF);
        ret = ADC;
        adcValue[i] = ret;
    }
    for(i=0;i<8-1;i++)
    {
        for(j=0;j<8-1-i;j++)
        {
            if(adcValue[j]>adcValue[j+1])
            {
                temp = adcValue[j];
                adcValue[j] = adcValue[j+1];
                adcValue[j+1] = temp;
            }
        }
    }
    ret = 0;
    for(i=1;i<7;i++)
    {
        ret += adcValue[i];
    }
    ret /= 6;
    return ret;
}

int main(void) 
{
    float f;
    avr_uart_init();
    DDRC &= ~(_BV(PC0));
    PORTC &= ~(_BV(PC0));
    adcInit();
    while(1)
    {
        f = adcRead(0)/1023.0*5.0;
        xprintf("%.2f\n",f);
        delay_ms(1000);
    }
    return 0;
}