#ifndef __DIGITAL_DECODER_H__
#define __DIGITAL_DECODER_H__

#include <stdint.h>
#include <map>

class DigitalDecoder
{
  public:
    DigitalDecoder() = default;
    
    void handleData(char data);
    
  private:
    
    struct deviceState_t
    {
        uint64_t lastUpdateTime;
        uint64_t lastAlarmTime;
        
        uint8_t lastRawState;
        
        bool tamper;
        bool alarm;
        bool batteryLow;
        
        bool isMotionDetector;
    };

    void sendDeviceState(uint32_t serial, deviceState_t ds);
    void updateDeviceState(uint32_t serial, uint8_t state);
    void handlePayload(uint64_t payload);
    void handleBit(bool value);
    void decodeBit(bool value);

    unsigned int samplesSinceEdge = 0;
    bool lastSample = false;
    

    std::map<uint32_t, deviceState_t> deviceStateMap;
};

#endif
