/**

Code for the CO Sensor 


*/

#define ASML_VIN_PIN A4
#define ASML_HEATER_PIN 4

const int ASML_HEATING_TIME = 30; //heat for 14 ms

void setup()  { 
  
  Serial.begin(9600);
  pinMode(ASML_HEATER_PIN, OUTPUT);
  pinMode(ASML_VIN_PIN, INPUT);
  pinMode(13, OUTPUT);
} 

void loop()  { 
  
  float pinValue = 0.0;
  float vin = 0.00;
  digitalWrite(13, HIGH); //indicate sensor read process has started
  
  //power up the source for the heater
  digitalWrite(ASML_HEATER_PIN, HIGH);
  delay(ASML_HEATING_TIME); //let it heat up for a bit
  digitalWrite(ASML_HEATER_PIN, LOW);

  delay(900);

  //read the value 
  pinValue = analogRead(ASML_VIN_PIN);
  
  vin = (5.0/1023.0)*pinValue;
  vin = 5 - vin;
  
  Serial.print("vin:");
  Serial.print(vin);
  Serial.print("\n");
  
  digitalWrite(13, LOW);
  
  //sleep for n milliseconds
  //delay(2000);
  
}
