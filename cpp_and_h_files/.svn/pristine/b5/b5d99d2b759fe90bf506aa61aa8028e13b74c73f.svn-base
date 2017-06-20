/*
  XBeeFunc.h - Library for handling some XBee AT functions in transparent mode
  Created by Daniel Pothier.
*/
#ifndef XBeeFunc_h
#define XBeeFunc_h

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

class XBeeFunc
{
  public:
    XBeeFunc(HardwareSerial *serialPort);
    unsigned long getXBeeSerialNum();
    boolean setPanID(unsigned long panID);

  private:
    byte getsTimeout(char *buf, uint16_t timeout);
    HardwareSerial *serial;
};

#endif
