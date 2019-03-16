
#ifndef DHT_READER_H
#define DHT_READER_H

extern "C"
{
	#include <stm32f1xx_hal_gpio.h>
}

#include "DHTData.h"
#include "Timer.h"
#include "Utils.h"
#include "Definitions.h"

class DHTReader
{
public:
	DHTReader(GPIO_TypeDef* gpio, uint16_t pin) : m_gpio(gpio), m_pin(pin)
	{
		if(!m_gpio)
		{
			Error_Handler();
		}
	}

	~DHTReader() = default;

	DHTData getData()
	{
		//output && low
		GPIO_InitTypeDef GPIO_InitStruct = {0};

		HAL_GPIO_WritePin(m_gpio, m_pin, GPIO_PIN_RESET);

		GPIO_InitStruct.Pin = m_pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(m_gpio, &GPIO_InitStruct);

		HAL_Delay(18);

		//set input && pullup
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(m_gpio, &GPIO_InitStruct);

		auto pulseLength = pulseIn(m_gpio, m_pin, LOW, 100);

		if(pulseLength < 75)
		{
			return DHTData{DHTData::Status::TimeoutOnSync1};
		}

		if(!waitForState(m_gpio, m_pin, LOW, 100)) //give it 100us to start transmission
		{
			return DHTData{DHTData::Status::TimeoutOnSync2};
		}

		byte data[5] = {0};
		byte count = 0;

		while(count < 40)
		{
			pulseLength = pulseIn(m_gpio, m_pin, HIGH, 140);

			if(pulseLength == 0)
			{
				return DHTData{DHTData::Status::TimeoutOnDataReceive};
			}

			if(pulseLength > 60)
			{
				const auto index = count / BITS_IN_BYTE;
				const auto bit = BITS_IN_BYTE - 1 - count % BITS_IN_BYTE; //reverse order
				bitSet(data[index], bit);
			}

			count++;
		}

		const byte crc = data[0] + data[1] + data[2] + data[3];

		if(crc != data[4])
		{
			return DHTData{DHTData::Status::ChecksumFailed};
		}

		return DHTData{toFloat(data[2], data[3]), toFloat(data[0], data[1])};
	}

private:
	GPIO_TypeDef* m_gpio;
	uint16_t m_pin;
};

#endif /* DHT_READER_H */
