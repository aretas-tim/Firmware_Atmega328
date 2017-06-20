#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>
#include <K30.h>
#include <SoftwareSerial.h>
#include <Adafruit_ADS1015.h>

#define PM_EN_PIN 2
#define CO2I2CADDR 0x68

unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long BLINK_DELAY = 2000;
unsigned long CYCLE_INTERVAL = 2000; //length of time that controls the sensor gas sensor read cycle

unsigned long mac = 0;

XBeeFunc xbee(&Serial);
HIH6130 trh(&Serial);
SoftwareSerial oz(6,7); //rxpin txpin
SoftwareSerial pm(4,3);
K30 k30(&Serial, CO2I2CADDR);
Adafruit_ADS1115 ads;   

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder

boolean POUT = true; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = false;

int LED_STATUS = HIGH;

unsigned long panID = 0x8EE;

boolean O3_AUTO_INIT = false;

byte buffer[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

boolean GOTSTART1 = false;
boolean GOTSTART2 = false;
byte BUF_INDEX = 0;
byte BUF_SZ = 22;

int lastPM1 = -1;
int lastPM2_5 = -1;
int lastPM10 = -1;

const byte NUM_SAMPLES = 3; //number of samples
const int SAMPLE_INTERVAL = 100; //interval between samples in miliseconds

const boolean AVG = false;

const byte voc_read_queue_sz = 60;

int voc_read_queue[voc_read_queue_sz] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

byte voc_index = 0;

const float VOC_P1 = 0.06; //P2 = 1.52
const float VOC_M = 714.28;
const float VOC_B = -42.857;


void setup(){

  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  
  //now try and set the PANID
  //xbee.setPanID(panID);
  
  Serial.begin(9600);
  pm.begin(9600);
  Serial.println("INIT");
  
  pinMode(13, OUTPUT);
  pinMode(PM_EN_PIN, OUTPUT);
  digitalWrite(PM_EN_PIN, LOW);
  
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
  
    trh.printTRH(POUT | CALIBRATING, mac);
    getOzone(POUT | CALIBRATING);
    readPMData();
    k30.printCO2(POUT | CALIBRATING, mac);
    getVOC(POUT | CALIBRATING, AVG);
    
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

void readPMData(){

  digitalWrite(PM_EN_PIN, HIGH);
  pm.begin(9600);
  delay(2000);
  if(pm.available()){
    readData();
  }
  delay(1000);
  digitalWrite(PM_EN_PIN, LOW);
  
   if((POUT == true) || (CALIBRATING == true)){
      
      Serial.print(mac, DEC);
      Serial.print(",");
      Serial.print(0x20, DEC);
      Serial.print(",");
      Serial.print(lastPM1, DEC);
      Serial.print('\n');
      
      Serial.print(mac, DEC);
      Serial.print(",");
      Serial.print(0x21, DEC);
      Serial.print(",");
      Serial.print(lastPM2_5, DEC);
      Serial.print('\n');
      
      Serial.print(mac, DEC);
      Serial.print(",");
      Serial.print(0x22, DEC);
      Serial.print(",");
      Serial.print(lastPM10, DEC);
      Serial.print('\n');
  }
  
}

boolean getOzone(boolean p){
  
  oz.begin(19200);
  
  int index = 0;
  boolean found = false;
  int i = 0;
  int BUF_SZ = 32;
  
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
  
    Serial.println("O3 S");
    
  }
  
  //oz.flush();
  int inc = 0;
  while(oz.available()){
    oz.read();
    inc++;
    if(inc > 128) break;
  }
  delay(20);
  
  
  
  // this overflows the timer so that we get a fresh reading (supposedly) 
  oz.print('{');
  oz.print('T');
  oz.print('3');
  oz.print('<');
  oz.print('}');
  delay(1000);

  
  
/**
  oz.print('{');
  oz.print('S');
  oz.print('}');
  delay(100);
**/
  inc = 0;
  while(oz.available()){
    oz.read();
    inc++;
    if(inc > 128) break;
  }
  
  
  oz.print('{');
  oz.print('M');
  oz.print('}');
  
  delay(800);  
  byte ib = 0;
  
  while(oz.available()){
    
    if(index < BUF_SZ){
      buf[index] = oz.read();
    }else{
      break;
    }
    index++;
  }
  
  if(index >= 18){
    
    //perform a couple more sanity checks
  
    //first make sure we have a "complete" response
    if(buf[0] != 0x7B){
      return false;
    }
    
    //Serial.print("BUF:");
    
    //check for the terminating }
    for(i = 0; i < BUF_SZ; i++){
      
      //Serial.print("-");
      //Serial.print(buf[i], HEX);
      
      if(buf[i] == 0x7D){
        found = true;
      }
    }
    
    //Serial.print("\n");
    
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
    
    //Serial.print(buf[12], HEX);
    //Serial.print(",");
    //Serial.print(buf[13], HEX);
    //Serial.print(",");
    //Serial.print(buf[14], HEX);
    //Serial.print('\n');  
    
    
    temp_t = assembleTemp(buf[12], buf[13], buf[14]);
    temp = ((float)temp_t / 10) - 40;
    
    
    //Serial.print(temp_t, HEX);
    //Serial.print(",");
    //Serial.print(temp);  
    //Serial.print(",");
    
    //get the ozone reading
    s1_ozone_t = assembleByte(buf[2], buf[3]);
    
    //Serial.print('\n');
    //Serial.print("O3_1,");
    //Serial.print(s1_ozone_t, DEC);    
    //Serial.print("\n");    
    
    s2_ozone_t = assembleByte(buf[4], buf[5]);   
   //s2_ozone_t = assembleByte(0xF, 0xF);  
    
    //Serial.print("O3_2,");
    //Serial.print(s2_ozone_t, DEC);
    //Serial.print('\n');
    
    
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
    Serial.println("O3 D");
  }
  
  return false;
  
}

unsigned int assembleByte(byte byteH, byte byteL){
  
  byteH = (0x0F & byteH);
  byteL = (0x0F & byteL);
  
  unsigned int Nb = 0;
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

void readData() {

  byte c;

  boolean b = false;

  while (pm.available()) {
    
    c = pm.read();

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

  hi = buffer[2];
  lo = buffer[3]; // stores b2 as an unsigned value

  result1 = result1 + (hi);
  result1 = result1 << 8;
  result1 = result1 + (lo);
  
  lastPM1 = result1;

  if(CALIBRATING == true){
    Serial.print("PM 1.0 BUF [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result1); Serial.print("]"); Serial.println();
  }
  //display.print("PM 1.0 "); display.print(result1); display.print(" "); display.print((char)229); display.print("g/m^3"); display.println();

  hi = (buffer[4]);
  lo = (buffer[5]);

  result2 = result2 + (hi);
  result2 = result2 << 8;
  result2 = result2 + (lo);
  
  lastPM2_5 = result2;

  if(CALIBRATING == true){
    Serial.print("PM 2.5 BUF [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: [");  Serial.print(result2); Serial.print("]"); Serial.println();
  }
  //display.print("PM 2.5 "); display.print(result2); display.print(" "); display.print((char)229); display.print("g/m^3"); display.println();

  hi = (buffer[6]);
  lo = (buffer[7]); // stores b2 as an unsigned value

  result3 = result3 + (hi);
  result3 = result3 << 8;
  result3 = result3 + (lo);
  
  lastPM10 = result3;

  if(CALIBRATING == true){
    Serial.print("PM 10 BUF [0x"); Serial.print(hi, HEX); Serial.print(",0x"); Serial.print(lo, HEX); Serial.print("] RES: ["); Serial.print(result3); Serial.print("]"); Serial.println();
    Serial.println();
  }
  
}

void getVOC(boolean p, boolean AVG){
  
   //do RH now
  float sensorValue = 0.0; 
  float tmp = 0.0;
  float vin = 0.0;
  float ret = 0.0;
  
  int16_t results;
  
  /* Be sure to update this value based on the IC and the gain settings! */
  //float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */
  
  //get the average of a few readings
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += ads.readADC_Differential_2_3();  
    delay(10);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;
  
  if(voc_index >= (voc_read_queue_sz - 1)){
    voc_index = 0;
  }
  
  voc_read_queue[voc_index] = sensorValue;
  
  voc_index++;
  
  if(AVG == true){
    
    sensorValue = getAvg(voc_read_queue, voc_read_queue_sz);  
    
  }
  
  //vin = (sensorValue / 1023.0) * 5.0;
  vin = (sensorValue * multiplier)/1000;
  ret = (VOC_M * vin) + VOC_B;
  
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

int getAvg(int arr[], byte sz){
  
  long total = 0;
  byte i = 0;
  byte count = 0;
  
  for(i = 0; i < sz; i++){
    
    if(arr[i] > 0){
      
      total = total + arr[i];
      count++;
    }
  }
  
  total = total / count;
  return (int)total;
}

