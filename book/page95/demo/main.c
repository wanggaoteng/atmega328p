#include "stdlib.h"
#include "stdio.h"
#include "functions.h"

int usart_putchar(char c)
{
    if(c == '\n')
        usart_putchar('\r');
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return 0;
}

void miniprintf(char *fmt, ...)
{
    va_list ap;
    char strval[6];
    char *p;
    int nval;
    uint8_t i;
    va_start(ap, fmt);
    for(p = fmt; *p; p++)
    {
        if(*p != '%')
        {
            usart_putchar(*p);
            continue;
        }
        p++;
        switch(*p)
        {
            case 'd': 
                nval = va_arg(ap, int);
                itoa(nval, strval, 10);
                break;
            case 'x':
                nval = va_arg(ap, int);
                itoa(nval, strval, 16);
                break;
        }
        i = 0;
        for(i = 0; strval[i]; i++)
            usart_putchar(strval[i]);
    }
    va_end(ap);
}

void usart_ini(void)
{
    set_bits(UCSR0B, _BV(TXEN0));
    UBRR0 = 51;
}

int main(void) 
{
    uint8_t i = 0;
    usart_ini();
    while(1)
    {
        miniprintf("dec: %d\n", i);
        miniprintf("hex: %x\n", i);
        i++;
        delay_ms(1000);
    }
    return 0;
}
