/*
  HIH4030.h - Library for handling HIH4030 humidity sensor functions 
  Created by Daniel Pothier.
*/
#ifndef HIH4030_h
#define HIH4030_h

#include "WProgram.h"
#include <SensorCalibration.h>

#define DEFAULT_RH_X1 0 
#define DEFAULT_RH_X2 28.5  //test RH
#define DEFAULT_RH_Y1 0.85  //voltage @0% RH
#define DEFAULT_RH_Y2 1.49  //voltage @X2 RH

#define NUM_SAMPLES 3

#define FAKE_INIT_TEMP -255.5

class HIH4030
{
  public:
    HIH4030(HardwareSerial *serialPort, int inputPin, SensorCalibration *cal);
    void printRH(boolean p, unsigned long mac, float tempC = FAKE_INIT_TEMP);
    boolean calibrating;
    
  private:
    int inputPin;
    float getRH(float sensorValue, float tempC);
    HardwareSerial *serial; //serial port to use to print the sensor data
    SensorCalibration *_cal;
};

#endif
