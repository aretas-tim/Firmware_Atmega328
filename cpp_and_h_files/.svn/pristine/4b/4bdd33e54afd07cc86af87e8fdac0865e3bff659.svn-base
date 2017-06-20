#include "SensorCalibration.h"
#include <string.h>
#include <avr/pgmspace.h>

SensorCalibration::SensorCalibration(float x1 = 0.00, float x2 = 0.00, float y1 = 0.00, float y2 = 0.00){
 
  _x1 = x1;
  _x2 = x2;
  _y1 = y1;
  _y2 = y2;
  
}

float SensorCalibration::m(){
 
  return (_y2 - _y1) / (_x2 - _x1); 
  
}

float SensorCalibration::b(){

  return(_y1 - (m() * _x1));
}

