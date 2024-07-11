#include "functions.h"

//set_register_bits(&PORTB, 2, PB4, PB5);
// void set_register_bits( volatile uint8_t *mcu_register, int n, ... )
// {
// 	uint8_t i = 0, temp = 0x00;
// 	va_list pointer;
// 	va_start( pointer, n );
// 	for( i = 0; i < n; i++ )
// 	{
// 		temp |= _BV( va_arg( pointer, int ) );
// 	}
// 	*mcu_register |= temp;
// }

//clear_register_bits(&PORTB, 2, PB4, PB5);
// void clear_register_bits( volatile uint8_t *mcu_register, int n, ... )
// {
// 	uint8_t i = 0, temp = 0xff;
// 	va_list pointer;
// 	va_start( pointer, n );
// 	for( i = 0; i < n; i++ )
// 	{
// 		temp &= ~_BV( va_arg( pointer, int ) );
// 	}
// 	*mcu_register &= temp;
// }

void delay_ms(uint16_t t)
{
    uint16_t i;
    for(i = 0; i < t; i++)
    {
        _delay_loop_2(2000);//8M, _delay_loop_2(1) is four clock cycle (4/8000000)
    }
}

void delay_us(uint16_t t)
{
    uint16_t i;
    for(i = 0; i < t; i++)
    {
        _delay_loop_2(2);
    }
}

//n: number of blinks
//delay: blink time interval
//n>0,delay>0  normal 
//n=0,delay>0  cycle 
//n=0,delay=0  toggle 
void blink(uint8_t n,uint16_t delay)
{
	DDRB |= _BV(PB5);
	if((n>0)&&(delay>0))
	{
		while(n)
		{

			PORTB |= _BV(PB5);
			delay_ms(delay);
			PORTB &= ~_BV(PB5);
			delay_ms(delay);
			n--;
		}
	}
	else if((n==0)&&(delay>0))
	{
		while(1)
		{
			PORTB |= _BV(PB5);
			delay_ms(delay);
			PORTB &= ~_BV(PB5);
			delay_ms(delay);
		}
	}
	else if((n==0)&&(delay==0))
	{
		if(bit_is_set(PINB,PB5))
			PORTB &= ~_BV(PB5);
		else
			PORTB |= _BV(PB5);
	}
}


////////////////////////////////////////////////////////////////////////////////////
//printf, use it directly as it has been in stdio.h, but not recommend, https://github.com/JohannCahier/avr_uart
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#ifndef BAUD
#define BAUD 9600
#endif
#include "util/setbaud.h"

void avr_uart_init(void) 
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */

	stdout = &avr_uart_output;
    stdin  = &avr_uart_input;

	xdev_out(avr_uart_putchar);
}

void avr_uart_putchar(char c, FILE *stream) 
{
    if (c == '\n') {
        avr_uart_putchar('\r', stream);
    }
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

char avr_uart_getchar(FILE *stream) 
{
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}


char avr_uart_getchar_echo(FILE *stream) 
{
    loop_until_bit_is_set(UCSR0A, RXC0);
    char c=UDR0;
    avr_uart_putchar(c, stream);
    return c;
}

// return 0 if no data available, read it if available
bool avr_uart_data_avaiable(char *data) 
{
    if (UCSR0A & (1<<RXC0)) {
        *data = UDR0;
        return true;
    }
    return false;
}

FILE avr_uart_output = FDEV_SETUP_STREAM((int (*)(char, struct __file *))avr_uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE avr_uart_input = FDEV_SETUP_STREAM(NULL, (int	(*)(struct __file *))avr_uart_getchar, _FDEV_SETUP_READ);
FILE avr_uart_input_echo = FDEV_SETUP_STREAM((int (*)(char, struct __file *))avr_uart_putchar, (int	(*)(struct __file *))avr_uart_getchar_echo, _FDEV_SETUP_RW);

///////////////////////////////////////////////////////////////////////////////////end printf

//xprintf, recommend, http://elm-chan.org/fsw/strf/xprintf.html
#define SZB_OUTPUT	32

#if XF_USE_OUTPUT
#include <stdarg.h>
void (*xfunc_output)(int);	/* Pointer to the default output device */
static char *strptr;		/* Pointer to the output memory (used by xsprintf) */


#if XF_USE_FP
/*----------------------------------------------*/
/* Floating point output                        */
/*----------------------------------------------*/
#include <math.h>


static int ilog10 (double n)	/* Calculate log10(n) in integer output */
{
	int rv = 0;

	while (n >= 10) {	/* Decimate digit in right shift */
		if (n >= 100000) {
			n /= 100000; rv += 5;
		} else {
			n /= 10; rv++;
		}
	}
	while (n < 1) {		/* Decimate digit in left shift */
		if (n < 0.00001) {
			n *= 100000; rv -= 5;
		} else {
			n *= 10; rv--;
		}
	}
	return rv;
}


static double i10x (int n)	/* Calculate 10^n */
{
	double rv = 1;

	while (n > 0) {		/* Left shift */
		if (n >= 5) {
			rv *= 100000; n -= 5;
		} else {
			rv *= 10; n--;
		}
	}
	while (n < 0) {		/* Right shift */
		if (n <= -5) {
			rv /= 100000; n += 5;
		} else {
			rv /= 10; n++;
		}
	}
	return rv;
}


static void ftoa (
	char* buf,	/* Buffer to output the generated string */
	double val,	/* Real number to output */
	int prec,	/* Number of fractinal digits */
	char fmt	/* Notation */
)
{
	int d;
	int e = 0, m = 0;
	char sign = 0;
	double w;
	const char *er = 0;


	if (isnan(val)) {			/* Not a number? */
		er = "NaN";
	} else {
		if (prec < 0) prec = 6;	/* Default precision (6 fractional digits) */
		if (val < 0) {			/* Nagative value? */
			val = -val; sign = '-';
		} else {
			sign = '+';
		}
		if (isinf(val)) {		/* Infinite? */
			er = "INF";
		} else {
			if (fmt == 'f') {	/* Decimal notation? */
				val += i10x(-prec) / 2;	/* Round (nearest) */
				m = ilog10(val);
				if (m < 0) m = 0;
				if (m + prec + 3 >= SZB_OUTPUT) er = "OV";	/* Buffer overflow? */
			} else {			/* E notation */
				if (val != 0) {		/* Not a true zero? */
					val += i10x(ilog10(val) - prec) / 2;	/* Round (nearest) */
					e = ilog10(val);
					if (e > 99 || prec + 6 >= SZB_OUTPUT) {	/* Buffer overflow or E > +99? */
						er = "OV";
					} else {
						if (e < -99) e = -99;
						val /= i10x(e);	/* Normalize */
					}
				}
			}
		}
		if (!er) {	/* Not error condition */
			if (sign == '-') *buf++ = sign;	/* Add a - if negative value */
			do {				/* Put decimal number */
				w = i10x(m);				/* Snip the highest digit d */
				d = val / w; val -= d * w;
				if (m == -1) *buf++ = XF_DPC;	/* Insert a decimal separarot if get into fractional part */
				*buf++ = '0' + d;			/* Put the digit */
			} while (--m >= -prec);			/* Output all digits specified by prec */
			if (fmt != 'f') {	/* Put exponent if needed */
				*buf++ = fmt;
				if (e < 0) {
					e = -e; *buf++ = '-';
				} else {
					*buf++ = '+';
				}
				*buf++ = '0' + e / 10;
				*buf++ = '0' + e % 10;
			}
		}
	}
	if (er) {	/* Error condition? */
		if (sign) *buf++ = sign;		/* Add sign if needed */
		do *buf++ = *er++; while (*er);	/* Put error symbol */
	}
	*buf = 0;	/* Term */
}
#endif	/* XF_USE_FLOAT */


/*----------------------------------------------*/
/* Put a character                              */
/*----------------------------------------------*/

void xputc (
	int chr				/* Character to be output */
)
{
	xfputc(xfunc_output, chr);	/* Output it to the default output device */
}


void xfputc (			/* Put a character to the specified device */
	void(*func)(int),	/* Pointer to the output function (null:strptr) */
	int chr				/* Character to be output */
)
{
	if (XF_CRLF && chr == '\n') xfputc(func, '\r');	/* CR -> CRLF */

	if (func) {
		func(chr);		/* Write a character to the output device */
	} else if (strptr) {
		 *strptr++ = chr;	/* Write a character to the memory */
	}
}



/*----------------------------------------------*/
/* Put a null-terminated string                 */
/*----------------------------------------------*/

void xputs (			/* Put a string to the default device */
	const char* str		/* Pointer to the string */
)
{
	xfputs(xfunc_output, str);
}


void xfputs (			/* Put a string to the specified device */
	void(*func)(int),	/* Pointer to the output function */
	const char*	str		/* Pointer to the string */
)
{
	while (*str) {			/* Put the string */
		xfputc(func, *str++);
	}
}



/*----------------------------------------------*/
/* Formatted string output                      */
/*----------------------------------------------*/
/*  xprintf("%d", 1234);			"1234"
    xprintf("%6d,%3d%%", -200, 5);	"  -200,  5%"
    xprintf("%-6u", 100);			"100   "
    xprintf("%ld", 12345678);		"12345678"
    xprintf("%llu", 0x100000000);	"4294967296"	<XF_USE_LLI>
    xprintf("%lld", -1LL);			"-1"			<XF_USE_LLI>
    xprintf("%04x", 0xA3);			"00a3"
    xprintf("%08lX", 0x123ABC);		"00123ABC"
    xprintf("%016b", 0x550F);		"0101010100001111"
    xprintf("%*d", 6, 100);			"   100"
    xprintf("%s", "String");		"String"
    xprintf("%5s", "abc");			"  abc"
    xprintf("%-5s", "abc");			"abc  "
    xprintf("%-5s", "abcdefg");		"abcdefg"
    xprintf("%-5.5s", "abcdefg");	"abcde"
    xprintf("%-.5s", "abcdefg");	"abcde"
    xprintf("%-5.5s", "abc");		"abc  "
    xprintf("%c", 'a');				"a"
    xprintf("%12f", 10.0);			"   10.000000"	<XF_USE_FP>
    xprintf("%.4E", 123.45678);		"1.2346E+02"	<XF_USE_FP>
*/

static void xvfprintf (
	void(*func)(int),	/* Pointer to the output function */
	const char*	fmt,	/* Pointer to the format string */
	va_list arp			/* Pointer to arguments */
)
{
	unsigned int r, i, j, w, f;
	int n, prec;
	char str[SZB_OUTPUT], c, d, *p, pad;
#if XF_USE_LLI
	long long v;
	unsigned long long uv;
#else
	long v;
	unsigned long uv;
#endif

	for (;;) {
		c = *fmt++;					/* Get a format character */
		if (!c) break;				/* End of format? */
		if (c != '%') {				/* Pass it through if not a % sequense */
			xfputc(func, c); continue;
		}
		f = w = 0;			 		/* Clear parms */
		pad = ' '; prec = -1;
		c = *fmt++;					/* Get first char of the sequense */
		if (c == '0') {				/* Flag: left '0' padded */
			pad = '0'; c = *fmt++;
		} else {
			if (c == '-') {			/* Flag: left justified */
				f = 2; c = *fmt++;
			}
		}
		if (c == '*') {				/* Minimum width from an argument */
			n = va_arg(arp, int);
			if (n < 0) {			/* Flag: left justified */
				n = 0 - n; f = 2;
			}
			w = n; c = *fmt++;
		} else {
			while (c >= '0' && c <= '9') {	/* Minimum width */
				w = w * 10 + c - '0';
				c = *fmt++;
			}
		}
		if (c == '.') {				/* Precision */
			c = *fmt++;
			if (c == '*') {				/* Precision from an argument */
				prec = va_arg(arp, int);
				c = *fmt++;
			} else {
				prec = 0;
				while (c >= '0' && c <= '9') {
					prec = prec * 10 + c - '0';
					c = *fmt++;
				}
			}
		}
		if (c == 'l') {		/* Prefix: Size is long */
			f |= 4; c = *fmt++;
#if XF_USE_LLI
			if (c == 'l') {	/* Prefix: Size is long long */
				f |= 8; c = *fmt++;
			}
#endif
		}
		if (!c) break;				/* End of format? */
		switch (c) {				/* Type is... */
		case 'b':					/* Unsigned binary */
			r = 2; break;
		case 'o':					/* Unsigned octal */
			r = 8; break;
		case 'd':					/* Signed decimal */
		case 'u':					/* Unsigned decimal */
			r = 10; break;
		case 'x':					/* Hexdecimal (lower case) */
		case 'X':					/* Hexdecimal (upper case) */
			r = 16; break;
		case 'c':					/* A character */
			xfputc(func, (char)va_arg(arp, int)); continue;
		case 's':					/* String */
			p = va_arg(arp, char*);		/* Get a pointer argument */
			if (!p) p = "";				/* Null ptr generates a null string */
			j = strlen(p);
			if (prec >= 0 && j > (unsigned int)prec) j = prec;	/* Limited length of string body */
			for ( ; !(f & 2) && j < w; j++) xfputc(func, pad);	/* Left pads */
			while (*p && prec--) xfputc(func, *p++);/* String body */
			while (j++ < w) xfputc(func, ' ');		/* Right pads */
			continue;
#if XF_USE_FP
		case 'f':					/* Float (decimal) */
		case 'e':					/* Float (e) */
		case 'E':					/* Float (E) */
			ftoa(p = str, va_arg(arp, double), prec, c);	/* Make fp string */
			for (j = strlen(p); !(f & 2) && j < w; j++) xfputc(func, pad);	/* Left pads */
			while (*p) xfputc(func, *p++);		/* Value */
			while (j++ < w) xfputc(func, ' ');	/* Right pads */
			continue;
#endif
		default:					/* Unknown type (passthrough) */
			xfputc(func, c); continue;
		}

		/* Get an integer argument and put it in numeral */
#if XF_USE_LLI
		if (f & 8) {	/* long long argument? */
			v = (long long)va_arg(arp, long long);
		} else {
			if (f & 4) {	/* long argument? */
				v = (c == 'd') ? (long long)va_arg(arp, long) : (long long)va_arg(arp, unsigned long);
			} else {		/* int/short/char argument */
				v = (c == 'd') ? (long long)va_arg(arp, int) : (long long)va_arg(arp, unsigned int);
			}
		}
#else
		if (f & 4) {	/* long argument? */
			v = (long)va_arg(arp, long);
		} else {		/* int/short/char argument */
			v = (c == 'd') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int);
		}
#endif
		if (c == 'd' && v < 0) {	/* Negative value? */
			v = 0 - v; f |= 1;
		}
		i = 0; uv = v;
		do {	/* Make an integer number string */
			d = (char)(uv % r); uv /= r;
			if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
			str[i++] = d + '0';
		} while (uv != 0 && i < sizeof str);
		if (f & 1) str[i++] = '-';					/* Sign */
		for (j = i; !(f & 2) && j < w; j++) xfputc(func, pad);	/* Left pads */
		do xfputc(func, str[--i]); while (i != 0);	/* Value */
		while (j++ < w) xfputc(func, ' ');			/* Right pads */
	}
}


void xprintf (			/* Put a formatted string to the default device */
	const char*	fmt,	/* Pointer to the format string */
	...					/* Optional arguments */
)
{
	va_list arp;


	va_start(arp, fmt);
	xvfprintf(xfunc_output, fmt, arp);
	va_end(arp);
}


void xfprintf (			/* Put a formatted string to the specified device */
	void(*func)(int),	/* Pointer to the output function */
	const char*	fmt,	/* Pointer to the format string */
	...					/* Optional arguments */
)
{
	va_list arp;


	va_start(arp, fmt);
	xvfprintf(func, fmt, arp);
	va_end(arp);
}


void xsprintf (			/* Put a formatted string to the memory */
	char* buff,			/* Pointer to the output buffer */
	const char*	fmt,	/* Pointer to the format string */
	...					/* Optional arguments */
)
{
	va_list arp;


	strptr = buff;		/* Enable destination for memory */
	va_start(arp, fmt);
	xvfprintf(0, fmt, arp);
	va_end(arp);
	*strptr = 0;		/* Terminate output string */
	strptr = 0;			/* Disable destination for memory */
}



#if XF_USE_DUMP
/*----------------------------------------------*/
/* Dump a line of binary dump                   */
/*----------------------------------------------*/

void put_dump (
	const void* buff,		/* Pointer to the array to be dumped */
	unsigned long addr,		/* Heading address value */
	int len,				/* Number of items to be dumped */
	size_t width			/* Size of buff[0] (1, 2 or 4) */
)
{
	int i;
	const unsigned char *bp;
	const unsigned short *sp;
	const unsigned long *lp;


	xprintf("%08lX ", addr);		/* address */

	switch (width) {
	case sizeof (char):
		bp = buff;
		for (i = 0; i < len; i++) {		/* Hexdecimal dump in (char) */
			xprintf(" %02X", bp[i]);
		}
		xputs("  ");
		for (i = 0; i < len; i++) {		/* ASCII dump */
			xputc((unsigned char)((bp[i] >= ' ' && bp[i] <= '~') ? bp[i] : '.'));
		}
		break;
	case sizeof (short):
		sp = buff;
		do {							/* Hexdecimal dump in (short) */
			xprintf(" %04X", *sp++);
		} while (--len);
		break;
	case sizeof (long):
		lp = buff;
		do {							/* Hexdecimal dump in (short) */
			xprintf(" %08lX", *lp++);
		} while (--len);
		break;
	}

	xputc('\n');
}
#endif	/* XF_USE_DUMP */

#endif	/* XF_USE_OUTPUT */



#if XF_USE_INPUT
int (*xfunc_input)(void);	/* Pointer to the default input stream */

/*----------------------------------------------*/
/* Get a line from the input                    */
/*----------------------------------------------*/


int xgets (			/* 0:End of stream, 1:A line arrived */
	char* buff,		/* Pointer to the buffer */
	int len			/* Buffer length */
)
{
	int c, i;

	
	if (!xfunc_input) return 0;	/* No input function is specified */
	xfputc(xfunc_output, '\0'); /* Do not delete this line */
	i = 0;
	for (;;) {
		c = xfunc_input();			/* Get a char from the incoming stream */
		if (c < 0 || c == '\r') break;	/* End of stream or CR? */
		if (c == '\b' && i) {		/* BS? */
			i--;
			if (XF_INPUT_ECHO) xputc(c);
			continue;
		}
		if (c >= ' ' && i < len - 1) {	/* Visible chars? */
			buff[i++] = c;
			if (XF_INPUT_ECHO) xputc(c);
		}
	}
	if (XF_INPUT_ECHO) {
		xputc('\r');
		xputc('\n');
	}
	buff[i] = 0;	/* Terminate with a \0 */
	return (int)(c == '\r');
}


/*----------------------------------------------*/
/* Get a value of integer string                */
/*----------------------------------------------*/
/*	"123 -5   0x3ff 0b1111 0377  w "
	    ^                           1st call returns 123 and next ptr
	       ^                        2nd call returns -5 and next ptr
                   ^                3rd call returns 1023 and next ptr
                          ^         4th call returns 15 and next ptr
                               ^    5th call returns 255 and next ptr
                                  ^ 6th call fails and returns 0
*/

int xatoi (			/* 0:Failed, 1:Successful */
	char **str,		/* Pointer to pointer to the string */
	long *res		/* Pointer to the valiable to store the value */
)
{
	unsigned long val;
	unsigned char c, r, s = 0;


	*res = 0;

	while ((c = **str) == ' ') (*str)++;	/* Skip leading spaces */

	if (c == '-') {		/* negative? */
		s = 1;
		c = *(++(*str));
	}

	if (c == '0') {
		c = *(++(*str));
		switch (c) {
		case 'x':		/* hexdecimal */
			r = 16; c = *(++(*str));
			break;
		case 'b':		/* binary */
			r = 2; c = *(++(*str));
			break;
		default:
			if (c <= ' ') return 1;	/* single zero */
			if (c < '0' || c > '9') return 0;	/* invalid char */
			r = 8;		/* octal */
		}
	} else {
		if (c < '0' || c > '9') return 0;	/* EOL or invalid char */
		r = 10;			/* decimal */
	}

	val = 0;
	while (c > ' ') {
		if (c >= 'a') c -= 0x20;
		c -= '0';
		if (c >= 17) {
			c -= 7;
			if (c <= 9) return 0;	/* invalid char */
		}
		if (c >= r) return 0;		/* invalid char for current radix */
		val = val * r + c;
		c = *(++(*str));
	}
	if (s) val = 0 - val;			/* apply sign if needed */

	*res = val;
	return 1;
}


#if XF_USE_FP
/*----------------------------------------------*/
/* Get a value of the real number string        */
/*----------------------------------------------*/
/* Float version of xatoi
*/

int xatof (			/* 0:Failed, 1:Successful */
	char **str,		/* Pointer to pointer to the string */
	double *res		/* Pointer to the valiable to store the value */
)
{
	double val;
	int s, f, e;
	unsigned char c;


	*res = 0;
	s = f = 0;

	while ((c = **str) == ' ') (*str)++;	/* Skip leading spaces */
	if (c == '-') {			/* Negative? */
		c = *(++(*str)); s = 1; 
	} else if (c == '+') {	/* Positive? */
		c = *(++(*str));
	}
	if (c == XF_DPC) {		/* Leading dp? */
		f = -1; 			/* Start at fractional part */
		c = *(++(*str));
	}
	if (c <= ' ') return 0;	/* Wrong termination? */
	val = 0;
	while (c > ' ') {		/* Get a value of decimal */
		if (c == XF_DPC) {	/* Embedded dp? */
			if (f < 0) return 0;	/* Wrong dp? */
			f = -1;			/* Enter fractional part */
		} else {
			if (c < '0' || c > '9') break;	/* End of decimal? */
			c -= '0';
			if (f == 0) {	/* In integer part */
				val = val * 10 + c;
			} else {		/* In fractional part */
				val += i10x(f--) * c;
			}
		}
		c = *(++(*str));
	}
	if (c > ' ') {	/* It may be an exponent */
		if (c != 'e' && c != 'E') return 0;	/* Wrong character? */
		c = *(++(*str));
		if (c == '-') {
			c = *(++(*str)); s |= 2;	/* Negative exponent */
		} else if (c == '+') {
			c = *(++(*str));			/* Positive exponent */
		}
		if (c <= ' ') return 0;	/* Wrong termination? */
		e = 0;
		while (c > ' ') {		/* Get value of exponent */
			c -= '0';
			if (c > 9) return 0;	/* Not a numeral? */
			e = e * 10 + c;
			c = *(++(*str));
		}
		val *= i10x((s & 2) ? -e : e);	/* Apply exponent */
	}

	if (s & 1) val = -val;	/* Negate sign if needed */

	*res = val;
	return 1;
}
#endif /* XF_USE_FP */

#endif /* XF_USE_INPUT */

////////////////////////////////////////////////////////////////////////////////////end xprintf

//SW_I2C_AT24CXX
#if SW_I2C_AT24CXX

//i2c driver
//the i2c pins should use external pull-up resistors, do not rely on the internal pull-up resistors
void i2c_init(void)
{
    set_bits(DDRC, _BV(SDA) | _BV(SCL));//configure SDA/SCL as output
    set_bits(PORTC, _BV(SDA) | _BV(SCL));//SDA = 1, SCL = 1
}

//Generate I2C START signal
void i2c_start(void)
{
    set_bits(DDRC, _BV(SDA));
    set_bits(PORTC, _BV(SDA));//SDA = 1
    set_bits(DDRC, _BV(SCL));
    set_bits(PORTC, _BV(SCL));//SCL = 1
    delay_us(4);
    clear_bits(PORTC, _BV(SDA));//SDA = 0, when SCL is HIGH, SDA changes from HIGH to LOW
    delay_us(4);
    clear_bits(PORTC, _BV(SCL));//SCL = 0, clamps the IIC bus, now the bus is busy, preparing to send or receive
}

//Generate I2C STOP signal
void i2c_stop(void)
{
    set_bits(DDRC, _BV(SCL));
    clear_bits(PORTC, _BV(SCL));//SCL = 0
    set_bits(DDRC, _BV(SDA));
    clear_bits(PORTC, _BV(SDA));//SDA = 0
    delay_us(4);
    set_bits(PORTC, _BV(SCL));//SCL = 1
    set_bits(PORTC, _BV(SDA));//SDA = 1, when SCL is HIGH, SDA changes from LOW to HIGH
    delay_us(4);//as SCL = 1, SDA = 1, now the bus is free
}

//wait for ack: 1, failure; 0, get ack
uint8_t i2c_wait_ack(void)
{
    uint8_t err_time = 0;
    clear_bits(DDRC, _BV(SDA));//configure SDA as input
    set_bits(PORTC, _BV(SDA));//enable pull-up resistor of SDA
    delay_us(1);
    set_bits(DDRC, _BV(SCL));
    set_bits(PORTC, _BV(SCL));//SCL = 1
    delay_us(1);
    while(bit_is_set(PINC, SDA))//wait for ack
    {
        err_time++;
        if(err_time > 250)
        {
            i2c_stop();
            return 1;
        }
    }
    clear_bits(PORTC, _BV(SCL));//SCL = 0
    return 0;
}

void i2c_ack(void)
{
    set_bits(DDRC, _BV(SCL));
    clear_bits(PORTC, _BV(SCL));//SCL = 0
    set_bits(DDRC, _BV(SDA));
    clear_bits(PORTC, _BV(SDA));//SDA = 0
    delay_us(2);
    set_bits(PORTC, _BV(SCL));//SCL = 1
    delay_us(2);
    clear_bits(PORTC, _BV(SCL));//SCL = 0 
}

void i2c_nack(void)
{
    set_bits(DDRC, _BV(SCL));
    clear_bits(PORTC, _BV(SCL));//SCL = 0
    set_bits(DDRC, _BV(SDA));
    set_bits(PORTC, _BV(SDA));//SDA = 1
    delay_us(2);
    set_bits(PORTC, _BV(SCL));//SCL = 1
    delay_us(2);
    clear_bits(PORTC, _BV(SCL));//SCL = 0
}

void i2c_send_byte(uint8_t txd)
{
    uint8_t i;
    set_bits(DDRC, _BV(SDA) | _BV(SCL));
    clear_bits(PORTC, _BV(SCL));//SCL = 0
    for(i = 0; i < 8; i++)
    {
        if((txd << i) & 0x80)
            set_bits(PORTC, _BV(SDA));
        else
            clear_bits(PORTC, _BV(SDA));
        delay_us(2);
        set_bits(PORTC, _BV(SCL));//SCL = 1
        delay_us(2);
        clear_bits(PORTC, _BV(SCL));//SCL = 0
        delay_us(2);
    }
}

//ack = 1, send ACK; ack = 0, send NACK
uint8_t i2c_read_byte(uint8_t ack)
{
    uint8_t i, recv = 0;
    clear_bits(DDRC, _BV(SDA));//configure SDA as input
	set_bits(PORTC, _BV(SDA));//enable pull-up resistor
    set_bits(DDRC, _BV(SCL));
    for(i = 0; i < 8; i++)
    {
        clear_bits(PORTC, _BV(SCL));//SCL = 0
        delay_us(2);
        set_bits(PORTC, _BV(SCL));//SCL = 1
        recv <<= 1;
        if(bit_is_set(PINC, SDA))
            recv |= 0x01;
        delay_us(1);
    }
    if(ack)
        i2c_ack();
    else 
        i2c_nack();
    return recv;
}

//AT24CXX
void at24cxx_init(void)
{
    i2c_init();
}

//read one byte at the specified address of AT24CXX
//hard_addr: the hardware address of AT24CXX
//read_addr: the specified address
//return: the byte
uint8_t at24cxx_read_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr)
{
    uint8_t temp = 0;
    i2c_start();
    if(ee_type > AT24C16)
    {
        i2c_send_byte(hard_addr);//send write command
        i2c_wait_ack();
        i2c_send_byte(read_addr >> 8);//send high address
    }
    else
    {
		i2c_send_byte(hard_addr | (((read_addr >> 8) & 0x07) << 1));
    }
    i2c_wait_ack();
    i2c_send_byte(read_addr % 256);//send low address
    i2c_wait_ack();
    i2c_start();
    i2c_send_byte(hard_addr | TW_READ);//read
    i2c_wait_ack();
    temp = i2c_read_byte(0);
    i2c_stop();//generate stop signal
    return temp;
}

//write one byte at the specified address of AT24CXX
void at24cxx_write_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t data_to_write)
{
    i2c_start();
    if(ee_type > AT24C16)
    {
        i2c_send_byte(hard_addr);
        i2c_wait_ack();
        i2c_send_byte(write_addr >> 8);
    }
    else
    {
		i2c_send_byte(hard_addr | (((write_addr >> 8) & 0x07) << 1));
    }
    i2c_wait_ack();
    i2c_send_byte(write_addr % 256);
    i2c_wait_ack();
    i2c_send_byte(data_to_write);
    i2c_wait_ack();
    i2c_stop();
    delay_ms(10); // do not delete/change the delay time
}

//write length bytes starting from the specified address in AT24CXX
//this function is used to write 16bit or 32bit data
//write_addr: the address to start writing to
//data_to_write: the data to be written
//length: the length of data in byte, must =2 or =4
void at24cxx_write_length_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint32_t data_to_write, uint8_t length)
{  	
	uint8_t i;
	for(i = 0; i < length; i++)
	{
		at24cxx_write_one_byte(ee_type, hard_addr, write_addr + i, (data_to_write >> (8 * i)) & 0xff);
	}												    
}

//starting from the specified address in AT24CXX, read length bytes
//this function is used to read 16bit or 32bit data
//read_addr: the address to start reading
//return value: data
//length: the length of data in byte, must =2 or =4
uint32_t at24cxx_read_length_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t length)
{  	
	uint8_t i;
	uint32_t temp = 0;
	for(i = 0; i < length; i++)
	{
		temp <<= 8;
		temp += at24cxx_read_one_byte(ee_type, hard_addr, read_addr + length - i - 1); 	 				   
	}
	return temp;												    
}

//check whether AT24CXX is ok
//the last address (255) of 24XX is used here to store the flag word
//if using other 24C series, this address needs to be modified
//return 1: detection failed
//return 0: detection success
uint8_t at24cxx_check(uint16_t ee_type, uint8_t hard_addr)
{
	uint8_t temp;
	temp = at24cxx_read_one_byte(ee_type, hard_addr, 255);//avoid writing AT24CXX every time start the mcu			   
	if(temp == 0x55)
        return 0;		   
	else
	{
		at24cxx_write_one_byte(ee_type, hard_addr, 255, 0x55);
	    temp = at24cxx_read_one_byte(ee_type, hard_addr, 255);	  
		if(temp == 0x55)
            return 0;
	}
	return 1;											  
}

//starting from the specified address in AT24CXX, read the specified number of data one by one
//read_addr: the address to start reading, 0-255 for 24c02
//buffer: the first address of the data array
//number_to_read: the number of data to be read
void at24cxx_read(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t *buffer, uint16_t number_to_read)
{
	while(number_to_read)
	{
		*buffer++ = at24cxx_read_one_byte(ee_type, hard_addr, read_addr++);	
		number_to_read--;
	}
}

//write the specified number of data one by one starting from the specified address in AT24CXX
//write_addr: the address to start writing, 0-255 for 24c02
//buffer: the first address of the data array
//number_to_write: number of data to write
void at24cxx_write(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t *buffer, uint16_t number_to_write)
{
	while(number_to_write--)
	{
		at24cxx_write_one_byte(ee_type, hard_addr, write_addr, *buffer);
		write_addr++;
		buffer++;
	}
}

// sequential write, the max number to write is device depended, some vendor's devices would accept 16/32, 
// but 8 seems to be the lowest common denominator, read the datasheet of the device for a better setting of number_to_write
// if not sure, set number_to_write = 8 for universality
void at24cxx_n_write(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t *buffer, uint16_t number_to_write)
{
	uint8_t i;
    i2c_start();
    if(ee_type > AT24C16)
    {
        i2c_send_byte(hard_addr);
        i2c_wait_ack();
        i2c_send_byte(write_addr >> 8);
    }
    else
    {
		i2c_send_byte(hard_addr | (((write_addr >> 8) & 0x07) << 1));
    }
    i2c_wait_ack();
    i2c_send_byte(write_addr % 256);
    i2c_wait_ack();
	for(i = 0; i < number_to_write; i++)
	{
		i2c_send_byte(buffer[i]);
		i2c_wait_ack();
	}
	i2c_stop();
	delay_ms(10);
}

// sequential read, no max number to read restriction
void at24cxx_n_read(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t *buffer, uint16_t number_to_read)
{
	uint8_t i;
    i2c_start();
    if(ee_type > AT24C16)
    {
        i2c_send_byte(hard_addr);
        i2c_wait_ack();
        i2c_send_byte(read_addr >> 8);
    }
    else
    {
		i2c_send_byte(hard_addr | (((read_addr >> 8) & 0x07) << 1));
    }
    i2c_wait_ack();
    i2c_send_byte(read_addr % 256);
    i2c_wait_ack();	
	i2c_start();
    i2c_send_byte(hard_addr | TW_READ);//read
    i2c_wait_ack();
	for(i = 0; i < number_to_read-1; i++)
	{
		buffer[i] = i2c_read_byte(1);
	}
	buffer[i] = i2c_read_byte(0);
	i2c_stop();
	delay_ms(10);
}

#endif // SW_I2C_AT24CXX

#if HW_I2C_AT24CXX_OPTIONAL

void at24cxx_init(void)
{
    i2c_init();
}

uint8_t at24cxx_read_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr)
{
    uint8_t temp = 0;
    i2c_start();
	i2c_wait();
	if((TW_STATUS != TW_REP_START) && (TW_STATUS != TW_START))
		error();
    if(ee_type > AT24C16)
    {
		TWDR = hard_addr | TW_WRITE;
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_SLA_ACK)
			error();
		TWDR = read_addr >> 8;
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_DATA_ACK)
			error();
    }
    else
    {
		TWDR = hard_addr | (((read_addr >> 8) & 0x07) << 1);
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_SLA_ACK)
			error();
    }
	TWDR = read_addr % 256;
	i2c_send();
	i2c_wait();
	if(TW_STATUS != TW_MT_DATA_ACK)
		error();
    i2c_start();
	i2c_wait();
	if((TW_STATUS != TW_REP_START) && (TW_STATUS != TW_START))
		error();
	TWDR = hard_addr | TW_READ;
	i2c_send();
	i2c_wait();
	if(TW_STATUS != TW_MR_SLA_ACK)
		error();	
	i2c_send_nack(); // do not change the order
	i2c_wait();
	if(TW_STATUS != TW_MR_DATA_NACK)
		error();
	i2c_stop();		
    temp = TWDR;
    return temp;
}

void at24cxx_write_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t data_to_write)
{
    i2c_start();
	i2c_wait();
	if((TW_STATUS != TW_REP_START) && (TW_STATUS != TW_START))
		error();
	if(ee_type > AT24C16)
    {
		TWDR = hard_addr | TW_WRITE;
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_SLA_ACK)
			error();
		TWDR = write_addr >> 8;
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_DATA_ACK)
			error();
    }
    else
    {
		TWDR = hard_addr | (((write_addr >> 8) & 0x07) << 1);
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_SLA_ACK)
			error();
    }
	TWDR = write_addr % 256;
	i2c_send();
	i2c_wait();
	if(TW_STATUS != TW_MT_DATA_ACK)
		error();
	TWDR = data_to_write;
	i2c_send();
	i2c_wait();
	if(TW_STATUS != TW_MT_DATA_ACK)
		error();
    i2c_stop();
    delay_ms(10); // do not delete/change the delay time
}

void at24cxx_read(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t *buffer, uint16_t number_to_read)
{
	while(number_to_read)
	{
		*buffer++ = at24cxx_read_one_byte(ee_type, hard_addr, read_addr++);	
		number_to_read--;
	}
}

void at24cxx_write(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t *buffer, uint16_t number_to_write)
{
	while(number_to_write--)
	{
		at24cxx_write_one_byte(ee_type, hard_addr, write_addr, *buffer);
		write_addr++;
		buffer++;
	}
}

void at24cxx_write_length_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint32_t data_to_write, uint8_t length)
{  	
	uint8_t i;
	for(i = 0; i < length; i++)
	{
		at24cxx_write_one_byte(ee_type, hard_addr, write_addr + i, (data_to_write >> (8 * i)) & 0xff);
	}												    
}

uint32_t at24cxx_read_length_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t length)
{  	
	uint8_t i;
	uint32_t temp = 0;
	for(i = 0; i < length; i++)
	{
		temp <<= 8;
		temp += at24cxx_read_one_byte(ee_type, hard_addr, read_addr + length - i - 1); 	 				   
	}
	return temp;												    
}

uint8_t at24cxx_check(uint16_t ee_type, uint8_t hard_addr)
{
	uint8_t temp;
	temp = at24cxx_read_one_byte(ee_type, hard_addr, 255);//avoid writing AT24CXX every time start the mcu			   
	if(temp == 0x55)
        return 0;		   
	else
	{
		at24cxx_write_one_byte(ee_type, hard_addr, 255, 0x55);
	    temp = at24cxx_read_one_byte(ee_type, hard_addr, 255);	  
		if(temp == 0x55)
            return 0;
	}
	return 1;											  
}

// sequential write, the max number to write is device depended, some vendor's devices would accept 16/32, 
// but 8 seems to be the lowest common denominator, read the datasheet of the device for a better setting of number_to_write
// if not sure, set number_to_write = 8 for universality
void at24cxx_n_write(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t *buffer, uint16_t number_to_write)
{
	uint8_t i;
    i2c_start();
	i2c_wait();
	if((TW_STATUS != TW_REP_START) && (TW_STATUS != TW_START))
		error();
	if(ee_type > AT24C16)
    {
		TWDR = hard_addr | TW_WRITE;
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_SLA_ACK)
			error();
		TWDR = write_addr >> 8;
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_DATA_ACK)
			error();
    }
    else
    {
		TWDR = hard_addr | (((write_addr >> 8) & 0x07) << 1);
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_SLA_ACK)
			error();
    }
	TWDR = write_addr % 256;
	i2c_send();
	i2c_wait();
	if(TW_STATUS != TW_MT_DATA_ACK)
		error();
	for(i = 0; i < number_to_write; i++)
	{
		TWDR = buffer[i];
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_DATA_ACK)
			error();		
	}	
	i2c_stop();
	delay_ms(10);
}

// sequential read, no max number to read restriction
void at24cxx_n_read(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t *buffer, uint16_t number_to_read)
{
	uint8_t i;
    i2c_start();
	i2c_wait();
	if((TW_STATUS != TW_REP_START) && (TW_STATUS != TW_START))
		error();
    if(ee_type > AT24C16)
    {
		TWDR = hard_addr | TW_WRITE;
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_SLA_ACK)
			error();
		TWDR = read_addr >> 8;
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_DATA_ACK)
			error();
    }
    else
    {
		TWDR = hard_addr | (((read_addr >> 8) & 0x07) << 1);
		i2c_send();
		i2c_wait();
		if(TW_STATUS != TW_MT_SLA_ACK)
			error();
    }
	TWDR = read_addr % 256;
	i2c_send();
	i2c_wait();
	if(TW_STATUS != TW_MT_DATA_ACK)
		error();
    i2c_start();
	i2c_wait();
	if((TW_STATUS != TW_REP_START) && (TW_STATUS != TW_START))
		error();
	TWDR = hard_addr | TW_READ;
	i2c_send();
	i2c_wait();
	if(TW_STATUS != TW_MR_SLA_ACK)
		error();	
	for(i = 0; i < number_to_read-1; i++)	
	{
		i2c_send_ack(); // do not change the order
		i2c_wait();
		if(TW_STATUS != TW_MR_DATA_ACK)
			error();
		buffer[i] = TWDR;
	}
	i2c_send_nack(); // do not change the order
	i2c_wait();
	if(TW_STATUS != TW_MR_DATA_NACK)
		error();
	buffer[i] = TWDR;
	i2c_stop();		 
}

void error(void)
{
    xprintf("error: TWI status %x\n", TW_STATUS);
    exit(0);
}

#endif // HW_I2C_AT24CXX_OPTIONAL

#if HW_I2C_AT24CXX

void at24cxx_init(void)
{
    i2c_init();
}

int at24cxx_check(uint16_t ee_type, uint8_t hard_addr)
{
	uint8_t temp;
	at24cxx_read_one_byte(ee_type, hard_addr, 255, &temp);//avoid writing AT24CXX every time start the mcu			   
	if(temp == 0x55)
        return 0;		   
	else
	{
		temp = 0x55;
		at24cxx_write_one_byte(ee_type, hard_addr, 255, &temp);
	    at24cxx_read_one_byte(ee_type, hard_addr, 255, &temp);	  
		if(temp == 0x55)
            return 0;
	}
	return -1;											  
}

int at24cxx_read_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, uint8_t *buffer)
{
    uint8_t sla, n = 0;
	int res = 0;
	if(ee_type > AT24C16)
		sla = hard_addr;
	else
		sla = hard_addr | (((read_addr >> 8) & 0x07) << 1);
	restart: // note8
	if(n++ > MAX_ITER)
	{
		return -1;
	}
	begin:
    i2c_start();
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_REP_START: // OK, but should not happen
		case TW_START:
			break;
		case TW_MT_ARB_LOST: // note9
			goto begin;
		default:
			return -1; // not in start condition, do not send stop condition
	}
	TWDR = sla | TW_WRITE;
	i2c_send();
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_MT_SLA_ACK:
			break;
		case TW_MT_SLA_NACK: // nack during select: device busy writing
			goto restart; // note11
		case TW_MT_ARB_LOST: // re-arbitrate
			goto begin;
		default:
			goto error; // must send stop condition
	}
	if(ee_type > AT24C16) // for 16-bit word address device
	{
		TWDR = (read_addr >> 8); // send high 8 bits of addr
		i2c_send();
		i2c_wait();
		switch(TW_STATUS)
		{
			case TW_MT_DATA_ACK:
				break;
			case TW_MT_DATA_NACK:
				goto quit;
			case TW_MT_ARB_LOST:
				goto begin;
			default:
				goto error; // must send stop condition
		}
	}
	TWDR = read_addr; // low 8 bits of addr
	i2c_send();
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_MT_DATA_ACK:
			break;
		case TW_MT_DATA_NACK:
			goto quit;
		case TW_MT_ARB_LOST:
			goto begin;
		default:
			goto error;
	}
    i2c_start();
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_START: // OK, but should not happen
		case TW_REP_START:
			break;
		case TW_MT_ARB_LOST:
			goto begin;
		default:
			goto error;
	}	
	TWDR = sla | TW_READ;
	i2c_send();
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_MR_SLA_ACK:
			break;
		case TW_MR_SLA_NACK:
			goto quit;
		case TW_MR_ARB_LOST:
			goto begin;
		default:
			goto error;
	}
	i2c_send_nack(); // do not change the order
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_MR_DATA_NACK:
			break;
		case TW_MR_DATA_ACK:
			goto quit;
		default:
			goto error;
	}
	*buffer = TWDR;
	quit:
	i2c_stop();
	return res;
	error:
	res = -1;
	goto quit;
}

int at24cxx_write_one_byte(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, uint8_t *buffer)
{
	uint8_t sla, n = 0;
	int res = 0;
	if(ee_type > AT24C16)
		sla = hard_addr;
	else
		sla = hard_addr | (((write_addr >> 8) & 0x07) << 1);
	restart:
    if (n++ >= MAX_ITER)
	{
        return -1;
	}
	begin: // note15
    i2c_start();
	i2c_wait();
    switch(TW_STATUS)
    {
        case TW_REP_START:		// OK, but should not happen 
        case TW_START:
            break;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            return -1;		// not in start condition 
    }
    TWDR = sla | TW_WRITE;
    i2c_send();
    i2c_wait();
    switch(TW_STATUS)
    {
        case TW_MT_SLA_ACK:
            break;
        case TW_MT_SLA_NACK:	// nack during select: device busy writing 
            goto restart;
        case TW_MT_ARB_LOST:	// re-arbitrate 
            goto begin;
        default:
            goto error;		// must send stop condition 
    }
    if(ee_type > AT24C16)
	{
        TWDR = (write_addr >> 8); // 16-bit word address device, send high 8 bits of addr
        i2c_send(); // send 
        i2c_wait(); // wait for transmission
        switch(TW_STATUS)
        {
            case TW_MT_DATA_ACK:
                break;
            case TW_MT_DATA_NACK:
                goto quit;
            case TW_MT_ARB_LOST:
                goto begin;
            default:
                goto error; // must send stop condition
        }
	}
	TWDR = write_addr; // low 8 bits of addr 
    i2c_send(); 
    i2c_wait();
    switch(TW_STATUS)
    {
        case TW_MT_DATA_ACK:
            break;
        case TW_MT_DATA_NACK:
            goto quit;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            goto error;		
    }
	TWDR = *buffer;
	i2c_send();
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_MT_DATA_ACK:
			break;
		case TW_MT_DATA_NACK:
			goto error; // device write protected, note16
		default:
			goto error;
	}
	quit:
	i2c_stop();
	return res;
	error:
	res = -1;
	goto quit;
}

static int at24cxx_page_write(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, int len, uint8_t *buffer)
{
    uint8_t sla, n = 0;
    int res = 0;
    uint16_t end_addr;
    if (write_addr + len <= (write_addr | (PAGE_SIZE - 1)))
        end_addr = write_addr + len;
    else
        end_addr = (write_addr | (PAGE_SIZE - 1)) + 1;
    len = end_addr - write_addr;
	if(ee_type > AT24C16)
		sla = hard_addr; // 16-bit address devices need only TWI Device Address
	else 
		sla = hard_addr | (((write_addr >> 8) & 0x07) << 1);
    restart:
    if (n++ >= MAX_ITER)
	{
        return -1;
	}
    begin: // note15
    i2c_start();
    i2c_wait(); 
    switch(TW_STATUS)
    {
        case TW_REP_START:		// OK, but should not happen 
        case TW_START:
            break;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            return -1;		// not in start condition 
    }
    TWDR = sla | TW_WRITE;
    i2c_send();
    i2c_wait();
    switch(TW_STATUS)
    {
        case TW_MT_SLA_ACK:
            break;
        case TW_MT_SLA_NACK:	// nack during select: device busy writing 
            goto restart;
        case TW_MT_ARB_LOST:	// re-arbitrate 
            goto begin;
        default:
            goto error;		// must send stop condition 
    }
    if(ee_type > AT24C16)
	{
        TWDR = (write_addr >> 8); // 16-bit word address device, send high 8 bits of addr
        i2c_send(); 
        i2c_wait(); 
        switch(TW_STATUS)
        {
            case TW_MT_DATA_ACK:
                break;
            case TW_MT_DATA_NACK:
                goto quit;
            case TW_MT_ARB_LOST:
                goto begin;
            default:
                goto error; // must send stop condition
        }
	}
    TWDR = write_addr;		// low 8 bits of addr 
    i2c_send(); 
    i2c_wait();
    switch(TW_STATUS)
    {
        case TW_MT_DATA_ACK:
            break;
        case TW_MT_DATA_NACK:
            goto quit;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            goto error;		
    }
    for(; len > 0; len--)
    {
        TWDR = *buffer;
		buffer++;
        i2c_send(); 
        i2c_wait(); 
        switch(TW_STATUS)
	    {
	        case TW_MT_DATA_NACK:
	            goto error;		// device write protected, note16
            case TW_MT_DATA_ACK:
                res++;
	            break;
	        default:
	            goto error;
	    }
    }
    quit:
    i2c_stop(); // send stop condition 
    return res;
    error:
    res = -1;
    goto quit;
}

int at24cxx_n_write(uint16_t ee_type, uint8_t hard_addr, uint16_t write_addr, int len, uint8_t *buffer)
{
    int res, total = 0;
    do
    {
        res = at24cxx_page_write(ee_type, hard_addr, write_addr, len, buffer);
        if (res == -1)
	        return -1;
        write_addr += res;
        len -= res;
        buffer += res;
        total += res;
    }while (len > 0);
    return total;
}

// sequential read, no max number to read restriction
int at24cxx_n_read(uint16_t ee_type, uint8_t hard_addr, uint16_t read_addr, int len, uint8_t *buffer)
{
	uint8_t sla, n = 0;
	int res = 0;
	if(ee_type > AT24C16)
		sla = hard_addr; // 16-bit address devices need only TWI Device Address
	else
		sla = hard_addr | (((read_addr >> 8) & 0x07) << 1);
	restart: // Note[8]
	if(n++ >= MAX_ITER)
		return -1;
	begin:
	i2c_start();
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_REP_START: // OK, but should not happen
		case TW_START:
			break;
		case TW_MT_ARB_LOST: // Note[9]
			goto begin;
		default:
			return -1; // not in start condition, do not send stop condition
	}
	TWDR = sla | TW_WRITE; // Note[10]
	i2c_send();
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_MT_SLA_ACK:
			break;
		case TW_MT_SLA_NACK: // nack during select: device busy writing
			goto restart; // Note[11]
		case TW_MT_ARB_LOST: // re-arbitrate
			goto begin;
		default:
			goto error; // must send stop condition
	}
	if(ee_type > AT24C16)
	{
		TWDR = (read_addr >> 8); // 16-bit word address device, send high 8 bits of addr
		i2c_send();
		i2c_wait();
		switch(TW_STATUS)
		{
			case TW_MT_DATA_ACK:
				break;
			case TW_MT_DATA_NACK:
				goto quit;
			case TW_MT_ARB_LOST:
				goto begin;
			default:
				goto error; // must send stop condition
		}
	}
	TWDR = read_addr; // low 8 bits of addr
	i2c_send();
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_MT_DATA_ACK:
			break;
		case TW_MT_DATA_NACK:
			goto quit;
		case TW_MT_ARB_LOST:
			goto begin;
		default:
			goto error;
	}
	i2c_start();
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_START: // OK, but should not happen
		case TW_REP_START:
			break;
		case TW_MT_ARB_LOST:
			goto begin;
		default:
			goto error;
	}
	TWDR = sla | TW_READ;
	i2c_send();
	i2c_wait();
	switch(TW_STATUS)
	{
		case TW_MR_SLA_ACK:
			break;
		case TW_MR_SLA_NACK:
			goto quit;
		case TW_MR_ARB_LOST:
			goto begin;
		default:
			goto error;
	} // after send (sla | TW_READ) to slave, and the status is TW_MR_SLA_ACK, then the TWDR contains the data from slave
	for(; len > 0; len--) // Note[13]
	{
		if(len == 1)
			i2c_send_nack(); // master send nack
		else
			i2c_send_ack(); // master send ack
		i2c_wait();
		switch(TW_STATUS)
		{
			case TW_MR_DATA_NACK:
				len = 0; // force end of loop
			case TW_MR_DATA_ACK:
				*buffer++ = TWDR;
				res++;
				if(TW_STATUS == TW_MR_DATA_NACK)
					goto quit;
				break;
			default:
				goto error;
		}
	}
	quit: // Note[14]
	i2c_stop();
	return res;
	error:
	res = -1;
	goto quit;
}

#endif // HW_I2C_AT24CXX



