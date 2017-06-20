/**

Code for the CO Sensor 


*/

#define C2442_VIN_PIN A2
#define C2442_HEATER_PIN 2
#define S1P_PIN 5

const int C2442_HEATING_TIME = 14; //heat for 14 ms

void setup()  { 
  
  Serial.begin(9600);
  pinMode(C2442_HEATER_PIN, OUTPUT);
  pinMode(S1P_PIN, OUTPUT);
  pinMode(C2442_VIN_PIN, INPUT);
  pinMode(13, OUTPUT);
} 

void loop()  { 
  
  float pinValue = 0.0;
  float vin = 0.00;
  digitalWrite(13, HIGH); //indicate sensor read process has started
  
  //power up the source for the heater
  digitalWrite(C2442_HEATER_PIN, HIGH);   
  delay(C2442_HEATING_TIME); //let it heat up for a bit
  digitalWrite(C2442_HEATER_PIN, LOW);

  delayMicroseconds(980000);
  digitalWrite(S1P_PIN, HIGH);
  delayMicroseconds(2500);
  //read the value 
  pinValue = analogRead(C2442_VIN_PIN);
  
  delayMicroseconds(2500);
  digitalWrite(S1P_PIN, LOW);  
  vin = (5.0/1023.0)*pinValue;
  
  Serial.print("vin:");
  Serial.print(vin);
  Serial.print("\n");
  
  digitalWrite(13, LOW);
  
  //sleep for n milliseconds
  delay(500);
  
}
