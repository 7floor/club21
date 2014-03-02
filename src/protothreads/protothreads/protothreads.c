#define F_CPU 8000000

#include <avr/io.h>
#include "systimer.h"
#include "usart_printf.h"
#include "pt.h"

#define LEDPORT PORTB
#define LEDDDR DDRB
#define LED PB1

#define BEEPPORT PORTD
#define BEEPDDR DDRD
#define BEEP PD4

#define BUTTONPIN PINB
#define BUTTONPORT PORTB
#define BUTTONDDR DDRB
#define BUTTON PB0

void led_on()
{
	LEDPORT &= ~(1 << LED);
}

void led_off()
{
	LEDPORT |= 1 << LED;
}

void beep_on()
{
	BEEPPORT |= 1 << BEEP;
}

void beep_off()
{
	BEEPPORT &= ~(1 << BEEP);
}

bool is_button_down()
{
	return !(BUTTONPIN & (1 << BUTTON));
}

static struct pt pt_blinker, pt_beeper, pt_writer, pt_reader;
bool allow_work;

PT_THREAD(blinker(struct pt *pt))
{
	static timer t;
	
	PT_BEGIN(pt);
	
	while(1)
	{
		PT_WAIT_UNTIL(pt, allow_work);
		led_on();
		timer_set(&t, 100);
		PT_WAIT_UNTIL(pt, timer_expired(&t));
		led_off();
		timer_set(&t, 100);
		PT_WAIT_UNTIL(pt, timer_expired(&t));
	}
	
	PT_END(pt);
}

PT_THREAD(beeper(struct pt *pt))
{
	static timer t;
	
	PT_BEGIN(pt);
	
	while(1)
	{
		PT_WAIT_UNTIL(pt, allow_work);
		beep_on();
		timer_set(&t, 50);
		PT_WAIT_UNTIL(pt, timer_expired(&t));
		beep_off();
		timer_set(&t, 950);
		PT_WAIT_UNTIL(pt, timer_expired(&t));
	}
	
	PT_END(pt);
}

PT_THREAD(writer(struct pt *pt))
{
	static timer t;
	
	PT_BEGIN(pt);
	
	while(1)
	{
		PT_WAIT_UNTIL(pt, allow_work);
		printf("Hello\n");
		timer_set(&t, 250);
		PT_WAIT_UNTIL(pt, timer_expired(&t));
		printf("World!\n");
		timer_set(&t, 250);
		PT_WAIT_UNTIL(pt, timer_expired(&t));
	}
	
	PT_END(pt);
}

PT_THREAD(reader(struct pt *pt))
{
	static timer t;
	static bool state;
	PT_BEGIN(pt);
	
	while(1)
	{
		timer_set(&t, 25);
		PT_WAIT_UNTIL(pt, timer_expired(&t));
		
		bool new_state = is_button_down();
		if (state != new_state)
		{
			state = new_state;
			if (state)
			{
				allow_work = !allow_work;
			}				
		}
	}
	
	PT_END(pt);
}

void init_io()
{
	LEDDDR |= 1 << LED;
	BEEPDDR |= 1 << BEEP;
	BUTTONDDR &= ~(1 << BUTTON);
	BUTTONPORT |= (1 << BUTTON); // pull-up
	
	led_off();
	beep_off();
}

int main(void)
{
	init_io();
	
	init_systimer();

	#define BAUD 9600
	#define UBRR_VALUE F_CPU/16/BAUD-1
	usart_init(UBRR_VALUE);
	
	sei();
	
	PT_INIT(&pt_blinker);
	PT_INIT(&pt_beeper);
	PT_INIT(&pt_writer);
	PT_INIT(&pt_reader);
  
	while(1)
	{
		blinker(&pt_blinker);
		beeper(&pt_beeper);
		writer(&pt_writer);
		reader(&pt_reader);
	}
}
