
#ifndef DHT_DATA_H
#define DHT_DATA_H

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

#endif /* DHT_DATA */
