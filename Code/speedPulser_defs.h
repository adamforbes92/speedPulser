#define _PWM_LOGLEVEL_ 4
#include "ESP32_FastPWM.h"

#define baudSerial 115200     // baud rate for debug
#define serialDebug 1       // for Serial feedback - disable on release(!) ** CAN CHANGE THIS **
#define testSpeedo 1        // for testing only, vary final frequency for speed - disable on release(!) ** CAN CHANGE THIS **

#define pinMotOutput 21     // motor PWM output
#define pinSpeedOutput 20    // interrupt supporting pin for speed input
#define pinDir 10            // motor feedback pin

#define incomingType 0      // 0 = Can2Cluster; 1 = hall sensor  ** CAN CHANGE THIS **

#define minFreqCAN 0        // min frequency for top speed using Can2Cluster - should match Can2Cluster _def ** CAN CHANGE THIS **
#define maxFreqCAN 500      // max frequency for top speed using Can2Cluster - should match Can2Cluster _def ** CAN CHANGE THIS **
#define minFreqHall 0       // min frequency for top speed using the 02J / 02M hall sensor  ** CAN CHANGE THIS **
#define maxFreqHall 500     // max frequency for top speed using the 02J / 02M hall sensor ** CAN CHANGE THIS **

#define minSpeed 0          // minimum cluster speed in kmh ** CAN CHANGE THIS **
#define maxSpeed 200        // minimum cluster speed in kmh ** CAN CHANGE THIS **

#define motorLowerLimit 5  // minimum motor speed in % (motor doesn't gain speed if % < than this) ** CAN CHANGE THIS **
#define motorUpperLimit 40  // minimum motor speed in % (motor doesn't spin if % > than this) ** CAN CHANGE THIS **
#define sweepSpeed 20

#ifdef ENABLE_DEBUG
#define DEBUG(x, ...) Serial.printf(x "\n", ##__VA_ARGS__)
#define DEBUG_(x, ...) Serial.printf(x, ##__VA_ARGS__)
#else
#define DEBUG(x, ...)
#define DEBUG_(x, ...)
#endif

extern unsigned long dutyCycleIncoming = 0;  // Duty Cycle % coming in from Can2Cluster or Hall

extern float tempDuty = motorLowerLimit + 0;  // for testing only, set fixed duty.  Can set to 0 to speed up / slow down on repeat

extern float frequency = 10000;  // PWM Hz (motor supplied is 10kHz)
extern float dutyCycle = 0;    // starting / default Hz: 100Hz is motor 'off'