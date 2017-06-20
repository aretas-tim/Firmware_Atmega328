/**
 * So the purpose of this class will be to read, checksum and parse incoming serial packets, 
 * then either dispatch an action or process the action
 * 
 * for instance we will get a broadcast packet from the bridge with a setpoint for the temp
 * with all packets we will need to inspect the packet header for the mac address and make sure it
 * is the same mac address of the radio and if not, we ignore the packet 
 * 
 * the next part will be the action type
 * 
 * the next part will be the "payload" 
 * 
 * we should also add in checksum bytes and a terminating \n
 * 
 * in the case of something like a temp setpoint we may just want to use the eeprom as a type of semaphore
 * 
 * so if we decode a temperature setpoint packet, we write that into the eeprom memory and the Thermostat class just checks
 * the eeprom address on every loop, however we only have 1024 bytes of eeprom memory so we'll need to be extremely frugal!
 * 
 * AND:  
 * An EEPROM write takes 3.3 ms to complete. 
 * The EEPROM memory has a specified life of 100,000 write/erase cycles, so you may need to be careful about how often you write to it.
 * 
 * we may also want to consider not processing any instructions if encryption isn't enabled on the radio  
 * 
 * 
 */
#include "BridgeSerial.h"
#include <EEPromAnything.h>
#include <EEAddresses.h>
#include <Thermostat.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <NewSoftSerial.h>

#define BUF_LENGTH 50

#define DEBUG

char DELIMITER = 0x2C;

BridgeSerial::BridgeSerial(HardwareSerial *serialPort){
	haveSoftSerial = false;
  _serial = serialPort;  
}

BridgeSerial::BridgeSerial(HardwareSerial *serialPort, NewSoftSerial *softSerial){
	
  _serial = serialPort;  
  _softserial = softSerial;
  haveSoftSerial = true;
  
}

int BridgeSerial::available(){
	return (int)_serial->available();
}


boolean BridgeSerial::readBuffer(unsigned long *mac){
	
	char buf[BUF_LENGTH];
	CMD cmd = {0,0,0};
	byte getsRT = 0;
	
	double ee_data = 0;
	
	if((getsRT = getsTimeout(buf, 1000)) > 0){
		
#ifdef DEBUG
		if(haveSoftSerial){
			_softserial->print("DECODING PACKET [");
			_softserial->print(buf);
			_softserial->print("][");
			_softserial->print(getsRT, DEC);
			_softserial->println("]");
		}
#endif
	
		if(decodePacket(buf, mac, &cmd)){
			
			if(cmd.mac != *mac){
#ifdef DEBUG
				if(haveSoftSerial){
					
					_softserial->print("MACS DON'T MATCH: [");
					_softserial->print(*mac, DEC);
					_softserial->print("] [");
					_softserial->print(cmd.mac, DEC);
					_softserial->println("]");
					
				}
				
				
#endif
				return false;
			}
			
#ifdef DEBUG
			if(haveSoftSerial){
				
				_softserial->print("EXECUTING CMD: [");
				_softserial->print(cmd.cmd, DEC);
				_softserial->println("]");
				
			}
#endif
			
			//do something
			switch(cmd.cmd){
				
			//set the temperature setpoint eeprom ADDRESS
			case SETPOINT_T_A:
				
				if(cmd.data > MAX_TEMP_SP){
#ifdef DEBUG
					if(haveSoftSerial){ _softserial->println("> MAX TEMP SP"); }
#endif
					break;
				}
				
				if(cmd.data < MIN_TEMP_SP){
#ifdef DEBUG
					if(haveSoftSerial){ _softserial->println("< MAX TEMP SP"); }
#endif
					break;
				}
				
				//minimize writes, check and see if the data is the same
				EEPROM_readAnything(SETPOINT_T_A_ADDR, ee_data);
				
#ifdef DEBUG
				if(haveSoftSerial){
					_softserial->print("EESP:");
					_softserial->println(ee_data, DEC);
				}
#endif
				
				if(ee_data != cmd.data){
					
#ifdef DEBUG
					if(haveSoftSerial){ _softserial->print("WR SP DATA"); }
#endif					
					EEPROM_writeAnything(SETPOINT_T_A_ADDR, cmd.data);
					
				}
				
				ackSetpoint(mac, &cmd.data);
				
				break;
				
			default:
#ifdef DEBUG
				if(haveSoftSerial){ _softserial->println("INVALID CMD"); }
#endif
					break;
				
			}
			
		}else{
			
#ifdef DEBUG
			if(haveSoftSerial){	_softserial->println("DECODING PACKET FAILED"); }
#endif
			return false;
		}
		
	}
	

	
	return false;
}

void BridgeSerial::ackSetpoint(unsigned long *mac, double *setpoint){
	
	/** 
	 * TODO: think about moving this function to Thermostat.cpp 
	 * since that is when it is actually for sure 
	 * acknowledged that it is received 
	 * 
	 * **/
	_serial->print(*mac, DEC);
	_serial->print(DELIMITER);
	_serial->print(ACK_SP, DEC);
	_serial->print(DELIMITER);
	_serial->print(*setpoint, DEC);
	_serial->print('\n');
}

boolean BridgeSerial::decodePacket(char *buf, unsigned long *mac, CMD *cmd){
	
	boolean gotMac = false;
	boolean gotCmd = false;
	boolean gotDat = false;
	
	char plc[BUF_LENGTH];
	byte plc_incr = 0;
	byte pos = 0;
	
	for(byte i = 0; i < BUF_LENGTH; i++){
		
		/**
		 * we could use a position indicator here instead (i.e. if pos == 1 if pos == 2 etc
		 * but this achieves the same thing and we can re-use the booleans for a return status
		 */
		
#ifdef DEBUG2
		if(haveSoftSerial){	_softserial->println(buf[i]); }
#endif
		
		if(((buf[i] == DELIMITER) && (i > 0)) || (buf[i] == '\n')){
			
			pos++;
			//don't copy it, try and convert it
			
#ifdef DEBUG
			if(haveSoftSerial){
				_softserial->print("GOT TOKEN, POS:");
				_softserial->println(pos, DEC);
			}
#endif
			
			if(pos == 1){
				
#ifdef DEBUG
				if(haveSoftSerial){	_softserial->println("GETTING CMD MAC"); }
#endif
				
				//try and parse for MAC
				cmd->mac = strtoul(plc, NULL, 10);
				
				memset(plc, '\0', sizeof(char) * BUF_LENGTH);
				plc_incr = 0;
				
				if(cmd->mac != 0){
					
					gotMac = true;
					continue;
					
				}else{
#ifdef DEBUG
					if(haveSoftSerial){	_softserial->println("PARSING MAC FAILED"); }
#endif
					gotMac = false;
					break;
			
				}
				
			}
			
			if(pos == 2){
				
#ifdef DEBUG
				if(haveSoftSerial){_softserial->println("GETTING CMD CMD"); }
#endif
				
				//try and parse for CMD
				cmd->cmd = (byte)strtoul(plc, NULL, 10);
				
				memset(plc, '\0', sizeof(char) * BUF_LENGTH);
				plc_incr = 0;
				
				if(cmd->cmd != 0){
					
					gotCmd = true;
#ifdef DEBUG
					if(haveSoftSerial){	_softserial->println("GOT CMD"); }
#endif
					continue;
					
				}else{
#ifdef DEBUG
					if(haveSoftSerial){	_softserial->println("PARSING CMD FAILED"); }
#endif
					gotCmd = false;
					break;
				}
				
			}
			
			if(pos == 3){
				
#ifdef DEBUG
				if(haveSoftSerial){ _softserial->println("GETTING CMD DATA"); }
#endif
				
				//try and parse for DATA PAYLOAD
				cmd->data = strtod(plc, NULL);
				
#ifdef DEBUG
				if(haveSoftSerial){
					_softserial->print("CMD DATA:");
					_softserial->print(cmd->data, DEC);
					_softserial->print('\n');
				}
#endif
				
				memset(plc, '\0', sizeof(char) * BUF_LENGTH);
				plc_incr = 0;
				
				if((cmd->data != 0)){
					
					gotDat = true;
					//we should be done here so break
					break;
					
				}else{
#ifdef DEBUG
					if(haveSoftSerial){	_softserial->println("PARSING DATA FAILED"); }
#endif
					gotDat = false;
					break;
				}
				
			}
			
		}
		
		plc[plc_incr] = buf[i];
		plc_incr++;
		
	}//break or loop finish
	
	memset(plc, '\0', sizeof(char) * BUF_LENGTH);
	memset(buf, '\0', sizeof(char) * BUF_LENGTH);
	
	return(gotMac && gotCmd && gotDat);
	
}



//battle tested gets function for serial reads
byte BridgeSerial::getsTimeout(char *buf, uint16_t timeout) {
  
  byte count = 0;
  long timeIsOut = 0;
  char c;
  *buf = 0;
  timeIsOut = millis() + timeout;
  
  while (timeIsOut > millis() && count < (BUF_LENGTH - 1)) {
	  
	  if (_serial->available() > 0) {
		  
		  count++;
		  c = _serial->read();
		  *buf++ = c;
		  timeIsOut = millis() + timeout;
	  }
	  
	  if((c == 0x0A) || (c == 0x0D)){
		  break;
	  }
  }
  
  //null terminate it
  if (count != 0) {
    *buf = 0;
    count++;
  }
  
  return count;
}

void BridgeSerial::sendValue(unsigned long mac, int sensorType, float value, HardwareSerial *serial){
	
	serial->print(mac, DEC);
	serial->print(",");
	serial->print(sensorType, DEC);
	serial->print(",");
	serial->print(value);
	serial->print("\n");
	
}