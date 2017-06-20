#include "HIH4030.h"
#include <SensorTypes.h>
#include <string.h>
#include <avr/pgmspace.h>

HIH4030::HIH4030(HardwareSerial *serialPort, int ioPin, SensorCalibration *cal){
 
  serial = serialPort;
  pinMode(ioPin, INPUT);
  inputPin = ioPin;
  calibrating = false;
  _cal = cal;
}

void HIH4030::printRH(boolean p, unsigned long mac, float tempC){
  
  float sensorValue = 0.0;
  float tmp = 0.0;
  
  //get the avg of N reads i.e. try and dampen out spurious reads
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(inputPin);
    delay(10);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;

  if(p == true){
    
    serial->print(mac, DEC);
    serial->print(",");
    serial->print(RH_SENSOR_TYPE, DEC);
    serial->print(",");
    serial->print(getRH(sensorValue, tempC));
    serial->print("\n");
    
  }
  
}

float HIH4030::getRH(float sensorValue, float tempC){
    
  float vin = (sensorValue / 1023.0) * 5.0;
  
  if(calibrating == true){
    
    return vin;
  
  }else{
    
    float ret = (vin - _cal->b())/_cal->m();
    
    ///temperature compensation
    if(tempC != FAKE_INIT_TEMP){
      ret = ret / (1.0546 - (0.00216 * tempC));  
    }
    return ret;
    
  }
  
}

