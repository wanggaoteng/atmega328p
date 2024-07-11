#include "functions.h"

#define MASTER  0
#define SLAVE   1

#define TWI_ADDRESS 0x32

#if MASTER

#define KEY 0x04

void twi_stop(void)
{
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
}

uint8_t twi_start(void)
{
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    while((TWCR & _BV(TWINT)) == 0);
    return TW_STATUS;
}

uint8_t twi_write_byte(uint8_t c)
{
    TWDR = c;
    TWCR = _BV(TWINT) | _BV(TWEN);
    while((TWCR & _BV(TWINT)) == 0);
    return TW_STATUS;
}

uint8_t twi_read_byte(uint8_t *c, uint8_t ack)
{
    uint8_t temp = _BV(TWINT) | _BV(TWEN);
    if(ack)
        temp |= _BV(TWEA);
    TWCR = temp;
    while((TWCR & _BV(TWINT)) == 0);
    *c = TWDR;
    return TW_STATUS;
}

uint8_t key_down(void)
{
    if(bit_is_clear(PIND, PD2))
    {
        delay_ms(20);
        if(bit_is_clear(PIND, PD2))
            return 1;
    }
    return 0;
}

int main(void) 
{
    uint8_t i;
    TWBR = (8000000UL / 100000UL - 16) / 2;
    TWSR = 0;
    TWCR = 0x44;
    while(1)
    {
        if(key_down())
        {
            twi_start();
            delay_ms(100);
            twi_write_byte(TWI_ADDRESS | TW_WRITE);
            delay_ms(100);
            for(i = 0; i < 10; i++)
            {
                twi_write_byte(i);
                delay_ms(100);
            }
            twi_stop();
        }
    }
    return 0;
}

#endif // MASTER

#if SLAVE

int main(void) 
{
    uint8_t i, j = 0;

    avr_uart_init();

    TWAR = TWI_ADDRESS | _BV(TWGCE);
    TWCR = _BV(TWEA) | _BV(TWEN);

    while(1)
    {
        while((TWCR & _BV(TWINT)) == 0);
        i = TW_STATUS;
        switch(i)
        {
        case TW_SR_SLA_ACK:
            xprintf("START\nSLA+W\n");
            break;
        case TW_SR_DATA_ACK:
            if(j == 0)
                xprintf("receive: %d", TWDR);
            else
                xprintf(" %d", TWDR);
            j++;
            break;
        case TW_SR_STOP:
            xprintf(";\nSTOP\n\n");
            j = 0;
            break;
        default:
            xprintf("error: %x\n", i);
            break;
        }
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWINT);
    }

    return 0;
}

#endif // SLAVE