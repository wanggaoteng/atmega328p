#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include "avr/io.h"
#include "avr/interrupt.h"
#include "util/atomic.h"
#include "util/delay_basic.h"
#include "util/twi.h"

//sfr_defs.h
//#define _BV(bit) (1 << (bit))
//#define bit_is_set(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))
//#define bit_is_clear(sfr, bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
//#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
//#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))

//set_bits(PORTB, _BV(PB4)|_BV(PB5));
#define set_bits(port, mask) ( (port) |= (mask) )
//clear_bits(PORTB, _BV(PB4)|_BV(PB5));
#define clear_bits(port, mask) ( (port) &= ~(mask) )

/*
set_bits(PORTB, PB4, PB5);
#define set_bits(port, ...) \
do \
{ \
    uint8_t n = 0, temp = 0x00; \
    char *str = #__VA_ARGS__; \
    while(*str) \
    { \
        if(*str >= '0' && *str <= '9') \
            n = *str - '0'; \
        temp |= 1 << n; \
        str++; \
    } \
    port |= temp; \
}while(0)

clear_bits(PORTB, PB4, PB5);
#define clear_bits(port, ...) \
do \
{ \
    uint8_t n = 0, temp = 0xff; \
    char *str = #__VA_ARGS__; \
    while(*str) \
    { \
        if(*str >= '0' && *str <= '9') \
            n = *str - '0'; \
        temp &= ~(1 << n); \
        str++; \
    } \
    port &= temp; \
}while(0)
*/

// void set_register_bits( volatile uint8_t *mcu_register, int n, ... );
// void clear_register_bits( volatile uint8_t *mcu_register, int n, ... );

void delay_ms(uint16_t t);
void delay_us(uint16_t t);

void blink(uint8_t n,uint16_t delay);

////////////////////////////////////////////////////////////////////////////////////
//printf
#include "stdio.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

// init, to be called before using any further this library
void avr_uart_init(void);


// blocking functions for writing/reading one byte to/from the UART
void avr_uart_putchar(char c, FILE *stream);
char avr_uart_getchar(FILE *stream);
char avr_uart_getchar_echo(FILE *stream);
//non-blocking read. Return false if no data available.
//Return true, with data set to the available byte,
//NB : reading a bytes consumes it in any case, blocking or not.
bool avr_uart_data_avaiable(char *data);


extern FILE avr_uart_output;
extern FILE avr_uart_input;
extern FILE avr_uart_input_echo;

#ifdef __cplusplus
}
#endif
///////////////////////////////////////////////////////////////////////////////////end printf

///////////////////////////////////////////////////////////////////////////////////
//xprintf
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XF_USE_OUTPUT	1	/* 1: Enable output functions */
#define	XF_CRLF			0	/* 1: Convert \n ==> \r\n in the output char */
#define	XF_USE_DUMP		0	/* 1: Enable put_dump function */
#define	XF_USE_LLI		0	/* 1: Enable long long integer in size prefix ll */
#define	XF_USE_FP		0	/* 1: Enable support for floating point in type e and f */
#define XF_DPC			'.'	/* Decimal separator for floating point */
#define XF_USE_INPUT	0	/* 1: Enable input functions */
#define	XF_INPUT_ECHO	0	/* 1: Echo back input chars in xgets function */

#if defined(__GNUC__) && __GNUC__ >= 10
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

#if XF_USE_OUTPUT
#define xdev_out(func) xfunc_output = (void(*)(int))(func)
extern void (*xfunc_output)(int);
void xputc (int chr);
void xfputc (void (*func)(int), int chr);
void xputs (const char* str);
void xfputs (void (*func)(int), const char* str);
void xprintf (const char* fmt, ...);
void xsprintf (char* buff, const char* fmt, ...);
void xfprintf (void (*func)(int), const char* fmt, ...);
void put_dump (const void* buff, unsigned long addr, int len, size_t width);
#endif

#if XF_USE_INPUT
#define xdev_in(func) xfunc_input = (int(*)(void))(func)
extern int (*xfunc_input)(void);
int xgets (char* buff, int len);
int xatoi (char** str, long* res);
int xatof (char** str, double* res);
#endif

#ifdef __cplusplus
}
#endif // end xprintf

// SW_I2C
#define SW_I2C_AT24CXX          0

#if SW_I2C_AT24CXX

#define AT24C01		    127 // word address 8-bit
#define AT24C02		    255
#define AT24C04		    511
#define AT24C08		    1023
#define AT24C16		    2047
#define AT24C32		    4095 // word address 16-bit
#define AT24C64	        8191
#define AT24C128	    16383
#define AT24C256	    32767  

//hardware address for 24Cxx EEPROM:
//1 0 1 0 A2 A1 A0 R/~W	24C01/24C02
//1 0 1 0 A2 A1 NC R/~W	24C04
//1 0 1 0 A2 NC NC R/~W	24C08
//1 0 1 0 NC NC NC R/~W	24C16  
#define AT24CXX_ADDR(_A2, _A1, _A0)     ( 0xa0 ^ ( ((_A2)<<3) | ((_A1)<<2) | ((_A0)<<1) ) )
#define A2              0       //0 for ground, 1 for vcc
#define A1              0
#define A0              0

#define SDA PC4
#define SCL PC5

#define TW_READ         1       //read command
#define TW_WRITE        0       //write command

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_wait_ack(void);
void i2c_ack(void);
void i2c_nack(void);
void i2c_send_byte(uint8_t txd);
uint8_t i2c_read_byte(uint8_t ack);

//for word address 8-bit or 16-bit
void at24cxx_init(void);
uint8_t at24cxx_check(uint16_t ee_type, uint8_t hard_addr);
uint8_t at24cxx_read_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr);
void at24cxx_write_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t data_to_write);
void at24cxx_write_length_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint32_t data_to_write, uint8_t length);
uint32_t at24cxx_read_length_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t length);
void at24cxx_read(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t *buffer, uint16_t number_to_read);
void at24cxx_write(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t *buffer, uint16_t number_to_write);
void at24cxx_n_read(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t *buffer, uint16_t number_to_read);
void at24cxx_n_write(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t *buffer, uint16_t number_to_write);

#endif // end SW_I2C_AT24CXX

#define HW_I2C_AT24CXX_OPTIONAL        0

#if HW_I2C_AT24CXX_OPTIONAL

#include "stdlib.h"
#include "util/twi.h"
#define AT24C01		            127 // word address 8-bit
#define AT24C02		            255
#define AT24C04		            511
#define AT24C08		            1023
#define AT24C16		            2047
#define AT24C32		            4095 // word address 16-bit
#define AT24C64	                8191
#define AT24C128	            16383
#define AT24C256	            32767  

//hardware address for 24Cxx EEPROM:
//1 0 1 0 A2 A1 A0 R/~W	24C01/24C02
//1 0 1 0 A2 A1 NC R/~W	24C04
//1 0 1 0 A2 NC NC R/~W	24C08
//1 0 1 0 NC NC NC R/~W	24C16  
#define AT24CXX_ADDR(_A2, _A1, _A0)     ( 0xa0 ^ ( ((_A2)<<3) | ((_A1)<<2) | ((_A0)<<1) ) )
#define A2              0       //0 for ground, 1 for vcc
#define A1              0
#define A0              0

#define i2c_init()                      {TWBR = (8000000UL / 100000UL - 16) / 2;TWSR = 0;TWCR = 0x44;} // 100k
#define i2c_start()                     (TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN))
#define i2c_stop()                      (TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN))
#define i2c_send()                      (TWCR = _BV(TWINT) | _BV(TWEN))
#define i2c_send_ack()                  (TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA))
#define i2c_send_nack()                 (TWCR = _BV(TWINT) | _BV(TWEN))
#define i2c_read_ack()                  (TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA))
#define i2c_read_nack()                 (TWCR = _BV(TWINT) | _BV(TWEN))
#define i2c_wait()                      while(!(TWCR & _BV(TWINT)))

// void i2c_init();
void at24cxx_init(void);
uint8_t at24cxx_read_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr);
void at24cxx_write_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t data_to_write);
void at24cxx_write_length_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint32_t data_to_write, uint8_t length);
uint32_t at24cxx_read_length_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t length);
uint8_t at24cxx_check(uint16_t ee_type, uint8_t hard_addr);
void at24cxx_read(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t *buffer, uint16_t number_to_read);
void at24cxx_write(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t *buffer, uint16_t number_to_write);
void at24cxx_n_read(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t *buffer, uint16_t number_to_read);
void at24cxx_n_write(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t *buffer, uint16_t number_to_write);
void error(void);

#endif // HW_I2C_AT24CXX_OPTIONAL

#define HW_I2C_AT24CXX          0

#if HW_I2C_AT24CXX

#include "util/twi.h"
#define AT24C01		    127 // word address 8-bit
#define AT24C02		    255
#define AT24C04		    511
#define AT24C08		    1023
#define AT24C16		    2047
#define AT24C32		    4095 // word address 16-bit
#define AT24C64	        8191
#define AT24C128	    16383
#define AT24C256	    32767  

#define MAX_ITER        200
#define PAGE_SIZE       8 // refer to the datasheet of at24cxx, write operation

//hardware address for 24Cxx EEPROM:
//1 0 1 0 A2 A1 A0 R/~W	24C01/24C02
//1 0 1 0 A2 A1 NC R/~W	24C04
//1 0 1 0 A2 NC NC R/~W	24C08
//1 0 1 0 NC NC NC R/~W	24C16  
#define AT24CXX_ADDR(_A2, _A1, _A0)     ( 0xa0 ^ ( ((_A2)<<3) | ((_A1)<<2) | ((_A0)<<1) ) )
#define A2                              0       //0 for ground, 1 for vcc
#define A1                              0
#define A0                              0

#define i2c_init()                      {TWBR = (8000000UL / 100000UL - 16) / 2;TWSR = 0;TWCR = 0x44;} // 100k
#define i2c_start()                     (TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN))
#define i2c_stop()                      (TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN))
#define i2c_send()                      (TWCR = _BV(TWINT) | _BV(TWEN))
#define i2c_send_ack()                  (TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA))
#define i2c_send_nack()                 (TWCR = _BV(TWINT) | _BV(TWEN))
#define i2c_read_ack()                  (TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA))
#define i2c_read_nack()                 (TWCR = _BV(TWINT) | _BV(TWEN))
#define i2c_wait()                      while(!(TWCR & _BV(TWINT)))

void at24cxx_init(void);
int at24cxx_check(uint16_t ee_type, uint8_t hard_addr);
int at24cxx_read_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t *buffer);
int at24cxx_write_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t *buffer);
int at24cxx_n_write(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, int len, uint8_t *buffer);
int at24cxx_n_read(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, int len, uint8_t *buffer);

#endif // HW_I2C_AT24CXX

#define HW_I2C_DS1307          0

#if HW_I2C_DS1307

#include "util/twi.h"

#define MAX_ITER        200
#define PAGE_SIZE       8

#define DS1307_ADDR     0xd0

#define i2c_init()                      {TWBR = (8000000UL / 100000UL - 16) / 2;TWSR = 0;TWCR = 0x44;} // 100k
#define i2c_start()                     (TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN))
#define i2c_stop()                      (TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN))
#define i2c_send()                      (TWCR = _BV(TWINT) | _BV(TWEN))
#define i2c_send_ack()                  (TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA))
#define i2c_send_nack()                 (TWCR = _BV(TWINT) | _BV(TWEN))
#define i2c_read_ack()                  (TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA))
#define i2c_read_nack()                 (TWCR = _BV(TWINT) | _BV(TWEN))
#define i2c_wait()                      while(!(TWCR & _BV(TWINT)))

void ds1307_init(void);
int ds1307_read_one_byte(uint8_t hard_addr, uint8_t read_addr, uint8_t *buffer);
int ds1307_write_one_byte(uint8_t hard_addr, uint8_t write_addr, uint8_t *buffer);
int ds1307_n_write(uint8_t hard_addr, uint8_t write_addr, int len, uint8_t *buffer);
int ds1307_n_read(uint8_t hard_addr, uint8_t read_addr, int len, uint8_t *buffer);
uint8_t ds1307_update(uint8_t hard_addr, uint8_t dir);
uint8_t dec2bcd(uint8_t dec);
uint8_t bcd2dec(uint8_t bcd);
void ds1307_dec2bcd(void);
void ds1307_bcd2dec(void);
void ds1307_sqw_output(uint8_t hard_addr, uint8_t en, uint8_t level);

#endif // HW_I2C_DS1307


#endif //__FUNCTIONS_H__