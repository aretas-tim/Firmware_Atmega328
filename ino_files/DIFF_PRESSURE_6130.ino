#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>

#define DPA_SENSOR_PIN A1
#define PIN_RED   5
#define PIN_GREEN 6
#define PIN_BLUE  3

//#define DEBUG

const byte DPA_SENSOR_TYPE = 0x63;

const byte NUM_SAMPLES = 3; //number of samples
const int SAMPLE_INTERVAL = 100; //interval between samples in miliseconds
unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long CYCLE_INTERVAL = 1000; //length of time that controls the sensor gas sensor read cycle

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder

boolean POUT = false; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = false;

const float DPA_P1 = 2.5219;
const float DPA_M = 0.0402;

unsigned long mac = 0;


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
  
  pinMode(DPA_SENSOR_PIN,INPUT);
  pinMode(13, OUTPUT);
  
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  digitalWrite(PIN_RED, HIGH);
  digitalWrite(PIN_GREEN, HIGH);
  digitalWrite(PIN_BLUE, HIGH);
  
  Serial.println("RED");
  analogWrite(PIN_RED, 0);
  delay(3000);
  digitalWrite(PIN_RED, HIGH);
  

  Serial.println("GREEN");  
  analogWrite(PIN_GREEN, 0);
  delay(3000);
  digitalWrite(PIN_GREEN, HIGH);

  
  Serial.println("BLUE");
  analogWrite(PIN_BLUE, 0);
  delay(3000);
  digitalWrite(PIN_BLUE, HIGH);
  
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
    
    //trh.printTRH(POUT, mac);
    getDPA(POUT | CALIBRATING);
    
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

void getDPA(boolean p){
  
   //do RH now
  float sensorValue = 0.0; 
  float tmp = 0.0;
  
  //get the average of a few readings
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(DPA_SENSOR_PIN);
    delay(10);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;
  
  float vin = (sensorValue / 1023.0) * 5.0;
  float ret = (vin - DPA_P1) / DPA_M;
  
  setDPColor(sensorValue);
  
  if(p == true){
  
    Serial.print(mac, DEC);
    Serial.print(",");
    Serial.print(DPA_SENSOR_TYPE, DEC);
    Serial.print(",");
    
    if(CALIBRATING == true){
      
      Serial.print(sensorValue,4);
      
    }else{
      
      Serial.print(ret,1);
      
    }
    Serial.print("\n");
  
  }
  
  delay(10);
  
}

void setDPColor(int sensorValue){
  
  int plc = sensorValue - 512;
  
  int r = 0;
  int g = 0;
  int b = 0;
  
  if(plc < 0){
    
    plc = abs(plc);
    
    r = plc / 2;
    g = (512 - plc) / 2;
    b = 0;
    
  }else{
    
    r = 0;
    g = (512 - plc) / 2;
    b = plc / 2;
    
  }
  
#ifdef DEBUG
  
  Serial.print("R:"); Serial.print(r);
  Serial.print("G:"); Serial.print(g);
  Serial.print("B:"); Serial.print(b);
  Serial.print('\n');
  
#endif 

  setColor(r, g, b);
  
}

void setColor(int red, int green, int blue){
  
  if(red < 35) red = 0;
  if(red > 255) red = 255;
  if(green < 35) green = 0;
  if(green > 255) green = 255;
  if(blue < 35) blue = 0;
  if(blue > 255) blue = 255;
  
  //invert everything for PWM
  red = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
  
  analogWrite(PIN_RED, red);
  analogWrite(PIN_GREEN, green);
  analogWrite(PIN_BLUE, blue);
  
}

