/*

Basic temperature sensor implementation

Temporary line format will be 
SENSOR TYPE,TEMP C\n
 
 */
 
#include <Wire.h>

int tempSensorPin = A0;    // select the input pin for the data line on the sensor 
int rhSensorPin = A1;

const byte TEMP_SENSOR_TYPE = 0xF6;
const byte RH_SENSOR_TYPE = 0xF8;
const byte CO2_SENSOR_TYPE = 0xB5;

const byte NUM_SAMPLES = 3; //number of samples
const int SAMPLE_INTERVAL = 100; //interval between samples in miliseconds

unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long BLINK_DELAY = 2000;

int co2Addr = 0x68;

long mac = 1082014889;
long pm0 = 0; //placeholders for last time
long pm1 = 0;
int LED_STATUS = LOW;


void setup() {

  Wire.begin();
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(tempSensorPin, INPUT);
  pinMode(rhSensorPin, INPUT);
  Serial.print("INIT ARETAS SENSOR BOARD SUCCESS\n");
  
}

void loop() {  
 
 unsigned long currentMillis = millis();
 
 long tmp = 0;
 float sensorValue = 0.0; 

 if((currentMillis - pm0 > POLLING_DELAY) || (pm0 == 0)) {
    
    //save the last time we polled the sensors
    pm0 = currentMillis;  
    
    sensorValue = analogRead(tempSensorPin);
    
    Serial.print(mac);
    Serial.print(",");
    Serial.print(TEMP_SENSOR_TYPE, DEC);
    Serial.print(",");
    Serial.print(getTemp(sensorValue));
    Serial.print("\n");
    
    delay(100);
    
    //do RH now
    sensorValue = 0.0;  
    sensorValue = analogRead(rhSensorPin);
    
    Serial.print(mac, DEC);
    Serial.print(",");
    Serial.print(RH_SENSOR_TYPE, DEC);
    Serial.print(",");
    Serial.print(getRH(sensorValue));
    Serial.print("\n");
    
    delay(100);
    
    //do the CO2 cycle
    int co2Value = readCO2();
    
    if(co2Value > 0){
      
      Serial.print(mac, DEC);
      Serial.print(",");
      Serial.print(CO2_SENSOR_TYPE, DEC);
      Serial.print(",");
      Serial.print(co2Value);
      Serial.print("\n");
      
    }else{
      Serial.println("Checksum failed / Communication failure");
    }
    
  
  }
  
   if((currentMillis - pm1 > BLINK_DELAY) || (pm1 == 0)) {
    
    //save the last time we polled the sensors
    pm1 = currentMillis;  
    digitalWrite(13, LED_STATUS);
    
    if(LED_STATUS == LOW){
      LED_STATUS = HIGH;
    }else {
      LED_STATUS = LOW;
    }
  
   }
  
}

float getTemp(float sensorValue){
  
  float vin = (sensorValue / 1023.0) * 5.0;
  float ret = ((vin * 1000.0) - 500.0) / 10.0;
  return ret;
  
}

float getRH(float sensorValue){
  
  float vin = (sensorValue / 1023.0) * 5.0;
  //float ret = ((vin - 0.8)/0.031);
  float ret = (vin - 0.958)/0.0307;
  return ret;
  
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
  
  //begin write 
  Wire.beginTransmission(co2Addr);
  Wire.send(0x22);
  Wire.send(0x00);
  Wire.send(0x08);
  Wire.send(0x2A);
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
