
#ifndef TIMER_H
#define TIMER_H

extern "C"
{
	#include <stm32f1xx_hal_tim.h>
}

#include "Definitions.h"

#define F_CPU 72000000
#define PRESCALE 288
#define CLOCK_CYCLES_PER_SEC (F_CPU / PRESCALE)
#define CLOCK_CYCLES_PER_US (1000000.f / CLOCK_CYCLES_PER_SEC)

TIM_HandleTypeDef htim3;

volatile unsigned long timerOverflows = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim3)
	{
		timerOverflows++;
	}
}

unsigned long micros()
{
	__disable_irq(); //disable interrupts

	int overflows = timerOverflows;
	auto ticks = htim3.Instance->CNT;

	__enable_irq(); //enable interrupts

	//total overflows + counting ticks by cycle rate in microseconds
	//each overflow happens when 2 byte register is full therefore 65536
	return (overflows * 65536 + ticks) * CLOCK_CYCLES_PER_US;
}

bool waitForState(GPIO_TypeDef* gpio, uint16_t pin, byte state, unsigned int timeout, unsigned long start = micros())
{
	while (HAL_GPIO_ReadPin(gpio, pin) != state)
	{
		auto diff = micros() - start;
		if(diff >= timeout)
		{
			return false;
		}
	}

	return true;
}

unsigned long pulseIn(GPIO_TypeDef* gpio, uint16_t pin, byte state, unsigned int timeout)
{
	auto start = micros();

	if(!waitForState(gpio, pin, !state, timeout, start))
	{ //wait till initial state is set
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
