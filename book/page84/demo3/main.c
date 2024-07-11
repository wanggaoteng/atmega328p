#include "functions.h"

#define USART_REC_LEN 32

uint8_t USART_RX_BUF[USART_REC_LEN];

//bit7: receive complete flag
//bit6: receive 0x0d
//bit5~bit0: number of valid bytes received
uint8_t USART_RX_STA = 0;

void usart_init(void)
{
    set_bits(UCSR0B, _BV(RXCIE0) | _BV(TXCIE0) | _BV(RXEN0) | _BV(TXEN0));
    UBRR0 = 51;
}

int main(void) 
{
    usart_init();
    sei();
    while(1)
    {
        delay_ms(10);
        if(USART_RX_STA & 0x80)
        {
            loop_until_bit_is_set(UCSR0A, UDRE0);
            UDR0 = USART_RX_BUF[0];
        }
    }
    return 0;
}

ISR(USART_RX_vect)
{
    uint8_t res;
    res = UDR0;
    if((USART_RX_STA & 0x80) == 0)
    {
        if(USART_RX_STA & 0x40)
        {
            if(res != 0x0a)
                USART_RX_STA = 0;
            else
                USART_RX_STA |= 0x80;
        }
        else
        {
            if(res == 0x0d)
                USART_RX_STA |= 0x40;
            else
            {
                USART_RX_BUF[USART_RX_STA & 0x3f] = res;
                USART_RX_STA++;
                if(USART_RX_STA > (USART_REC_LEN - 1))
                    USART_RX_STA = 0;
            }
        }
    }
}

ISR(USART_TX_vect)
{
    uint8_t len, i;
    len = USART_RX_STA & 0x3f;
    for(i = 1; i < len; i++)
    {
        loop_until_bit_is_set(UCSR0A, UDRE0);
        UDR0 = USART_RX_BUF[i];
    }
    USART_RX_STA = 0;
}
