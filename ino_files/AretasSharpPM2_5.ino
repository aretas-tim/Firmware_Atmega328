#define PMIN  A1
#define PMOUT A0

#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>

// My QRcode: 1491800B82 1.23 **** 24.9
// 14918 is produce at 2014-09-18 (WTF!)
// 1.23 is reference voltage
// 24.9 is temperature of reference voltage
     
int count = 0;
float valueP = 0; // reading of DN7C3
float valueT = 0; // reading of LM35
float averageP = 0;
float averageT = 0;
float historyP = 0;
unsigned long timer = 0;
     
unsigned long A = 0;
unsigned long B = 0;    

    
HIH6130 trh(&Serial);

TRH lastTRH = {0,0,0};

void setup() {
  
  pinMode(PMIN, INPUT); // DN7C3 Vo Pin
  pinMode(PMOUT, OUTPUT);
  digitalWrite(PMOUT, HIGH);
  
  Serial.begin(9600);
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Sharp DN7C3 PM2.5 Sensor");
  
  for(int i = 0; i < 1000; i++) {
    
    delayMicroseconds(9680);
    digitalWrite(PMOUT, HIGH);
    delayMicroseconds(320);
    digitalWrite(PMOUT, LOW);
  }
  
  timer = micros();
}

void loop() {
  
  trh.getTRH(&lastTRH);
  
  if ((micros() - timer) >= 9680) {  // per 10ms
  
    timer = micros();
    digitalWrite(PMOUT, HIGH);    // turn on led
    A = micros();            // measure pulse begin time
    delayMicroseconds(280);  // Arduino ADC need about 100us
    valueP = analogRead(PMIN); // PM2.5 reading
    B = micros();            // measure pulse end time
    digitalWrite(PMOUT, LOW);   // turn off led
   
    count++;
    averageP = (averageP * (count - 1) + valueP * (4960.0 / 1023.0)) / count;
     
    if (count >= 100) { // 100 times average, 1 second
    
      float temp = lastTRH.temp; // get Temperature
      float diffrent = averageP - (110 + (temp-23.92)*6); // 1100 is ture reference voltage base on my circuits
      float pm25ug = diffrent * 0.6;
      
      if (historyP == 0) {  
        
          historyP = pm25ug;
        
        }else {
        
          historyP = (historyP * 9 + pm25ug) / 10;
      
      }
     
      // post to Serial
      Serial.print("T: ");
      Serial.print(temp, 2);
      Serial.print(" C\tV: ");
      Serial.print(averageP, 2);
      Serial.print(" mV\t P: ");
      Serial.print(pm25ug, 2);
      Serial.print(" ug/m3\t avgP: ");
      Serial.print(historyP, 2);
      Serial.print(" ug/m3\t Pulse: ");
      Serial.println(B - A);
    
      // clear for next time
      count = 0;
      valueP = 0;
      valueT = 0;
      averageP = 0;
      averageT = 0;
    }
  }
}
