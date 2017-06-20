#include "K30.h"
#include <SensorTypes.h>
#include <Wire.h>
#include <string.h>
#include <avr/pgmspace.h>

K30::K30(HardwareSerial *serialPort, int addr){

  serial = serialPort;  
  i2cAddr = addr;
  Wire.begin();
  
}

//overloaded method for when we already have the CO2 value
void K30::printCO2(boolean p, unsigned long mac, int value){
	
	int co2Value = value;
	
	if(p == true){
		
		if(co2Value > 0){
			
			serial->print(mac, DEC);
			serial->print(",");
			serial->print(CO2_SENSOR_TYPE, DEC);
			serial->print(",");
			serial->print(co2Value);
			serial->print("\n");
			
		  }else{
			Serial.println("K30 READ FAILURE");
		  }
		
	}
	
}

void K30::printCO2(boolean p, unsigned long mac){

    if(p == true){
      //do the CO2 cycle
      int co2Value = readCO2();

	if(co2Value <= 0){
       //try again... probably a read failure
       delay(10);
       co2Value = readCO2();
      }
      
      if(co2Value > 0){
        
        serial->print(mac, DEC);
        serial->print(",");
        serial->print(CO2_SENSOR_TYPE, DEC);
        serial->print(",");
        serial->print(co2Value);
        serial->print("\n");
        
      }else{
        Serial.println("K30 READ FAILURE");
      }
    }
  
}

///////////////////////////////////////////////////////////////////
// Function : int readCO2()
// Returns : CO2 Value upon success, 0 upon checksum failure
// Assumes : - Wire library has been imported successfully.
//- LED is connected to IO pin 13
//- CO2 sensor address is defined in co2_addr
///////////////////////////////////////////////////////////////////
int K30::readCO2(){
  
  int co2_value = 0;
  // We will store the CO2 value inside this variable.
  
  //begin write 
  Wire.beginTransmission(i2cAddr);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x08);
  Wire.write(0x2A);
  Wire.endTransmission();
  //end write 
  
  /*
  We wait 10ms for the sensor to process our command.
  The sensors's primary duties are to accurately
  measure CO2 values. Waiting 10ms will ensure the
  data is properly written to RAM
  */
  delay(10);
  
  /*
  Since we requested 2 bytes from the sensor we must
  read in 4 bytes. This includes the payload, checksum,
  and command status byte.
  */
  Wire.requestFrom(i2cAddr, 4, true);
  byte i = 0;
  byte buffer[4] = {0, 0, 0, 0};
  /*
  Wire.available() is not nessessary. Implementation is obscure but we leave
  it in here for portability and to future proof our code
  */

  while(Wire.available() && (i < 4)){
    
    buffer[i] = Wire.read();
    i++;
  }

  Wire.endTransmission();

  /*
  Using some bitwise manipulation we will shift our buffer
  into an integer for general consumption
  */
  co2_value = 0;
  co2_value |= buffer[1] & 0xFF;
  co2_value = co2_value << 8;
  co2_value |= buffer[2] & 0xFF;
  
  byte sum = 0;
  sum = buffer[0] + buffer[1] + buffer[2];
  
  //Checksum Byte
  //Byte addition utilizes overflow
  if(sum == buffer[3]){

    return co2_value;
  
  }else{
  
    // Failure!
    /*
    Checksum failure can be due to a number of factors,
    fuzzy electrons, sensor busy, etc.
    */
    return 0;
  }
}




