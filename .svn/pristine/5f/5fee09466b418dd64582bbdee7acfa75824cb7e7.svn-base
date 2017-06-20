#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>
#include <NewSoftSerial.h>

unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long BLINK_DELAY = 2000;
unsigned long CYCLE_INTERVAL = 2000; //length of time that controls the sensor gas sensor read cycle

unsigned long mac = 0;

XBeeFunc xbee(&Serial);
HIH6130 trh(&Serial);
NewSoftSerial oz(3,2); //rxpin txpin

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder

boolean POUT = true; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = true;

int LED_STATUS = HIGH;

unsigned long panID = 0x8EE;

boolean O3_AUTO_INIT = false;

void setup(){

  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  
  //now try and set the PANID
  //xbee.setPanID(panID);
  
  Serial.begin(9600);
  Serial.println("INIT ARETAS BOARD SUCCESS");
  
  oz.begin(19200);
  
  pinMode(13, OUTPUT);
  
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
  
    trh.printTRH(POUT, mac);
    getOzone(POUT | CALIBRATING);
    delay(1500);
    
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

boolean getOzone(boolean p){
  
  int index = 0;
  boolean found = false;
  int i = 0;
  int BUF_SZ = 20;
  
  byte buf[BUF_SZ];
  
  unsigned int temp_t;
  float temp;
  int s1_ozone_t = 0;
  int s2_ozone_t = 0;
  
  if(CALIBRATING == true){
  
    Serial.println("Getting Ozone...");
    
  }
  
  /* this overflows the timer so that we get a fresh reading (supposedly) 
  oz.print('{');
  oz.print('T');
  oz.print('3');
  oz.print('<');
  oz.print('}');
  delay(1000);
  oz.flush();
  */
  
  if(O3_AUTO_INIT == false){
    oz.print('{');
    oz.print('S');
    oz.print('}');
    delay(1000);
    oz.flush();
    O3_AUTO_INIT = true;
  }
  oz.print('{', BYTE);
  oz.print('M', BYTE);
  oz.print('}', BYTE);
  
  delay(100);  
  byte ib = 0;
  
  while(oz.available()){
    
    if(index < BUF_SZ){
      buf[index] = oz.read();
    }else{
      break;
    }
    index++;
  }
  
  if(index >= 20){
    
    //perform a couple more sanity checks
  
    //first make sure we have a "complete" response
    if(buf[0] != 0x7B){
      return false;
    }
    
    //check for the terminating }
    for(i = 0; i < BUF_SZ; i++){
      
      if(buf[i] == 0x7D){
        found = true;
      }
    }
    
    if(found == false){
     return false; 
    }
    
    //hypothetically, we now have a complete "packet"
    //we can now decode the T+RH+Ozone    
    //Master { M }
    //OZ-47 { M O31h O31l O32h O32l Rs1hl Rs1lh Rs1ll Rs2hl Rs2lh Rs2ll Thl Tlh Tll Hh Hl }
    //Temperature and humidity are measured and digital values
    //are sent to the MiCS-OZ-47 micro and then sent on the
    //serial bus within the measurement string:
    //Nibble name Thl Tlh Tll Hh Hl
    //Coded value 2 ; = 3 2
    //Hex value 2 B D 3 2
    //Dec value 701 50
    //Temperature = (Temperature measured /10) – 40 = (701/10) – 40 = 30.1°C
    //Temperature is coded between 0 and 1638 (-40 °C to 123.8 °C)
    /*
    Serial.print(buf[12], HEX);
    Serial.print(",");
    Serial.print(buf[13], HEX);
    Serial.print(",");
    Serial.print(buf[14], HEX);
    Serial.print('\n');  
    */
    
    temp_t = assembleTemp(buf[12], buf[13], buf[14]);
    temp = ((float)temp_t / 10) - 40;
    
    /*
    Serial.print(temp_t, HEX);
    Serial.print(",");
    Serial.print(temp);  
    Serial.print(",");
    */
    //get the ozone reading
    s1_ozone_t = assembleByte(buf[2], buf[3]);
    
    /*
    Serial.print(s1_ozone_t, DEC);    
    Serial.print(",");    
    */
    s2_ozone_t = assembleByte(buf[4], buf[5]);    
    
    /*
    Serial.print(s2_ozone_t, DEC);
    Serial.print('\n');
    */
    
    if(p == true){
      
      Serial.print(mac, DEC);
      Serial.print(",");
      Serial.print(O3_SENSOR_TYPE, DEC);
      Serial.print(",");
      Serial.print(s1_ozone_t, DEC);
      Serial.print('\n');
      
    }
    
  }
  if(CALIBRATING == true){
    Serial.println("Done...");
  }
  
  return false;
  
}

unsigned char assembleByte(unsigned char byteH, unsigned char byteL){
  
  unsigned char Nb;
  Nb = (byteH << 4) | (byteL & 0x0F);
  
  return Nb;

}

unsigned int assembleTemp(byte Thl, byte Tlh, byte Tll){
  
  unsigned int ret;
  ret = 0;
  
  Thl = Thl & 0x0F;
  ret = ret | Thl;
  ret = ret << 8;
  Tlh = Tlh & 0x0F;
  Tlh = Tlh << 4;
  ret = ret | Tlh;
  ret = ret | (Tll & 0x0F);
  
  return ret;
  
}

