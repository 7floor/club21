#ifndef USART_PRINTF_H
#define USART_PRINTF_H

#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>

int usart_send_byte(char u8Data, FILE *stream)
{
	if(u8Data == '\n')
	{
		usart_send_byte('\r', 0);
	}
	//wait while previous byte is completed
	while(!(UCSRA&(1<<UDRE))){};
	// Transmit data
	UDR = u8Data;
	return 0;
}

int usart_receive_byte(FILE *stream)
{
	uint8_t u8Data;
	// Wait for byte to be received
	while(!(UCSRA&(1<<RXC))){};
	u8Data=UDR;
	//echo input data
	usart_send_byte(u8Data,stream);
	// Return received data
	return u8Data;
}

static FILE usart_stream = FDEV_SETUP_STREAM(usart_send_byte, usart_receive_byte, _FDEV_SETUP_RW);

void usart_init( uint16_t ubrr) {
	// Set baud rate
	UBRRH = (uint8_t)(ubrr>>8);
	UBRRL = (uint8_t)ubrr;
	// Enable receiver and transmitter
	UCSRB = (1<<RXEN)|(1<<TXEN);
	// Set frame format: 8data, 1stop bit
	#ifdef URSEL
	UCSRC = (1<<URSEL)|(3<<UCSZ0);
	#else
	UCSRC = (3<<UCSZ0);
	#endif
	// link to standard streams
	stdin = stdout = &usart_stream;
}


#endif // USART_PRINTF_H
