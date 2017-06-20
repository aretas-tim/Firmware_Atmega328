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
int mainCount = 0;

unsigned long valueP = 0; // reading of DN7C3
float valueT = 0;
float averageP = 0;
float averageT = 0;
unsigned long timer = 0;
unsigned long timerT = 0;

Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */ 

float VCCMAIN = 4.97;

float scalar = VCCMAIN / 1023.0;
    
HIH6130 trh(&Serial);

TRH lastTRH = {0,0,0};

void setup() {
  
  pinMode(PMIN, INPUT); // DN7C3 Vo Pin
  pinMode(PMOUT, OUTPUT);
  digitalWrite(PMOUT, HIGH);
  
  Serial.begin(9600);

  Serial.println("Sharp DN7C3 PM2.5 Sensor");
  
  VCCMAIN = getVCCMV();
  scalar = VCCMAIN / 1023.0;
  
  Serial.print("VCC MAIN:"); Serial.print(VCCMAIN); Serial.print("mV"); Serial.print("\n");
  
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
  
  timer = micros();
  timerT = millis();
}

void loop() {
  
    //only reget temp every second
  if((millis() - timerT) >= 1000){
    
    VCCMAIN = getVCCMV();
    scalar = VCCMAIN / 1023.0;
    
    trh.getTRH(&lastTRH);
    timerT = millis();
  }
  
  valueP = 0;
  count = 0;
  
  for(int i = 0; i < 100; i++){
    
      digitalWrite(PMOUT, HIGH);    // turn on led
      delayMicroseconds(260);  // Arduino ADC need about 100us
      valueP = valueP + analogRead(PMIN); // PM2.5 reading
      delayMicroseconds(40);
      digitalWrite(PMOUT, LOW);   // turn off led
      delayMicroseconds(9680);
      
      count++;
  
  }
  
  Serial.print('\n');
  
  mainCount++;
  
  averageP = (float)valueP / (float)count;
  
  valueT = valueT + averageP;
  
  Serial.print("AVG:");
  Serial.print(averageP);
  Serial.print(" AVG V:");
  Serial.print(averageP * scalar);
  Serial.print(" RUNNING AVG:");
  Serial.print((valueT / mainCount) * scalar);
  Serial.print(" TEMP:");
  Serial.print(lastTRH.temp);
  Serial.print('\n');
  
  
}

float getVCCMV(){
  
    int16_t results;
  
  /* Be sure to update this value based on the IC and the gain settings! */
  float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  //float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  results = ads.readADC_Differential_0_1();  
  
  return(results * multiplier);
  
}
