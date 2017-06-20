/**

Code for the NO2 Sensor 


*/

#define NO2_ANALOG_READ_PIN 3
#define PWM_HEATER_PIN 11

const int HEATING_TIME = 10000; //heat for 10 seconds

void setup()  { 
  
  Serial.begin(9600);
  pinMode(PWM_HEATER_PIN, OUTPUT);
  pinMode(NO2_ANALOG_READ_PIN, INPUT);
} 

void loop()  { 
  
  float pinValue = 0.0;
  float vin = 0.00;
  
  //power up the PWM source for the heater
  analogWrite(PWM_HEATER_PIN, 1023);    
  delay(HEATING_TIME); //let it heat up for a bit
  
  //read the value 
  pinValue = analogRead(NO2_ANALOG_READ_PIN);
  vin = (5.0/1023.0)*pinValue;
  
  Serial.print("vin:");
  Serial.print(vin);
  Serial.print("\n");
  
  //turn the heater off
  analogWrite(PWM_HEATER_PIN,0);
  
  
  //sleep for 30 seconds
  delay(20000);
  
}
