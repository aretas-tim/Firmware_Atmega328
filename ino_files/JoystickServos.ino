// Controlling a servo position using a potentiometer (variable resistor) 
// by Michal Rinott <http://people.interaction-ivrea.it/m.rinott> 

#include <Servo.h> 
 
Servo myservo1;  // create servo object to control a servo 
Servo myservo2;
 
int x = A0;  // analog pin used to connect the potentiometer
int y = A1;

int val1, val2;    // variable to read the value from the analog pin 

unsigned long CM0 = 0; //current millis placeholder 
unsigned long LM0 = 0; //last millis placeholder
int PRINT_INTERVAL = 2000;

void setup() {
  
  Serial.begin(9600); 
  myservo1.attach(8);
  myservo2.attach(9);  // attaches the servo on pin 9 to the servo object 
  Serial.print("INIT");
} 
 
void loop() 
{ 
  
  int servo1Pos, servo2Pos;
  
  val1 = analogRead(x);
  val2 = analogRead(y);
  
  CM0 = millis();
  
  if((CM0 - LM0) > PRINT_INTERVAL){
    
    LM0 = CM0;
    
    Serial.print("X VAL:");
    Serial.println(val1);
    
    Serial.print("Y VAL:" );
    Serial.println(val2);
    
  }
  servo1Pos = map(val1, 387, 639, 0, 179);     // scale it to use it with the servo (value between 0 and 180) 
  servo2Pos = map(val2, 385, 636, 0, 178);
  
  myservo1.write(servo1Pos);                  // sets the servo position according to the scaled value 
  myservo2.write(servo2Pos);
  delay(15);                           // waits for the servo to get there 
} 
