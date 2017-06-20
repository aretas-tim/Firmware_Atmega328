#include <Wire.h>

int co2Addr = 0x68;

void setup(){
  
  Serial.begin(9600);
  Wire.begin ();
  pinMode(13, OUTPUT); // We will use this pin as a read-indicator
  Serial.println("CO2 init");
  
}

void loop() {
  
  int co2Value = readCO2();
  
  if(co2Value > 0){
    
    Serial.print("CO2 Value: ");
    Serial.println(co2Value);
    
  }else{
    Serial.println("Checksum failed / Communication failure");
  }
  
  delay(60000);

}



///////////////////////////////////////////////////////////////////
// Function : int readCO2()
// Returns : CO2 Value upon success, 0 upon checksum failure
// Assumes : - Wire library has been imported successfully.
//- LED is connected to IO pin 13
//- CO2 sensor address is defined in co2_addr
///////////////////////////////////////////////////////////////////
int readCO2(){
  
  int co2_value = 0;
  // We will store the CO2 value inside this variable.
  digitalWrite(13, HIGH);
  // On most Arduino platforms this pin is used as an indicator light.

  //////////////////////////
  /* Begin Write Sequence */
  //////////////////////////
  Wire.beginTransmission(co2Addr);
  Wire.send(0x22);
  Wire.send(0x00);
  Wire.send(0x08);
  Wire.send(0x2A);
  Wire.endTransmission();
  /////////////////////////
  /* End Write Sequence. */
  /////////////////////////
  /*
  We wait 10ms for the sensor to process our command.
  The sensors's primary duties are to accurately
  measure CO2 values. Waiting 10ms will ensure the
  data is properly written to RAM
  */
  delay(10);
  /////////////////////////
  /* Begin Read Sequence */
  /////////////////////////
  /*
  Since we requested 2 bytes from the sensor we must
  read in 4 bytes. This includes the payload, checksum,
  and command status byte.
  */
  Wire.requestFrom(co2Addr, 4);
  byte i = 0;
  byte buffer[4] = {0, 0, 0, 0};
  /*
  Wire.available() is not nessessary. Implementation is obscure but we leave
  it in here for portability and to future proof our code
  */

  while(Wire.available()){
    
    buffer[i] = Wire.receive();
    i++;
  }

  ///////////////////////
  /* End Read Sequence */
  ///////////////////////

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
    
    // Success!
    digitalWrite(13, LOW);
    return co2_value;
  
  }else{
  
    // Failure!
    /*
    Checksum failure can be due to a number of factors,
    fuzzy electrons, sensor busy, etc.
    */
    digitalWrite(13, LOW);
    return 0;
  }
}



