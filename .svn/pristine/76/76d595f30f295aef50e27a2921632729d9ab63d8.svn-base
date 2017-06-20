#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>

#define C2442_VIN_PIN A0
#define C2442_HEATER_PIN 5
#define S1P_PIN 4
#define NO2_ANALOG_READ_PIN A1
#define NO2_HEATER_PIN 6
#define ASML_VIN_PIN A2
#define ASML_HEATER_PIN 7

const int NO2_HEATING_TIME = 40; //heat for xx milliseconds
const int ASML_HEATING_TIME = 300; //heat for xx ms
const int C2442_HEATING_TIME = 14; //heat for xx ms

const byte NUM_SAMPLES = 3; //number of samples
const int SAMPLE_INTERVAL = 100; //interval between samples in miliseconds
unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long CYCLE_INTERVAL = 2000; //length of time that controls the sensor gas sensor read cycle

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder

boolean POUT = false; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = true;

unsigned long mac = 0;
int LED_STATUS = HIGH;

XBeeFunc xbee(&Serial);
HIH6130 trh(&Serial);

void setup()  { 
  
  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  
  Serial.begin(9600);

  pinMode(NO2_HEATER_PIN, OUTPUT);
  pinMode(NO2_ANALOG_READ_PIN, INPUT);
  pinMode(ASML_HEATER_PIN, OUTPUT);
  pinMode(ASML_VIN_PIN, INPUT);
  pinMode(C2442_HEATER_PIN, OUTPUT);
  pinMode(S1P_PIN, OUTPUT);
  pinMode(C2442_VIN_PIN, INPUT);
  pinMode(13, OUTPUT);
  
  Serial.print("INIT ARETAS SENSOR BOARD SUCCESS");
  
}

void loop() {
  
  unsigned long currentMillis = millis();
  
  //main cycle interval (5 seconds or so)
  if((currentMillis - cm0 > CYCLE_INTERVAL) || (cm0 == 0)) {
    
    cm0 = currentMillis;

    if((currentMillis - pm0 > POLLING_DELAY) || (pm0 == 0)) {
     
      POUT = true;  
      //save the last time we polled the sensors
      pm0 = currentMillis; 
      
    }else{
      
      POUT = false;
    
    }  
    
    //getNO2Mics(POUT | CALIBRATING);
    //getCO2442(POUT | CALIBRATING);
    getCOASML(POUT | CALIBRATING);
    trh.printTRH(POUT, mac);
    
    if(CALIBRATING){
      Serial.print("\n"); //make the output easier to read
    }    
    //blink
    digitalWrite(13, LED_STATUS);
    
    if(LED_STATUS == LOW){
      LED_STATUS = HIGH;
    }else {
      LED_STATUS = LOW;
    }
    
  }

  
}

void getCOASML(boolean p){
  
  float pinValue = 0.0;
  float vin = 0.00;
  
  //power up the source for the heater
  digitalWrite(ASML_HEATER_PIN, HIGH);
  delay(ASML_HEATING_TIME); //let it heat up for a bit
  digitalWrite(ASML_HEATER_PIN, LOW);

  delay(900);

  //read the value 
  pinValue = analogRead(ASML_VIN_PIN);
  
  vin = (5.0/1023.0)*pinValue;
  //vin = 5 - vin;
  
  if(p == true){
    
    Serial.print(mac);
    Serial.print(",");
    Serial.print(CO_SENSOR_TYPE, DEC);
    Serial.print(",");
    
    if(CALIBRATING){
      //just print the raw vin
      Serial.print(vin);
    }else{
      Serial.print(calcASMLPPM(vin));
    }
  
    Serial.print("\n");
    
  }
  
  delay(50);
  
}

float calcASMLPPM(float vin){
  
  float ret = 0.0;  
  ret = vin;

  return ret;
  
}

void getNO2Mics(boolean p){
  
  float pinValue = 0.0;
  float vin = 0.0;
  
  //power up the source for the heater
  digitalWrite(NO2_HEATER_PIN, HIGH);   

  delay(NO2_HEATING_TIME); //let it heat up for a bit
  
  //read the value 
  pinValue = analogRead(NO2_ANALOG_READ_PIN);
  vin = (5.0/1023.0)*pinValue;
  
  if(p == true){
    
    Serial.print(mac);
    Serial.print(",");
    Serial.print(NO2_SENSOR_TYPE, DEC);
    Serial.print(",");
    
    if(CALIBRATING){
      //just print the raw vin
      Serial.print(vin);
    }else{
      Serial.print(calcNO2PPM(vin));
    }
  
    Serial.print("\n");
    
  }
  
  //turn the heater off
  digitalWrite(NO2_HEATER_PIN, LOW);
  
  //sleep for n milliseconds... this is probably less than useful
  delay(100);
  
}

float calcNO2PPM(float vin){
  
  float ret = 0.0;  

  ret = vin;

  return ret;
  
}

void getCO2442(boolean p){
  
  float pinValue = 0.0;
  float vin = 0.00;
  
  //power up the source for the heater
  digitalWrite(C2442_HEATER_PIN, HIGH);   
  delay(C2442_HEATING_TIME); //let it heat up for a bit
  digitalWrite(C2442_HEATER_PIN, LOW);

  delayMicroseconds(980000);
  digitalWrite(S1P_PIN, HIGH);
  delayMicroseconds(2500);
  //read the value 
  pinValue = analogRead(C2442_VIN_PIN);
  
  delayMicroseconds(2500);
  digitalWrite(S1P_PIN, LOW);  
  vin = (5.0/1023.0)*pinValue;
  
  if(p == true){
    
    Serial.print(mac);
    Serial.print(",");
    Serial.print(CO_SENSOR_TYPE, DEC);
    Serial.print(",");
    
    if(CALIBRATING){
      //just print the raw vin
      Serial.print(vin);
    }else{
      Serial.print(calcCOPPM(vin));
    }
  
    Serial.print("\n");
    
  }
  
  //sleep for n milliseconds this is probably less than useful
  delay(100);
  
}

float calcCOPPM(float vin){
  
  float ret = 0.0;  
  
  ret = vin;

  return ret;
  
}

