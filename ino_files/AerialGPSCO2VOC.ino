#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <K30.h>
#include <HIH6130.h>
#include "LiquidCrystal.h"
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

// If you're using a GPS module:
// Connect the GPS Power pin to 5V
// Connect the GPS Ground pin to ground
// If using software serial (sketch example default):
//   Connect the GPS TX (transmit) pin to Digital 3
//   Connect the GPS RX (receive) pin to Digital 2
// If using hardware serial (e.g. Arduino Mega):
//   Connect the GPS TX (transmit) pin to Arduino RX1, RX2 or RX3
//   Connect the GPS RX (receive) pin to matching TX1, TX2 or TX3

// If you're using the Adafruit GPS shield, change 
// SoftwareSerial mySerial(3, 2); -> SoftwareSerial mySerial(8, 7);
// and make sure the switch is set to SoftSerial

// If using software serial, keep this line enabled
// (you can change the pin numbers to match your wiring):
SoftwareSerial mySerial(3, 2);

// If using hardware serial (e.g. Arduino Mega), comment out the
// above SoftwareSerial line, and enable this line instead
// (you can change the Serial number to match your wiring):

//HardwareSerial mySerial = Serial1;


Adafruit_GPS GPS(&mySerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences. 
#define GPSECHO  false

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

#define CO2I2CADDR 0x68

unsigned long POLLING_DELAY = 5000; //the polling interval in miliseconds
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
boolean CALIBRATING = false;

int LED_STATUS = HIGH;

void setup(){

  mac = xbee.getXBeeSerialNum(115200);
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum(115200);
  }
  Serial.begin(115200);
  Serial.println("INIT ARETAS BOARD SUCCESS");
  pinMode(13, OUTPUT);
  
   // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  // the nice thing about this code is you can have a timer0 interrupt go off
  // every 1 millisecond, and read data from the GPS for you. that makes the
  // loop code a heck of a lot easier!
  useInterrupt(true);

  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
  
  // set up the LCD's number of rows and columns: 
  lcd.begin(8, 2);
  // Print a message to the LCD.
  lcd.print("INIT 1");
  
}


// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
#endif
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

uint32_t timer = millis();

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
    trh.printTRH(POUT | CALIBRATING, mac);
    k30.printCO2(POUT | CALIBRATING, mac);
    
    //printLCD(currentMillis);
    
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
  
    // in case you are not using the interrupt above, you'll
  // need to 'hand query' the GPS, not suggested :(
  if (! usingInterrupt) {
    // read data from the GPS in the 'main loop'
    char c = GPS.read();
    // if you want to debug, this is a good time to do it!
    if (GPSECHO)
      if (c) Serial.print(c);
  }
  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    
    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", "); 
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      Serial.print("Location (in degrees, works with Google Maps): ");
      Serial.print(GPS.latitudeDegrees, 4);
      Serial.print(", "); 
      Serial.println(GPS.longitudeDegrees, 4);
      
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
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

