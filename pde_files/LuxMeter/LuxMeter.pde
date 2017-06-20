/*

Basic temperature sensor implementation

Temporary line format will be 
SENSOR TYPE,TEMP C\n
 
 */

int sensorPin = A2;    // select the input pin for the data line on the sensor 
const byte SENSOR_TYPE = 0xF6;
const byte NUM_SAMPLES = 3; //number of samples
const int SAMPLE_INTERVAL = 100; //interval between samples in miliseconds
const int POLLING_DELAY = 120000; //the polling interval in miliseconds

void setup() {
  
  Serial.begin(9600);
  
}

void loop() {   
 
  long tmp = 0;
  float sensorValue = 0.0; 
  
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(sensorPin);
    delay(100);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;
  
  Serial.print(SENSOR_TYPE);
  Serial.print(",");
  Serial.print(getV(sensorValue));
  Serial.print("\n");
  
  delay(1000);
}

float getV(float sensorValue){
  
 return (sensorValue / 1023) * 5.0; 
  
}
float getTemp(float sensorValue){
  
  float vin = (sensorValue / 1023) * 5.0;
  float ret = ((vin * 200) * 0.2222) - 61.111;
  
  return ret;
  
}
