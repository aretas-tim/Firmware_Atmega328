#define PMIN  A0
#define PMOUT 8

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>
#include <Adafruit_ADS1015.h>

// My QRcode: 1491800B82 1.23 **** 24.9
// 14918 is produce at 2014-09-18 (WTF!)
// 1.23 is reference voltage
// 24.9 is temperature of reference voltage
     
int count = 0;
unsigned long timerS = 0;
unsigned long timerT = 0;

unsigned long valI = 0;

float CAL_TEMP = 22.24;
float CAL_MV   = 805;
float PM = 0;

Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */ 

int VCC_MV = 4970;
float scalar = (float)VCC_MV / 1023.0;
    
HIH6130 trh(&Serial);

TRH lastTRH = {0,0,0};

void setup() {
  
  pinMode(PMIN, INPUT); // DN7C3 Vo Pin
  pinMode(PMOUT, OUTPUT);
  digitalWrite(PMOUT, HIGH);
  
  Serial.begin(9600);
  Serial.println("Sharp DN7C3 PM2.5 Sensor");
  
  VCC_MV = getVCCMV();
  scalar = VCC_MV / 1023.0;
  
  Serial.print("VCC MAIN:"); Serial.print(VCC_MV); Serial.print("mV"); Serial.print("\n");
  
  ads.begin();
  
  // set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
  
  for(int i = 0; i < 100; i++) { 
    
    delayMicroseconds(9680);
    digitalWrite(PMOUT, HIGH);
    delayMicroseconds(320);
    digitalWrite(PMOUT, LOW);
  }
  
  timerS = micros();
  timerT = millis();
}

void loop() {
  
  //only reget temp every second
  if((millis() - timerT) >= 1000){
    
    VCC_MV = getVCCMV();
    scalar = VCC_MV / 1023.0;
    
    trh.getTRH(&lastTRH);
    timerT = millis();
  }
  
  count = 0;
  valI = 0;

  for(int i = 0; i < 100; i++){ //1 second of readings
    
    digitalWrite(PMOUT, HIGH);
    delayMicroseconds(176);
    valI = valI + analogRead(PMIN);  
    delayMicroseconds(40);
    digitalWrite(PMOUT, LOW);
    delayMicroseconds(9680);
    count++;
    
  }
 
 PM = getPM2d5(valI/count); 
 
 Serial.print("PM:");
 Serial.print(PM);
 Serial.print(" VAL:");
 Serial.print((float)valI/(float)count);
 Serial.print(" Vo:");
 Serial.print((scalar * ((float)valI/(float)count)));
 Serial.print("mV");
 Serial.print("\n");
 
}

float getPM2d5(float x){
  
  float vo = scalar * x;
  float dltMv = vo - getVS();
  return (0.6 * dltMv);
  
}


float getVS(){
  
  if((lastTRH.temp < 40) && (lastTRH.temp > - 10)){
    
    return (CAL_MV - (6 * (CAL_TEMP - lastTRH.temp)));
    
  }else{
    
    return CAL_MV - (((CAL_TEMP - lastTRH.temp) * 6) + (lastTRH.temp - 40)*1.5);
    
  }
  
}

float getVCCMV(){
  
    int16_t results;
  
  /* Be sure to update this value based on the IC and the gain settings! */
  float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  //float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  results = ads.readADC_Differential_0_1();  
  
  return(results * multiplier);
  
}
