#define _PWM_LOGLEVEL_ 0
#include "ESP32_FastPWM.h"
#include <RunningMedian.h>

#define baudSerial 115200  // baud rate for debug serial
#define serialDebug 0      // for Serial feedback - disable on release(!) ** CAN CHANGE THIS **
#define testSpeedo 0       // for testing only, vary final pwmFrequency for speed - disable on release(!) ** CAN CHANGE THIS **

#define hasNeedleSweep 0  // for needle sweep ** CAN CHANGE THIS **
#define sweepSpeed 18     // for needle sweep rate of change ** CAN CHANGE THIS **

#define incomingType 1  // 0 = Can2Cluster; 1 = hall sensor  ** CAN CHANGE THIS **

#define averageFilter 6  // number of samples to take to average/remove erraticness from freq. changes ** CAN CHANGE THIS **

#define minFreqHall 0    // min frequency for top speed using the 02J / 02M hall sensor  ** CAN CHANGE THIS **
#define maxFreqHall 200  // max frequency for top speed using the 02J / 02M hall sensor ** CAN CHANGE THIS **
#define minFreqCAN 0     // min frequency for top speed using the 02J / 02M hall sensor  ** CAN CHANGE THIS **
#define maxFreqCAN 200   // max frequency for top speed using the 02J / 02M hall sensor ** CAN CHANGE THIS **

#define minSpeed 0    // minimum cluster speed in kmh on the cluster ** CAN CHANGE THIS **
#define maxSpeed 200  // minimum cluster speed in kmh on the cluster ** CAN CHANGE THIS **

#define pinMotorOutput 2  // pin for motor PWM output - needs stepped up to 5v for the motor (NPN transistor on the board)
#define pinSpeedInput 5   // interrupt supporting pin for speed input.  ESP32 C3 doesn't like them all, so if changing test this first(!)
#define pinDirection 10   // motor direction pin (currently unused)
#define pinOnboardLED 8   // for feedback for input checking / flash LED on input

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
extern long tempSpeed = 0;                   // for testing only, set fixed speed in kmh.  Can set to 0 to speed up / slow down on repeat with testSpeed enabled
extern long pwmFrequency = 10000;            // PWM Hz (motor supplied is 10kHz)
extern long dutyCycle = 0;                   // starting / default Hz: 0% is motor 'off'
extern int pwmResolution = 10;               // number of bits for motor resolution.  Can use 8 or 10, although 8 makes it a bit 'jumpy'

extern int rawCount = 0;

// for 8 bit resolution
//uint8_t motorPerformance[] = { 0, 15, 20, 25, 32, 34, 39, 42, 46, 50, 53, 58, 62, 65, 70, 73, 78, 80, 83, 88, 90, 92, 96, 99, 102, 105, 108, 109, 111, 113, 115, 118, 120, 122, 123, 125, 130, 131, 135, 136, 138, 140, 141, 143, 144, 148, 150, 151, 152, 153, 154, 156, 157, 158, 159, 160, 162, 164, 166, 168, 169, 170, 171, 172, 175, 176, 178, 179, 179, 180, 181, 182, 183, 183, 184, 185, 185, 186, 186, 187, 187, 177, 189, 190, 190, 191, 191, 192, 192, 193, 193, 194, 195, 195, 196, 197, 198, 199, 200, 200 };

// for 10 bit resolution
uint8_t motorPerformance[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 26, 29, 30, 31, 32, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 43, 44, 45, 48, 50, 52, 52, 53, 54, 55, 56, 57, 57, 59, 59, 61, 61, 62, 62, 63, 64, 65, 67, 67, 69, 69, 70, 71, 72, 73, 74, 75, 75, 76, 76, 76, 77, 77, 77, 78, 78, 78, 78, 79, 79, 80, 81, 82, 83, 84, 85, 85, 85, 86, 87, 89, 90, 91, 91, 92, 92, 93, 95, 95, 96, 98, 99, 100, 101, 102, 102, 104, 105, 105, 107, 109, 110, 110, 111, 112, 112, 114, 114, 114, 115, 116, 116, 117, 119, 120, 121, 121, 122, 122, 122, 125, 125, 125, 126, 126, 127, 127, 128, 129, 130, 130, 130, 131, 131, 131, 131, 132, 132, 132, 132, 133, 134, 135, 135, 137, 137, 138, 138, 138, 139, 139, 140, 140, 140, 141, 141, 141, 142, 142, 143, 143, 144, 145, 145, 145, 146, 147, 148, 148, 148, 149, 149, 150, 150, 150, 151, 151, 151, 151, 151, 152, 153, 154, 155, 155, 155, 156, 156, 156, 156, 157, 157, 158, 159, 159, 159, 159, 159, 160, 160, 160, 160, 160, 162, 162, 162, 162, 162, 162, 163, 163, 164, 164, 165, 165, 166, 166, 166, 166, 167, 167, 167, 167, 168, 168, 168, 169, 170, 170, 171, 171, 171, 172, 172, 172, 172, 172, 173, 173, 174, 174, 175, 176, 176, 176, 176, 176, 176, 177, 177, 178, 178, 178, 179, 179, 179, 179, 179, 179, 179, 179, 180, 180, 180, 180, 180, 180, 181, 181, 181, 181, 182, 182, 182, 183, 183, 184, 184, 184, 184, 184, 184, 185, 185, 185, 185, 185, 185, 185, 185, 186, 186, 186, 186, 186, 186, 186, 186, 186, 186, 186, 186, 186, 187, 188, 188, 188, 188, 189, 189, 189, 190, 190, 191, 191, 191, 191, 191, 191, 192, 192, 192, 192, 192, 193, 193, 193, 193, 194, 194, 194, 194, 195, 195, 195, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 198, 198, 198, 198, 199, 199, 199, 199, 199, 200, 200, 200, 200, 200 };

// for 10 bit resolution, with smoothing - could be more inaccurate
//uint8_t motorPerformance[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 11, 16, 19, 22, 25, 27, 29, 31, 32, 34, 35, 36, 37, 39, 40, 41, 41, 42, 44, 45, 47, 48, 49, 51, 52, 53, 54, 55, 56, 57, 58, 58, 59, 60, 61, 62, 62, 64, 64, 66, 66, 67, 68, 69, 70, 71, 72, 73, 74, 74, 75, 75, 76, 76, 77, 77, 77, 77, 78, 78, 79, 79, 80, 81, 81, 82, 83, 84, 84, 85, 86, 87, 88, 89, 90, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 112, 113, 114, 114, 115, 116, 117, 118, 119, 120, 120, 121, 122, 123, 123, 124, 124, 125, 126, 126, 127, 128, 128, 129, 129, 130, 130, 130, 131, 131, 131, 131, 132, 132, 133, 134, 134, 135, 136, 136, 137, 137, 138, 138, 139, 139, 140, 140, 140, 141, 141, 141, 142, 142, 143, 144, 144, 144, 145, 146, 146, 147, 147, 148, 148, 149, 149, 150, 150, 150, 150, 151, 151, 151, 152, 153, 153, 154, 154, 155, 155, 155, 156, 156, 157, 157, 158, 158, 158, 158, 159, 159, 159, 159, 160, 160, 161, 161, 161, 161, 162, 162, 162, 163, 163, 163, 164, 164, 165, 165, 165, 166, 166, 166, 166, 167, 167, 167, 168, 168, 169, 169, 170, 170, 171, 171, 171, 171, 172, 172, 172, 173, 173, 173, 174, 175, 175, 175, 175, 176, 176, 176, 177, 177, 177, 178, 178, 178, 178, 179, 179, 179, 179, 179, 179, 180, 180, 180, 180, 180, 180, 180, 181, 181, 181, 181, 182, 182, 183, 183, 183, 183, 184, 184, 184, 184, 184, 185, 185, 185, 185, 185, 185, 185, 186, 186, 186, 186, 186, 186, 186, 186, 186, 186, 186, 186, 187, 187, 187, 187, 188, 188, 188, 189, 189, 190, 190, 190, 190, 191, 191, 191, 191, 191, 192, 192, 192, 192, 192, 193, 193, 193, 193, 194, 194, 194, 194, 195, 195, 195, 195, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 197, 198, 198, 198, 198, 198, 199, 199, 199, 199, 199, 200 };

extern uint8_t incomingFreqMedian[] = { 0, 0, 0, 0, 0 };

extern bool ledOnboard = false;
extern int ledCounter = 0;