/*
  K30.h - Library for handling Senseaire CO2 functions 
  Created by Daniel Pothier.
*/
#ifndef K30_h
#define K30_h

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

class K30
{
  public:
    K30(HardwareSerial *serialPort, int addr);
    void printCO2(boolean p, unsigned long mac);
    void printCO2(boolean p, unsigned long mac, int value);
    int readCO2();

  private:
    int i2cAddr; //hardware i2c address
    HardwareSerial *serial; //serial port to use to print the co2 value
};

#endif
