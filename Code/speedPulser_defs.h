#define _PWM_LOGLEVEL_ 0
#include "AVR_PWM.h"

#define baudSerial 9600     // baud rate for debug
#define serialDebug 1       // for Serial feedback - disable on release(!) ** CAN CHANGE THIS **
#define testSpeedo 0        // for testing only, vary final frequency for speed - disable on release(!) ** CAN CHANGE THIS **

#define pinMotOutput 10     // motor PWM output
#define pinSpeedOutput 2    // interrupt supporting pin for speed input
#define pinDir 9            // motor feedback pin

#define incomingType 0      // 0 = Can2Cluster; 1 = hall sensor  ** CAN CHANGE THIS **

#define minFreqCAN 0        // min frequency for top speed using Can2Cluster ** CAN CHANGE THIS **
#define maxFreqCAN 500      // max frequency for top speed using Can2Cluster ** CAN CHANGE THIS **
#define minFreqHall 0       // min frequency for top speed using the 02J / 02M hall sensor  ** CAN CHANGE THIS **
#define maxFreqHall 500     // max frequency for top speed using the 02J / 02M hall sensor ** CAN CHANGE THIS **

#define minSpeed 0          // minimum cluster speed in kmh ** CAN CHANGE THIS **
#define maxSpeed 200        // minimum cluster speed in kmh ** CAN CHANGE THIS **

#define motorLowerLimit 25  // minimum motor speed in % (motor doesn't gain speed if % < than this) ** CAN CHANGE THIS **
#define motorUpperLimit 93  // minimum motor speed in % (motor doesn't spin if % > than this) ** CAN CHANGE THIS **

extern unsigned long dutyCycleIncoming = 0;  // Duty Cycle % coming in from Can2Cluster or Hall

extern float tempDuty = 76;  // for testing only, set fixed duty.  Can set to 0 to speed up / slow down on repeat

extern float frequency = 10000;  // PWM Hz (motor supplied is 10kHz)
extern float dutyCycle = 100;    // starting / default Hz: 100Hz is motor 'off'