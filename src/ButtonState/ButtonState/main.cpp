/*
 * ButtonState.cpp
 *
 * Created: 2/22/2014 6:10:09 PM
 *  Author: Alexander_Puzynia
 */ 


#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "uartdebug/uartdebug.h"

// IO config
#define IODDR DDRB
#define IOPORT PORTB
#define IOPIN PINB
#define BUTTON_PIN PB0
#define LED_ON_PRESS_PIN PB1
#define LED_WHEN_PRESSED_PIN PB2
#define LED_WHEN_MIDDLE_PIN PB4
#define LED_WHEN_LONG_PIN PB5

// timer 0 to give 1ms ticks, prescaler = 64 @ 8MHz
#define TIMER_0_PRESCALER ((0 << CS02) | (1 << CS01) | (1 << CS00))
#define TCNT0_TICKS_PER_MS (F_CPU / 64 / 1000)

// button, numbers are milliseconds
#define BUTTON_SCAN_PERIOD 25

#define BUTTON_DURATION1 2000
#define BUTTON_AUTOREPEAT1 250

#define BUTTON_DURATION2 5000
#define BUTTON_AUTOREPEAT2 100

// library-like defaults and redefines
#ifndef BUTTON_SCAN_PERIOD
	#define BUTTON_SCAN_PERIOD 50
#endif

#ifndef BUTTON_DURATION1
	#define BUTTON_DURATION1 2000
#endif

#define BUTTON_DURATION1_SCANS ((BUTTON_DURATION1) / (BUTTON_SCAN_PERIOD))
#if BUTTON_DURATION1_SCANS > 255
	#error BUTTON_DURATION1 too large or BUTTON_SCAN_PERIOD too small
#endif
#ifdef BUTTON_AUTOREPEAT1
	#define ENABLE_BUTTON_AUTOREPEAT
	#define BUTTON_AUTOREPEAT1_SCANS ((BUTTON_AUTOREPEAT1) / (BUTTON_SCAN_PERIOD))
	#if BUTTON_AUTOREPEAT1_SCANS > 255
		#error BUTTON_AUTOREPEAT1 too large or BUTTON_SCAN_PERIOD too small
	#endif
#else
	#undef ENABLE_BUTTON_AUTOREPEAT
#endif

#ifdef BUTTON_DURATION2
	#if BUTTON_DURATION2 < BUTTON_DURATION1 + BUTTON_SCAN_PERIOD
		#error BUTTON_DURATION2, if defined, should be greater than BUTTON_DURATION1 + BUTTON_SCAN_PERIOD
	#endif
	#define ENABLE_BUTTON_DURATION2
	#define BUTTON_DURATION2_SCANS ((BUTTON_DURATION2) / (BUTTON_SCAN_PERIOD))
	#if BUTTON_DURATION2_SCANS > 255
		#error BUTTON_DURATION2 too large or BUTTON_SCAN_PERIOD too small
	#endif
	#ifdef ENABLE_BUTTON_AUTOREPEAT
		#ifdef BUTTON_AUTOREPEAT2
			#define BUTTON_AUTOREPEAT2_SCANS ((BUTTON_AUTOREPEAT2) / (BUTTON_SCAN_PERIOD))
			#if BUTTON_AUTOREPEAT2_SCANS > 255
				#error BUTTON_AUTOREPEAT2 too large or BUTTON_SCAN_PERIOD too small
			#endif
		#else
			#define BUTTON_AUTOREPEAT2_SCANS BUTTON_AUTOREPEAT1_SCANS
		#endif
	#endif
#else
	#undef ENABLE_BUTTON_DURATION2
#endif

// declarations

enum button_state_t 
{
	RELEASED
	,PRESSED
	,PRESSED_LONG
#ifdef ENABLE_BUTTON_DURATION2
	,PRESSED_LONGER
#endif
};
	
bool isButtonPressed();
void buttonTimerHandler();
#ifdef ENABLE_BUTTON_AUTOREPEAT
void onButtonStateChanged(button_state_t buttonState, bool repeated);
#else
void onButtonStateChanged(button_state_t buttonState);
#endif

// implementations

ISR(TIMER0_OVF_vect)
{
	TCNT0 -= TCNT0_TICKS_PER_MS;

	buttonTimerHandler();
}

bool isButtonPressed()
{
	return !(IOPIN & (1 << BUTTON_PIN));
}

void buttonTimerHandler()
{
	static uint8_t buttonTimeout = BUTTON_SCAN_PERIOD;

	if (--buttonTimeout) return;

	buttonTimeout = BUTTON_SCAN_PERIOD;
	
	static bool lastPressed = false;
	static button_state_t lastState = RELEASED;
	static uint8_t pressedDuration;
#ifdef ENABLE_BUTTON_AUTOREPEAT
	static uint8_t autoRepeatDuration;
	bool repeated = false;
#endif

	button_state_t state = lastState;
	bool pressed = isButtonPressed();
	
	if (lastPressed != pressed)
	{
		lastPressed = pressed;
		pressedDuration = 0;
#ifdef ENABLE_BUTTON_AUTOREPEAT
		autoRepeatDuration = 0;
#endif
		if (pressed)
		{
			state = PRESSED;
		}
		else
		{
			state = RELEASED;
		}
	}
	else if (state != RELEASED)
	{
		if (pressedDuration < 0xff) pressedDuration++;

#ifdef ENABLE_BUTTON_AUTOREPEAT
		
		if (autoRepeatDuration)
		{
			autoRepeatDuration--;
			if (!autoRepeatDuration)
			{
				repeated = true;
			}
		}
		else
		{
			switch (state)
			{
				case PRESSED_LONG:
				autoRepeatDuration = BUTTON_AUTOREPEAT1_SCANS - 1;
				break;
#ifdef ENABLE_BUTTON_DURATION2				
				case PRESSED_LONGER:
				autoRepeatDuration = BUTTON_AUTOREPEAT2_SCANS - 1;
				break;
#endif			
				default:
				break;
			}
		}
		
#endif
		
		switch (pressedDuration)
		{
			case BUTTON_DURATION1_SCANS:
			state = PRESSED_LONG;
#ifdef ENABLE_BUTTON_AUTOREPEAT
			repeated = false;
#endif
			break;
#ifdef ENABLE_BUTTON_DURATION2
			case BUTTON_DURATION2_SCANS:
			state = PRESSED_LONGER;
#ifdef ENABLE_BUTTON_AUTOREPEAT
			repeated = false;
#endif
			break;
#endif
		}
	}
	
	if (lastState != state
#ifdef ENABLE_BUTTON_AUTOREPEAT
		|| repeated
#endif
	   )
	{
		lastState = state;
#ifdef ENABLE_BUTTON_AUTOREPEAT
		onButtonStateChanged(state, repeated);
#else
		onButtonStateChanged(state);
#endif
		#ifdef DEBUG
		NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE)
		{
			debug_out_int(pressedDuration);
			debug_out_str_P(", state: ");
			debug_out_int(state);
#ifdef ENABLE_BUTTON_AUTOREPEAT
			if (repeated) debug_out_str_P(", repeated");
#endif
			debug_newline();
		}
		#endif
	}
}

#ifdef ENABLE_BUTTON_AUTOREPEAT
void onButtonStateChanged(button_state_t buttonState, bool repeated)
#else
void onButtonStateChanged(button_state_t buttonState)
#endif
{
	if (buttonState != RELEASED)	
	{
		IOPORT ^= (1 << LED_ON_PRESS_PIN); // toggle it
	}
	
#ifdef ENABLE_BUTTON_AUTOREPEAT
	if (repeated) return;
#endif			
	switch (buttonState)
	{
		case RELEASED:
		IOPORT |= (1 << LED_WHEN_PRESSED_PIN) |(1 << LED_WHEN_MIDDLE_PIN) | (1 << LED_WHEN_LONG_PIN);  
		break;

		case PRESSED:
		IOPORT &= ~(1 << LED_WHEN_PRESSED_PIN);
		break;

		case PRESSED_LONG:
		IOPORT &= ~(1 << LED_WHEN_MIDDLE_PIN);
		break;
#ifdef ENABLE_BUTTON_DURATION2
		case PRESSED_LONGER:
		IOPORT &= ~(1 << LED_WHEN_LONG_PIN);
		break;
#endif
	}
}

int main(void)
{
	uartdebug_init();
	
	// IO Port config
	IODDR = (1 << LED_ON_PRESS_PIN) | (1 << LED_WHEN_PRESSED_PIN) | (1 << LED_WHEN_MIDDLE_PIN) | (1 << LED_WHEN_LONG_PIN); // LEDs to output
	IOPORT = (1 << BUTTON_PIN) | (1 << LED_ON_PRESS_PIN)  | (1 << LED_WHEN_PRESSED_PIN) | (1 << LED_WHEN_MIDDLE_PIN) | (1 << LED_WHEN_LONG_PIN); // Button with pull-up, and turn leds off
	// TIMER 0
	TCCR0B = TIMER_0_PRESCALER;
	TIMSK = (1 << TOIE0); // int on overflow
	
	sei();
	
	while (1)
	{
	}
}