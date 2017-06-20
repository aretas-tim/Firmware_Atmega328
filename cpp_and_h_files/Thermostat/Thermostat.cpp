#include "Thermostat.h"
#include <avr/pgmspace.h>
#include <EEPromAnything.h>
#include <EEAddresses.h>
#include <NewSoftSerial.h>

/* the hysteresis will dampen frequent oscillations it is the number of degrees, 
 * centered on the setpoint that we will use as a fuzzy threshold */
#define HYSTERESIS_T 1

/* 
 * deadband is where no control actions will take place 
 * 
 * A thermostat which sets a single temperature and automatically controls both 
 * heating and cooling systems without a mode change exhibits a deadband range around the target temperature. 
 * The low end of the deadband is just above the temperature where the heating system turns on. 
 * The high end of the deadband is just below the temperature where the air-conditioning system starts.
 * 
 * */
#define DEADBAND_H 26
#define DEADBAND_L 10

/* this is the minimum time (in milliseconds) to wait between ON cycles 
 * 300000 = 5 mins (5*60*1000)
 * */
#define CYCLE_IDLE_TIME_MS 300000

/* 
 * maximum amount of time we'll let the heating cycle run before taking a break 
 * 1800000 = 30 mins (30*60*1000)
 * */
#define MAX_CYCLE_TIME_MS 1800000 

/* arbitrary min and max setpoints */
#define MAX_TEMP 32
#define MIN_TEMP 0

#define FALLBACK_SETPOINT 21

#define ON 1
#define OFF 0

#define DEBUG

/*
 * the last time the cycle (in microprocessor time) started 
 */
unsigned long _lastStartTime = 0;

/*
 * the last time the cycle (in microprocessor time) ended 
 */
unsigned long _lastEndTime = 0;
/*
 * the setpoint in degrees C for the temperature
 */
double _setpoint = 0;
/* 
 * state is either on or off (controls the main furnace relay)
 */
byte _STATE = 0;

boolean FIRSTRUN = true;

NewSoftSerial *_serial;

Thermostat::Thermostat(double setpoint, NewSoftSerial *serial){
	
	_setpoint = setpoint;
	_serial = serial;

}

Thermostat::Thermostat(NewSoftSerial *serial){

	_serial = serial;
	
	double ee_sp = 0; //eeprom setpoint
	
	//check and see if we have a valid setpoint in eeprom
	EEPROM_readAnything(SETPOINT_T_A_ADDR, ee_sp);
	
	if(isValidSetpoint(&ee_sp)){
		_setpoint = ee_sp;
	}else{
		_setpoint = FALLBACK_SETPOINT;
	}

}


boolean Thermostat::isValidSetpoint(double *ee_sp){
	
	boolean ret = true;

	if(*ee_sp > MAX_TEMP_SP) ret = false;
	if(*ee_sp < MIN_TEMP_SP) ret = false;
	if(*ee_sp < MIN_TEMP)    ret = false;
	if(*ee_sp > MAX_TEMP)    ret = false;
	
	return true;
	
}

/** this function returns the state the heating relay should be in (on or off) **/ 
byte Thermostat::getHeatingState(double curTemp){
	
	unsigned long now = millis();
	
	unsigned long tdiff  = now - _lastStartTime; //amount of time elapsed since the last cycle started
	unsigned long tdiff2 = now - _lastEndTime; //amount of time elapsed since the last cycle ran
	
	double ee_sp = 0; //eeprom setpoint
	
	//minimize writes, check and see if the data is the same
	EEPROM_readAnything(SETPOINT_T_A_ADDR, ee_sp);
	
	if((ee_sp != _setpoint) && isValidSetpoint(&ee_sp)){
		
		_setpoint = ee_sp;
		
	}
	
	if(_STATE == ON){
		
		/**
		* if we've been running too long 
	    */
	
		if(tdiff >= (MAX_CYCLE_TIME_MS)){
			
			_STATE = OFF;
			_lastEndTime = now;
			
#ifdef DEBUG
			_serial->print("TURNING OFF CONTROL, CYCLE TIME EXCEEDED. ELAPSED TIME: ");
			_serial->print(tdiff, DEC);
			_serial->print(" MAX CYCLE TIME: ");
			_serial->print(MAX_CYCLE_TIME_MS);
			_serial->print('\n');
#endif
			return _STATE;
		}
		
		//TODO: this is where we can add in some overshoot compensation
		if(curTemp >= _setpoint){

			_STATE = OFF;
			_lastEndTime = now;
			
#ifdef DEBUG
			_serial->print("TURNING OFF CONTROL, TEMP EXCEEDS THRESHOLD: ");
			_serial->println(curTemp, DEC);
#endif
			return _STATE;
		}
		
	/** for whatever reason the current state is off 
	 * check the following
	 * 
	 * if curTemp is below setpoint + 1/2 of hysteresis 
	 * if we have waited at least CYCLE_IDLE_TIME 
	 */	
	}else if(_STATE == OFF){
		
		_serial->println("OFF");
		
		if(curTemp < _setpoint){
			
			//are we outside of the hysteresis?
			if(fuzzyThreshold(curTemp, _setpoint, HYSTERESIS_T) && ((tdiff2 >= CYCLE_IDLE_TIME_MS) || (FIRSTRUN == true))){
				
				if(FIRSTRUN == true){
					FIRSTRUN = false;
				}
				
				_lastStartTime = now;
				_STATE = ON;
				
#ifdef DEBUG
			_serial->println("TURNING ON: ");
			_serial->println(curTemp, DEC);
#endif
			}else{
#ifdef DEBUG
				_serial->print("CURTEMP [");
				_serial->print(curTemp);
				_serial->print("] SP [");
				_serial->print(_setpoint);
				_serial->print(" TDIFF2 [");
				_serial->print(tdiff2);
				_serial->print("]");
#endif
			}
			
		}
		
		return _STATE;
		
	}
	
	return _STATE;
	
}
/**
 * This function provides the hysteresis to prevent excess hunting around the setpoint 
 */
boolean Thermostat::fuzzyThreshold(double a, double b, double MAX_ERR){
	
	double diff = a - b;
	diff = abs(diff);
	MAX_ERR = MAX_ERR / 2;
	if(diff > MAX_ERR){
		return true;
	}
	return false;
}


void Thermostat::changeSetPoint(double setpoint){
	_setpoint = setpoint;
}

double Thermostat::getSetPoint(){
	return _setpoint;
}
