
#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>

#include "Definitions.h"
#include "Utils.h"

#define PRESCALE 64
#define CLOCK_CYCLES_PER_SEC (F_CPU / PRESCALE)
#define CLOCK_CYCLES_PER_US (1000000 / CLOCK_CYCLES_PER_SEC)

volatile unsigned long overflowCount = 0; //stores total count of overflows

void clock_init()
{
	//set timer2 normal mode
	TCCR2A = 0;
	
	//enable prescaler of 64
	TCCR2B |= (1 << CS22);

	//clear timer counter
	TCNT2 = 0;

	//enable overflow interrupt
	TIMSK2 |= (1 << TOIE2);

	//enable global interrupts
	sei();
}

ISR(TIMER2_OVF_vect)
{
	//interrupt routine gets called on every timer overflow
	overflowCount++;
}

unsigned long micros()
{
	byte oldSREG = SREG; //save register
	
	cli(); //disable interrupts
	
	auto overflows = overflowCount;
	auto ticks = TCNT2;

	//extra check if overflow already happened but not yet interrupted
	if ((TIFR2 & (1 << TOV2)) && (ticks < 255))
	{
		overflows++;
	}

	//restore previous interrupt enabled state
	SREG = oldSREG;

	//total overflows + counting ticks by cycle rate in microseconds
	return (overflows * 256 + ticks) * CLOCK_CYCLES_PER_US;
}

bool waitForState(volatile uint8_t* gpio, uint16_t pin, byte state, unsigned int timeout, unsigned long start = micros())
{
	while(getBit(*gpio, pin) != state)
	{
		auto diff = micros() - start;
		if(diff >= timeout)
		{
			return false;
		}
	}

	return true;
}

long pulseIn(volatile uint8_t* gpio, uint16_t pin, byte state, unsigned int timeout)
{
	auto start = micros();

	if(!waitForState(gpio, pin, !state, timeout, start))
	{ //wait till expected state is set
		return 0;
	}

	if(!waitForState(gpio, pin, state, timeout, start))
	{ //wait for transition HIGH -> LOW or LOW -> HIGH
		return 0;
	}

	auto end = micros();

	if(!waitForState(gpio, pin, !state, timeout, start))
	{ //wait for transition LOW -> HIGH or HIGH -> LOW
		return 0;
	}

	return micros() - end;
}

#endif /* TIMER_H */