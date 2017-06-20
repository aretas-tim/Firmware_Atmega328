#include <Wire.h>
#include <NewSoftSerial.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <Thermostat.h>
#include <EEPROM.h>
#include <BridgeSerial.h>
#include <NoiseMeter.h>
#include <math.h>

unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long BLINK_DELAY = 2000;
unsigned long CYCLE_INTERVAL = 2000; //length of time that controls the sensor gas sensor read cycle

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder

int LED_STATUS = HIGH;
boolean POUT = false;
boolean CALIBRATING = false;

unsigned long mac = 0;

float avgSPL = 0.0;

XBeeFunc xbee(&Serial);
NoiseMeter nm(0);

void setup(){

  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }

  Serial.begin(9600);
  Serial.println("INIT ARETAS BOARD SUCCESS");
  pinMode(13, OUTPUT);
  
}

void loop(){

  unsigned long currentMillis = millis();
  
  //main cycle interval (N seconds or so)
  if((currentMillis - cm0 > CYCLE_INTERVAL) || (cm0 == 0)) {
    
    nm.run(); //do the cycle polling for peakSPL 
    
    cm0 = currentMillis;

    if((currentMillis - pm0 > POLLING_DELAY) || (pm0 == 0)) {
     
      POUT = true;  
      //save the last time we polled the sensors
      pm0 = currentMillis; 
      
      BridgeSerial::sendValue(mac, SPL_PEAK_TYPE, nm.getPeakSPL(), &Serial);
      BridgeSerial::sendValue(mac, SPL_AVG_TYPE, nm.getSample(), &Serial);
      
    }else{
      
      POUT = false;
    
    }
    
    if(CALIBRATING & POUT){
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

