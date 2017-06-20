#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <K30.h>
#include <HIH6130.h>
#include <Narcoleptic.h>

#define CO2I2CADDR 0x68
#define XBEE_DTR_PIN 5
#define I2C_PWR_PIN 7

unsigned long mac = 0;

XBeeFunc xbee(&Serial);
K30 k30(&Serial, CO2I2CADDR);
HIH6130 trh(&Serial);

int lastCO2Value = 0;
TRH lastTRH = {0,0,0};

void setup(){

  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  Serial.begin(9600);
  Serial.println("INIT ARETAS BOARD SUCCESS");
  //pinMode(13, OUTPUT);
  pinMode(XBEE_DTR_PIN, OUTPUT);
  pinMode(I2C_PWR_PIN, OUTPUT);
  digitalWrite(XBEE_DTR_PIN, LOW);
  digitalWrite(I2C_PWR_PIN, HIGH); 
}

void loop(){
  
  Serial.begin(9600);
  delay(10);
  
  //turn on the I2C modules
  digitalWrite(I2C_PWR_PIN, HIGH);

  //try waiting xx seconds for k30 module
  delay(15000);
  
  lastCO2Value = k30.readCO2();
  
  if(lastCO2Value <= 0){
    k30.readCO2();
  }
  
  trh.getTRH(&lastTRH);
  
  //turn off the I2C modules (conserve current)
  digitalWrite(I2C_PWR_PIN, LOW);
  
  delay(100);
  
  //turn on the radio
  digitalWrite(XBEE_DTR_PIN, LOW);
  //wait some specified time (ideally we could loop over a digitalRead on the PWRMON pin of the xbee
  delay(1000);
   
  k30.printCO2(true, mac, lastCO2Value);
  trh.printTRH(true, mac, &lastTRH);
  delay(5); //if this delay isn't introduced, the serial port never finishes printing the last char and garbage is emitted
  Serial.flush();
  Serial.end();

  //power down everything
  digitalWrite(XBEE_DTR_PIN, HIGH);
  
  //sleeep for a long time
  Narcoleptic.delay(30000);
  Narcoleptic.delay(30000);
  Narcoleptic.delay(30000);
  
  Narcoleptic.delay(30000);
  Narcoleptic.delay(30000);
  Narcoleptic.delay(30000);
  
}
