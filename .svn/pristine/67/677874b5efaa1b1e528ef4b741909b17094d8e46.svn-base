/********************************************************

 ********************************************************/
 
#include <Wire.h>
#include <XBeeFunc.h>
#include <SensorTypes.h>
#include <SensorCalibration.h>
#include <HIH6130.h>
#include <Thermostat.h>
#include <EEPROMAnything.h>
#include <EEAddresses.h>
#include <BridgeSerial.h>
#include <NewSoftSerial.h>


#define HEAT_RELAY_PIN 6

#define DEBUG

NewSoftSerial SoftSerial(3,2); //rxpin txpin

HIH6130 trh(&Serial);
TRH lastTRH = {0,0,0};
Thermostat T(&SoftSerial);
BridgeSerial _bridgeSerial(&Serial, &SoftSerial);
XBeeFunc xbee(&Serial);

unsigned long mac = 0;

unsigned long MIN_CYCLE_INTERVAL = 5000;
unsigned long REPORT_INTERVAL = 120000;
unsigned long CYCLE_START_TIME = 0;
unsigned long LAST_REPORT_TIME = 0;
boolean heatingState = false;
boolean _FIRSTRUN = true;

void setup()
{
  
  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  
  //fake it for now
  mac = 1234567890;
  
  CYCLE_START_TIME = millis();
  pinMode(HEAT_RELAY_PIN, OUTPUT);
  
  Serial.begin(9600);
  
  SoftSerial.begin(9600);
  SoftSerial.println("INIT ARETAS BOARD SUCCESS");
  
  SoftSerial.print("SYSTEM MAC [");
  SoftSerial.print(mac);
  SoftSerial.println("]");
  SoftSerial.print("INIT TIME:");
  SoftSerial.print(CYCLE_START_TIME);
  SoftSerial.print('\n');  
  
}

void loop(){
  
  unsigned long now = millis();
  
  if(_bridgeSerial.available() > 0){
    
#ifdef DEBUG
    SoftSerial.println("READING BUFFER");
#endif

    _bridgeSerial.readBuffer(&mac);
  }
  
  if((now - CYCLE_START_TIME) >= MIN_CYCLE_INTERVAL){
    
    CYCLE_START_TIME = now;
    
    trh.getTRH(&lastTRH); 
    heatingState = T.getHeatingState((double)lastTRH.temp);
    
    #ifdef DEBUG
      SoftSerial.print("TEMP: ");
      SoftSerial.print(lastTRH.temp);
      SoftSerial.print(" SETPOINT: ");
      SoftSerial.print(T.getSetPoint(), DEC);
      SoftSerial.print(" HEATING STATE: "); 
      SoftSerial.print(heatingState, DEC);
      SoftSerial.print('\n');
    #endif
    
    switch(heatingState){
      case 1:
        digitalWrite(HEAT_RELAY_PIN, HIGH);
        break;
        
      case 0:
        digitalWrite(HEAT_RELAY_PIN, LOW);
        break;
        
      default:
        break;
    }
    
  }
  
  if(((now - LAST_REPORT_TIME) > REPORT_INTERVAL) || (_FIRSTRUN == true)){
    
    if(_FIRSTRUN == true){
      _FIRSTRUN = false;
    }
    
    trh.printTRH(true, mac, &lastTRH);
    LAST_REPORT_TIME = now;
    
  }
  
}

