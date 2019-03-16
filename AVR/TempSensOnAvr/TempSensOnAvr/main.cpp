
#include <stdio.h>
#include <util/delay.h>

#include "Util/DHTReader.h"

extern "C"
{
    #include "Driver/uart.h"
}

#define UART_BAUD_RATE 9600

int main()
{
    uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));

    clock_init();

    DHTReader reader(&DDRB, &PORTB, &PINB, 0); //running on digital pin 8

    while(true)
    {
		//sensor requires delay for accurate measurement, including startup
		_delay_ms(2000);
		
		const DHTData& data = reader.getData();

		if (data.status == DHTData::Status::Valid)
		{
			char buff[64] = {0};
			snprintf(buff, sizeof(buff), "Sample OK: %.2f *C %.2f %% H\n\r", static_cast<double>(data.temperature), static_cast<double>(data.humidity));
			uart_puts(buff);
		}
		else if (data.status == DHTData::Status::TimeoutOnSync1)
		{
			uart_puts("Read DHT11 timed out on the first step of sync\n\r");
		}
		else if (data.status == DHTData::Status::TimeoutOnSync2)
		{
			uart_puts("Read DHT11 timed out on the second step of sync\n\r");
		}
		else if (data.status == DHTData::Status::TimeoutOnDataReceive)
		{
			uart_puts("Read DHT11 timed out while receiving data\n\r");
		}
		else if (data.status == DHTData::Status::ChecksumFailed)
		{
			uart_puts("Read DHT11 with failed checksum\n\r");
		}
		else if (data.status == DHTData::Status::Other)
		{
			uart_puts("Read DHT11 failed with unknown reason\n\r");
		}
    }
}