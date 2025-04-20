/*
SpeedPulser - Forbes Automotive '25
VW MK1 & MK2 analog speed converter.  Takes a 5v/12v square wave input from Can2Cluster or an OEM Hall Sensor and converts it into a PWM signal for a motor.  To get speeds low enough, the motor voltage needs reduced
from 12v to ~9v.  This allows <10mph readings while still allowing high (160mph) readings.  Clusters supported are 1540 (rotations per mile) =~ (1540*160)/60 = 4100rpm

Also supports 12v hall sensors from 02J / 02M etc.  According to documentation, 1Hz = 1km/h

Motor performance plotted with Duty Cycle & Resulting Speed.  Basic Excel located in GitHub for reference - the motor isn't linear(!)

Uses 'AVR_PWM' for easier PWM control.
*/

#include "speedPulser_defs.h"

ESP32_FAST_PWM* motorPWM;
RunningMedian samples = RunningMedian(averageFilter);

// interrupt routine for the incoming pulse
void incomingHz() {                                               // Interrupt 0 service routine
  static unsigned long previousMicros = micros();                 // remember variable, initialize first time
  unsigned long presentMicros = micros();                         // read microseconds
  unsigned long revolutionTime = presentMicros - previousMicros;  // works fine with wrap-around of micros()
  if (revolutionTime < 1000UL) return;                            // avoid divide by 0, also debounce, speed can't be over 60,000 was 1000UL
  dutyCycleIncoming = (60000000UL / revolutionTime) / 60;         // calculate
  previousMicros = presentMicros;

  ledCounter++;
}

void setup() {
#ifdef serialDebug
  Serial.begin(115200);
  DEBUG_PRINTLN("Initialising SpeedPulser...");
#endif

  basicInit();                                                // init PWM, Serial, Pin IO etc.  Kept in '_io.ino' for cleanliness due to the number of Serial outputs
  motorPWM->setPWM(pinMotorOutput, pwmFrequency, dutyCycle);  // set motor to off in first instance (100% duty)

  if (hasNeedleSweep) {
    needleSweep();  // enable needle sweep (in _io.ino)
  }
}

void loop() {
  if (ledCounter > averageFilter) {
    ledOnboard = !ledOnboard;                 // flip-flop the led trigger
    digitalWrite(pinOnboardLED, ledOnboard);  // flash the LED to show the presence of incoming pulses
    ledCounter = 0;                           // reset the counter
  }

  if (testSpeedo) {
    testSpeed();  // if tempDuty > 0, set to fixed duty
  } else {
    if (dutyCycle != dutyCycleIncoming) {  // only update PWM IF speed has changed (can cause flicker otherwise)
      if (incomingType == 0) {             // if 'Can2Cluster'
        DEBUG_PRINTF("     DutyIncoming: %d", dutyCycleIncoming);
        dutyCycleIncoming = map(dutyCycleIncoming, minFreqCAN, maxFreqCAN, minSpeed, maxSpeed);  // map incoming range to this codes range.  Max Hz should match Max Speed - i.e., 200Hz = 200kmh, or 500Hz = 200kmh...
        DEBUG_PRINTF("     DutyPostProc1: %d", dutyCycle);

        // build up the array to get the average of pulsers and ignore outliars
        if (rawCount < averageFilter - 1) {
          samples.add(dutyCycleIncoming);
          rawCount++;
        } else {
          dutyCycle = samples.getAverage(averageFilter / 2);
          DEBUG_PRINTF("     getAverage: %d", dutyCycle);

          if (speedOffsetPositive) {
            dutyCycle = dutyCycle + speedOffset;
          } else {
            dutyCycle = dutyCycle - speedOffset;
          }
          
          dutyCycle = findClosestMatch(dutyCycle);             // find the closest final duty based on the incoming duty (use motor perfomance array)
          motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);  // set the duty of the motor from the calculations
          DEBUG_PRINTF("     FindClosetMatch: %d", dutyCycle);

          rawCount = 0;     // reset the counter
          samples.clear();  // clear the array
        }
      }

      if (incomingType == 1) {  // if 'Hall'
        DEBUG_PRINTF("     DutyIncoming: %d", dutyCycleIncoming);
        dutyCycleIncoming = map(dutyCycleIncoming, minFreqHall, maxFreqHall, minSpeed, maxSpeed);  // map incoming range to this codes range.  Max Hz should match Max Speed - i.e., 200Hz = 200kmh, or 500Hz = 200kmh...
        DEBUG_PRINTF("     DutyPostProc1: %d", dutyCycle);

        if (rawCount < averageFilter - 1) {
          samples.add(dutyCycleIncoming);
          rawCount++;
        } else {
          dutyCycle = samples.getAverage(averageFilter / 2);
          DEBUG_PRINTF("     getAverage: %d", dutyCycle);

          if (speedOffsetPositive) {
            dutyCycle = dutyCycle + speedOffset;
          } else {
            dutyCycle = dutyCycle - speedOffset;
          }
          dutyCycle = findClosestMatch(dutyCycle);

          motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
          DEBUG_PRINTF("     FindClosetMatch: %d", dutyCycle);
          rawCount = 0;
          samples.clear();
        }
      }
      dutyCycle = dutyCycleIncoming;  // re-introduce?
      DEBUG_PRINTLN("");
    }
  }
}

uint16_t findClosestMatch(uint16_t val) {
  uint16_t closest = 0;
  for (uint16_t i = 0; i < sizeof motorPerformance / sizeof motorPerformance[0]; i++) {
    if (abs(val - closest) >= abs(val - motorPerformance[i]))
      closest = motorPerformance[i];
  }
  for (uint16_t i = 0; i < sizeof motorPerformance / sizeof motorPerformance[0]; i++) {
    if (motorPerformance[i] == closest) {
      return i;
    }
  }
}