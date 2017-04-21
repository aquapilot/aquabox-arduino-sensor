//
// Created by Sebastien on 21.04.2017.
//

#ifndef __S_MESSAGE_H__
#define __S_MESSAGE_H__

#include "SensorType.h"

/**
* Sensor message (type 'S')
*/

struct S_message
{
    uint16_t measure;
    uint16_t voltage_reading;
    uint8_t lost_packets;
    uint8_t reserved;
    SensorType sensorType;
    static char buffer[];
    S_message(void): measure(0), voltage_reading(0) {}
    char* toString(void);
};

#endif
