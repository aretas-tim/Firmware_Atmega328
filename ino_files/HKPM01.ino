#include <SoftwareSerial.h>
#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

SoftwareSerial __serial(2, 3); // RX, TX
XBeeFunc xbee(&Serial);
HIH6130 trh(&Serial);

#define OLED_RESET 5
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder

unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long BLINK_DELAY = 2000;
unsigned long CYCLE_INTERVAL = 2000; //length of time that controls the sensor gas sensor read cycle

boolean POUT = false; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = true;

int LED_STATUS = HIGH;

unsigned long panID = 0x8EE;
unsigned long mac = 0;

byte buffer[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

boolean GOTSTART1 = false;
boolean GOTSTART2 = false;
byte BUF_INDEX = 0;
byte BUF_SZ = 22;

TRH lastTRH = {0,0,0};
float VCC = 0.0;

void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  __serial.begin(9600);
  
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.display();
  delay(10000);
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(F("ARETAS"));
  display.println(F("PARTICLE"));
  display.println(F("SENSOR"));
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();
  
  ads.begin();

  Serial.println(F("INIT SUCCESS"));
  pinMode(13, OUTPUT);

}

void loop() {

  if (__serial.available()){ readData(); }
  
  unsigned long currentMillis = millis();
  int16_t results;
  
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
  
    trh.printTRH(POUT, mac);
    trh.getTRH(&lastTRH);
    
    float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

    results = ads.readADC_Differential_0_1();  
    
    VCC = results * multiplier;
    
    Serial.print("VCC:");
    Serial.println(VCC);
    
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

void readData() {

  byte c;

  boolean b = false;

  while (__serial.available()) {
    
    c = __serial.read();

    if ((c == 0x42) && (GOTSTART1 == false)) {

      GOTSTART1 = true;

    }

    if ((c == 0x4D) && (GOTSTART1 == true)) {

      GOTSTART2 = true;
      b = true;

    }

    if ((GOTSTART1 == true) && (GOTSTART2 == true) && (b == false)) {

      buffer[BUF_INDEX] = c;
      BUF_INDEX++;

    }

    b = false;

    if (BUF_INDEX >= 21) {

      GOTSTART1 = false;
      GOTSTART2 = false;
      BUF_INDEX = 0;

      parseBuffer(buffer);

      for (byte i = 0; i < 21; i++) {
        buffer[i] = 0;
      }

    }

  }


}

void parseBuffer(byte buffer[]) {
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  /**
  PM 1.0 concentration (ug/m^3)
  1. Data1 upper 8 bits
  2. Data1 lower 8 bits
  */
  byte  hi = 0;
  byte lo = 0;

  uint16_t result1 = 0;
  uint16_t result2 = 0;
  uint16_t result3 = 0;
  
  uint16_t result4 = 0;
  uint16_t result5 = 0;
  uint16_t result6 = 0;
  uint16_t result7 = 0;
  uint16_t result8 = 0;

  hi = buffer[2];
  lo = buffer[3]; // stores b2 as an unsigned value

  result1 = result1 + (hi);
  result1 = result1 << 8;
  result1 = result1 + (lo);

  Serial.print("PM 1.0 BUF [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result1); Serial.print("]"); Serial.println();
  display.print("PM 1.0 "); display.print(result1); display.print(" "); display.print((char)229); display.print("g/m^3"); display.println();

  hi = (buffer[4]);
  lo = (buffer[5]);

  result2 = result2 + (hi);
  result2 = result2 << 8;
  result2 = result2 + (lo);

  Serial.print("PM 2.5 BUF [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: [");  Serial.print(result2); Serial.print("]"); Serial.println();
  display.print("PM 2.5 "); display.print(result2); display.print(" "); display.print((char)229); display.print("g/m^3"); display.println();

  hi = (buffer[6]);
  lo = (buffer[7]); // stores b2 as an unsigned value

  result3 = result3 + (hi);
  result3 = result3 << 8;
  result3 = result3 + (lo);

  Serial.print("PM 10 BUF [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result3); Serial.print("]"); Serial.println();
  Serial.println();
  display.print("PM 10 "); display.print(result3); display.print(" "); display.print((char)229); display.print("g/m^3"); display.println();
  
  
  hi = (buffer[8]);
  lo = (buffer[9]); // stores b2 as an unsigned value

  result4 = result4 + (hi);
  result4 = result4 << 8;
  result4 = result4 + (lo);

  Serial.print("0.3um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result4); Serial.print("]"); Serial.println();
  
  hi = (buffer[10]);
  lo = (buffer[11]); // stores b2 as an unsigned value

  result5 = result5 + (hi);
  result5 = result5 << 8;
  result5 = result5 + (lo);

  Serial.print("0.5um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result5); Serial.print("]"); Serial.println();
  
  hi = (buffer[12]);
  lo = (buffer[13]); // stores b2 as an unsigned value

  result6 = result6 + (hi);
  result6 = result6 << 8;
  result6 = result6 + (lo);

  Serial.print("1.0um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result6); Serial.print("]"); Serial.println();
  
  hi = (buffer[14]);
  lo = (buffer[15]); // stores b2 as an unsigned value

  result7 = result7 + (hi);
  result7 = result7 << 8;
  result7 = result7 + (lo);

  Serial.print("2.5um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result7); Serial.print("]"); Serial.println();
  
  hi = (buffer[16]);
  lo = (buffer[17]); // stores b2 as an unsigned value

  result8 = result8 + (hi);
  result8 = result8 << 8;
  result8 = result8 + (lo);

  Serial.print("5.0um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result8); Serial.print("]"); Serial.println();
  
  hi = (buffer[18]);
  lo = (buffer[19]); // stores b2 as an unsigned value

  result9 = result9 + (hi);
  result9 = result9 << 8;
  result9 = result9 + (lo);

  Serial.print("10.0um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result9); Serial.print("]"); Serial.println();
  
  
  display.print("TEMP:"); display.print(lastTRH.temp); + display.print((char)248); display.print("C"); 
  display.println();
  display.print("RH:"); display.print(lastTRH.rh); + display.print('%'); 
  display.println();  
  display.display();
  
  drawIAQGraph(result1, result2, result3);

}

void drawIAQGraph(uint16_t value1, uint16_t value2, uint16_t value3 ){
  
  // Scale input to width of display:
  int w = map(value1, 0, 5000, 0, display.width());

  // Draw filled part of bar starting from left of screen:
  display.fillRect(0, 59, w, 5, 1);
  
  w = map(value2, 0, 5000, 0, display.width());
  display.fillRect(0, 53, w, 5, 1);
  
  w = map(value3, 0, 5000, 0, display.width());
  display.fillRect(0, 47 , w, 5, 1); 
  
  // Erase the area to the right of the bar:
  //display.fillRect(w, 0, display.width() - w, display.height(), 0);

  // Update the screen:
  display.display();
  
}
