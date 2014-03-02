/*
 * systimer.h
 *
 * Created: 02.03.2014 21:27:31
 *  Author: Dmitry
 */ 


#ifndef SYSTIMER_H_
#define SYSTIMER_H_

#include <stdint.h>
#include <stdbool.h>
#include <util/atomic.h>
#include <avr/io.h>

typedef uint_least16_t systime_t;

typedef struct 
{
	systime_t start;
	systime_t interval;
} timer;

void init_systimer();
systime_t get_systime();
void timer_set(timer *t, systime_t msecs);
bool timer_expired(timer *t);

//////////////////////////////////////////////////////////////////////////

#define TCNT0_TICKS_PER_MS (F_CPU / 64 / 1000)

static volatile systime_t _systime;

ISR(TIMER0_OVF_vect)
{
	TCNT0 -= TCNT0_TICKS_PER_MS;
	_systime++;
}

void init_systimer()
{
	TCCR0B = 0 << CS02 | 1 << CS01 | 1 << CS00;
	TIMSK = 1 << TOIE0;
}

systime_t get_systime()
{
	systime_t result;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		result = _systime;
	}
	return result;
}

void timer_set(timer *t, systime_t msecs)
{
	t->interval = msecs;
	t->start = get_systime();
}

bool timer_expired(timer *t)
{
	return (get_systime() - t->start) >= t->interval;
}

#endif /* SYSTIMER_H_ */