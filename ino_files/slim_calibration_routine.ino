#include <EEPROM.h>
#include <SoftwareSerial.h>
#include "EC4-20-NO2.h"
#include "EC4-500-CO.h" 
#include "XBeeFunc.h"
#include "slim_calibration.h"
#include "EEPROMAnything.h"


//the string that will be used to compare user input with
//set to "calibrate;" because that is the commend to enter sensor calibration routine
char enter_calibration[] = "calibrate;";
//the character that the user will end all input with.
//used to determine th end of a string
char line_delimiter= ';';

//the global array that will hold the sensor slope and intercept
//that will be read from the EEPROM on start up. will be stored like
// CO_slope_intercept[] = {slope,intercept};
float CO_slope_yIntercept[2];

//the global array the will hold the sensor slope and intercept
//that will be read from the EEPROM on start up. will be stored like
// NO2_slope_intercept[] = {slope,intercept};
float NO2_slope_yIntercept[2];

//Sensor, calibration routine and XBEE objects
EC4_500_C0 CO_sensor = EC4_500_C0(&Serial, CO_SENSOR_PIN);
EC4_20_NO2 NO2_sensor = EC4_20_NO2(&Serial, NO2_SENSOR_PIN);
Calibration cal_routine = Calibration(&Serial, line_delimiter);
XBeeFunc xb = XBeeFunc(&Serial);



void setup() {
  
  //set the PAN ID if XBEE is not an XBEE PRO
    //xb.setPanID(panID);
    Serial.print(F("All commands sent through the Arduino Serial Monitor must end with \'"));
    Serial.print(line_delimiter);Serial.println(F("\' in order to be understood."));

    //this function reads two floats from EEPROM at the address specified by NO2_DATA_EEPROM_ADDRESS
    // after the call, NO2_slope_intercept[] contains the slope and y-intercept values calculated during the
    //previous calibration routine. The array has the form NO2_slope_intercept[] = {slope,intercept};
    //if the returned values are non-sensical, then it probably means the sensors have not been calibrated
    EEPROM_readAnything(NO2_DATA_EEPROM_ADDRESS, NO2_slope_yIntercept);
    Serial.print(F("EEPROM N02 slope and intercept are: ")); Serial.print(NO2_slope_yIntercept[0]);Serial.print(F(", "));Serial.println(NO2_slope_yIntercept[1]);

    //this function reads two floats from EEPROM at the address specified by NO2_DATA_EEPROM_ADDRESS
    // after the call, NO2_slope_intercept[] contains the slope and y-intercept values calculated during the
    //previous calibration routine. The array has the form NO2_slope_intercept[] = {slope,intercept};
    //if the returned values are non-sensical, then it probably means the sensors have not been calibrated
    EEPROM_readAnything(CO_DATA_EEPROM_ADDRESS, CO_slope_yIntercept);
    Serial.print(F("EEPROM CO slope and intercept are: ")); Serial.print(CO_slope_yIntercept[0]);Serial.print(F(", "));Serial.println(CO_slope_yIntercept[1]);


}

void loop() {
  // the initial state of the calibration routine checks to see if there is any serial data
   // available. if so it checks the input data to see if the user entered the command to initate
   // calibration.
    if(cal_routine.getUserInput(line_delimiter)){
        
        //if input Serial data matches the "enter_calibration" command
        if(cal_routine.checkCommand(enter_calibration)){

            //enters the calibration routine loop defined in calibration.cpp
            cal_routine.changeState(initial);
            cal_routine.calibrationRoutine();
      
        }
    }

}
