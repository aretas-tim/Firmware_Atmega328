#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <K30.h>
#include <HIH6130.h>
#include "LiquidCrystal.h"

#define CO2I2CADDR 0x68

unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long BLINK_DELAY = 2000;
unsigned long CYCLE_INTERVAL = 2000; //length of time that controls the sensor gas sensor read cycle
unsigned long LCD_PRINT_INTERVAL = 4000;
unsigned long mac = 0;

XBeeFunc xbee(&Serial);
K30 k30(&Serial, CO2I2CADDR);
HIH6130 trh(&Serial);
LiquidCrystal lcd(0);
TRH lastTRH = {0,0,0};

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder
unsigned long lm0 = 0; //lcd cycle millis placeholder
byte lcdPos = 0;

boolean POUT = true; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = true;

int LED_STATUS = HIGH;

void setup(){

  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  Serial.begin(9600);
  Serial.println("INIT ARETAS BOARD SUCCESS");
  pinMode(13, OUTPUT);
  
  // set up the LCD's number of rows and columns: 
  lcd.begin(8, 2);
  // Print a message to the LCD.
  lcd.print("INIT 1");
  
}

void loop(){
  
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
    
    trh.getTRH(&lastTRH); 
    trh.printTRH(POUT, mac);
    k30.printCO2(POUT, mac);
    
    printLCD(currentMillis);
    
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

void printLCD(unsigned long now){
  
  if((now - lm0 > LCD_PRINT_INTERVAL) || (lm0 == 0)) {
    
    lm0 = now;
    
    switch(lcdPos){
      
      case 0:
       //print T
       lcd.clear();
       lcd.setCursor(0, 0);
       lcd.print("TEMP:");
       lcd.setCursor(0,1);
       lcd.print(lastTRH.temp, DEC);
       lcd.setCursor(6,1);
       lcd.print((char)223);
       lcd.setCursor(7,1);
       lcd.print("C");

       lcdPos = 1;
       break;
       
      case 1:
       //print RH
       lcd.clear();
       lcd.setCursor(0, 0);
       lcd.print("RH:");
       lcd.setCursor(0,1);
       lcd.print(lastTRH.rh, DEC);       
       lcdPos = 2;
       break;
      
      case 2: 
       //print CO2
       lcd.clear();
       lcd.setCursor(0, 0);
       lcd.print("CO2:");
       lcd.setCursor(0,1);
       lcd.print(k30.readCO2(), DEC);
       lcd.setCursor(5,1);
       lcd.print("PPM");
       lcdPos = 0;
      
       break;
      
      default:
       lcdPos = 0;
       break;
       
    }
    
  }
  
}
