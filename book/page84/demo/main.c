#include "functions.h"

uint8_t txd_pos = 0;
uint8_t txd_len = 0;
uint8_t rxd_pos = 0;
uint8_t rxd_len = 0;

uint8_t send_buf[16];
uint8_t recv_buf[16];

uint8_t is_recv_complete(void)
{
    return rxd_len == 0;
}

void send_to_uart(uint8_t size)
{
    txd_pos = 0;
    txd_len = size;
    UDR0 = send_buf[0];
    delay_ms(50);
    while (txd_len > 0);    
}

void recv_from_uart(uint8_t size, uint8_t wait)
{
    rxd_pos = 0;
    rxd_len = size;
    if(wait)
        while(rxd_len > 0);
}

int main(void) 
{
    uint8_t i = 0;
    set_bits(UCSR0B, _BV(RXCIE0) | _BV(TXCIE0) | _BV(RXEN0) | _BV(TXEN0));
    UBRR0 = 51;
    sei();
    while(1)
    {
        recv_from_uart(16, 0);
        while(!is_recv_complete())
        {
            delay_ms(1);
        }
        for(i = 0; i < 16; i++)
        {
            send_buf[i] = recv_buf[i];
        }
        send_to_uart(16);
    }
    return 0;
}

ISR(USART_RX_vect)
{
    uint8_t c = UDR0;
    if(rxd_len > 0)
    {
        recv_buf[rxd_pos++] = c;
        rxd_len--;
    }
}

ISR(USART_TX_vect)
{
    if(--txd_len > 0)
        UDR0 = send_buf[++txd_pos];
}
