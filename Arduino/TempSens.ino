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
    DHTReader(byte pin) : m_pin(pin) {}
    ~DHTReader() = default;

    DHTData getData()
    {
        pinMode(m_pin, OUTPUT);
        digitalWrite(m_pin, LOW);
      
        delay(18);
    
        pinMode(m_pin, INPUT_PULLUP);
      
        auto pulseLength = pulseIn(m_pin, LOW, 100);
    
        if(pulseLength < 75)
        {
            return DHTData{DHTData::Status::TimeoutOnSync1};
        }
    
        byte retries = 0;
        while(digitalRead(m_pin) != LOW)
        {
            if(retries++ > 250)
            {
                return DHTData{DHTData::Status::TimeoutOnSync2};
            }
        }
    
        byte data[5] = {0};
        byte count = 0;
        
        while(count < 40)
        {
            pulseLength = pulseIn(m_pin, HIGH, 140);
      
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
    byte m_pin;
};

#define DHT_PIN 8
static DHTReader reader(DHT_PIN);

void setup() 
{
    Serial.begin(9600);
    pinMode(DHT_PIN, OUTPUT);
    digitalWrite(DHT_PIN, HIGH);
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
