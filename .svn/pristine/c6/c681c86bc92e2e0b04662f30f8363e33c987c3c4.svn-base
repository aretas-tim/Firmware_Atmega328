/*

Basic temperature sensor implementation

Temporary line format will be 
SENSOR TYPE,TEMP C\n
 
 */
int tempSensorPin = A0;    // select the input pin for the data line on the sensor 
int rhSensorPin = A1;

const byte TEMP_SENSOR_TYPE = 0xF6;
const byte RH_SENSOR_TYPE = 0xF8;

const byte NUM_SAMPLES = 3; //number of samples
const int SAMPLE_INTERVAL = 100; //interval between samples in miliseconds

unsigned long POLLING_DELAY = 2500; //the polling interval in miliseconds
unsigned long BLINK_DELAY = 2000;

long mac = 0;
long pm0 = 0; //placeholders for last time
long pm1 = 0;
int LED_STATUS = LOW;


void setup() {

  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(tempSensorPin, INPUT);
  pinMode(rhSensorPin, INPUT);
  Serial.print("INIT ARETAS SENSOR BOARD SUCCESS\n");
  
}

void loop() {  
 
 unsigned long currentMillis = millis();
 
 long tmp = 0;
 float sensorValue = 0.0; 

 if((currentMillis - pm0 > POLLING_DELAY) || (pm0 == 0)) {
    
    //save the last time we polled the sensors
    pm0 = currentMillis;  
    
    sensorValue = analogRead(tempSensorPin);
    
    Serial.print(mac);
    Serial.print(",");
    Serial.print(TEMP_SENSOR_TYPE, DEC);
    Serial.print(",");
    Serial.print(getTemp(sensorValue));
    Serial.print("\n");
    
    delay(100);
    
    //do RH now
    sensorValue = 0.0;  
    sensorValue = analogRead(rhSensorPin);
    
    Serial.print(mac, DEC);
    Serial.print(",");
    Serial.print(RH_SENSOR_TYPE, DEC);
    Serial.print(",");
    Serial.print(getRH(sensorValue));
    Serial.print("\n");
    
    delay(100);
  }
  
   if((currentMillis - pm1 > BLINK_DELAY) || (pm1 == 0)) {
    
    //save the last time we polled the sensors
    pm1 = currentMillis;  
    digitalWrite(13, LED_STATUS);
    
    if(LED_STATUS == LOW){
      LED_STATUS = HIGH;
    }else {
      LED_STATUS = LOW;
    }
  
   }
  
}

float getTemp(float sensorValue){
  
  float vin = (sensorValue / 1023.0) * 5.0;
  float ret = ((vin * 1000.0) - 500.0) / 10.0;
  return ret;
  
}

float getRH(float sensorValue){
  
  float vin = (sensorValue / 1023.0) * 5.0;
  //float ret = ((vin - 0.8)/0.031);
  float ret = (vin - 0.958)/0.0307;
  return ret;
  
}

