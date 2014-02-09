/*
* stepper.c
*
* Created: 09.02.2014 18:01:01
*  Author: Dmitry
*/

#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#define COIL_A1 PINB0
#define COIL_A2 PINB1
#define COIL_B1 PINB2
#define COIL_B2 PINB3

#define COIL_AN ((0 << COIL_A1) | (0 << COIL_A2))
#define COIL_AF ((1 << COIL_A1) | (0 << COIL_A2))
#define COIL_AB ((0 << COIL_A1) | (1 << COIL_A2))

#define COIL_BN ((0 << COIL_B1) | (0 << COIL_B2))
#define COIL_BF ((1 << COIL_B1) | (0 << COIL_B2))
#define COIL_BB ((0 << COIL_B1) | (1 << COIL_B2))

unsigned char steps[4] =
{
	COIL_AF | COIL_BF,
	COIL_AB | COIL_BF,
	COIL_AB | COIL_BB,
	COIL_AF | COIL_BB
};

unsigned char hsteps[8] =
{
	COIL_AF | COIL_BN,
	COIL_AF | COIL_BF,
	
	COIL_AN | COIL_BF,
	COIL_AB | COIL_BF,
	
	COIL_AB | COIL_BN,
	COIL_AB | COIL_BB,
	
	COIL_AN | COIL_BB,
	COIL_AF | COIL_BB
};

unsigned char stepno = 0;
volatile unsigned char mode = 0;

void step(bool forward)
{
	stepno = (stepno + (forward ? 1 : 3)) & 0x03;
	PORTB = steps[stepno];
}

void hstep(bool forward)
{
	stepno = (stepno + (forward ? 1 : 7)) & 0x07;
	PORTB = hsteps[stepno];
}

ISR(INT0_vect)
{
	_delay_ms(50);
	if (PIND & (1 << PIND2)) return; // we want the button still low after debounce delay
	
	mode++;
	if (mode > 4) mode = 0;
}

void setup()
{
	DDRB = (1 << COIL_A1) | (1 << COIL_A2) | (1 << COIL_B1) | (1 << COIL_B2);
	
	DDRD = 0;
	PORTD = 0xff;
	
	MCUCR |= (1 << ISC01);
	GIMSK |= (1 << INT0);
	sei();
}

// hstep starts from 2ms
// step starts from 3ms (hardly), 4ms (easily)
int main(void)
{
	setup();
	while(1)
	{
		while (mode == 0);
		
		switch(mode)
		{
			case 1:
			step(true);
			break;
			
			case 2:
			step(false);
			break;
			
			case 3:
			hstep(true);
			break;
			
			case 4:
			hstep(false);
			break;
		}
		_delay_ms(20);
	}
}