#define PIN_RED   5
#define PIN_GREEN 6
#define PIN_BLUE  3


#include <Wire.h>
#include <I2Cdev.h>
#include <IAQ2000.h>

IAQ2000 iaq;


void setup()  { 
  
  Serial.begin(9600);
  
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  digitalWrite(PIN_RED, HIGH);
  digitalWrite(PIN_GREEN, HIGH);
  digitalWrite(PIN_BLUE, HIGH);
  
  Serial.println("RED");
  analogWrite(PIN_RED, 0);
  delay(2000);
  digitalWrite(PIN_RED, HIGH);
  

  Serial.println("GREEN");  
  analogWrite(PIN_GREEN, 0);
  delay(2000);
  digitalWrite(PIN_GREEN, HIGH);

  
  Serial.println("BLUE");
  analogWrite(PIN_BLUE, 0);
  delay(2000);
  digitalWrite(PIN_BLUE, HIGH);
  
  Serial.print("INIT ARETAS SENSOR BOARD SUCCESS\n");
  
}

void loop() {

 Serial.println("START");
 int i = 0;

 while(i < 1024){
 
  i++;
  Serial.print("IAQ:");
  Serial.print(iaq.getIaq(), DEC);
  Serial.print('\n');
  setDPColor(i);
  delay(1000); 
  
 } 

  Serial.println("END");  
  
}

void setDPColor(int sensorValue){
  
  int plc = sensorValue - 512;
  
  int r = 0;
  int g = 0;
  int b = 0;
  
  if(plc < 0){
    
    plc = abs(plc);
    
    r = plc / 2;
    g = (512 - plc) / 2;
    b = 0;
    
  }else{
    
    r = 0;
    g = (512 - plc) / 2;
    b = plc / 2;
    
  }

  setColor(r, g, b);
  
}

void setColor(int red, int green, int blue){
  
  if(red < 0) red = 0;
  if(red > 255) red = 255;
  if(green < 0) green = 0;
  if(green > 255) green = 255;
  if(blue < 0) blue = 0;
  if(blue > 255) blue = 255;
  
  //invert everything for PWM
  red = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
  
  analogWrite(PIN_RED, red);
  analogWrite(PIN_GREEN, green);
  analogWrite(PIN_BLUE, blue);
  
}
