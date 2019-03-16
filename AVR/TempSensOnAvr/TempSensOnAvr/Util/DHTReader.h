
#ifndef DHT_READER_H
#define DHT_READER_H

#include <avr/io.h>
#include <util/delay.h>

#include "DHTData.h"
#include "Timer.h"
#include "Utils.h"
#include "Definitions.h"

class DHTReader
{
public:
    DHTReader(volatile uint8_t* directionReg, volatile uint8_t* portReg, volatile uint8_t* pinReg, uint8_t pin) : m_directionReg(directionReg), m_portReg(portReg), m_pinReg(pinReg), m_pin(pin) {}
    ~DHTReader() = default;

    DHTData getData()
    {
        //output && low
        bitSet(*m_directionReg, m_pin);
        bitClear(*m_portReg, m_pin);
		
        _delay_ms(20);

        //set input && pullup
        bitClear(*m_directionReg, m_pin);
        bitSet(*m_portReg, m_pin);

        auto pulseLength = pulseIn(&PINB, 0, LOW, 100);

        if(pulseLength < 75)
        {
            return DHTData{DHTData::Status::TimeoutOnSync1};
        }

        if(!waitForState(m_pinReg, m_pin, LOW, 5000))
        {
            return DHTData{DHTData::Status::TimeoutOnSync2};
        }

        byte data[5] = {0};
        byte count = 0;

        while(count < 40)
        {
            pulseLength = pulseIn(m_pinReg, m_pin, HIGH, 140);

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
    volatile uint8_t* const m_directionReg;
    volatile uint8_t* const m_portReg;
    volatile uint8_t* const m_pinReg;
    const uint8_t m_pin;
};

#endif /* DHT_READER_H */
