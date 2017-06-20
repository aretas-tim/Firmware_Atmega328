#include <Wire.h>
#include <SensorTypes.h>
#include <XBeeFunc.h>
#include <SensorCalibration.h>
#include <HIH6130.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Narcoleptic.h>

#define IRLED1PIN 6
#define IRLED2PIN 9
#define LED1PIN 3
#define LED2PIN 5
#define I2CPWRPIN 4
#define XBEE_DTR_PIN 2

unsigned long POLLING_DELAY = 120000; //the polling interval in miliseconds
unsigned long BLINK_DELAY = 2000;
unsigned long CYCLE_INTERVAL = 2000; //length of time that controls the sensor gas sensor read cycle

unsigned long mac = 0;

XBeeFunc xbee(&Serial);
HIH6130 trh(&Serial);
Adafruit_TSL2561_Unified tsl_a = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 00001);
Adafruit_TSL2561_Unified tsl_b = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 00002);

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder

boolean POUT = false; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = true;

int LED_STATUS = HIGH;

float SENSOR_A_OFFSET = 0;
float SENSOR_B_OFFSET = 0;

int PWM_POWER = 0;

unsigned long panID = 0x808;

void setup(){
  
  pinMode(XBEE_DTR_PIN, OUTPUT);     //radio sleep mode pin
  digitalWrite(XBEE_DTR_PIN, LOW);  //radio sleep mode pin 
  
  mac = xbee.getXBeeSerialNum();
  //try twice.. for some reason it fails occasionally
  if(mac == 0){
    mac = xbee.getXBeeSerialNum();
  }
  
  
  Serial.begin(9600);
  Serial.println("INIT ARETAS BOARD SUCCESS");

  pinMode(13, OUTPUT);  //INDICATOR LED
  
  pinMode(I2CPWRPIN, OUTPUT);    //turn on i2c power
  digitalWrite(I2CPWRPIN, HIGH);  //turn on i2c power
  
  pinMode(LED1PIN, OUTPUT);
  pinMode(LED2PIN, OUTPUT);
  
  Serial.println("INIT SENSORS SUCCESS");
  
  int i = 0;
  for(i = 0; i < 255; i++){
    
    analogWrite(LED1PIN, i);
    analogWrite(LED2PIN, i);  
    delay(20);
    
  }
  
}

void configureSensors(){
 
 //tsl_a.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
 //tsl_b.enableAutoRange(true);
 
 tsl_a.setGain(TSL2561_GAIN_16X);
 tsl_b.setGain(TSL2561_GAIN_16X); 
  
 /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) 
     options are: TSL2561_INTEGRATIONTIME_13MS, TSL2561_INTEGRATIONTIME_101MS, TSL2561_INTEGRATIONTIME_402MS
 */
 //tsl_a.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS); 
 //tsl_b.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS); 
 tsl_a.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS); 
 tsl_b.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS); 
  
}

void loop(){
      
  digitalWrite(XBEE_DTR_PIN, LOW);

  //wait some specified time (ideally we could loop over a digitalRead on the PWRMON pin of the xbee
  delay(1000);
  
  Serial.begin(9600);
  
  trh.printTRH(true, mac);
      
  Serial.print('\n');
  
  delay(5); //if this delay isn't introduced, the serial port never finishes printing the last char and garbage is emitted
  
  Serial.flush();
  Serial.end();
  
  //power down everything
  digitalWrite(XBEE_DTR_PIN, HIGH); 
      
  //blink
  digitalWrite(13, LED_STATUS);
  
  if(LED_STATUS == LOW){
      
      LED_STATUS = HIGH;
    
  }else {
      
      LED_STATUS = LOW;
  }
  
  PWM_POWER = PWM_POWER + 5;
  
  if(PWM_POWER >= 255){
    PWM_POWER = 0;
  }
    
  
  Narcoleptic.delay(1000);

}


