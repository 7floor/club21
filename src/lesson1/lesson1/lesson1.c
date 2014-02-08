/*
 * lesson1.c
 *
 * Created: 01.02.2014 16:31:24
 *  Author: Dmitry
 */ 

#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile unsigned char flag;

ISR(INT0_vect)
{
	flag ^= 1;
	_delay_ms(20);
}

int main(void)
{
	DDRB = (1 << PINB0) | (1 << PINB1);
	PORTB = 0xff ^ (1 << PINB0);
	
	DDRD = 0;
	PORTD = 0xff;
	
	MCUCR |= (1 << ISC01);
	GIMSK |= (1 << INT0);
	sei();
	
    while(1)
    {
		while (flag);
		
		_delay_ms(250);
		PORTB ^= (1 << PINB0) | (1 << PINB1);
    }
}