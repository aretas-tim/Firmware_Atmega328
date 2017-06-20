/*
  BridgeSerial.h - Bridge communication library
  Created by Daniel Pothier.
*/
#ifndef BridgeSerial_h
#define BridgeSerial_h

/* start of the cmd protocol section */
#define SETPOINT_T_A 0x01 //set setpoint 1 command
#define SETPOINT_T_B 0x02 //set setpoint 2 command
#define ACK_SP 0xFA       //ACK setpoint header

#include "WProgram.h"
#include <NewSoftSerial.h>

typedef struct {
	unsigned long mac;
	byte cmd;
	double data;
}CMD;

class BridgeSerial
{
  public:
	BridgeSerial(HardwareSerial *serialPort);
	BridgeSerial(HardwareSerial *serialPort, NewSoftSerial *softSerial);
	boolean readBuffer(unsigned long *mac);
	int available();
	static void sendValue(unsigned long mac, int sensorType, float value, HardwareSerial *serial);

  private:
    byte getsTimeout(char *buf, uint16_t timeout);
    boolean decodePacket(char *buf, unsigned long *mac, CMD *cmd);
    HardwareSerial *_serial;
    NewSoftSerial  *_softserial;
    boolean haveSoftSerial;
    void ackSetpoint(unsigned long *mac, double *setpoint);
};

#endif