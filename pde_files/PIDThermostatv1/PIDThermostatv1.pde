/********************************************************
 * PID RelayOutput Example
 * Same as basic example, except that this time, the output
 * is going to a digital pin which (we presume) is controlling
 * a relay.  The pid is designed to output an analog value,
 * but the relay can only be On/Off.
 *
 *   To connect them together we use "time proportioning
 * control"  Tt's essentially a really slow version of PWM.
 * First we decide on a window size (5000mS say.) We then 
 * set the pid to adjust its output between 0 and that window
 * size.  Lastly, we add some logic that translates the PID
 * output into "Relay On Time" with the remainder of the 
 * window being "Relay Off Time"
 ********************************************************/

#include <PID_v1.h>
#include <Wire.h>
#include <SensorTypes.h>
#include <SensorCalibration.h>
#include <HIH6130.h>
#define RelayPin 6

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint,2,5,1, DIRECT);

int WindowSize = 5000;
unsigned long windowStartTime;

HIH6130 trh(&Serial);
TRH lastTRH = {0,0,0};

boolean once = false;

void setup()
{
  windowStartTime = millis();

  //initialize the variables we're linked to
  Setpoint = 23;

  //tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, WindowSize);

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
  
  pinMode(RelayPin, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("INIT ARETAS BOARD SUCCESS");
}

void loop()
{
  
  trh.getTRH(&lastTRH);
  
  Input = (double)lastTRH.temp; //get temperature 
  
  myPID.Compute();

  /************************************************
   * turn the output pin on/off based on pid output
   ************************************************/
  unsigned long now = millis();
  
  if(now - windowStartTime > WindowSize){   
    //time to shift the Relay Window
    windowStartTime += WindowSize;
    Serial.print("TEMP:");
    Serial.print(Input);
    Serial.print('\n');
  }
  
  if(Output > now - windowStartTime){
    //digitalWrite(RelayPin,HIGH);
    Serial.println("HIGH");
  }else{
    digitalWrite(RelayPin,LOW);
    Serial.println("LOW");
  }

}

