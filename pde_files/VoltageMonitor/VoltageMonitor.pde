const int analogInPin0 = A0;
const int analogInPin1 = A1;

void setup(){
  Serial.begin(9600); 
}

void loop(){
  
 getVoltage();
 delay(5); 
  
}

void getVoltage(){
 
 float ret = 0; 
 int sensorValue = 0;
 
 sensorValue = analogRead(analogInPin0);
 Serial.print("sensor 0 value:");
 Serial.print(sensorValue);
 Serial.print(" voltage:");
 Serial.print(((float)sensorValue / 1023) * 5.0);
 Serial.print('\n');
 
 sensorValue = analogRead(analogInPin1);
 Serial.print("sensor 1 value:");
 Serial.print(sensorValue);
 Serial.print(" voltage:");
 Serial.print(((float)sensorValue / 1023) * 5.0);
 Serial.print('\n'); 
 Serial.print('\n');
}
