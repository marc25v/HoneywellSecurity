#ifndef __DIGITAL_DECODER_H__
#define __DIGITAL_DECODER_H__

#include <stdint.h>
#include <map>

class DigitalDecoder
{
  public:
    DigitalDecoder() = default;
    
    void handleData(char data);
    void setRxGood(bool state);
  
  protected:
    bool isPayloadValid(uint64_t payload, uint64_t polynomial=0) const;
  
  private:
    
    struct deviceState_t
    {
        uint64_t lastUpdateTime;
        uint64_t lastAlarmTime;
        
        uint8_t lastRawState;
        
        bool tamper;
        bool alarm;
        bool batteryLow;
        bool heartbeat;
        
        bool isMotionDetector;
    };

    void sendDeviceState(uint32_t serial, deviceState_t ds);
    void updateDeviceState(uint32_t serial, uint8_t state);
    void writeDeviceState();
    void sendDeviceState();
    void updateSensorState(uint32_t serial, uint64_t payload);
    void updateKeypadState(uint32_t serial, uint64_t payload);
    void updateKeyfobState(uint32_t serial, uint64_t payload);
    void handlePayload(uint64_t payload);
    void handleBit(bool value);
    void decodeBit(bool value);
    void checkForTimeouts();


    unsigned int samplesSinceEdge = 0;
    bool lastSample = false;
    bool rxGood = false;
    uint64_t lastRxGoodUpdateTime = 0;
    //Mqtt &mqtt;  //Not using the mqtt in vondruska release
    uint32_t packetCount = 0;
    uint32_t errorCount = 0;
  
   struct sensorState_t
    {
        uint64_t lastUpdateTime;
        bool hasLostSupervision;

        bool loop1;
        bool loop2;
        bool loop3;
        bool tamper;
        bool lowBat;
    };

    struct keypadState_t
    {
        uint64_t lastUpdateTime;
        bool hasLostSupervision;
        
        std::string phrase;

        char sequence;
        bool lowBat;
    };

    std::map<uint32_t, sensorState_t> sensorStatusMap;
    std::map<uint32_t, keypadState_t> keypadStatusMap;
    uint64_t lastKeyfobPayload;
    std::map<uint32_t, deviceState_t> deviceStateMap;
};

#endif
