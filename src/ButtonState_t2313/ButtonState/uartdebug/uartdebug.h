/*
 * uartdebug.h
 *
 * Created: 01.03.2014 23:48:01
 *  Author: Dmitry
 */ 


#ifndef UARTDEBUG_H_
#define UARTDEBUG_H_

#ifdef DEBUG
#define UART_RX0_BUFFER_SIZE 0
#define UART_TX0_BUFFER_SIZE 16

#include "uart.c"

#include <avr/pgmspace.h>
#include <stdlib.h>

#define BAUD_RATE 9600

const char PROGMEM newline[] = "\r\n";
#define uart_newline() uart_puts_p(newline)

void uart_put_int( const int val )
{
	char buffer[10];
	uart_puts( itoa( val, buffer, 10 ) );
}

#define uartdebug_init()	uart_init((UART_BAUD_SELECT(BAUD_RATE, F_CPU)))
#define debug_newline()		uart_newline()
#define debug_out_int(a)	uart_put_int(a)
#define debug_out_str(a)	uart_puts(a)
#define debug_out_str_p(a)	uart_puts_p(a)
#define debug_out_str_P(a)	uart_puts_P(a)

#else

#define uartdebug_init()
#define debug_newline()
#define debug_out_int(a)
#define debug_out_str(a)
#define debug_out_str_p(a)
#define debug_out_str_P(a)

#endif

#endif /* UARTDEBUG_H_ */