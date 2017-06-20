/*
  Thermostat.h - Library for handling thermostat functions
  Created by Daniel Pothier.
*/
#ifndef Thermostat_h
#define Thermostat_h

#include "WProgram.h"
#include <NewsoftSerial.h>

/*
 * maximum and minimum temperature setpoints 
 */
#define MAX_TEMP_SP 45
#define MIN_TEMP_SP 15

class Thermostat
{
  public:
	Thermostat(double setpoint, NewSoftSerial *serial);
	Thermostat(NewSoftSerial *serial);
	byte getHeatingState(double curTemp);
	void changeSetPoint(double setpoint);
	double getSetPoint();

  private:
	boolean fuzzyThreshold(double a, double b, double MAX_ERR);
	NewSoftSerial *_serial;
	boolean isValidSetpoint(double *ee_sp);
	
};

#endif
