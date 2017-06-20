/*
  MCP9700.h - Library for handling MCP9700 temperature sensor functions 
  Created by Daniel Pothier.
*/
#ifndef MCP9700_h
#define MCP9700_h

#include "WProgram.h"

#define NUM_SAMPLES 3

class MCP9700
{
  public:
    MCP9700(HardwareSerial *serialPort, int inputPin);
    void printTempC(boolean p, unsigned long mac);
    float getLastTempC();

  private:
    float lastTempC;
    int inputPin;
    float getTemp(float sensorValue);
    HardwareSerial *serial; //serial port to use to print the sensor data
};

#endif
