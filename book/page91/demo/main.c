#define __STDIO_FDEVOPEN_COMPAT_12

#include "avr/pgmspace.h"
#include "functions.h"
#include "stdio.h"

uint8_t str[32];

int usart_putchar(char c)
{
    if(c == '\n')
        usart_putchar('\r');
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return 0;
}

int usart_getchar(void)
{
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

void usart_ini(void)
{
    set_bits(UCSR0B, _BV(RXEN0) | _BV(TXEN0));
    UBRR0 = 51;
    fdevopen(usart_putchar, usart_getchar, 0);
}

int main(void) 
{
    int temp;
    usart_ini();
    while(1)
    {
        printf("input a string: ");
        scanf("%s", str);
        printf("output: %s\n", str);
        printf_P(PSTR("input a number: "));
        scanf_P(PSTR("%d"), &temp);
        printf_P(PSTR("output: %d\n"), temp);
        printf("-------------------------------------\n");
    }
    return 0;
}
