#define BITS_IN_BYTE 8

struct DHTData
{
    enum class Status
    {
        Valid,
        TimeoutOnSync1,
        TimeoutOnSync2,
        TimeoutOnDataReceive,
        ChecksumFailed,
        Other
    };

    DHTData(Status stat) : temperature(-1.f), humidity(-1.f), status(stat) {}
    DHTData(float temp, float hum) : temperature(temp), humidity(hum), status(Status::Valid) {}
  
    float temperature;
    float humidity;
    Status status;
};

float toFloat(byte intPart, byte floatPart)
{
    float f = floatPart;

    while(f >= 1.f)
    {
      f /= 10.f;
    }

    return static_cast<float>(intPart) + f;
}

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
        
        delay(18);
    
        //set input && pullup
        bitClear(*m_directionReg, m_pin);
        bitSet(*m_portReg, m_pin);
      
        auto pulseLength = pulseInImpl(LOW, 100);
    
        if(pulseLength < 75)
        {
            return DHTData{DHTData::Status::TimeoutOnSync1};
        }
    
        if(!waitForState(LOW, 100))
        {
            return DHTData{DHTData::Status::TimeoutOnSync2};
        }
    
        byte data[5] = {0};
        byte count = 0;
        
        while(count < 40)
        {
            pulseLength = pulseInImpl(HIGH, 140);
      
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
    bool waitForState(byte state, unsigned long timeout, unsigned long start = micros())
    {
        while(bitRead(*m_pinReg, m_pin) != state)
        {
            auto diff = micros() - start;
            if(diff >= timeout)
            {
               return false;
            }
        }
    
        return true;
    }
    
    long pulseInImpl(byte state, unsigned int timeout)
    {
        auto start = micros();
    
        if(!waitForState(!state, timeout, start))
        { //wait untill initial state is set
            return 0;
        }
    
        if(!waitForState(state, timeout, start))
        { //wait for transition HIGH -> LOW or LOW -> HIGH
            return 0;
        }
    
        auto end = micros();
    
        if(!waitForState(!state, timeout, start))
        { //wait for transition LOW -> HIGH or HIGH -> LOW
          return 0;
        }
    
        return micros() - end;
    }

    volatile uint8_t* const m_directionReg;
    volatile uint8_t* const m_portReg;
    volatile uint8_t* const m_pinReg;
    const uint8_t m_pin;
};

static DHTReader reader(&DDRB, &PORTB, &PINB, 0); //digital pin 8

void setup() 
{
    Serial.begin(9600);
    bitSet(DDRB, 0);
    bitSet(PORTB, 0);
}

void loop() 
{
    //sensor requires delay for accurate measurement, including startup
    delay(2000);
  
    const DHTData& data = reader.getData();

    if (data.status == DHTData::Status::Valid) 
    {
        Serial.print("Sample OK: ");
        Serial.print(data.temperature); 
        Serial.print(" *C, "); 
        Serial.print(data.humidity); 
        Serial.println("% H");
    }
    else if (data.status == DHTData::Status::TimeoutOnSync1)
    {
        Serial.println("Read DHT11 timed out on the first step of sync");
    }
    else if (data.status == DHTData::Status::TimeoutOnSync2)
    {
        Serial.println("Read DHT11 timed out on the second step of sync");
    }
    else if (data.status == DHTData::Status::TimeoutOnDataReceive)
    {
        Serial.println("Read DHT11 timed out while receiving data");
    }
    else if (data.status == DHTData::Status::ChecksumFailed)
    {
        Serial.println("Read DHT11 with failed checksum");
    }
    else if (data.status == DHTData::Status::Other)
    {
        Serial.println("Read DHT11 failed with unknown reason");
    }
}
