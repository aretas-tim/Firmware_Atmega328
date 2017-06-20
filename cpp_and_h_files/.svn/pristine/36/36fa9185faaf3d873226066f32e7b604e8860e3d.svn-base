/*
  HIH4030.h - Library for handling HIH4030 humidity sensor functions 
  Created by Daniel Pothier.
*/
#ifndef HIH6130_h
#define HIH6130_h

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif
#include <SensorCalibration.h>

typedef struct {
  byte status;
  float temp;
  float rh;
}TRH;

class HIH6130
{
  public:
    HIH6130(HardwareSerial *serialPort);
    void printTRH(boolean p, unsigned long mac);
    void printTRH(boolean p, unsigned long mac, TRH *trh);
    void getTRH(TRH *trh);
    
  private:
    boolean WIRE_STARTED;
    HardwareSerial *serial; //serial port to use to print the sensor data
    byte fetch(unsigned int *p_H_dat, unsigned int *p_T_dat);
};

#endif
