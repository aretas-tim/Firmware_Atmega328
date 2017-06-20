// MultiChannels
//
// rcarduino.blogspot.com
//
// A simple approach for reading three RC Channels using pin change interrupts
//
// See related posts - 
// http://rcarduino.blogspot.co.uk/2012/01/how-to-read-rc-receiver-with.html
// http://rcarduino.blogspot.co.uk/2012/03/need-more-interrupts-to-read-more.html
// http://rcarduino.blogspot.co.uk/2012/01/can-i-control-more-than-x-servos-with.html
//
// rcarduino.blogspot.com
//

// include the pinchangeint library - see the links in the related topics section above for details
#include <PinChangeInt.h>

#include <Servo.h>

// Assign your channel in pins
#define THROTTLE_IN_PIN 8
#define STEERING_IN_PIN 9
#define AUX_IN_PIN 10

// Assign your channel out pins
#define THROTTLE_OUT_PIN 5
#define STEERING_OUT_PIN 6
#define AUX_OUT_PIN 7

#define IN true
#define OUT false;

#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Servo objects generate the signals expected by Electronic Speed Controllers and Servos
// We will use the objects to output the signals we read in
// this example code provides a straight pass through of the signal with no custom processing
Servo servoThrottle;
Servo servoSteering;
Servo servoAux;

// These bit flags are set in bUpdateFlagsShared to indicate which
// channels have new signals
#define THROTTLE_FLAG 1
#define STEERING_FLAG 2
#define AUX_FLAG 4

//pins for the motor controller
#define EN_PIN_A 2
#define IN_PIN_A1 3
#define IN_PIN_A2 4

//pin for the magnetic limit sensor 
#define STOP_LIMIT1_PIN A2

// holds the update flags defined above
volatile uint8_t bUpdateFlagsShared;

// shared variables are updated by the ISR and read by loop.
// In loop we immediatley take local copies so that the ISR can keep ownership of the 
// shared ones. To access these in loop
// we first turn interrupts off with noInterrupts
// we take a copy to use in loop and the turn interrupts back on
// as quickly as possible, this ensures that we are always able to receive new signals
volatile uint16_t unThrottleInShared;
volatile uint16_t unSteeringInShared;
volatile uint16_t unAuxInShared;

// These are used to record the rising edge of a pulse in the calcInput functions
// They do not need to be volatile as they are only used in the ISR. If we wanted
// to refer to these in loop and the ISR then they would need to be declared volatile
uint32_t ulThrottleStart;
uint32_t ulSteeringStart;
uint32_t ulAuxStart;

unsigned long cm0 = 0;
unsigned long dm0 = 0;
unsigned long CYCLE_INTERVAL = 500;
unsigned long STROKE_MILLIS = 4000;
byte DEBOUNCE_INTERVAL = 20;

boolean STROKE_MODE = false;
boolean HIT_STOP_LIMIT = false;
boolean AUTO = false;

//debouncing
unsigned long am0 = 0;
unsigned long bm0 = 0;
boolean reqChange = false;

//interval for reading ADC STOP LIMIT (ms)
unsigned int ADC_STOP_LIMIT_INTERVAL = 50;
//placeholder for ADC stop limit millis() reading
unsigned long em0 = 0;

int lastAuxIn = 0;

boolean ACTUATOR_DIRECTION = IN;

unsigned int STROKE_DELAY_DURATION = 0;

//this is the approximate measured value of the stop limit magnetic sensor where we want to stop it
byte STOP_SENSOR_LIMIT = 204; //roughly 1 volt 

void setup()
{
  Serial.begin(9600);
  
  #if FASTADC
   // set prescale to 16
   sbi(ADCSRA,ADPS2) ;
   cbi(ADCSRA,ADPS1) ;
   cbi(ADCSRA,ADPS0) ;
  #endif
  
  Serial.println("multiChannels");

  // attach servo objects, these will generate the correct 
  // pulses for driving Electronic speed controllers, servos or other devices
  // designed to interface directly with RC Receivers  
  servoThrottle.attach(THROTTLE_OUT_PIN);
  servoSteering.attach(STEERING_OUT_PIN);
  servoAux.attach(AUX_OUT_PIN);
  
  pinMode(EN_PIN_A, OUTPUT);
  pinMode(IN_PIN_A1, OUTPUT);
  pinMode(IN_PIN_A2, OUTPUT);
  pinMode(STOP_LIMIT1_PIN, INPUT);
  
  Serial.print("RETRACTING ACTUATOR");
  retractServo();
  delay(10000);

  // using the PinChangeInt library, attach the interrupts
  // used to read the channels
  PCintPort::attachInterrupt(THROTTLE_IN_PIN, calcThrottle,CHANGE); 
  PCintPort::attachInterrupt(STEERING_IN_PIN, calcSteering,CHANGE); 
  PCintPort::attachInterrupt(AUX_IN_PIN, calcAux,CHANGE); 
  
  cm0 = millis();
  dm0 = millis();
  em0 = millis();
}

void loop()
{
  // create local variables to hold a local copies of the channel inputs
  // these are declared static so that thier values will be retained 
  // between calls to loop.
  static uint16_t unThrottleIn;
  static uint16_t unSteeringIn;
  static uint16_t unAuxIn;
  // local copy of update flags
  static uint8_t bUpdateFlags;
  
  int stop_sensor_val = 0;

  // check shared update flags to see if any channels have a new signal
  if(bUpdateFlagsShared)
  {
    noInterrupts(); // turn interrupts off quickly while we take local copies of the shared variables

    // take a local copy of which channels were updated in case we need to use this in the rest of loop
    bUpdateFlags = bUpdateFlagsShared;
    
    // in the current code, the shared values are always populated
    // so we could copy them without testing the flags
    // however in the future this could change, so lets
    // only copy when the flags tell us we can.
    
    if(bUpdateFlags & THROTTLE_FLAG)
    {
      unThrottleIn = unThrottleInShared;
    }
    
    if(bUpdateFlags & STEERING_FLAG)
    {
      unSteeringIn = unSteeringInShared;
    }
    
    if(bUpdateFlags & AUX_FLAG)
    {
      unAuxIn = unAuxInShared;
    }
     
    // clear shared copy of updated flags as we have already taken the updates
    // we still have a local copy if we need to use it in bUpdateFlags
    bUpdateFlagsShared = 0;
    
    interrupts(); // we have local copies of the inputs, so now we can turn interrupts back on
    // as soon as interrupts are back on, we can no longer use the shared copies, the interrupt
    // service routines own these and could update them at any time. During the update, the 
    // shared copies may contain junk. Luckily we have our local copies to work with :-)
  }
  
  // do any processing from here onwards
  // only use the local values unAuxIn, unThrottleIn and unSteeringIn, the shared
  // variables unAuxInShared, unThrottleInShared, unSteeringInShared are always owned by 
  // the interrupt routines and should not be used in loop
  
  // the following code provides simple pass through 
  // this is a good initial test, the Arduino will pass through
  // receiver input as if the Arduino is not there.
  // This should be used to confirm the circuit and power
  // before attempting any custom processing in a project.
  
  // we are checking to see if the channel value has changed, this is indicated  
  // by the flags. For the simple pass through we don't really need this check,
  // but for a more complex project where a new signal requires significant processing
  // this allows us to only calculate new values when we have new inputs, rather than
  // on every cycle.
  if(bUpdateFlags & THROTTLE_FLAG)
  {
    if(servoThrottle.readMicroseconds() != unThrottleIn)
    {
      servoThrottle.writeMicroseconds(unThrottleIn);
    }
  }
  
  if(bUpdateFlags & STEERING_FLAG)
  {
    if(servoSteering.readMicroseconds() != unSteeringIn)
    {
      servoSteering.writeMicroseconds(unSteeringIn);
    }
  }
  
  if(bUpdateFlags & AUX_FLAG)
  {
    if(servoAux.readMicroseconds() != unAuxIn)
    {
      servoAux.writeMicroseconds(unAuxIn);
    }
  }
  
  bUpdateFlags = 0;
  
  unsigned long currentMillis = millis();
  
  /**
  takes a long time to read via analogIn and we don't need high precision 
  for the limit sensor we'll use fast analogRead and only 
  every 100 microsecods or so (the actuator moves pretty slowly)
  **/
  
  //main cycle interval 
  if((currentMillis - cm0 > CYCLE_INTERVAL) || (cm0 == 0)) {
  
    cm0 = currentMillis;
    
    Serial.print("STEERING:");
    Serial.print(unSteeringIn);
    Serial.print(" AUX:");
    Serial.print(unAuxIn);
    Serial.print(" MODE:");
    Serial.print(AUTO);
    Serial.print(" STOP LIMIT:");
    Serial.print(HIT_STOP_LIMIT);
    Serial.print("\n");
    
  }
  
  //ADC read cycle for stop limit etc
  //main cycle interval 
  if((currentMillis - em0 > ADC_STOP_LIMIT_INTERVAL) || (em0 == 0)) {
  
    em0 = currentMillis;
    
    stop_sensor_val = analogRead(STOP_LIMIT1_PIN);
    
    if(stop_sensor_val < STOP_SENSOR_LIMIT){
      HIT_STOP_LIMIT = true;
    }else{
      HIT_STOP_LIMIT = false;
    }
    
    
  }
  
  if((AUTO == true) && (unAuxIn > 1200)){
    
    //we should turn off after debouncing
    if(reqChange == false){
      reqChange = true;
      am0 = millis();
    }
    
    if(reqChange == true){
      if((millis() - am0) > DEBOUNCE_INTERVAL){
        AUTO = false;
        disableServo();
        reqChange = false;
        am0 = 0;
      }
    }
    
  }
  
  if((AUTO == false) && (unAuxIn < 1200)){
    
    //turn on after debouncing
    if(reqChange == false){
      reqChange = true;
      am0 = millis();
    }
    
    if(reqChange == true){
      if((millis() - am0) > DEBOUNCE_INTERVAL){
        AUTO = true;
        reqChange = false;
        am0 = 0;
      }
    }
    
  }
  
  if(AUTO == true){
    
    if(((currentMillis - dm0) > STROKE_MILLIS) || (dm0 == 0)) {
    
      STROKE_MODE = !STROKE_MODE;
      
      //at the "end" of the stroke there needs to be a delay 
      //where the head pauses for a few seconds 
      disableServo();
      
      delay(random(500,6000)); 
      
      dm0 = millis();
    }
      
    if(STROKE_MODE == true){
    
      extendServo();
  
    }else{
  
      retractServo();
    }

  }
  
  if(AUTO == false){
    
    //steer it manually with deadband (time and control jitter)
    if((unSteeringIn > 1400) && (unSteeringIn < 1600)){
      //do nothing
      disableServo();
    }
    
    if(unSteeringIn > 1600){
      ACTUATOR_DIRECTION = IN;
      retractServo();
    }
    
    if(unSteeringIn < 1400){
      ACTUATOR_DIRECTION = OUT;
      extendServo();
    }
    
  }
}

void retractServo(){
  
  digitalWrite(IN_PIN_A1, LOW);
  digitalWrite(IN_PIN_A2, HIGH);
  digitalWrite(EN_PIN_A, HIGH);
  
}

void extendServo(){
  
  if(HIT_STOP_LIMIT != true){
  
    digitalWrite(IN_PIN_A1, HIGH);
    digitalWrite(IN_PIN_A2, LOW);
    digitalWrite(EN_PIN_A, HIGH);
    
  }else{
    disableServo();
  }
  
}

void disableServo(){
  digitalWrite(EN_PIN_A, LOW);
}


// simple interrupt service routine
void calcThrottle()
{
  // if the pin is high, its a rising edge of the signal pulse, so lets record its value
  if(digitalRead(THROTTLE_IN_PIN) == HIGH)
  { 
    ulThrottleStart = micros();
  }
  else
  {
    // else it must be a falling edge, so lets get the time and subtract the time of the rising edge
    // this gives use the time between the rising and falling edges i.e. the pulse duration.
    unThrottleInShared = (uint16_t)(micros() - ulThrottleStart);
    // use set the throttle flag to indicate that a new throttle signal has been received
    bUpdateFlagsShared |= THROTTLE_FLAG;
  }
}

void calcSteering()
{
  if(digitalRead(STEERING_IN_PIN) == HIGH)
  { 
    ulSteeringStart = micros();
  }
  else
  {
    unSteeringInShared = (uint16_t)(micros() - ulSteeringStart);
    bUpdateFlagsShared |= STEERING_FLAG;
  }
}

void calcAux()
{
  if(digitalRead(AUX_IN_PIN) == HIGH)
  { 
    ulAuxStart = micros();
  }
  else
  {
    unAuxInShared = (uint16_t)(micros() - ulAuxStart);
    bUpdateFlagsShared |= AUX_FLAG;
  }
}
