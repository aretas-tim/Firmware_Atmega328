#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>

#define VOC_SENSOR_PIN A2

const byte NUM_SAMPLES = 4; //number of samples
const int SAMPLE_INTERVAL = 100; //interval between samples in miliseconds

unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long CYCLE_INTERVAL = 4000; //length of time that controls the sensor gas sensor read cycle

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder

boolean POUT = false; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = false;

unsigned long mac = 0;

const float VOC_P1 = 0.05;
const float VOC_M = 0.0073;

int PPD_INPUT_PIN = 9;
unsigned long PPD_DURATION;
unsigned long PPD_STARTTIME;
unsigned long PPD_SAMPLETIME_MS = 30000;
unsigned long PPD_LOWPULSEOCCUPANCY= 0;
float PPD_RATIO = 0;
float PPD_CONCENTRATION = 0;

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
  
  pinMode(VOC_SENSOR_PIN,INPUT);
  pinMode(PPD_INPUT_PIN, INPUT);
  pinMode(13, OUTPUT);
  
  PPD_LOWPULSEOCCUPANCY = 0;
  PPD_STARTTIME = millis();
  
  Serial.print("INIT ARETAS SENSOR BOARD SUCCESS\n");
  
}

void loop() {
  
  unsigned long currentMillis = millis();

  //ppd needs a different cycle 
  getPPD(POUT | CALIBRATING);
  
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
    printPPD(POUT | CALIBRATING);
   
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

void printPPD(boolean p){
  
  if(p == true){
  
      Serial.print(mac, DEC);
      Serial.print(",");
      Serial.print(PM_SENSOR_TYPE, DEC);
      Serial.print(",");
    
      if(CALIBRATING == true){
      
        Serial.print(PPD_LOWPULSEOCCUPANCY);
        Serial.print(",");
        Serial.print(PPD_RATIO);
        Serial.print(",");
        Serial.println(PPD_CONCENTRATION);
      
      }else{
      
        Serial.println(PPD_CONCENTRATION);
      
      }
    
      Serial.print("\n");
      
    }
}

void getPPD(boolean p){
  
  PPD_DURATION = pulseIn(PPD_INPUT_PIN, LOW);
  PPD_LOWPULSEOCCUPANCY = PPD_LOWPULSEOCCUPANCY + PPD_DURATION;
  
  if ((millis()-PPD_STARTTIME) > PPD_SAMPLETIME_MS){
    
    PPD_RATIO = PPD_LOWPULSEOCCUPANCY/(PPD_SAMPLETIME_MS * 10.0);  // Integer percentage 0=>100
    PPD_CONCENTRATION = (1.1 * pow(PPD_RATIO,3)) + (-3.8 * pow(PPD_RATIO,2)) + (520 * PPD_RATIO) + 0.62; // using spec sheet curve
    
    if(PPD_RATIO == 0){
      PPD_CONCENTRATION = 0;
    }
    
    PPD_LOWPULSEOCCUPANCY = 0;
    PPD_STARTTIME = millis();
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






