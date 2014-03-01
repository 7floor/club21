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

// IO config
#define IODDR DDRB
#define IOPORT PORTB
#define IOPIN PINB
#define BUTTON_PIN PB0
#define LED_ON_PRESS_PIN PB1
#define LED_WHEN_PRESSED_PIN PB2
#define LED_WHEN_MIDDLE_PIN PB4
#define LED_WHEN_LONG_PIN PB5

// timer 0 to give 1ms ticks
#define TIMER_0_PRESCALER_64 ((0 << CS02) | (1 << CS01) | (1 << CS00))
#define TCNT0_TICKS_PER_MS (F_CPU / 64 / 1000)

// button, numbers are milliseconds
#define BUTTON_SCAN_PERIOD 50
#define BUTTON_DURATION_LONG (2000 / BUTTON_SCAN_PERIOD)
#define BUTTON_DURATION_LONGER (5000 / BUTTON_SCAN_PERIOD)
#define BUTTON_AUTOREPEAT_LONG (250 / BUTTON_SCAN_PERIOD)
#define BUTTON_AUTOREPEAT_LONGER (100 / BUTTON_SCAN_PERIOD)

#define ENABLE_BUTTON_PRESSED_LONGER 1
#define ENABLE_BUTTON_AUTO_REPEAT 1

#if ENABLE_BUTTON_PRESSED_LONGER
	#define BUTTON_DURATION_MAX BUTTON_DURATION_LONGER
#else
	#define BUTTON_DURATION_MAX BUTTON_DURATION_LONG
#endif

#if ENABLE_BUTTON_AUTO_REPEAT
	#if ENABLE_BUTTON_PRESSED_LONGER
	#endif
#endif

enum buttonStateType 
{
	RELEASED
	,PRESSED
	,PRESSED_LONG
#if ENABLE_BUTTON_PRESSED_LONGER
	,PRESSED_LONGER
#endif
};
	
bool isButtonPressed();
void buttonTimerHandler();
void onButtonStateChanged(buttonStateType buttonState);

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
	static buttonStateType lastState = RELEASED;
	static uint8_t pressedDuration;
#if ENABLE_BUTTON_AUTO_REPEAT
	static uint8_t autoRepeatDuration;
#endif	
	buttonStateType state = lastState;
	bool pressed = isButtonPressed();
	
	if (lastPressed != pressed)
	{
		lastPressed = pressed;
		pressedDuration = 0;
#if ENABLE_BUTTON_AUTO_REPEAT
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
	
	if (state != RELEASED)
	{
		if (pressedDuration < BUTTON_DURATION_MAX) pressedDuration++;
		
#if ENABLE_BUTTON_AUTO_REPEAT
		
		if (autoRepeatDuration)
		{
			autoRepeatDuration--;
			if (!autoRepeatDuration)
			{
				lastState = RELEASED;
			}
		}
		else
		{
			switch (state)
			{
				case PRESSED_LONG:
				autoRepeatDuration = BUTTON_AUTOREPEAT_LONG;
				break;
#if ENABLE_BUTTON_PRESSED_LONGER				
				case PRESSED_LONGER:
				autoRepeatDuration = BUTTON_AUTOREPEAT_LONGER;
				break;
#endif				
				default:
				break;
			}
		}
		
#endif
		
		switch (pressedDuration)
		{
			case BUTTON_DURATION_LONG:
			state = PRESSED_LONG;
			break;
#if ENABLE_BUTTON_PRESSED_LONGER
			case BUTTON_DURATION_LONGER:
			state = PRESSED_LONGER;
			break;
#endif
		}
		
	}
	
	if (lastState != state)
	{
		lastState = state;
		onButtonStateChanged(state);
	}
}

void onButtonStateChanged(buttonStateType buttonState)
{
	if (buttonState != RELEASED)	
	{
		IOPORT ^= (1 << LED_ON_PRESS_PIN); // toggle it
	}
			
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
#if ENABLE_BUTTON_PRESSED_LONGER
		case PRESSED_LONGER:
		IOPORT &= ~(1 << LED_WHEN_LONG_PIN);
		break;
#endif
	}
}

int main(void)
{
	// IO Port config
	IODDR = (1 << LED_ON_PRESS_PIN) | (1 << LED_WHEN_PRESSED_PIN) | (1 << LED_WHEN_MIDDLE_PIN) | (1 << LED_WHEN_LONG_PIN); // LEDs to output
	IOPORT = (1 << BUTTON_PIN) | (1 << LED_ON_PRESS_PIN)  | (1 << LED_WHEN_PRESSED_PIN) | (1 << LED_WHEN_MIDDLE_PIN) | (1 << LED_WHEN_LONG_PIN); // Button with pull-up, and turn leds off
	// TIMER 0
	TCCR0B = TIMER_0_PRESCALER_64;
	TIMSK = (1 << TOIE0); // int on overflow
	
	sei();
	
	while (1)
	{
	}
}