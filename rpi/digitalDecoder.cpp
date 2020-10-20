#include "digitalDecoder.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <iomanip>
#include <locale>
#include <ctime>
#include <csignal>

// Pulse checks seem to be about 60-70 minutes apart
#define RX_TIMEOUT_MIN      (90)

// Give each sensor 3 intervals before we flag a problem
#define SENSOR_TIMEOUT_MIN  (90*5)

#define SYNC_MASK    0xFFFF000000000000ul
#define SYNC_PATTERN 0xFFFE000000000000ul

// Don't send these messages more than once per minute unless there is a state change
#define RX_GOOD_MIN_SEC (60)
#define UPDATE_MIN_SEC (60)

#define BASE_TOPIC "ha/sensor/alarm/"
#define SENSOR_TOPIC BASE_TOPIC"sensor/"
#define KEYFOB_TOPIC BASE_TOPIC"keyfob/"
#define KEYPAD_TOPIC BASE_TOPIC"keypad/"
//UPDATE 2020:  Defining
#define OPEN_SENSOR_MSG "OPEN_SENSOR_MSG"
#define CLOSED_SENSOR_MSG "CLOSED_SENSOR_MSG"
#define TAMPER_MSG "TAMPER_MSG"
#define UNTAMPERED_MSG "UNTAMPERED_MSG"
#define LOW_BAT_MSG "LOW_BAT_MSG"
#define OK_BAT_MSG "OK_BAT_MSG"

void DigitalDecoder::setRxGood(bool state)
{
    std::string topic(BASE_TOPIC);
    timeval now;

    topic += "rx_status";

    gettimeofday(&now, nullptr);

    if (rxGood != state || (now.tv_sec - lastRxGoodUpdateTime) > RX_GOOD_MIN_SEC)
    {
              std::ostringstream oss;
    oss << "/usr/bin/mosquitto_pub";
    oss << " -h 192.168.0.35";
    oss << " -u mqtt-alarm2";
    oss << " -P honeywell54312!";
    oss << " -i HoneywellSecurity -r";
    oss << " -t '" << topic.c_str() << "' ";
    oss << " -m '";
    oss << "{";
    oss << "\"state\": " << (state ? "OK" : "FAILED");
    oss << "}'";

    std::cout << oss.str() << std::endl;

    system(oss.str().c_str());
        
    }

    // Reset watchdog either way
    alarm(RX_TIMEOUT_MIN*60);

    rxGood = state;
    lastRxGoodUpdateTime = now.tv_sec;
}

void DigitalDecoder::updateSensorState(uint32_t serial, uint64_t payload)
{
    timeval now;
    gettimeofday(&now, nullptr);

    struct sensorState_t lastState;
    struct sensorState_t currentState;

    currentState.lastUpdateTime = now.tv_sec;
    currentState.hasLostSupervision = false;

    currentState.loop1 = payload  & 0x000000800000;
    currentState.loop2 = payload  & 0x000000200000;
    currentState.loop3 = payload  & 0x000000100000;
    currentState.tamper = payload & 0x000000400000;
    currentState.lowBat = payload & 0x000000080000;

    bool supervised = payload & 0x000000040000;
    // bool repeated = payload & 0x000000020000;

    //std::cout << "Payload:" << std::hex << payload << " Serial:" << std::dec << serial << std::boolalpha << " Loop1:" << currentState.loop1 << std::endl;

    auto found = sensorStatusMap.find(serial);
    if (found == sensorStatusMap.end())
    {
        // if there wasn't a state, make up a state that is opposite to our current state
        // so that we send everything.

        lastState.hasLostSupervision = !currentState.hasLostSupervision;
        lastState.loop1 = !currentState.loop1;
        lastState.loop2 = !currentState.loop2;
        lastState.loop3 = !currentState.loop3;
        lastState.tamper = !currentState.tamper;
        lastState.lowBat = !currentState.lowBat;
        lastState.lastUpdateTime = 0;
    }
    else
    {
        lastState = found->second;
    }
    
    // Since the sensor will frequently blast out the same signal many times, we only want to treat
    // the first detected signal as the supervisory signal. 
//    bool supervised = (payload & 0x000000040000) && ((currentState.lastUpdateTime - lastState.lastUpdateTime) > 2);

    if ((currentState.loop1 != lastState.loop1) || supervised)
    {
        
        std::ostringstream oss;
    oss << "/usr/bin/mosquitto_pub";
    oss << " -h 192.168.0.35";
    oss << " -u mqtt-alarm2";
    oss << " -P honeywell54312!";
    oss << " -i HoneywellSecurity -r";
    oss << " -t 'ha/sensor/alarm/loop1/" << serial << "' ";
    oss << " -m '";
    oss << "{";
    oss << "\"serial\": " << serial << ",";
    oss << "\"state\": " << (currentState.loop1 ? OPEN_SENSOR_MSG : CLOSED_SENSOR_MSG);
    oss << "}'";

    std::cout << oss.str() << std::endl;

    system(oss.str().c_str());
        
        /*std::ostringstream topic;
        topic << SENSOR_TOPIC << serial << "/loop1";
        mqtt.send(topic.str().c_str(), currentState.loop1 ? OPEN_SENSOR_MSG : CLOSED_SENSOR_MSG, supervised ? 0 : 1);
    */
    }

    if ((currentState.loop2 != lastState.loop2) || supervised)
    {
          std::ostringstream oss;
    oss << "/usr/bin/mosquitto_pub";
    oss << " -h 192.168.0.35";
    oss << " -u mqtt-alarm2";
    oss << " -P honeywell54312!";
    oss << " -i HoneywellSecurity -r";
    oss << " -t 'ha/sensor/alarm/loop2/" << serial << "' ";
    oss << " -m '";
    oss << "{";
    oss << "\"serial\": " << serial << ",";
    oss << "\"state\": " << (currentState.loop2 ? OPEN_SENSOR_MSG : CLOSED_SENSOR_MSG);
    oss << "}'";

    std::cout << oss.str() << std::endl;

    system(oss.str().c_str());
      
    }

    if ((currentState.loop3 != lastState.loop3) || supervised)
    {
          std::ostringstream oss;
    oss << "/usr/bin/mosquitto_pub";
    oss << " -h 192.168.0.35";
    oss << " -u mqtt-alarm2";
    oss << " -P honeywell54312!";
    oss << " -i HoneywellSecurity -r";
    oss << " -t 'ha/sensor/alarm/loop3/" << serial << "' ";
    oss << " -m '";
    oss << "{";
    oss << "\"serial\": " << serial << ",";
    oss << "\"state\": " << (currentState.loop3 ? OPEN_SENSOR_MSG : CLOSED_SENSOR_MSG);
    oss << "}'";

    std::cout << oss.str() << std::endl;

    system(oss.str().c_str());
      
    }

    if ((currentState.tamper != lastState.tamper) || supervised)
    {
     
    std::ostringstream oss;
    oss << "/usr/bin/mosquitto_pub";
    oss << " -h 192.168.0.35";
    oss << " -u mqtt-alarm2";
    oss << " -P honeywell54312!";
    oss << " -i HoneywellSecurity -r";
    oss << " -t 'ha/sensor/alarm/tamper/" << serial << "' ";
    oss << " -m '";
    oss << "{";
    oss << "\"serial\": " << serial << ",";
    oss << "\"state\": " << (currentState.tamper ? TAMPER_MSG : UNTAMPERED_MSG);
    oss << "}'";

    std::cout << oss.str() << std::endl;

    system(oss.str().c_str());
        
       /* 
        std::ostringstream topic;
        topic << SENSOR_TOPIC << serial << "/tamper";
        mqtt.send(topic.str().c_str(), currentState.tamper ? TAMPER_MSG : UNTAMPERED_MSG, supervised ? 0 : 1);
    */
    }

    if ((currentState.lowBat != lastState.lowBat) || supervised)
    {
     std::ostringstream oss;
    oss << "/usr/bin/mosquitto_pub";
    oss << " -h 192.168.0.35";
    oss << " -u mqtt-alarm2";
    oss << " -P honeywell54312!";
    oss << " -i HoneywellSecurity -r";
    oss << " -t 'ha/sensor/alarm/battery/" << serial << "' ";
    oss << " -m '";
    oss << "{";
    oss << "\"serial\": " << serial << ",";
    oss << "\"state\": " << (currentState.lowBat ? LOW_BAT_MSG : OK_BAT_MSG);
    oss << "}'";

    std::cout << oss.str() << std::endl;

    system(oss.str().c_str());
        /*
        std::ostringstream topic;
        topic << SENSOR_TOPIC << serial << "/battery";
        mqtt.send(topic.str().c_str(), currentState.lowBat ? LOW_BAT_MSG : OK_BAT_MSG, supervised ? 0 : 1);
    */
    }

    sensorStatusMap[serial] = currentState;
}


void DigitalDecoder::updateDeviceState(uint32_t serial, uint8_t state)
{
    deviceState_t ds;
    
    //
    // Extract prior information.
    //

    if(deviceStateMap.count(serial))
    {
        ds = deviceStateMap[serial];
    }
    else
    {
        ds.isMotionDetector = false;
    }
    
    //
    // Watch for anything that indicates this sensor is a motion detector.
    //

    if(state < 0x80) ds.isMotionDetector = true;
    
    // Decode motion/open bits
    if(ds.isMotionDetector)
    {
        ds.alarm = (state & 0x80);
    }
    else
    {
        ds.alarm = (state & 0x20);
    }
    
    //
    // Decode tamper bit.
    //

    ds.tamper = (state & 0x40);
    
    //
    // Decode battery low bit.
    //

    ds.batteryLow = (state & 0x08);

    //
    // Decode the heartbeat pulse.
    //

    ds.heartbeat = (state & 0x04);

    //
    // Timestamp.
    //

    timeval now;
    gettimeofday(&now, nullptr);
    ds.lastUpdateTime = now.tv_sec;
    
    if(ds.alarm) ds.lastAlarmTime = now.tv_sec;

    //
    // Put the answer back in the map.
    //

    deviceStateMap[serial] = ds;
        
    //
    // Send the notification if something changed
    //
    
    if(state != ds.lastRawState)
    {
        //Update 2020:  Commented this out to build successfully.
        //sendDeviceState(serial, ds);
    }

    deviceStateMap[serial].lastRawState = state;
    
    for(const auto &dd : deviceStateMap)
    {
        printf("%sDevice %7u: %s\n",dd.first==serial ? "*" : " ", dd.first, dd.second.alarm ? "ALARM" : "OK");
    }

    printf("\n");
}


void DigitalDecoder::handlePayload(uint64_t payload)
{
    uint64_t sof = (payload & 0xF00000000000) >> 44;
    uint64_t ser = (payload & 0x0FFFFF000000) >> 24;
    uint64_t typ = (payload & 0x000000FF0000) >> 16;
    uint64_t crc = (payload & 0x00000000FFFF) >>  0;
    
    //
    // Check CRC
    //
    const uint64_t polynomial = 0x18005;
    uint64_t sum = payload & (~SYNC_MASK);
    uint64_t current_divisor = polynomial << 31;
    
    while(current_divisor >= polynomial)
    {
#ifdef __arm__
        if(__builtin_clzll(sum) == __builtin_clzll(current_divisor))
#else        
        if(__builtin_clzl(sum) == __builtin_clzl(current_divisor))
#endif
        {
            sum ^= current_divisor;
        }        
        current_divisor >>= 1;
    }
    
    const bool valid = (sum == 0);
    
    //
    // Tell the world
    //
    if(valid)
    {
        updateDeviceState(ser, typ);
    }
    
    
    //
    // Print Packet
    //
// #ifdef __arm__
//     if(valid)    
//         printf("Valid Payload: %llX (Serial %llu, Status %llX)\n", payload, ser, typ);
//     else
//         printf("Invalid Payload: %llX\n", payload);
// #else    
//     if(valid)    
//         printf("Valid Payload: %lX (Serial %lu, Status %lX)\n", payload, ser, typ);
//     else
//         printf("Invalid Payload: %lX\n", payload);
// #endif
    
    static uint32_t packetCount = 0;
    static uint32_t errorCount = 0;
    
    packetCount++;
    if(!valid)
    {
        errorCount++;
        printf("%u/%u packets failed CRC\n", errorCount, packetCount);
    }
}

void DigitalDecoder::handleBit(bool value)
{
    static uint64_t payload = 0;

    payload <<= 1;
    payload |= (value ? 1 : 0);
    
    //
    // If we see a new pattern, but the previous payload wasn't complete.
    //

    if (((payload & 0xFFFEul) == 0xFFFEul) && (payload != 0xFFFEul))
    {
#ifdef __arm__
        printf("Previous payload: %llX\n", payload >> 16);
#else
        printf("Previous payload: %lX\n", payload >> 16);
#endif     
    }

#if 0
#ifdef __arm__
    printf("Got bit: %d, payload is now %llX\n", value?1:0, payload);
#else
    printf("Got bit: %d, payload is now %lX\n", value?1:0, payload);
#endif     
#endif

    if((payload & SYNC_MASK) == SYNC_PATTERN)
    {
        handlePayload(payload);
        payload = 0;
    }
}

void DigitalDecoder::decodeBit(bool value)
{
    enum ManchesterState
    {
        LOW_PHASE_A,
        LOW_PHASE_B,
        HIGH_PHASE_A,
        HIGH_PHASE_B
    };
    
    static ManchesterState state = LOW_PHASE_A;
    
    switch(state)
    {
        case LOW_PHASE_A:
        {
            state = value ? HIGH_PHASE_B : LOW_PHASE_A;
            break;
        }
        case LOW_PHASE_B:
        {
            handleBit(false);
            state = value ? HIGH_PHASE_A : LOW_PHASE_A;
            break;
        }
        case HIGH_PHASE_A:
        {
            state = value ? HIGH_PHASE_A : LOW_PHASE_B;
            break;
        }
        case HIGH_PHASE_B:
        {
            handleBit(true);
            state = value ? HIGH_PHASE_A : LOW_PHASE_A;
            break;
        }
    }
}

void DigitalDecoder::handleData(char data)
{
    static const int samplesPerBit = 8;
    
        
    if(data != 0 && data != 1) return;
        
    const bool thisSample = (data == 1);
    
    if(thisSample == lastSample)
    {
        samplesSinceEdge++;
        
        //if(samplesSinceEdge < 100)
        //{
        //    printf("At %d for %u\n", thisSample?1:0, samplesSinceEdge);
        //}
        
        if((samplesSinceEdge % samplesPerBit) == (samplesPerBit/2))
        {
            // This Sample is a new bit
            decodeBit(thisSample);
        }
    }
    else
    {
        samplesSinceEdge = 1;
    }
    lastSample = thisSample;
}
