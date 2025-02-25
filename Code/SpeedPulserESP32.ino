/*
SpeedPulser - Forbes Automotive '25
VW MK1 & MK2 analog speed converter.  Takes a 5v/12v square wave input from Can2Cluster and converts it into a PWM signal for a motor.  To get speeds low enough, the motor voltage needs reduced
from 12v to 9v.  This allows <10mph readings while still allowing high (120mph) readings.

Also supports 12v hall sensors from 02J / 02M etc.

PWM for the motors supplied are based on a grounded blue wire being 100%; so this PWM lib. reads 'backwards'.
Motor slowest speed = PWM DC of 94.6% @ 1000Hz
Motor highest speed = PWM DC of 0%

Notes:
94.0% = too low(!)
93.5% = 20mph
93.0% = 20mph
90.0% = 25mph
80.0% = 43mph
70.0% = 56mph
60.0% = 73mph
50.0% = 85mph
40.0% = 

Uses 'AVR_PWM' for easier PWM control.
*/

#include "speedPulser_defs.h"

ESP32_FAST_PWM* PWM_Instance;
int PWM_resolution = 12;
#define ENABLE_DEBUG

void incomingHz() {                                               // Interrupt 0 service routine
  static unsigned long previousMicros = micros();                 // remember variable, initialize first time
  unsigned long presentMicros = micros();                         // read microseconds
  unsigned long revolutionTime = presentMicros - previousMicros;  // works fine with wrap-around of micros()
  if (revolutionTime < 1000UL) return;                            // avoid divide by 0, also debounce, speed can't be over 60,000
  dutyCycleIncoming = (60000000UL / revolutionTime) / 60;         // calculate
  previousMicros = presentMicros;
}

void setup() {
#ifdef ENABLE_DEBUG
  Serial.begin(115200);
  DEBUG("Initialising SpeedPulser...");
#endif

  basicInit();                                         // init PWM, Serial, Pin IO etc.  Kept in '_io.ino' for cleanliness due to the number of Serial outputs
  PWM_Instance->setPWM(pinMotOutput, frequency, dutyCycle);  // set motor to off in first instance (100% duty)
}

void loop() {
  if (testSpeedo) {
    testSpeed();  // if tempDuty > 0, set to fixed duty, else sweep up/down
  } else {
    DEBUG("Duty Incoming: %d", dutyCycleIncoming);
    if (dutyCycle != dutyCycleIncoming) {                                                // only update PWM IF speed has changed (can cause flicker otherwise)
      if (incomingType == 0) {                                                           // if 'Can2Cluster'
        dutyCycle = map(dutyCycleIncoming, minFreqCAN, maxFreqCAN, minSpeed, maxSpeed);  // map incoming range to this codes range.  Should match, but sense check...
        dutyCycle = map(dutyCycle, minFreqCAN, 100, motorUpperLimit, motorLowerLimit);   // re-map the range to the motor limits.  Anything after xx% = 100, anything <xx% = lowest limit (highest speed)

        PWM_Instance->setPWM_manual(pinMotOutput, dutyCycle);
      }

      if (incomingType == 1) {                                                             // if 'Hall'
        dutyCycle = map(dutyCycleIncoming, minFreqHall, maxFreqHall, minSpeed, maxSpeed);  // map incoming range to this codes range.  Should match, but sense check...
        dutyCycle = map(dutyCycle, minFreqHall, 100, motorUpperLimit, motorLowerLimit);    // re-map the range to the motor limits.  Anything after xx% = 100, anything <xx% = lowest limit (highest speed)

        PWM_Instance->setPWM_manual(pinMotOutput, dutyCycle);
      }
      DEBUG("PWM: %d", i);
      //dutyCycle = dutyCycleIncoming; //potentially re-introduce?
    }
  }
}
