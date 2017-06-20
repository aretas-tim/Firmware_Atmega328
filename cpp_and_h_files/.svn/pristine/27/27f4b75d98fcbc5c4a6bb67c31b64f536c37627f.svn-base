#include "XBeeFunc.h"
#include <string.h>
#include <avr/pgmspace.h>

#define BUF_LENGTH 50

const char *atCmdMode = "+++";
const char *atCmdSl = "ATSL\r"; //get low serial number
const char *atCmdCn = "ATCN\r"; //exit AT command mode
const char *atCmdID = "ATID%x\r"; //set or read the PAN ID
const char *atCmdWr = "ATWR\r"; //write settings to nvram

XBeeFunc::XBeeFunc(HardwareSerial *serialPort){

  serial = serialPort;  
}

unsigned long XBeeFunc::getXBeeSerialNum(){
	
	boolean success = false;
	char buf[BUF_LENGTH];
	
	serial->begin(9600);
	delay(1000);
	
	serial->print(atCmdMode);  
	delay(1000); //guard time
	
	unsigned long ul = 0;
	
	if(getsTimeout(buf, 1000) > 0){
  
		if(strstr(buf, "OK")){
		  success = true;
		}
	}
	
	if(success){ //means we're in command mode
		
		success = false;
		serial->print(atCmdSl); //get the low serial number
		
		if(getsTimeout(buf, 1000) > 0){
			
			ul = strtoul(buf, NULL, 16);
			success = true;
		}
	}
	
	if(success){ //means we got the serial number
  
		serial->print(atCmdCn); //explicitly exit AT mode
		delay(10);
	}  
  
	delay(200);
  
	return ul;
  
}

boolean XBeeFunc::setPanID(unsigned long panID){
	
	boolean success = false;
	
	char buf[BUF_LENGTH];
	memset(buf, '\0', BUF_LENGTH);
	
	serial->begin(9600);
	serial->flush();
	
	delay(1000);
	
	serial->print(atCmdMode);  
	delay(1000); //guard time
	  
	if(getsTimeout(buf, 1000) > 0){
	
		if(strstr(buf, "OK")){
		  success = true;
		}
	}
	
	if(success != true){
		serial->println("COULD NOT ENTER COMMAND MODE\n");
		return false; //no point in continuing
	}
	
	memset(buf, '\0', BUF_LENGTH);
	
	if(success){ //means we're in command mode
		
		success = false;
		
		sprintf(buf, atCmdID, panID);
		serial->flush();
		serial->print(buf);
		
		memset(buf, '\0', BUF_LENGTH);
		
		delay(200);
		
		if(getsTimeout(buf, 2000) > 0){
			
			if(strstr(buf, "OK")){
				success = true;	  
			}else{
				
				serial->print("ERROR SETTING PANID:");
				serial->print(buf);
				serial->print('\n');
				//assert success is false
				success = false;
			}
		}else{
			serial->print("ERROR SETTING PANID NO RESPONSE\n");
			
		}
	}
	
	delay(20);
	
	//not really necessary, but write it to firmware
	if(success){ //means we managed to set the panid
		
		success = false;
		memset(buf, '\0', BUF_LENGTH);
		
		serial->print(atCmdWr); //try and write it to firmware
		
		delay(100);
		
		if(getsTimeout(buf, 1000) > 0){
			
			if(strstr(buf, "OK")){
				success = true;
			}else{
				serial->print("COULD NOT WRITE TO FIRMWARE:");
				serial->print(buf);
				serial->print('\n');
				//assert success is false
				success = false;
			}
		}else{
			serial->print("ERROR WRITING FIRMWARE NO RESPONSE\n");
		}
	}
	
	delay(200);
	
	if(success){ //means we managed to write to firmware
		
		serial->print(atCmdCn); //explicitly exit AT mode
		delay(10);
		
	}
	
	delay(200);	
	return success;

}

byte XBeeFunc::getsTimeout(char *buf, uint16_t timeout) {
  
  byte count = 0;
  long timeIsOut = 0;
  char c;
  *buf = 0;
  timeIsOut = millis() + timeout;
  
  while (timeIsOut > millis() && count < (BUF_LENGTH - 1)) {  
    if (serial->available() > 0) {
      count++;
      c = serial->read();
      *buf++ = c;
      timeIsOut = millis() + timeout;
    }
  }
  if (count != 0) {
    *buf = 0;
    count++;
  }
  return count;
}
