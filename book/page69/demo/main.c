#include "functions.h"

volatile uint16_t adcValue;

void adcInit(void)
{
    ADMUX |= _BV(REFS0);
    ADMUX &= ~(_BV(MUX3) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0));
    ADCSRA |= (_BV(ADEN) | _BV(ADIE) | _BV(ADPS1) | _BV(ADPS0));
}

int main(void) 
{
    float f;
    avr_uart_init();
    DDRC &= ~(_BV(PC0));
    PORTC &= ~(_BV(PC0));
    sei();
    adcInit(); 
    ADCSRA |= _BV(ADSC);
    while(1)
    {
        delay_ms(1000);
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            f = adcValue/1023.0*5.0;
        }
        xprintf("%.2f\n",f);
    }
    return 0;
}

ISR(ADC_vect)
{
    adcValue = ADC;
    ADCSRA |= _BV(ADSC);
}
