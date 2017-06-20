#include <SoftwareSerial.h>
#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>

//#define DEBUG

SoftwareSerial __serial(3, 2); // RX, TX
XBeeFunc xbee(&Serial);
HIH6130 trh(&Serial);

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder

unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long BLINK_DELAY = 2000;
unsigned long CYCLE_INTERVAL = 2000; //length of time that controls the sensor gas sensor read cycle

boolean POUT = false; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = false;

unsigned long panID = 0x8EE;
unsigned long mac = 0;

byte buffer[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

boolean GOTSTART1 = false;
boolean GOTSTART2 = false;
byte BUF_INDEX = 0;
byte BUF_SZ = 22;

TRH lastTRH = {0,0,0};
float VCC = 0.0;

int LED_STATUS = HIGH;

uint16_t last_PM_1 = -1;
uint16_t last_PM_25 = -1;
uint16_t last_PM_10 = -1;

uint16_t last_u03 = -1;
uint16_t last_u05 = -1;
uint16_t last_u10 = -1;
uint16_t last_u25 = -1;
uint16_t last_u50 = -1;
uint16_t last_u100 = -1;

void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  __serial.begin(9600);
  
  mac = xbee.getXBeeSerialNum();
  
  //try twice.. for some reason it fails occasionally
  
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }

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
      
      if(mac == 0){
        mac = xbee.getXBeeSerialNum();
      }
      
    }else{
      
      POUT = false;
    }
  
    trh.printTRH(POUT, mac);
    trh.getTRH(&lastTRH);
    if(CALIBRATING | POUT){
      printPM();
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

void printPM(){
  
  Serial.print(mac); Serial.print(","); Serial.print(0x20, DEC); Serial.print(","); Serial.print(last_PM_1); Serial.print('\n');
  Serial.print(mac); Serial.print(","); Serial.print(0x21, DEC); Serial.print(","); Serial.print(last_PM_25); Serial.print('\n');
  Serial.print(mac); Serial.print(","); Serial.print(0x22, DEC); Serial.print(","); Serial.print(last_PM_10); Serial.print('\n');
  Serial.print(mac); Serial.print(","); Serial.print(0x23, DEC); Serial.print(","); Serial.print(last_u03); Serial.print('\n');
  Serial.print(mac); Serial.print(","); Serial.print(0x24, DEC); Serial.print(","); Serial.print(last_u05); Serial.print('\n');
  Serial.print(mac); Serial.print(","); Serial.print(0x25, DEC); Serial.print(","); Serial.print(last_u10); Serial.print('\n');
  Serial.print(mac); Serial.print(","); Serial.print(0x26, DEC); Serial.print(","); Serial.print(last_u25); Serial.print('\n');
  Serial.print(mac); Serial.print(","); Serial.print(0x27, DEC); Serial.print(","); Serial.print(last_u50); Serial.print('\n');
  Serial.print(mac); Serial.print(","); Serial.print(0x28, DEC); Serial.print(","); Serial.print(last_u100); Serial.print('\n');
  
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
  uint16_t result9 = 0;

  hi = buffer[2];
  lo = buffer[3]; // stores b2 as an unsigned value

  result1 = result1 + (hi);
  result1 = result1 << 8;
  result1 = result1 + (lo);
  
#ifdef DEBUG
  Serial.print("PM 1.0 BUF [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result1); Serial.print("]"); Serial.println();
#endif

  hi = (buffer[4]);
  lo = (buffer[5]);

  result2 = result2 + (hi);
  result2 = result2 << 8;
  result2 = result2 + (lo);
  
#ifdef DEBUG
  Serial.print("PM 2.5 BUF [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: [");  Serial.print(result2); Serial.print("]"); Serial.println();
#endif
  
  hi = (buffer[6]);
  lo = (buffer[7]); // stores b2 as an unsigned value

  result3 = result3 + (hi);
  result3 = result3 << 8;
  result3 = result3 + (lo);
  
#ifdef DEBUG
  Serial.print("PM 10 BUF [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result3); Serial.print("]"); Serial.println();
#endif
  
  hi = (buffer[8]);
  lo = (buffer[9]); // stores b2 as an unsigned value

  result4 = result4 + (hi);
  result4 = result4 << 8;
  result4 = result4 + (lo);
  
#ifdef DEBUG
  Serial.print("0.3um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result4); Serial.print("]"); Serial.println();
#endif

  hi = (buffer[10]);
  lo = (buffer[11]); // stores b2 as an unsigned value

  result5 = result5 + (hi);
  result5 = result5 << 8;
  result5 = result5 + (lo);

#ifdef DEBUG
  Serial.print("0.5um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result5); Serial.print("]"); Serial.println();
#endif

  hi = (buffer[12]);
  lo = (buffer[13]); // stores b2 as an unsigned value

  result6 = result6 + (hi);
  result6 = result6 << 8;
  result6 = result6 + (lo);

#ifdef DEBUG
  Serial.print("1.0um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result6); Serial.print("]"); Serial.println();
#endif

  hi = (buffer[14]);
  lo = (buffer[15]); // stores b2 as an unsigned value

  result7 = result7 + (hi);
  result7 = result7 << 8;
  result7 = result7 + (lo);

#ifdef DEBUG
  Serial.print("2.5um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result7); Serial.print("]"); Serial.println();
#endif

  hi = (buffer[16]);
  lo = (buffer[17]); // stores b2 as an unsigned value

  result8 = result8 + (hi);
  result8 = result8 << 8;
  result8 = result8 + (lo);
  
#ifdef DEBUG
  Serial.print("5.0um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result8); Serial.print("]"); Serial.println();
#endif

  hi = (buffer[18]);
  lo = (buffer[19]); // stores b2 as an unsigned value

  result9 = result9 + (hi);
  result9 = result9 << 8;
  result9 = result9 + (lo);
  
#ifdef DEBUG
  Serial.print("10.0um particle count in 0.1L volume [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result9); Serial.print("]"); Serial.println();
  Serial.print('\n');
#endif


  last_PM_1 = result1;
  last_PM_25 = result2;
  last_PM_10 = result3;
  
  last_u03 = result4;
  last_u05 = result5;
  last_u10 = result6;
  last_u25 = result7;
  last_u50 = result8;
  last_u100 = result9;
  

}
