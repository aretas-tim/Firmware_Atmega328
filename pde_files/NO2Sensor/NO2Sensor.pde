/**

Code for the NO2 Sensor 


*/

#define NO2_ANALOG_READ_PIN A3
#define NO2_HEATER_PIN 3

const int NO2_HEATING_TIME = 2000; //heat for 2 seconds

void setup()  { 
  
  Serial.begin(9600);
  pinMode(NO2_HEATER_PIN, OUTPUT);
  pinMode(NO2_ANALOG_READ_PIN, INPUT);
  pinMode(13, OUTPUT);
} 

void loop()  { 
  
  float pinValue = 0.0;
  float vin = 0.00;
  
  //power up the source for the heater
  digitalWrite(NO2_HEATER_PIN, HIGH);   
  digitalWrite(13, HIGH);
  delay(NO2_HEATING_TIME); //let it heat up for a bit
  
  //read the value 
  pinValue = analogRead(NO2_ANALOG_READ_PIN);
  vin = (5.0/1023.0)*pinValue;
  
  Serial.print("vin:");
  Serial.print(vin);
  Serial.print("\n");
  
  //turn the heater off
  digitalWrite(NO2_HEATER_PIN, LOW);
  digitalWrite(13, LOW);
  
  //sleep for n milliseconds
  delay(1000);
  
}
