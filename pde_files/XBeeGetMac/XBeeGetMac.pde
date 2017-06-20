/*
  AnalogReadSerial
 Reads an analog input on pin 0, prints the result to the serial monitor 
 
 This example code is in the public domain.
 */

#include <XBeeFunc.h>

unsigned long mac = 0;
XBeeFunc xbee(&Serial);

void setup() {
  
  mac = xbee.getXBeeSerialNum();
  
  Serial.begin(9600);
  
}

void loop() {
  Serial.println(mac, DEC);
  delay(2000);
}

