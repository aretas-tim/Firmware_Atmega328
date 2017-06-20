/*

Basic temperature sensor implementation

Temporary line format will be 
SENSOR TYPE,TEMP C\n
 
 */

int vocSensorPin = A0;    // select the input pin for the data line on the sensor 

const byte VOC_SENSOR_TYPE = 0x60;

const byte NUM_SAMPLES = 3; //number of samples
const int SAMPLE_INTERVAL = 100; //interval between samples in miliseconds
const int POLLING_DELAY = 120000; //the polling interval in miliseconds

long mac = 1081750318;

void setup() {
  
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(vocSensorPin, INPUT);
  Serial.print("INIT ARETAS SENSOR BOARD SUCCESS");
  
}

void loop() {   
 
  long tmp = 0;
  
  float sensorValue = 0.0; 
  //get the 
  for(int i = 0; i < NUM_SAMPLES; i++){
    
    tmp += analogRead(vocSensorPin);
    delay(100);  
    
  }
  
  sensorValue = tmp / NUM_SAMPLES;
  
  Serial.print(mac);
  Serial.print(",");
  Serial.print(VOC_SENSOR_TYPE, DEC);
  Serial.print(",");
  Serial.print(getVOC(sensorValue));
  Serial.print("\n");
  
  delay(500);
  
  sensorValue = 0.0; 
  tmp = 0;
  
  
  digitalWrite(13, LOW);   // set the LED on
  
  delay(120000);
  digitalWrite(13, HIGH);   // set the LED on
  
}

float getVOC(float sensorValue){
  
  float vin = (sensorValue / 1023.0) * 5.0;
  //float ret = ((vin * 200) * 0.2222) - 61.111;
  float ret = (vin - 0.04) / 0.0073;
  //float ret = vin; //just return the raw voltage for now
  return ret;
  
}

