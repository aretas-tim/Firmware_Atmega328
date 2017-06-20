#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>
#include <K30.h>

#define VOC_SENSOR_PIN A2
#define CO2I2CADDR 0x68

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

const float VOC_P1 = 0.05;
const float VOC_M = 0.0073;

XBeeFunc xbee(&Serial);
HIH6130 trh(&Serial);
K30 k30(&Serial, CO2I2CADDR);

int LED_STATUS = HIGH;

void setup()  { 
  
  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  
  Serial.begin(9600);
  
  pinMode(VOC_SENSOR_PIN,INPUT);
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
     
    getVOC(POUT | CALIBRATING);
    trh.printTRH(POUT, mac);
    k30.printCO2(POUT, mac);
    
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

void getVOC(boolean p){
  
   //do RH now
  float sensorValue = 0.0; 
  float tmp = 0.0;
  
  //get the average of a few readings
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(VOC_SENSOR_PIN);
    delay(10);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;
  
  float vin = (sensorValue / 1023.0) * 5.0;
  float ret = (vin - VOC_P1) / VOC_M;
  
  if(ret < 0){ ret = 0; }
  
  if(p == true){
  
    Serial.print(mac, DEC);
    Serial.print(",");
    Serial.print(VOC_SENSOR_TYPE, DEC);
    Serial.print(",");
    
    if(CALIBRATING == true){
      
      Serial.print(vin);
      
    }else{
      
      Serial.print(ret);
      
    }
    Serial.print("\n");
  
  }
  
  delay(10);
  
}

