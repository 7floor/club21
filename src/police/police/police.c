/*
* police.c
*
* Created: 13.02.2014 20:47:41
*  Author: Dmitry
*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define T0DIV ((0 << CS02) | (1 << CS01) | (0 << CS00))
#define T1DIV ((0 << CS12) | (0 << CS11) | (1 << CS10))

bool dir;

ISR (TIMER0_OVF_vect)
{
	uint8_t val = OCR0A;
	
	if (dir)
	{
		val++;
		if (val == 255) dir = !dir;
	}
	else
	{
		val--;
		if (val == 0) dir = !dir;
	}
	
	OCR0A = val;
	OCR0B = (val < 128 ? val : (255 - val)) << 1;
	
	uint16_t snd = 400 + val;

	OCR1A = snd;
	if (TCNT1 >= snd - 8) TCNT1 = 0;

	//TCCR1B ^= T1DIV;
	//OCR1A = snd;
	//if (TCNT1 >= snd) TCNT1 = 0;
	//TCCR1B ^= T1DIV;
}

void setup()
{
	DDRB = (1 << PINB2) | (1 << PINB3);
	DDRD = (1 << PIND5);
	
	// timer 0
	// fast PWM
	// OC0A & OC0B enabled
	// IOclk/8 = 1/8 of 1MHz; 125000Hz; PWM = 125000/256 = ~488Hz; LED period will be 488/256 = ~1.9Hz
	TCCR0A = (1 << COM0A1) | (0 << COM0A0) | (1 << COM0B1) | (0 << COM0B0) | (1 << WGM00) | (1 << WGM01);
	TCCR0B = (0 << WGM02) | T0DIV; 
	
	// timer 1
	// CTC to OC1A
	// OC1A enabled, flip on overflow
	// IOclk = 1MHz; 400 .. 400+255=655; (*2 = 800..1310) 1MHz / (800..1310) = 1250..763Hz
	TCCR1A = (0 << COM1A1) | (1 << COM1A0) | (0 << WGM11) | (0 << WGM10);
	TCCR1B = (0 << WGM13) | (1 << WGM12) | T1DIV; 
	
	TIMSK = (1 << TOIE0);

	sei();
}

int main(void)
{
	setup();
	while(1);
}