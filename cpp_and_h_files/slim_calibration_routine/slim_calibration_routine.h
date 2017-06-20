
#include <Arduino.h> 
//#include "EC4-500-CO.h" 
//#include "EC4-20-NO2.h"
//#include <EEPROM.h>

#ifndef calibration_h // Basically, this prevents problems if someone accidently #include's the library twice.
#define calibration_h // starting .h
#endif

//min number a of bytes for a legal data packet sent over serial
#define MINIMUM_DATA_SIZE 1 
//max number of bytes for a legal data packet sent over serial
#define MAX_DATA_SIZE 49 
//the pin the CO sensor is wired to on the circuit board
#define CO_SENSOR_PIN 0 
//the pin the NO2 sensor is wired to on the circuit board
#define NO2_SENSOR_PIN 1
//the number of times a sensor reading is taken and averaged out when returning a measurement
#define NUM_LOOP 20
//The number of sensors on the board. Used when parsing user input and saving sensor value programatically 
#define NUM_SENSORS 2

//the address to which the NO2 slope and y-intercept will be written and read from
#define NO2_DATA_EEPROM_ADDRESS 0
//the address to which the CO slope and y-intercept will be written and read from
#define CO_DATA_EEPROM_ADDRESS 100

typedef enum States
{ initial,
  choose_sensor,
  input_gas_ppm,
  print_sensor_vals,
  enter_sensor_voltage,
  exit_calibration,
  check_user_input,
  idle,
  set_ppms,
  set_values,
  verify_ppms,
  verify_values,
  calc_slope_save
  
  };

typedef enum Sensors{ CO,NO2 };





class Calibration
{
  public:
  
    Calibration(HardwareSerial *serialPort, char delimiter);
    boolean getUserInput(char);
    boolean checkCommand(const char* command);
    void changeState(States next_state);
    int getCurrState();
    void calibrationRoutine();
    float getSensorVal(int inPin);
    void parseSerialData(char*, float*);
    void calcSlopeAndSave(float*,float*);
    
  private:

    HardwareSerial *serial;
  
    //
    char serialDataIn[50];
    //the variable used to keep track of the current state in the calibration routine
    int prev_state;
    //the variable used to keep track of the previous state in the calibration routine
    int curr_state;
    //the variable used to record the character that denotes the end of user input.
    char serial_delimiter;
    //the variable used to control when the calibration routine will be exited
    boolean exit_routine;
    // the pin on the Atmega the sensor that is currently being calibrated is connected to
    int current_sensor_pin;

    //the flag used to tell if the user has set the sensor values
    boolean values_set;
    //the flag used to tell if the user has set the ppm values
    boolean ppms_set;
    
    /*//the low ppm CO sensor value
    float low_CO_sensor_value;
    //the hi ppm CO sensor value
    float hi_CO_sensor_value;
    //the low ppm NO2 sensor value
    float low_NO2_sensor_value;
    //the hi ppm NO2 sensor value
    float hi_NO2_sensor_value;*/
    
    //this array will hold the values for the sensors
    //currently it stores them in the format: 
    //CO_low_val,CO_hi_val,NO2_low_val,NO2_hi_val
    //It is initialized to NUM_SENSORS*2 because there are
    //2 values recorded for each sensor (1 for low gas concentration,1 for high gas concentration)
    float sensor_values[NUM_SENSORS*2];
    
   /* //the low CO ppm value
    float low_CO_sensor_ppm;
    //the hi ppm CO ppm value
    float hi_CO_sensor_ppm;
    //the low ppm NO2 ppm value
    float low_NO2_sensor_ppm;
    //the hi ppm NO2 ppm value
    float hi_NO2_sensor_ppm;*/

    //this array will hold the ppm values for the test gasses
    //currently it stores them in the format: 
    //CO_low_ppm,CO_hi_ppm,NO2_low_ppm,NO2_hi_ppm
    //It is initialized to NUM_SENSORS*2 because there are
    //2 values recorded for each sensor (1 for low gas concentration,1 for high gas concentration)
    float gas_ppms[NUM_SENSORS*2];
    
    //when incomming serial data is deemed legal, the data is copied to this container
    //char* current_command;


    
    
};
