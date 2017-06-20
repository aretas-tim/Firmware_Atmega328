#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <K30.h>
#include <NewSoftSerial.h>
#include <HIH6130.h>

#define CO2I2CADDR 0x68

#define CO_SENSOR_PIN A0
#define NO2_SENSOR_PIN A1

boolean O3_AUTO_INIT = false;

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

const int CO_P1 = 513;
const float CO_M = 0.56;

const int NO2_P1 = 510;
const float NO2_M = -29.0;

int PPD_INPUT_PIN = 5;
unsigned long PPD_DURATION;
unsigned long PPD_STARTTIME;
unsigned long PPD_SAMPLETIME_MS = 30000;
unsigned long PPD_LOWPULSEOCCUPANCY= 0;
float PPD_RATIO = 0;
float PPD_CONCENTRATION = 0;

XBeeFunc xbee(&Serial);
HIH6130 trh(&Serial);
K30 k30(&Serial, CO2I2CADDR);
NewSoftSerial oz(3,2); //rxpin txpin

int LED_STATUS = HIGH;

void setup()  { 
  
  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  
  Serial.begin(9600);
  
  pinMode(CO_SENSOR_PIN,INPUT);
  pinMode(NO2_SENSOR_PIN,INPUT);
  pinMode(PPD_INPUT_PIN, INPUT);
  pinMode(13, OUTPUT);
  
  oz.begin(19200);
  
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
    
    trh.printTRH(POUT, mac);
    k30.printCO2(POUT, mac);    
    getCO(POUT | CALIBRATING);
    getNO2(POUT | CALIBRATING);
    getOzone(POUT | CALIBRATING);
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

void getNO2(boolean p){
  
  int sensorValue = 0; 
  int tmp = 0;
  
  //get the average of a few readings
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(NO2_SENSOR_PIN);
    delay(100);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;
  
  float ret = (float)(sensorValue - NO2_P1) / NO2_M;
  
  if(ret < 0){ ret = 0; }
  
  if(p == true){
  
    Serial.print(mac, DEC);
    Serial.print(",");
    Serial.print(NO2_SENSOR_TYPE, DEC);
    Serial.print(",");
    
    if(CALIBRATING == true){
      
      Serial.print(sensorValue);
      
    }else{
      
      Serial.print(ret, 1);
      
    }
    Serial.print("\n");
  
  }
  
  delay(10);
  
}


void getCO(boolean p){
  
  int sensorValue = 0;
  int tmp = 0;
  
  //get the average of a few readings
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(CO_SENSOR_PIN);
    delay(100);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;
  
  float ret = (float)(sensorValue - CO_P1) / CO_M;
  
  if(ret < 0){ ret = 0; }
  
  if(p == true){
  
    Serial.print(mac, DEC);
    Serial.print(",");
    Serial.print(CO_SENSOR_TYPE, DEC);
    Serial.print(",");
    
    if(CALIBRATING == true){
      
      Serial.print(sensorValue);
      
    }else{
      
      Serial.print(ret, 1);
      
    }
    Serial.print("\n");
  
  }
  
  delay(10);
  
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
  
  //zero it to be sure
  for(i = 0; i < BUF_SZ; i++){
    buf[i] = 0;
  }
  
  if(CALIBRATING == true){
  
    Serial.println("Getting Ozone...");
    
  }
  
  oz.flush();
  delay(20);
  
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
  
  delay(500);  
  byte ib = 0;
  
  while(oz.available()){
    
    if(index < BUF_SZ){
      buf[index] = oz.read();
    }else{
      break;
    }
    index++;
  }
  
  oz.flush();
  
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
    
    if((p == true) || (CALIBRATING == true)){
      
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

