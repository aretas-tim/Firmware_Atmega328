#include "MCP9700.h"
#include <SensorTypes.h>
#include <string.h>
#include <avr/pgmspace.h>

MCP9700::MCP9700(HardwareSerial *serialPort, int ioPin){
 
  lastTempC = 0.00;
  pinMode(ioPin, INPUT);
  inputPin = ioPin;
  serial = serialPort;
  
}

void MCP9700::printTempC(boolean p, unsigned long mac){
  
  float sensorValue = 0.0;
  float tmp = 0.0;
  
  //get the avg of N reads
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(inputPin);
    delay(10);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;

  if(p == true){
    
    serial->print(mac, DEC);
    serial->print(",");
    serial->print(TEMP_SENSOR_TYPE, DEC);
    serial->print(",");
    serial->print(getTemp(sensorValue));
    serial->print("\n");
    
  }
  
}

float MCP9700::getTemp(float sensorValue){
  
  float vin = (sensorValue / 1023.0) * 5.0;
  float ret = ((vin * 1000.0) - 500.0) / 10.0;
  lastTempC = ret;
  return ret;
  
}

float MCP9700::getLastTempC(){
  
  return lastTempC;
  
}
