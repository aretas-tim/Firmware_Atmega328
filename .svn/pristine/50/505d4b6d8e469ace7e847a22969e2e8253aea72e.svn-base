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
const int POLLING_DELAY = 120000; //the polling interval in miliseconds

long mac = 0x406FDE8C;

void setup() {
  
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(tempSensorPin, INPUT);
  pinMode(rhSensorPin, INPUT);
  Serial.print("INIT ARETAS SENSOR BOARD SUCCESS");
  
}

void loop() {   
 
  long tmp = 0;
  
  float sensorValue = 0.0; 
  //get the 
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(tempSensorPin);
    delay(100);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;
  
  Serial.print(mac);
  Serial.print(",");
  Serial.print(TEMP_SENSOR_TYPE, DEC);
  Serial.print(",");
  Serial.print(getTemp(sensorValue));
  Serial.print("\n");
  
  delay(500);
  
  //do RH now
  sensorValue = 0.0; 
  tmp = 0;
  
  //get the average of a few readings
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(rhSensorPin);
    delay(100);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;
  
  Serial.print(mac, DEC);
  Serial.print(",");
  Serial.print(RH_SENSOR_TYPE, DEC);
  Serial.print(",");
  Serial.print(getRH(sensorValue));
  Serial.print("\n");
  
  digitalWrite(13, LOW);   // set the LED on
  
  delay(120000);
  digitalWrite(13, HIGH);   // set the LED on
  
}

float getTemp(float sensorValue){
  
  float vin = (sensorValue / 1023.0) * 5.0;
  //float ret = ((vin * 200) * 0.2222) - 61.111;
  float ret = ((vin * 1000.0) - 500.0) / 10.0;
  return ret;
  
}

float getRH(float sensorValue){
  
  float vin = (sensorValue / 1023.0) * 5.0;
  float ret = ((vin - 0.8)/0.031);
  return ret;
  
}
