#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>

#define CO_SENSOR_PIN A0
#define NO2_SENSOR_PIN A1

const byte NUM_SAMPLES = 4; //number of samples
const int SAMPLE_INTERVAL = 100; //interval between samples in miliseconds

unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long CYCLE_INTERVAL = 2000; //length of time that controls the sensor gas sensor read cycle

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder

boolean POUT = false; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = true;

unsigned long mac = 0;

const int CO_P1 = 511;
const float CO_M = 0.0073;

const int NO2_P1 = 512;
const float NO2_M = 0.0073;

XBeeFunc xbee(&Serial);
HIH6130 trh(&Serial);

int LED_STATUS = HIGH;

void setup()  { 
  
  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  
  Serial.begin(9600);
  
  pinMode(CO_SENSOR_PIN,INPUT);
  pinMode(NO2_SENSOR_PIN,INPUT);
  pinMode(13, OUTPUT);
  
  Serial.print("INIT ARETAS SENSOR BOARD SUCCESS\n");
  
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
     
    getCO(POUT | CALIBRATING);
    getNO2(POUT | CALIBRATING);
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

void getNO2(boolean p){
  
  int sensorValue = 0; 
  int tmp = 0;
  
  //get the average of a few readings
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(NO2_SENSOR_PIN);
    delay(100);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;
  
  float ret = (float)(sensorValue - NO2_P1) / NO2_M;
  
  if(ret < 0){ ret = 0; }
  
  if(p == true){
  
    Serial.print(mac, DEC);
    Serial.print(",");
    Serial.print(NO2_SENSOR_TYPE, DEC);
    Serial.print(",");
    
    if(CALIBRATING == true){
      
      Serial.print(sensorValue);
      
    }else{
      
      Serial.print(ret, 1);
      
    }
    Serial.print("\n");
  
  }
  
  delay(10);
  
}


void getCO(boolean p){
  
  int sensorValue = 0;
  int tmp = 0;
  
  //get the average of a few readings
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(CO_SENSOR_PIN);
    delay(100);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;
  
  float ret = (float)(sensorValue - CO_P1) / CO_M;
  
  if(ret < 0){ ret = 0; }
  
  if(p == true){
  
    Serial.print(mac, DEC);
    Serial.print(",");
    Serial.print(CO_SENSOR_TYPE, DEC);
    Serial.print(",");
    
    if(CALIBRATING == true){
      
      Serial.print(sensorValue);
      
    }else{
      
      Serial.print(ret, 1);
      
    }
    Serial.print("\n");
  
  }
  
  delay(10);
  
}

