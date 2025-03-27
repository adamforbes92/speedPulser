#define _PWM_LOGLEVEL_ 0
#include "ESP32_FastPWM.h"
#include <RunningMedian.h>

#define baudSerial 115200  // baud rate for debug serial
#define serialDebug 1      // for Serial feedback - disable on release(!) ** CAN CHANGE THIS **
#define testSpeedo 0       // for testing only, vary final pwmFrequency for speed - disable on release(!) ** CAN CHANGE THIS **
#define hasNeedleSweep 0   // for needle sweep

#define pinMotorOutput 2  // pin for motor PWM output - needs stepped up to 5v for the motor (NPN transistor on the board)
#define pinSpeedInput 5   // interrupt supporting pin for speed input.  ESP32 C3 doesn't like them all, so if changing test this first(!)
#define pinDirection 10   // motor direction pin (currently unused)
#define pinOnboardLED 8   // for feedback for input

#define incomingType 1    // 0 = Can2Cluster; 1 = hall sensor  ** CAN CHANGE THIS **
#define averageFilter 6  // number of samples to take to average/remove erraticness from freq. changes

#define minFreqHall 0    // min frequency for top speed using the 02J / 02M hall sensor  ** CAN CHANGE THIS **
#define maxFreqHall 200  // max frequency for top speed using the 02J / 02M hall sensor ** CAN CHANGE THIS **

#define minSpeed 0    // minimum cluster speed in kmh on the cluster ** CAN CHANGE THIS **
#define maxSpeed 200  // minimum cluster speed in kmh on the cluster ** CAN CHANGE THIS **

#define sweepSpeed 50  // if in testing, this is the rate of change for freq.

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
extern long tempDuty = 0;                    // for testing only, set fixed duty.  Can set to 0 to speed up / slow down on repeat
extern long pwmFrequency = 10000;            // PWM Hz (motor supplied is 10kHz)
extern long dutyCycle = 0;                   // starting / default Hz: 100Hz is motor 'off'
extern int pwmResolution = 8;

extern long rawFreq = 0;
extern int rawCount = 0;

uint8_t motorPerformance[] = { 0, 0, 0, 25, 32, 34, 39, 42, 46, 50, 53, 58, 62, 65, 70, 73, 78, 80, 83, 88, 90, 92, 96, 99, 102, 105, 108, 109, 111, 113, 115, 118, 120, 122, 123, 125, 130, 131, 135, 136, 138, 140, 141, 143, 144, 148, 150, 151, 152, 153, 154, 156, 157, 158, 159, 160, 162, 164, 166, 168, 169, 170, 171, 172, 175, 176, 178, 179, 179, 180, 181, 182, 183, 183, 184, 185, 185, 186, 186, 187, 187, 177, 189, 190, 190, 191, 191, 192, 192, 193, 193, 194, 195, 195, 196, 197, 198, 199, 200, 200 };
extern uint8_t incomingFreqMedian[] = { 0, 0, 0, 0, 0 };

extern bool ledOnboard = false;
extern int ledCounter = 0;