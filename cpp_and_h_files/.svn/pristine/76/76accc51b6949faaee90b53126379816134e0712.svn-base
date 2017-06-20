/*
  SensorCalibration.h - Class to pass around and calculate calibration constants 
  Created by Daniel Pothier.
*/
#ifndef SensorCalibration_h
#define SensorCalibration_h

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

class SensorCalibration
{
  public:
    SensorCalibration(float x1, float x2, float y1, float y2);
    float m();
    float b();

  private:
    float _x1;
    float _x2;
    float _y1;
    float _y2;
};

#endif
