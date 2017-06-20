#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RAD_INPIN 8
#define XBEE_SLEEP_PIN 2
#define LED1PIN 3
#define LED2PIN 5
#define I2CPWRPIN 4

unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long BLINK_DELAY = 2000;
unsigned long CYCLE_INTERVAL = 2000; //length of time that controls the sensor gas sensor read cycle
unsigned long LCD_PRINT_INTERVAL = 4000;
unsigned long mac = 0;

XBeeFunc xbee(&Serial);
HIH6130 trh(&Serial);
TRH lastTRH = {0,0,0};
float lastCPM = 0.0;

#define OLED_RESET 10
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("ERR 10");
#endif

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder
unsigned long lm0 = 0; //lcd cycle millis placeholder
byte lcdPos = 0;

boolean POUT = true; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = false;

int LED_STATUS = HIGH;

unsigned long duration = 0;
unsigned long accumulator = 0;

unsigned long thisTime = 0;
unsigned long lastTime = 0;

boolean ledState;

void setup() {

  pinMode(XBEE_SLEEP_PIN,OUTPUT);
  digitalWrite(XBEE_SLEEP_PIN, LOW);  
  delay(1000);

  mac = xbee.getXBeeSerialNum();
  
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  
  Serial.begin(9600);
  Serial.println(F("INIT 1"));
  
  pinMode(13, OUTPUT);
  pinMode(RAD_INPIN, INPUT);
  
  //shut off the xbee
  digitalWrite(XBEE_SLEEP_PIN, HIGH);
  
  pinMode(I2CPWRPIN, OUTPUT);    //turn on i2c power
  digitalWrite(I2CPWRPIN, HIGH);  //turn on i2c power
  
  pinMode(LED1PIN, OUTPUT);
  pinMode(LED2PIN, OUTPUT);
  
  int i = 0;
  for(i = 0; i < 255; i++){
    analogWrite(LED1PIN, i);
    analogWrite(LED2PIN, i);
    delay(25);
  }
  
  Serial.println(F("INIT 2"));
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.display();
  delay(10000);
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(F("ARETAS"));
  display.println(F("RADIATION"));
  display.println(F("MONITOR"));
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();
  
  duration = 0;
  accumulator = 0;
  thisTime = 0;
  lastTime = 0;
  ledState = false;

}

void loop() {
  
  unsigned long currentMillis = millis();
  
  //main cycle interval (5 seconds or so ... going to drift on account of our pulseIn duration)
  if((currentMillis - cm0 > CYCLE_INTERVAL) || (cm0 == 0)) {
    
    cm0 = currentMillis;

    if((currentMillis - pm0 > POLLING_DELAY) || (pm0 == 0)) {
      
      //turn on the xbee
      digitalWrite(XBEE_SLEEP_PIN, LOW);
      delay(1000);
     
      POUT = true;  
      //save the last time we polled the sensors
      pm0 = currentMillis; 
      
      printRad(currentMillis);
      trh.printTRH(POUT, mac);
      
      //shut off the xbee
      digitalWrite(XBEE_SLEEP_PIN, HIGH);
      
    }else{
      
      POUT = false;
    
    }
    
    trh.getTRH(&lastTRH); 
    
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
  
  //the timeout is 500 microseconds (the longest pulse we expect to see)
  duration = pulseIn(RAD_INPIN, HIGH, 500);
  
  if(duration != 0){
    
    accumulator++;
    
  }
  
}

void printRad(unsigned long now){

  //total number of milliseconds that have elapses since we last printed RAD
  float duration = now - lastTime;
  float CPM = (float)accumulator / ((duration / 1000)/60);
  lastCPM = CPM;
  
  Serial.print(mac);
  Serial.print(",");
  Serial.print(0x88, DEC);
  Serial.print(",");
  Serial.print(CPM);
  Serial.print("\n");
  
  lastTime = now;
  accumulator = 0;
  
}

void printLCD(unsigned long now){
  
  if(((now - lm0) > LCD_PRINT_INTERVAL) || (lm0 == 0)) {
    
    lm0 = now;
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    
    display.print("RAD "); 
    display.print(lastCPM); 
    display.print(" "); 
    display.print((char)229); 
    display.print(""); 
    display.println();
  
    display.print("TEMP:"); 
    display.print(lastTRH.temp);
    display.print((char)248); 
    display.print("C"); 
    display.println();
    
    display.print("RH:"); 
    display.print(lastTRH.rh);
    display.print('%'); 
    display.println(); 
    
    
    display.display();
    
    // Scale input to width of display:
    int w = map(lastCPM, 0, 5000, 0, display.width());

    // Draw filled part of bar starting from left of screen:
    display.fillRect(0, 59, w, 5, 1);
  
    w = map(lastTRH.temp, 0, 150, 0, display.width());
    display.fillRect(0, 53, w, 5, 1);
  
    w = map(lastTRH.rh, 0, 100, 0, display.width());
    display.fillRect(0, 47 , w, 5, 1); 
  
    // Erase the area to the right of the bar:
    //display.fillRect(w, 0, display.width() - w, display.height(), 0);

    // Update the screen:
    display.display();
   
    }
    
  }


  
