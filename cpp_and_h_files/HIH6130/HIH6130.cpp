#include "HIH6130.h"
#include <SensorTypes.h>
#include <Wire.h>
#include <string.h>
#include <avr/pgmspace.h>

#define TRUE 1
#define FALSE 0

HIH6130::HIH6130(HardwareSerial *serialPort){
 
  WIRE_STARTED = false;
  serial = serialPort;
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  Wire.begin();
  
}

void HIH6130::getTRH(TRH *trh){
	
	byte _status;
	unsigned int H_dat, T_dat;
	float RH, TC;
	
	_status = fetch(&H_dat, &T_dat);
	
	trh->status = _status;
	
	//we don't have any data, just return
	if(_status != 0){
		return;
	}
	
	RH = (float)H_dat * 6.10e-3;
	TC = (float)T_dat * 1.007e-2 - 40;
	
	trh->rh = RH;
	trh->temp = TC;
	
	return;

}

void HIH6130::printTRH(boolean p, unsigned long mac, TRH *trh){
	
	  switch(trh->status){
	   
	   case 0: 
	    break;
	   
	   case 1:
	    serial->println("HIH6130: STALE DATA");
	    break;
	   
	   case 2:
	    serial->println("HIH6130: IN COMMAND MODE");
	    break;
	   
	   default:
	    serial->println("HIH6030: DIAGNOSTIC");
	    break; 
	    
	  }
	  
	  //we don't have any data, just return
	  if(trh->status != 0){
	    return;
	  }
	  
	  if(p == true){
	    
	    serial->print(mac, DEC);
	    serial->print(",");
	    serial->print(RH_SENSOR_TYPE, DEC);
	    serial->print(",");
	    serial->print(trh->rh, 2);
	    serial->print("\n");
	    
	    serial->print(mac, DEC);
	    serial->print(",");
	    serial->print(TEMP_SENSOR_TYPE, DEC);
	    serial->print(",");
	    serial->print(trh->temp, 2);
	    serial->print("\n");
	    
	  }
	
}

void HIH6130::printTRH(boolean p, unsigned long mac){
  
  byte _status;
  unsigned int H_dat, T_dat;
  float RH, TC;
  
  _status = fetch(&H_dat, &T_dat);
  
  switch(_status){
   
   case 0: 
    break;
   
   case 1:
    serial->println("HIH6130: STALE DATA");
    break;
   
   case 2:
    serial->println("HIH6130: IN COMMAND MODE");
    break;
   
   default:
    serial->println("HIH6030: DIAGNOSTIC");
    break; 
    
  }
  
  //we don't have any data, just return
  if(_status != 0){
    serial->print("INVALID STATUS:");
    serial->print(_status);
    serial->print("\n");
    return;
  }
  
  RH = (float)H_dat * 6.10e-3;
  TC = (float)T_dat * 1.007e-2 - 40;

  if(p == true){
    
    serial->print(mac, DEC);
    serial->print(",");
    serial->print(RH_SENSOR_TYPE, DEC);
    serial->print(",");
    serial->print(RH, 2);
    serial->print("\n");
    
    serial->print(mac, DEC);
    serial->print(",");
    serial->print(TEMP_SENSOR_TYPE, DEC);
    serial->print(",");
    serial->print(TC, 2);
    serial->print("\n");
    
  }
  
}

byte HIH6130::fetch(unsigned int *p_H_dat, unsigned int *p_T_dat){
  
      byte address, Hum_H, Hum_L, Temp_H, Temp_L, _status;
      unsigned int H_dat, T_dat;
      address = 0x27;

      Wire.beginTransmission(address); 
      Wire.endTransmission();

      delay(50);
      
      Wire.requestFrom((int)address, (int) 4, false);
      Hum_H = Wire.read();
      Hum_L = Wire.read();
      Temp_H = Wire.read();
      Temp_L = Wire.read();
      Wire.endTransmission();
      delay(50);
      
      _status = (Hum_H >> 6) & 0x03;
      Hum_H = Hum_H & 0x3f;
      H_dat = (((unsigned int)Hum_H) << 8) | Hum_L;
      T_dat = (((unsigned int)Temp_H) << 8) | Temp_L;
      T_dat = T_dat / 4;
      *p_H_dat = H_dat;
      *p_T_dat = T_dat;
      return(_status);
}    



