#define _PWM_LOGLEVEL_ 0
#include "ESP32_FastPWM.h"

#define baudSerial 115200  // baud rate for debug serial
#define serialDebug 1      // for Serial feedback - disable on release(!) ** CAN CHANGE THIS **
#define testSpeedo 0       // for testing only, vary final pwmFrequency for speed - disable on release(!) ** CAN CHANGE THIS **

#define pinMotorOutput 10  // pin for motor PWM output - needs stepped up to 5v for the motor (NPN transistor on the board)
#define pinSpeedInput 5    // interrupt supporting pin for speed input.  ESP32 C3 doesn't like them all, so if changing test this first(!)
#define pinDirection 10    // motor direction pin (currently unused)

#define incomingType 1  // 0 = Can2Cluster; 1 = hall sensor  ** CAN CHANGE THIS **

/*
#define minFreqCAN 0    // min frequency for top speed using Can2Cluster - should match Can2Cluster _def ** CAN CHANGE THIS **
#define maxFreqCAN 500  // max requency for top speed using Can2Cluster - should match Can2Cluster _def ** CAN CHANGE THIS **

#define minFreqHall 0    // min frequency for top speed using the 02J / 02M hall sensor  ** CAN CHANGE THIS **
#define maxFreqHall 200  // max frequency for top speed using the 02J / 02M hall sensor ** CAN CHANGE THIS **

#define minSpeed 0    // minimum cluster speed in kmh on the cluster ** CAN CHANGE THIS **
#define maxSpeed 200  // minimum cluster speed in kmh on the cluster ** CAN CHANGE THIS **
*/

#define canFactor 1 
#define hallFactor 1

#define motorLowerLimit 0   // minimum motor speed in % (motor doesn't gain speed if % < than this) ** CAN CHANGE THIS **
#define motorUpperLimit 94  // minimum motor speed in % (motor doesn't spin if % > than this) ** CAN CHANGE THIS **
#define sweepSpeed 10       // if in testing, this is the rate of change for freq.

#ifdef serialDebug
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(x...) Serial.printf(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(x...)
#endif

extern unsigned long dutyCycleIncoming = 0;  // Duty Cycle % coming in from Can2Cluster or Hall
extern long tempDuty = 0;                   // for testing only, set fixed duty.  Can set to 0 to speed up / slow down on repeat
extern long pwmFrequency = 10000;           // PWM Hz (motor supplied is 10kHz)
extern long dutyCycle = 0;                  // starting / default Hz: 100Hz is motor 'off'
extern int pwmResolution = 10;