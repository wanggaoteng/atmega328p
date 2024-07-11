#include "functions.h"

extern uint8_t time_bcd[7];
extern uint8_t time_dec[7];

int main(void) 
{
    uint8_t i;
    char c;
    uint8_t temp[7]; // line27: uint8_t --- %hhu        int --- %d
    avr_uart_init();
    ds1307_init();
    ds1307_write_one_byte(DS1307_ADDR, 0, (uint8_t *)&c);
    xprintf("p: print;    s: set;    h: help;\n");
    while(1)
    {
        scanf("%c", &c);
        if(c == 'p')
        {
            ds1307_update(DS1307_ADDR, 1);
            ds1307_bcd2dec();
            printf("current time: %02d-%02d-%02d\t%02d\t%02d:%02d:%02d\n", time_dec[6], time_dec[5], time_dec[4], time_dec[3], time_dec[2], time_dec[1], time_dec[0]);
        }
        else if(c == 's')
        {
            printf("input time: year month day week hour minute second\n");
            scanf("%hhu %hhu %hhu %hhu %hhu %hhu %hhu", temp+6, temp+5, temp+4, temp+3, temp+2, temp+1, temp+0);    
            for(i = 0; i < 7; i++)
            {
                time_dec[i] = temp[i];
            }     
            ds1307_dec2bcd();
            ds1307_update(DS1307_ADDR, 0);
            printf("done: %02d-%02d-%02d\t%02d\t%02d:%02d:%02d\n", time_dec[6], time_dec[5], time_dec[4], time_dec[3], time_dec[2], time_dec[1], time_dec[0]);
        }
        else if(c == 'h')
        {
            printf("p: print;    s: set;    h: help;\n");
        }
    }

    return 0;
}







