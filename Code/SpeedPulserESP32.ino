/*
SpeedPulser - Forbes Automotive '25
VW MK1 & MK2 analog speed converter.  Takes a 5v/12v square wave input from Can2Cluster or an OEM Hall Sensor and converts it into a PWM signal for a motor.  To get speeds low enough, the motor voltage needs reduced
from 12v to 9v.  This allows <10mph readings while still allowing high (120mph) readings.

Also supports 12v hall sensors from 02J / 02M etc.  According to documentation, 1Hz = 1km/h

Motor slowest speed = PWM DC of 15% @ 10kHz
Motor highest speed = PWM DC of 93% @ 10kHz

Uses 'AVR_PWM' for easier PWM control.
*/

#include "speedPulser_defs.h"

ESP32_FAST_PWM* motorPWM;

// interrupt routine for the incoming pulse
void incomingHz() {                                               // Interrupt 0 service routine
  static unsigned long previousMicros = micros();                 // remember variable, initialize first time
  unsigned long presentMicros = micros();                         // read microseconds
  unsigned long revolutionTime = presentMicros - previousMicros;  // works fine with wrap-around of micros()
  if (revolutionTime < 10UL) return;                              // avoid divide by 0, also debounce, speed can't be over 60,000 was 1000UL
  dutyCycleIncoming = (60000000UL / revolutionTime) / 60;         // calculate
  previousMicros = presentMicros;
}

void setup() {
#ifdef serialDebug
  Serial.begin(115200);
  DEBUG_PRINTLN("Initialising SpeedPulser...");
#endif

  basicInit();                                                // init PWM, Serial, Pin IO etc.  Kept in '_io.ino' for cleanliness due to the number of Serial outputs
  motorPWM->setPWM(pinMotorOutput, pwmFrequency, dutyCycle);  // set motor to off in first instance (100% duty)
}

void loop() {
  if (testSpeedo) {
    testSpeed();  // if tempDuty > 0, set to fixed duty, else sweep up/down
  } else {
    if (dutyCycle != dutyCycleIncoming) {
      // only update PWM IF speed has changed (can cause flicker otherwise)
      if (incomingType == 0) {  // if 'Can2Cluster'
        //DEBUG_PRINTF("     DutyIncoming: %d", dutyCycleIncoming);
        //dutyCycle = map(dutyCycleIncoming, minFreqCAN, maxFreqCAN, minSpeed, maxSpeed);  // map incoming range to this codes range.  Should match, but sense check...
        //DEBUG_PRINTF("     DutyPostProc1: %d", dutyCycle);

        dutyCycle *= canFactor;
        dutyCycle = map(dutyCycle, 0, 100, motorLowerLimit, motorUpperLimit);  // re-map the range to the motor limits.  Anything after xx% = 100, anything <xx% = lowest limit (highest speed)
        DEBUG_PRINTF("     DutyPostProc2: %d", dutyCycle);

        motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
      }

      if (incomingType == 1) {  // if 'Hall'
        //DEBUG_PRINTF("     DutyIncoming: %d", dutyCycleIncoming);
        //dutyCycle = map(dutyCycleIncoming, minFreqHall, maxFreqHall, minSpeed, maxSpeed);  // map incoming range to this codes range.  Should match, but sense check...
        //DEBUG_PRINTF("     DutyPostProc1: %d", dutyCycle);

        dutyCycle *= hallFactor;
        dutyCycle = map(dutyCycle, 0, 100, motorLowerLimit, motorUpperLimit);  // re-map the range to the motor limits.  Anything after xx% = 100, anything <xx% = lowest limit (highest speed)
        DEBUG_PRINTF("     DutyPostProc2: %d", dutyCycle);

        motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
      }
      //dutyCycle = dutyCycleIncoming; //potentially re-introduce?
      DEBUG_PRINTLN("");
    }
  }
}
