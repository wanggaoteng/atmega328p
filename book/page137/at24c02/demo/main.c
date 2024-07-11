#include "functions.h"

uint8_t a[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
uint8_t b[11];
int main(void) 
{
    int n, m;
    avr_uart_init();
    at24cxx_init();
    xprintf("at24c02: \n");
    n = at24cxx_n_write(AT24C02, AT24CXX_ADDR(A2, A1, A0), 0, 11, a);
    xprintf("%d\n", n);
    m = at24cxx_n_read(AT24C02, AT24CXX_ADDR(A2, A1, A0), 0, 11, b);
    xprintf("%d\n", m);
    for(int i = 0; i < 11; i++)
    {
        xprintf("%d ", b[i]);
    }
    xprintf("\n");
    xprintf("at24c32: \n");
    n = at24cxx_n_write(AT24C32, AT24CXX_ADDR(A2, A1, !A0), 0, 11, a);
    xprintf("%d\n", n);
    m = at24cxx_n_read(AT24C32, AT24CXX_ADDR(A2, A1, !A0), 0, 11, b);
    xprintf("%d\n", m);
    for(int i = 0; i < 11; i++)
    {
        xprintf("%d ", b[i]);
    }
    xprintf("\n");
    return 0;
}



