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

#define BUTTON_TIMEOUT 50
#define SLOW_BLINK_TIMEOUT 250
#define FAST_BLINK_TIMEOUT 100

#define MIDDLE_PRESS_DURATION 40
#define LONG_PRESS_DURATION 100
#define MAX_DURATION LONG_PRESS_DURATION

#define LED_PORT (PORTD)
#define LED_PIN (PD7)

enum buttonStateType {
	UNPRESSED,
	PRESSED,
	MIDDLE_PRESSED,
	LONG_PRESSED
	};
	
enum blinkStateType {
	NONE,
	SLOW,
	FAST
	};

volatile uint16_t milliseconds;
volatile uint16_t firstOccur;
uint16_t pressTimer;
volatile bool pressed;
volatile uint8_t buttonTimeout = BUTTON_TIMEOUT;
volatile uint8_t ledBlinkTimeout = 0;
volatile uint8_t buttonPressDuration = 0;
volatile buttonStateType buttonState = UNPRESSED;
volatile blinkStateType blinkState = NONE;

uint16_t millis();

void buttonTimerHandler();

void ledBlinkTimerHandler();

bool isButtonPressed();

void updateLedBlinkTimeout();

void onButtonStateChanged(buttonStateType buttonState);

void onBlinkStateChanged(blinkStateType blinkState);

ISR(TIMER0_OVF_vect)
{
	TCNT0 += 130;
	milliseconds++;
	buttonTimeout--;
	if (buttonTimeout == 0)
	{
		buttonTimeout = BUTTON_TIMEOUT;
		buttonTimerHandler();
	}
	if (blinkState != NONE)
	{
		ledBlinkTimeout--;
		if (ledBlinkTimeout == 0)
		{
			ledBlinkTimerHandler();
			updateLedBlinkTimeout();
		}
	}
}

void ledBlinkTimerHandler()
{
	LED_PORT ^= (1 << LED_PIN);
}

void onBlinkStateChanged(blinkStateType blinkState)
{
	updateLedBlinkTimeout();
}

void updateLedBlinkTimeout()
{
	switch (blinkState)
	{
		case SLOW:
		ledBlinkTimeout = SLOW_BLINK_TIMEOUT;
		break;
		case FAST:
		ledBlinkTimeout = FAST_BLINK_TIMEOUT;
		break;
		case NONE:
		ledBlinkTimeout = 0;
		break;
	}
}

void buttonTimerHandler()
{
	bool newState = isButtonPressed();
	buttonStateType oldButtonState = buttonState;
	
	if (!pressed && newState)
	{
		buttonState = PRESSED;
		buttonPressDuration = 0;
	}
	if (pressed && !newState)
	{
		buttonState = UNPRESSED;
	}
	pressed = newState;
	
	if (buttonState != UNPRESSED && buttonPressDuration <= MAX_DURATION)
	{
		buttonPressDuration++;
		if (buttonPressDuration >= MIDDLE_PRESS_DURATION)
		{
			buttonState = MIDDLE_PRESSED;
		}
		if (buttonPressDuration >= LONG_PRESS_DURATION)
		{
			buttonState = LONG_PRESSED;
		}
	}
	
	if (oldButtonState != buttonState)
	{
		onButtonStateChanged(buttonState);
	}
}

void onButtonStateChanged(buttonStateType buttonState)
{
	blinkStateType oldState = blinkState;
	
	switch (buttonState)
	{
		case UNPRESSED:
			blinkState = NONE;
			break;
		case PRESSED:
			LED_PORT ^= (1 << LED_PIN);
			break;
		case MIDDLE_PRESSED:
			blinkState = SLOW;
			break;
		case LONG_PRESSED:
			blinkState = FAST;
			break;
	}
	
	if (oldState != blinkState)
	{
		onBlinkStateChanged(blinkState);
	}
}

uint16_t millis()
{
	uint16_t value;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		value = milliseconds;
	}
	return value;
}

bool isButtonPressed()
{	
	return !(PIND & (1 << PIND2));
}

int main(void)
{
	TCCR0 = (0 << CS02) | (1 << CS01) | (1 << CS00);
	TIMSK = (1 << TOIE0);
	
	DDRB = 1 << DDB0;
	
	DDRD = (0 << DDD2) | (1 << DDD7);
	PORTD = (1 << PIND2);
	
	GICR = 0 << INT0;
	//MCUCR = (0 << ISC01) | (1 << ISC00);
	
	sei();
	
	pressTimer = millis();
	
	while (1)
	{
	}
}