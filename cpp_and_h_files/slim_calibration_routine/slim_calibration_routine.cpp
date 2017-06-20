#include "slim_calibration.h"
#include "EEPROMAnything.h"


//the array that holds all the commands that the user can type into the serial monitor to initiate state change
//any new commands that will be added must be added here and they must end with the appropriate delimiting character
//as of writing this the delimiting character is ";". so all the commands end with a ";"
const char* commands[] = {"CO;","NO2;","exit;","pause;", "set values;","set ppms;","verify values;","verify ppms;","calc slope;"};
//the parseing function uses the strtok() function which requires a string with all the delimiting characters. 
//In this case, the parsed values will be seperated by "," and the entered string will be terminated with ";"
char parsing_delimiters[] = {",;"};
//a flag used to ensure certain messages will only be printed once to the screen.
//this reduces the clutter of the srial monitor when calibrating
boolean print_once_flag = false;




/* Object Constructor */
//Sets the serial Port
//intializes state machine variables: prev_state, curr_state
//Sets delimiting character
//intitializes exit_routine flag to true
//sets the pin the sensor that is to be read and printed to the screen is on
Calibration::Calibration(HardwareSerial *serialPort, char delimiter){

    //the serial port used for printing
    serial = serialPort;
    // a variable used to keep track of the previous state
    prev_state = check_user_input;
    // a variable used to keep track of the current state
    curr_state = check_user_input;
    //the character used to denote the end of user input
    serial_delimiter = delimiter;
    //the variable used to control weather or not the calibration routine will be exited
    //initialized to true because the routine should not be enabled upon start up
    exit_routine = true;
    //initializing the current sensor pin to A0 (analog 0)
    current_sensor_pin = CO_SENSOR_PIN;
    //
    //current_command = "none";




}


//****************************************************************************
//This function is the calibration routine. It is a loop that continues to run as long as the
// exit_routine variable remains false. Inside that loop is a switch statement that depends on the 
// curr_state variable. It is called from the main .ino script
// THe states that are part of this routine as of writing this are:
//
//                    initial,
//                    choose_sensor,
//                    input_gas_ppm,
//                    print_sensor_vals,
//                    enter_sensor_voltage,
//                    exit_calibration,
//                    check_user_input,
//                    idle,
//                    set_ppms,
//                    set_values,
//                    verify_ppms,
//                    verify_values,
//                    calc_slope_save
//                   
//****************************************************************************
void Calibration::calibrationRoutine(){

    exit_routine = false;

    
    while(!exit_routine){
        switch (getCurrState()){ 
        
        //*************************************************************//
        // The Initial state prints stuff to screen and transitions to //
        // the idle state to wait for user input                       //
        //*************************************************************//
        case initial :
            //zero out all the sensor values that will be temporarily saved
            sensor_values[0] = 0.0;
            sensor_values[1] = 0.0;
            sensor_values[2] = 0.0;
            sensor_values[3] = 0.0;
            //zero out all the ppm values that will be temporarily saved
            gas_ppms[0] = 0.0;
            gas_ppms[1] = 0.0;
            gas_ppms[2] = 0.0;
            gas_ppms[3] = 0.0;
            //indicating to the state machine that the sensor and ppm values have not been set
            //going to be used in error checking to make sure the user doesn't save the incorrect slopes
            values_set = false;
            ppms_set = false;
            //the flag used to only print messages to the screen once
            //while in the idle state
            print_once_flag = false;
            Serial.println(F(""));
            Serial.println(F("Calibration started."));
            //Serial.println(F("Sensors on board:"));
            //Serial.println(sensor_names[CO]);
            //Serial.println(sensor_names[NO2]);
            //Serial.println(F(""));
            changeState(idle);
        break; 
         
        //*************************************************************//
        // the state that checks the users input commands are legal and//
        // then changes state based on those commands                  //
        //*************************************************************//
        case check_user_input :
              //if input Serial data matches the "CO;" command
              if(checkCommand(commands[0])){
                    Serial.println(F("You typed \"CO;\". Printing CO sensor values to the screen"));
                    //change the sensor input pin to the one the CO sensor is connected to
                    current_sensor_pin = (int) CO_SENSOR_PIN;
                    //proceed to the state that collects and prints out the current sensors measurements
                    changeState(print_sensor_vals);
               //if input Serial data matches the "NO2;" command
               }else if(checkCommand(commands[1])){
                    Serial.println(F("You typed \"NO2;\". Printing NO2 sensor values to the screen")); 
                    //change the sensor input pin to the one the NO2 sensor is connevcted to
                    current_sensor_pin = (int) NO2_SENSOR_PIN;
                    //proceed to the state that collects and prints out the current sensors measurements
                    changeState(print_sensor_vals);
               //if input Serial data matches the "exit;" command 
               }else if(checkCommand(commands[2])){
                    Serial.println(F("You typed \"exit;\". Exiting calibration mode."));
                    //exit the calibration routine altogether
                    changeState(exit_calibration);
                    exit_routine = true;
               //if input Serial data matches the "pause;" command   
               }else if(checkCommand(commands[3])){
                    Serial.println(F("You typed \"pause;\". Entering Idle mode."));
                    //proceed to the idle state that waits for user input
                    changeState(idle);
               //if input Serial data matches the "set values;" command      
               }else if(checkCommand(commands[4])){
                    Serial.println(F("You entered \"set values;\".")); 
                    Serial.println(F("Type the 4 sensor values you wish to use in the format \"CO low,CO hi,NO2 low,NO2 hi;\"."));
                    Serial.println(F("i.e 11.11,22.22,33.33,44.44; (no spaces, seperated by commas, ending in a semi-colon). "));
                    //proceed to the state that collects and parses the voltage values of the on board sensors
                    changeState(set_values);
               //if input Serial data matches the "set ppms;" command      
               }else if(checkCommand(commands[5])){
                    Serial.println(F("You entered \"set ppms;\".")); 
                    Serial.println(F("Type the 4 ppm values you wish to use in the format \"CO low,CO hi,NO2 low,NO2 hi;\"."));
                    Serial.println(F("i.e 0.0,10.0,0.0,2.0; (no spaces, seperated by commas, ending in a semi-colon). "));
                    //proceed to the state that collects and parses the ppm values of the on board sensors
                    changeState(set_ppms);
               //if input Serial data matches the "verify values;" command     
               }else if(checkCommand(commands[6])){
                    Serial.println(F("You entered \"verify values\""));
                    Serial.print(F("The current CO low value is: ")); Serial.println(sensor_values[0]);
                    Serial.print(F("The current CO hi value is: ")); Serial.println(sensor_values[1]);
                    Serial.print(F("The current NO2 low value is: ")); Serial.println(sensor_values[2]);
                    Serial.print(F("The current NO2 hi value is: ")); Serial.println(sensor_values[3]);
                    //proceed to the state that waits for user input
                    changeState(idle);
               //if input Serial data matches the "verify ppm;" command
               }else if(checkCommand(commands[7])){
                    Serial.println(F("You entered \"verify ppms\""));
                    Serial.print(F("The current CO low ppm is: ")); Serial.println(gas_ppms[0]);
                    Serial.print(F("The current CO hi ppm is: ")); Serial.println(gas_ppms[1]);
                    Serial.print(F("The current NO2 low ppm is: ")); Serial.println(gas_ppms[2]);
                    Serial.print(F("The current NO2 hi ppm is: ")); Serial.println(gas_ppms[3]);
                    //proceed to the state that waits for user input
                    changeState(idle);
               //if input Serial data matches the "calc slope;" command
               }else if(checkCommand(commands[8])){
                    Serial.println(F("You entered \"calc slope\""));
                    //if the sensor values have been set
                    if(values_set == true){
                        //Serial.print(F("values_set: "));Serial.println(values_set);
                        //if the ppm values have been set
                        if(ppms_set == true){
                            //Serial.print(F("ppms_set: "));Serial.println(ppms_set);
                            //proceed to the state that calculates the slop and y-intercept of the 
                            //sensor's linear ouput
                            changeState(calc_slope_save);
                        }else{
                            Serial.println(F("PPM values have not been set. Please do so to continue. "));
                        }
                     }else{
                        Serial.println(F("The sensor values have not been set. Please do so to continue. "));
                     }
                 }else{
                     //the command entered by the user was not one of the legally recognizable options
                     Serial.println(F("Did not recognize command. Enter either: "));
                     Serial.println(commands[0]);
                     Serial.println(commands[1]);
                     Serial.println(commands[2]);
                     Serial.println(commands[3]);
                     Serial.println(commands[4]);
                     Serial.println(commands[5]);
                     Serial.println(commands[6]);
                     Serial.println(commands[7]);
                     Serial.println(commands[8]);
                     //proceed to the state that waits for user input
                     changeState(idle);
                }
            break;
        
        //*************************************************************//
        // THe state where sensor values are printed to the screen     //
        //*************************************************************//
        case print_sensor_vals:
            //if the current sensor is CO, then get and print the CO sensor val
            if(current_sensor_pin == (int) CO_SENSOR_PIN){
                Serial.print(F("CO Voltage value: "));
                Serial.print(getSensorVal(CO_SENSOR_PIN));
                Serial.println(F(""));
                delay(1000);
            }else{
                //if the current sensor is NO2, then get and print the NO2 sensor val
                Serial.print(F("NO2 Voltage value: "));
                Serial.print(getSensorVal(NO2_SENSOR_PIN));
                Serial.println(F(""));
                delay(1000);  
            }
            //if the user typed something go to the check_user_input state
            //will keep printing sensor values untill this happens
            if(getUserInput(serial_delimiter)){
               changeState(check_user_input);
            }
        break;
        
        //*************************************************************//
        //The idle state. Where nothing is done but wait for user input//
        //*************************************************************//
        case idle:
            //if the below text hasn't been printed yet, print it.
            if(print_once_flag == false){
                Serial.println(F(""));
                Serial.println(F("To pause enter \"pause;\""));
                Serial.println(F("To print the CO sensor values to the screen type \"CO;\""));
                Serial.println(F("To print the NO2 sensor values to the screen type \"NO2;\""));
                Serial.println(F("To set the 4 sensor values type \"set values;\""));
                Serial.println(F("To set the 4 ppm values type \"set ppms;\""));
                Serial.println(F("To print the current ppm values type \"verify ppms;\""));
                Serial.println(F("To print the current sensor values type \"verify values;\""));
                Serial.println(F("Calculate the sensor slopes and y-intercepts, and write them to EEPROM type \"calc slope;\""));
                Serial.println(F("To exit type \"exit;\""));
                Serial.println(F(""));
                Serial.println(F("Waiting for user input.................."));
                Serial.println(F(""));
                print_once_flag = true;
            }
            //wait for user to type something in
            if(getUserInput(serial_delimiter)){
                print_once_flag = false;
                changeState(check_user_input);
                
            }
            //this delay may not be necissary
            //delay(10);
        break;
        
        //********************************************************************************//
        //the state where the user inputs the sensor values they want to set.              // 
        //the values must be in the format:                                               //
        //
        //                      CO_low_val,CO_hi_val,NO2_low_val,NO2_hi_val;              //
        //                       i.e     23.1,56.4,41.9,0.56;
        //********************************************************************************// 
        case set_values:
            //wait for the user to input the sensor values
            if(getUserInput(serial_delimiter)){
                Serial.print(F("You entered: "));Serial.println(serialDataIn);
                Serial.println(F(""));
                //send the user input to be parsed in the parsing function
                parseSerialData(parsing_delimiters, sensor_values);
                values_set = true;
                changeState(idle);
            } 
        break;
        
        //********************************************************************************//
        //the state where the user inputs the ppm values they want to set.              // 
        //the values must be in the format:                                               //
        //
        //                      CO_low_ppm,CO_hi_ppm,NO2_low_ppm,NO2_hi_ppm;              //
        //                       i.e     0.00,10.0,0.00,2.00;
        //********************************************************************************//
        case set_ppms:
            if(getUserInput(serial_delimiter)){
                Serial.print(F("You entered: "));Serial.println(serialDataIn);
                Serial.println(F(""));
                ppms_set = true;
                //send the user input to be parsed in the parsing function
                parseSerialData(parsing_delimiters, gas_ppms);
                changeState(idle);
            }
        break;
        
        case calc_slope_save:
            Serial.print(F("Calculating and writing values to EEPROM: "));
            Serial.println(F(""));
            calcSlopeAndSave(gas_ppms,sensor_values);
            changeState(idle);
        break;
        }//switch(getCurrState)
    }//while(!exit_routine)
}

//****************************************************************************
//This function gets serial data and copies it to the serialDataIn object field if serial data is available.
//It's input paramater is the character that terminates the input string. 
//Currently the delimiter character is ";".
//This function returns true if serial data was recieved.
//****************************************************************************
boolean Calibration::getUserInput(char delim){

    int index = 0;
    char inData[50];
    boolean hasSerial = false;
    while(Serial.available() > 0){
        char aChar = Serial.read();
        if(aChar == delim){
            inData[index] = aChar;
            index++;
            inData[index] = '\0'; // Keep the string NULL terminated
            if((index >= MINIMUM_DATA_SIZE)&&(index <= MAX_DATA_SIZE)){
                strcpy(serialDataIn,inData);
                hasSerial = true;
            }else{
                Serial.println("Data packet was not a legal size. ignoring.");
            }
            index = 0;
            inData[index] = '\0';
        }else{   
            inData[index] = aChar;
            index++;
            inData[index] = '\0';
            //added this delay because without it receivng the full transmission failed. 
            delay(2);
        }
    }//while serial.available
    return hasSerial;
}
    
//****************************************************************************    
//This function will check if the incomming serial data is equal to a given command.
// i.e This function compares the passed command to the most current serial data and 
//determine if they are equivalent
//****************************************************************************
boolean Calibration::checkCommand(const char* command){

    boolean is_command = false;
    //if command and incomming serial data are equal
    if(strcmp(command,serialDataIn) == 0){
        is_command = true;
    }
    return is_command;
}

//****************************************************************************
// This function reads and returns the voltage of the sensor attached to inPin
//****************************************************************************
float Calibration::getSensorVal(int inPin){

    int rawVal = 0;
    float voltage = 0.0;
    int i=0;
    //read the sensor value and add it to the total NUM_LOOP times (currently 20)
    for(i=0; i<NUM_LOOP ;i++){
        rawVal += analogRead(inPin);
    }
    //take the average
    rawVal = rawVal/NUM_LOOP;
    //convert the raw value to a voltage between 0-5v
    voltage = (float)(5.0*rawVal/1023.0);
    return voltage;
}

//****************************************************************************
// This function change the state of the state machine to the next_state input parameter
//****************************************************************************
void Calibration::changeState(States next_state){

    prev_state = curr_state;
    curr_state = next_state;
    return;
}

//****************************************************************************
// This function returns the current state of the state machine
//****************************************************************************
int Calibration::getCurrState(){

    return curr_state;
}

//****************************************************************************
//This function parses user input when the data is seperated by the characters
//passed in the delimiters parameter. The outputArray[] parameter is the destination of the parsed data
//This function allocates enough memory in the token_array[] to handle 2 values per sensor
//****************************************************************************
void Calibration::parseSerialData(char* delimiters, float outputArray[]){

    int i = 0;
    char* p;
    int index = 0;
    //create an array to temperarily contain the parsed data
    //each sensor will have two value attached to it, so the array must be
    //atleast (NUM_SENSORS*2)+1 in size. The +1 is for an extra null character 
    char* token_array[(NUM_SENSORS*2)+1];
    
    //parse the first value from serialDataIn and assign it to token_array
    p = strtok(serialDataIn, delimiters); //2nd argument is a char[] of delimiters
    token_array[index] = p;
    index++;
    //continue parseing the string untill the parses value is NULL
    while (p != '\0'){ 
        //strtok() expects NULL for string on subsequent calls to the same string
        p = strtok('\0', delimiters);  
        token_array[index] = p;
        index++;
    }
    //This loop converts the values contained in token_array to floats and 
    //assigns them to the input value_array. The loop iterates ((sizeof(token_array)/2)-1) times
    //because a char* is two bytes on the atmega328p, hence the sizeof(token_array)/2. 
    //The -1 is for the extra space in token_array that was allocated for the NULL character
    for(i = 0; i < ((sizeof(token_array)/2)-1); i++){
        //Serial.print("copying element: "); Serial.println(i);
        //Serial.print("token array element is: ");Serial.println(token_array[i]);
        outputArray[i] = ((float) atof(token_array[i]));
        //Serial.print("output element is: ");Serial.println(outputArray[i]);
    }
} 

void Calibration::calcSlopeAndSave(float ppms[],float sensorVals[]){

    //the temporary y axis variable used in calculate the slopes of the sensors
    float CO_y1 = ppms[0];
    float CO_y2 = ppms[1];
    float NO2_y1 = ppms[2];
    float NO2_y2 = ppms[3];
    //the temporary x axis variable used in calculate the slopes of the sensors
    float CO_x1 = sensorVals[0];
    float CO_x2 = sensorVals[1];
    float NO2_x1 = sensorVals[2];
    float NO2_x2 = sensorVals[3];
    //the array that will contain the slope and y-intercept for the NO2 sensor
    // it will be stored in the order NO2_slope_intercept[] =  {slope, y-intercept}
    float NO2_slope_intercept[2];
    //the array that will contain the slope and y-intercept for the CO sensor
    // it will be stored in the order CO_slope_intercept[] =  {slope, y-intercept}
    float CO_slope_intercept[2];
    
    //calculating the slope and y-intercept of the NO2 sensor
    NO2_slope_intercept[0] = (NO2_y2 - NO2_y1)/(NO2_x2 - NO2_x1);
    NO2_slope_intercept[1] = NO2_y1-(NO2_slope_intercept[0]*NO2_x1);
    //calculating the slope and y-intercept of the CO sensor
    CO_slope_intercept[0] = (CO_y2 - CO_y1)/(CO_x2 - CO_x1);
    CO_slope_intercept[1] = CO_y1-(CO_slope_intercept[0]*CO_x1);
    
    //Serial.print(F("before EEPROM N02 slope and intercept are: ")); Serial.println(NO2_slope_intercept[0]);Serial.println(F(", "));Serial.println(NO2_slope_intercept[1]);
    //Serial.print(F("before EEPROM C0 slope and intercept are: ")); Serial.println(CO_slope_intercept[0]);Serial.println(F(", "));Serial.println(CO_slope_intercept[1]);
    //save the slopes and y-intercepts of the sensors to EEPROM
    //the DATA_EEPROM_ADDRESS constants are defined in the "calibration.h" header file
    EEPROM_writeAnything(NO2_DATA_EEPROM_ADDRESS,NO2_slope_intercept);
    EEPROM_writeAnything(CO_DATA_EEPROM_ADDRESS,CO_slope_intercept);
    //read back the the slopes and y-intercepts of the sensors to EEPROM
    EEPROM_readAnything(NO2_DATA_EEPROM_ADDRESS, NO2_slope_intercept);
    Serial.print(F("The N02 slope and intercept values written to EEPROM are: ")); Serial.print(NO2_slope_intercept[0]);Serial.print(F(", "));Serial.println(NO2_slope_intercept[1]);
    Serial.println(F(""));
    EEPROM_readAnything(CO_DATA_EEPROM_ADDRESS, CO_slope_intercept);
    Serial.print(F("The CO slope and intercept values written to EEPROM are: ")); Serial.print(CO_slope_intercept[0]);Serial.print(F(", "));Serial.println(CO_slope_intercept[1]);
    Serial.println(F(""));
} 


