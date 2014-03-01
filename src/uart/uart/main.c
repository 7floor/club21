/*
 * uart.c
 *
 * Created: 01.03.2014 22:36:37
 *  Author: Dmitry
 */ 

#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
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

int main(void)
{
	uart_init((UART_BAUD_SELECT(BAUD_RATE, F_CPU)));

	sei();

	uart_puts_P("Hello World\r\n");

	uint8_t c;
    while(1)
    {
		uart_put_int(c++);
		uart_newline();
    }
}