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
SoftwareSerial pm(3,1);
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

const float VOC_P1 = 0.05; //P2 = 1.52
const float VOC_M = 0.010;

const float CO_P1 = 2.49;
const float CO_M = 0.004;


void setup(){

  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  
  //now try and set the PANID
  //xbee.setPanID(panID);
  
  Serial.begin(9600);
  Serial.println("INIT");
  Serial.print('\n');
  
  ads.begin();
  
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
    //readPMData();
    k30.printCO2(POUT | CALIBRATING, mac);
    getVOC(POUT | CALIBRATING, AVG);
    getCO(POUT | CALIBRATING);
    
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

void getCO(boolean p){
  
  float sensorValue = 0.0; 
  float tmp = 0.0;
  float vin = 0.0;
  float ret = 0.0;
  
  int16_t results;
  
  float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */
  
  //get the average of a few readings
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += ads.readADC_Differential_0_1(); 
    delay(100);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;

  vin = (sensorValue * multiplier)/1000;
  
  ret = (float)(vin - CO_P1) / CO_M;
  
  if(ret < 0){ ret = 0; }
  
  if(p == true){
  
    Serial.print(mac, DEC);
    Serial.print(",");
    Serial.print(CO_SENSOR_TYPE, DEC);
    Serial.print(",");
    
    if(CALIBRATING == true){

      Serial.print("CO RAW VALUE:");
      Serial.print(sensorValue);
      Serial.print('\n');

      Serial.print("ADC Voltage:");
      Serial.print((sensorValue * multiplier)/1000);
      Serial.print('\n');
      
    }else{
      
      Serial.print(ret, 1);
      
    }
    
    Serial.print("\n");
  
  }
  
  delay(10);
  
}

void getVOC(boolean p, boolean AVG){
  
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
  
  //ret = (VOC_M * vin) + VOC_B;
  ret = (float)(vin - VOC_P1) / VOC_M;
  
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

