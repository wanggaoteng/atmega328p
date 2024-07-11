#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include "avr/io.h"
#include "avr/interrupt.h"
#include "util/atomic.h"
#include "util/delay_basic.h"

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

void set_register_bits( volatile uint8_t *mcu_register, int n, ... );
void clear_register_bits( volatile uint8_t *mcu_register, int n, ... );

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
///////////////////////////////////////////////////////////////////////////////////

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
#endif
///////////////////////////////////////////////////////////////////////////////////






#endif