#include <Wire.h>
#include <SensorTypes.h>
#include <SensorCalibration.h>

const byte NUM_SAMPLES = 4; //number of samples
const int SAMPLE_INTERVAL = 100; //interval between samples in miliseconds

const byte I2C_SLAVE_ID = 0x1A;

unsigned long POLLING_DELAY = 60000; //the polling interval in miliseconds
unsigned long CYCLE_INTERVAL = 4000; //length of time that controls the sensor gas sensor read cycle

unsigned long pm0 = 0; //polling interval millis place holder
unsigned long pm1 = 0; //secondary polling interval millis placeholder
unsigned long cm0 = 0; //main cycle millis placeholder

boolean POUT = false; //whether or not to print to console (useful for or'ing when calibrating)
boolean CALIBRATING = false;

unsigned long mac = 0;

int PPD_INPUT_PIN = 5;
unsigned long PPD_DURATION;
unsigned long PPD_STARTTIME;
unsigned long PPD_SAMPLETIME_MS = 30000;
unsigned long PPD_LOWPULSEOCCUPANCY= 0;
float PPD_RATIO = 0;
unsigned long PPD_CONCENTRATION = 0;

int LED_STATUS = HIGH;

void setup()  {
 
  // Start I²C bus as a slave
  Wire.begin(I2C_SLAVE_ID);
  
  // Set the callback to call when data is received.
  Wire.onReceive(receiveCallback);
  Wire.onRequest(requestCallback); 
  
  Serial.begin(9600);

  pinMode(PPD_INPUT_PIN, INPUT);
  pinMode(13, OUTPUT);
  
  PPD_LOWPULSEOCCUPANCY = 0;
  PPD_STARTTIME = millis();
  
  Serial.print("INIT ARETAS SENSOR BOARD SUCCESS\n");
  
}

void loop() {
  
  unsigned long currentMillis = millis();

  //ppd needs a different cycle 
  getPPD(POUT | CALIBRATING);
  
  //main cycle interval (5 seconds or so)
  if((currentMillis - cm0 > CYCLE_INTERVAL) || (cm0 == 0)) {
    
    Serial.print(".");
    
    cm0 = currentMillis;

    if((currentMillis - pm0 > POLLING_DELAY) || (pm0 == 0)) {
     
      POUT = true;  
      //save the last time we polled the sensors
      pm0 = currentMillis; 
      
      
    }else{
      
      POUT = false;
    
    }
    
    printPPD(POUT | CALIBRATING);
   
    if(CALIBRATING){
      Serial.print("\n"); //make the output easier to read
    }    
    //blink
    digitalWrite(13, LED_STATUS);
    
    if(LED_STATUS == LOW){
      LED_STATUS = HIGH;
    }else {
      LED_STATUS = LOW;
    }
    
  }
  
}

void printPPD(boolean p){
  
  if(p == true){
  
      Serial.print(mac, DEC);
      Serial.print(",");
      Serial.print(PM_SENSOR_TYPE, DEC);
      Serial.print(",");
    
      if(CALIBRATING == true){
      
        Serial.print(PPD_LOWPULSEOCCUPANCY);
        Serial.print(",");
        Serial.print(PPD_RATIO);
        Serial.print(",");
        Serial.println(PPD_CONCENTRATION);
      
      }else{
      
        Serial.println(PPD_CONCENTRATION);
      
      }
    
      Serial.print("\n");
      
    }
}

void getPPD(boolean p){
  
  PPD_DURATION = pulseIn(PPD_INPUT_PIN, LOW);
  PPD_LOWPULSEOCCUPANCY = PPD_LOWPULSEOCCUPANCY + PPD_DURATION;
  
  if ((millis()-PPD_STARTTIME) > PPD_SAMPLETIME_MS){
    
    PPD_RATIO = PPD_LOWPULSEOCCUPANCY/(PPD_SAMPLETIME_MS * 10.0);  // Integer percentage 0=>100
    PPD_CONCENTRATION = (1.1 * pow(PPD_RATIO,3)) + (-3.8 * pow(PPD_RATIO,2)) + (520 * PPD_RATIO) + 0.62; // using spec sheet curve
    
    if(PPD_RATIO == 0){
      PPD_CONCENTRATION = 0;
    }
    
    PPD_LOWPULSEOCCUPANCY = 0;
    PPD_STARTTIME = millis();
  }  
}

// aCount is the number of bytes received.
void receiveCallback(int aCount) {
  
  if(aCount == 2) {
    
    int receivedValue  = Wire.read() << 8;
    receivedValue |= Wire.read();
    Serial.println(receivedValue);
    
  } else {
    
    Serial.print("Unexpected number of bytes received: ");
    Serial.println(aCount);
  }
}

void requestCallback(){
   
 unsigned char byteArray[4];
              
 // convert from an unsigned long int to a 4-byte array
 byteArray[3] = (unsigned char)((PPD_CONCENTRATION >> 24) & 0xFF) ;
 byteArray[2] = (unsigned char)((PPD_CONCENTRATION >> 16) & 0xFF) ;
 byteArray[1] = (unsigned char)((PPD_CONCENTRATION >> 8) & 0XFF);
 byteArray[0] = (unsigned char)((PPD_CONCENTRATION & 0XFF));
 
 Wire.write(byteArray, 4);
}

