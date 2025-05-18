/*
SpeedPulser - Forbes Automotive '25
Analog speed converter suitable for VW MK1 & MK2 Golf.  Tested on Ford & Fiat clusters, the only change is the CAD model. Likely compatible with other marques. 

Inputs are a 5v/12v square wave input from Can2Cluster or an OEM Hall Sensor and converts it into a PWM signal for a motor.  To get speeds low enough, the motor voltage needs reduced (hence the adjustable LM2596S on the PCB)
from 12v to ~9v.  This allows <10mph readings while still allowing high (160mph) readings.  Clusters supported are 1540 (rotations per mile) =~ (1540*160)/60 = 4100rpm

Default support is for 12v hall sensors from 02J / 02M etc.  According to VW documentation, 1Hz = 1km/h.  Other marques may have different calibrations (adjustable in '_defs.h')

Motor performance plotted with Duty Cycle & Resulting Speed.  Basic Excel located in GitHub for reference - the motor isn't linear so it cannot be assumed that x*y duty = z speed(!)
LED PWM can use various 'bits' for resolution.  8 bit results in a poorer resolution, therefore the speed can be 'jumpy'.  Default is 10 bit which makes it smoother.  Both are available.

Uses 'ESP32_FastPWM' for easier PWM control compared to LEDc
Uses 'RunningMedian' for capturing multiple input pulses to compare against.  Used to ignore 'outliars'

To calibrate or adapt to other models:
> Set 'testSpeed' to 1 & confirm tempSpeed = 0.  This will allow the motor to run through EVERY duty cycle from 0 to 385 (10-bit)
> Monitor Serial Monitor and record in the Excel (under Resulting Speed) the running speed of the cluster at each duty cycle
  > Note: duty cycle is >'100%' due to default 10 bit resolution 
> Copy each resulting speed into 'motorPerformance' 
> Done!

All main adjustable variables are in '_defs.h'.

V1.01 - initial release
V1.02 - added onboard LED pulse to confirm incoming pulses
V1.03 - added Fiat cluster - currently not working <40mph due to 'stickyness' of the cluster and motor not having enough bottom end torque.
      - set to 110mph max (with lower motor voltage), allows full range calibration between 20kmph and 180kmh.  Any more and you're speeding anyway...
V1.04 - added Ford cluster - actually much more linear compared to the VW one!
V1.05 - added 'Global Speed Offset' to allow for motors installed with slight binding.  Will keep the plotted duty/speed curve but offset the WHOLE thing
V1.06 - added 'durationReset' - to reset the motor/duty to 0 after xx ms.  This means when there is a break in pulses (either electrical issue or actually stopped, reset the motor)
V1.07 - 
todo - add WiFi connectivity for quick changing vars?
*/

#include "speedPulser_defs.h"

ESP32_FAST_PWM* motorPWM;                              // for PWM control.  ESP Boards need to be V2.0.17 - the latest version has known issues with LEDPWM(!)
RunningMedian samples = RunningMedian(averageFilter);  // for calculating median samples - there can be 'hickups' in the incoming signal, this helps remove them(!)

// interrupt routine for the incoming pulse
void incomingHz() {                                               // Interrupt 0 service routine
  static unsigned long previousMicros = micros();                 // remember variable, initialize first time
  unsigned long presentMicros = micros();                         // read microseconds
  unsigned long revolutionTime = presentMicros - previousMicros;  // works fine with wrap-around of micros()
  if (revolutionTime < 1000UL) return;                            // avoid divide by 0, also debounce, speed can't be over 60,000 was 1000UL
  dutyCycleIncoming = (60000000UL / revolutionTime) / 60;         // calculate
  previousMicros = presentMicros;
  lastPulse = millis();

  ledCounter++;  // count LED counter - is used to flash onboard LED to show the presence of incoming pulses
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

  // todo:
  // reset speed to zero if >durationReset
  if (millis() - lastPulse > durationReset) {
    motorPWM->setPWM_manual(pinMotorOutput, 0);
  }

  // check to see if in 'test mode' (testSpeedo = 1)
  if (testSpeedo) {
    testSpeed();  // if tempSpeed > 0, set to fixed duty, else, run through available duties
  }

  if (!testSpeedo) {
    if (dutyCycle != dutyCycleIncoming) {  // only update PWM IF speed has changed (can cause flicker otherwise)
      switch (incomingType) {
        case 0:  // can2cluster input
          DEBUG_PRINTF("     DutyIncomingC2C: %d", dutyCycleIncoming);
          dutyCycleIncoming = map(dutyCycleIncoming, minFreqCAN, maxFreqCAN, minSpeed, maxSpeed);  // map incoming range to this codes range.  Max Hz should match Max Speed - i.e., 200Hz = 200kmh, or 500Hz = 200kmh...
          DEBUG_PRINTF("     DutyPostProc1C2C: %d", dutyCycle);

          // build up the array to get the average of pulsers and ignore outliars
          if (rawCount < averageFilter) {
            samples.add(dutyCycleIncoming);
            rawCount++;
          }

          if (rawCount >= averageFilter) {
            dutyCycle = samples.getAverage(averageFilter / 2);
            DEBUG_PRINTF("     getAverageC2C: %d", dutyCycle);

            if (speedOffsetPositive) {
              dutyCycle = dutyCycle + speedOffset;
              dutyCycle = findClosestMatch(dutyCycle);
              motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
            } else {
              if (dutyCycle - speedOffset > 0) {
                dutyCycle = dutyCycle - speedOffset;
                dutyCycle = findClosestMatch(dutyCycle);
                motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
              }
            }

            DEBUG_PRINTF("     FindClosetMatchC2C: %d", dutyCycle);

            rawCount = 0;     // reset the counter
            samples.clear();  // clear the array
          }
          break;

        case 1:  // hall sensor input
          DEBUG_PRINTF("     DutyIncomingHall: %d", dutyCycleIncoming);
          dutyCycleIncoming = map(dutyCycleIncoming, minFreqHall, maxFreqHall, minSpeed, maxSpeed);  // map incoming range to this codes range.  Max Hz should match Max Speed - i.e., 200Hz = 200kmh, or 500Hz = 200kmh...
          DEBUG_PRINTF("     DutyPostProc1Hall: %d", dutyCycle);

          if (rawCount < averageFilter) {
            samples.add(dutyCycleIncoming);
            rawCount++;
          }

          if (rawCount >= averageFilter) {
            dutyCycle = samples.getAverage(averageFilter / 2);
            DEBUG_PRINTF("     getAverageHall: %d", dutyCycle);

            if (speedOffsetPositive) {
              dutyCycle = dutyCycle + speedOffset;
              dutyCycle = findClosestMatch(dutyCycle);
              motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
            } else {
              if (dutyCycle - speedOffset > 0) {
                dutyCycle = dutyCycle - speedOffset;
                dutyCycle = findClosestMatch(dutyCycle);
                motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
              }
            }

            DEBUG_PRINTF("     FindClosetMatchHall: %d", dutyCycle);
            rawCount = 0;
            samples.clear();
          }
          break;
      }

      dutyCycle = dutyCycleIncoming;  // re-introduce?  Could do some filter on big changes?  May skip over genuine changes though?
      DEBUG_PRINTLN("");
    }
  }
}

uint16_t findClosestMatch(uint16_t val) {
  // for finding the nearest match of speed from the incoming duty.  There may be instances where the incoming speed does not equal a value in the array, so find the 'nearest' value
  // has to be 16_t due to 10 bit resolution
  // the 'find'/function returns array position, which is equal to the duty cycle
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