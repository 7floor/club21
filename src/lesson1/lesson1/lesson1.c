/*
 * lesson1.c
 *
 * Created: 01.02.2014 16:31:24
 *  Author: Dmitry
 */ 


// for _delay_ms() to work correctly
#define F_CPU 8000000

#include <avr/io.h>

// for _delay_ms()
#include <util/delay.h>

// for ISR(INT0_vect)
#include <avr/interrupt.h>

// volatile is needed so that compiler will not optimize the access to it
volatile unsigned char flag;

// int0 handler
ISR(INT0_vect)
{
	// flip flag
	flag ^= 1;
	
	// wait long enough for button contacts stabilize (de-bounce)
	_delay_ms(20);
}

int main(void)
{
	// setup port b
	// pin b0 and b1 are outputs, others are inputs
	DDRB = (1 << PINB0) | (1 << PINB1);
	// inputs have pull-ups enabled, b1 output is high, b0 output is low
	PORTB = 0xff ^ (1 << PINB0);
	
	// setup port d
	// all pins are inputs
	DDRD = 0;
	// all inputs have pull-ups enabled
	PORTD = 0xff;
	
	// setup int0 to trigger on high-to-low transition
	MCUCR |= (1 << ISC01);
	// enable int0 interrupt
	GIMSK |= (1 << INT0);
	// globally enable interrupts
	sei();
	
    while(1)
    {
		// wait until flag is set
		while (flag);
		
		_delay_ms(250);
		
		// flip both b0 and b1 pins
		PORTB ^= (1 << PINB0) | (1 << PINB1);
    }
}
