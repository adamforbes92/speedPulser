/*
SpeedPulser - Forbes Automotive '25
Analog speed converter suitable for mechnical drive speedometers.  Tested on VW, Ford & Fiat clusters, the only change is the CAD model. Likely compatible with other marques.  

Inputs are a 5v/12v square wave input from Can2Cluster or an OEM Hall Sensor and converts it into a PWM signal for a BLDC motor.  To get speeds low enough, the motor voltage needs reduced (hence the adjustable LM2596S on the PCB)
from 12v to ~9v.  This allows <10mph readings while still allowing high (160mph) readings.  Clusters supported are 1540 (rotations per mile) =~ (1540*160)/60 = 4100rpm

Default support is for 12v hall sensors from 02J / 02M etc: 1Hz = 1km/h.  Other marques may have different calibrations (adjustable in '_defs.h')

Motor performance plotted with Duty Cycle & Resulting Speed.  Basic Excel located in GitHub for reference - the motor isn't linear so it cannot be assumed that x*y duty = z speed(!)
LED PWM can use various 'bits' for resolution.  8 bit results in a poorer resolution, therefore the speed can be 'jumpy'.  Default is 10 bit which makes it smoother.  Both are available but 10 bit is default.

Uses 'ESP32_FastPWM' for easier PWM control compared to LEDc
Uses 'RunningMedian' for capturing multiple input pulses to compare against.  Used to ignore 'outliars'

To calibrate or adapt to other models:
> Set 'testSpeed' to 1 & confirm tempSpeed = 0.  This will allow the motor to run through EVERY duty cycle from 0 to 385 (10-bit).  Time change can be adjusted if 'too quick' to record changes
> Monitor Serial Monitor and record in the Excel (under Resulting Speed) the running speed of the cluster at each duty cycle
  > Note: duty cycle is >'100%' due to default 10 bit resolution 
> Copy each resulting speed into new 'motorPerformance'
> Comment out the current motorPerformance and uncomment yours
> Adjust 'maxSpeed' to suit new range 
> Done!

All main adjustable variables are in '_defs.h'.
*/

#include "SpeedPulserESP32_defs.h"

ESP32_FAST_PWM* motorPWM;                              // for PWM control.  ESP Boards need to be V2.0.17 - the latest version has known issues with LEDPWM(!)
RunningMedian samples = RunningMedian(averageFilter);  // for calculating median samples - there can be 'hickups' in the incoming signal, this helps remove them(!)
TickTwo tickEEP(writeEEP, eepRefresh);
TickTwo tickWiFi(disconnectWifi, wifiDisable);  // timer for disconnecting wifi after 30s if no connections - saves power
Preferences pref;

// interrupt routine for the incoming pulse from opto
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
  delay(2000);
  Serial.begin(baudSerial);
  DEBUG_PRINTLN("Initialising SpeedPulser...");
#endif

  readEEP();         // read the EEPROM for previous saved states
  tickEEP.start();   // begin ticker for the EEPROM
  tickWiFi.start();  // begin ticker for the WiFi (to turn off after 60s)

  basicInit();                                                // init PWM, Serial, Pin IO etc.  Kept in '_io.ino' for cleanliness due to the number of Serial outputs
  motorPWM->setPWM(pinMotorOutput, pwmFrequency, dutyCycle);  // set motor to off in first instance (0% duty)

  if (hasNeedleSweep) {
    needleSweep();  // enable needle sweep (in _io.ino)
  }

  connectWifi();                       // enable / start WiFi
  WiFi.setSleep(false);                //For the ESP32: turn off sleeping to increase UI responsivness (at the cost of power use)
  setupUI();                           // setup wifi user interface
  WiFi.setTxPower(WIFI_POWER_8_5dBm);  // some C3 boards require a lower Tx power otherwise it won't appear

  updateLabels();
  updateMotorArray();
}

void loop() {
  tickEEP.update();   // refresh the EEP ticker
  tickWiFi.update();  // refresh the WiFi ticker

  if (ledCounter > averageFilter) {           // only flip-flop the LED every time the filter is filled.  This will reduce the 'on' time of the LED making it easier to see!
    ledOnboard = !ledOnboard;                 // flip-flop the led trigger
    digitalWrite(pinOnboardLED, ledOnboard);  // toggle the LED to show the presence of incoming pulses
    ledCounter = 0;                           // reset the counter
  }

  // reset speed to zero if >durationReset
  if (((millis() + 10) - lastPulse) > durationReset) {
    if (!testSpeedo) {
      motorPWM->setPWM_manual(pinMotorOutput, 0);
      ESPUI.updateLabel(label_speed, "Speed: 0");
    }
  }

  if (tempNeedleSweep) {
    needleSweep();
    tempNeedleSweep = false;
  }

  if (updateMotorPerformance) {
    updateMotorArray();
  }

  // check to see if in 'test mode' (testSpeedo = 1)
  if (testSpeedo) {
    testSpeed();  // if tempSpeed > 0, set to fixed duty, else, run through available duties
  }

  if (!testSpeedo) {
    if (dutyCycle != dutyCycleIncoming) {  // only update PWM IF speed has changed (can cause flicker otherwise)
      DEBUG_PRINTF("     DutyIncomingHall: %d", dutyCycleIncoming);
      dutyCycleIncoming = map(dutyCycleIncoming, 0, maxFreqHall, 0, maxSpeed);  // map incoming range to this codes range.  Max Hz should match Max Speed - i.e., 200Hz = 200kmh, or 500Hz = 200kmh...
      DEBUG_PRINTF("     DutyPostProc1Hall: %d", dutyCycle);

      if (rawCount < averageFilter) {
        samples.add(dutyCycleIncoming);
        rawCount++;
      }

      if (rawCount >= averageFilter) {
        dutyCycle = samples.getMedian();
        //dutyCycle = samples.getAverage();
        DEBUG_PRINTF("     getAverageHall: %d", dutyCycle);
        char buf[32];
        sprintf(buf, "Speed: %d", dutyCycle);
        ESPUI.updateLabel(label_speed, String(buf));

        if (speedOffsetPositive) {
          dutyCycle = dutyCycle + speedOffset;
          dutyCycle = dutyCycle * speedMultiplier;
          if (convertToMPH) {
            dutyCycle = dutyCycle * mphFactor;
          }
          dutyCycle = findClosestMatch(dutyCycle);
          motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
        } else {
          if (dutyCycle - speedOffset > 0) {
            dutyCycle = dutyCycle - speedOffset;
            dutyCycle = dutyCycle * speedMultiplier;
            if (convertToMPH) {
              dutyCycle = dutyCycle * mphFactor;
            }
            dutyCycle = findClosestMatch(dutyCycle);
            motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
          } else {
            motorPWM->setPWM_manual(pinMotorOutput, 0);
          }
        }

        DEBUG_PRINTF("     FindClosetMatchHall: %d", dutyCycle);
        rawCount = 0;
        samples.clear();
      }
    }
    dutyCycle = dutyCycleIncoming;  // re-introduce?  Could do some filter on big changes?  May skip over genuine changes though?
  }
}

uint16_t findClosestMatch(uint16_t val) {
  // for finding the nearest match of speed from the incoming duty.  There may be instances where the incoming speed does not equal a value in the array, so find the 'nearest' value
  // has to be 16_t due to 10 bit resolution
  // the 'find'/function returns array position, which is equal to the duty cycle
  uint16_t closest = 0;
  uint16_t closest2 = 0;
  uint16_t i = 0;

  for (i = 0; i < sizeof motorPerformance / sizeof motorPerformance[0]; i++) {
    if (abs(val - closest) >= abs(val - motorPerformance[i])) {
      closest = motorPerformance[i];
    }
  }

  for (i = 0; i < sizeof motorPerformance / sizeof motorPerformance[0]; i++) {
    if (motorPerformance[i] == closest) {
      closest2 = i;
    }
  }

  if (closest2 >= 385) {  // will run into the end of the array IF it's not found a speed, so return 0!
    return 0;
  } else {
    return closest2;  // else return the correct duty cycle from the found speed
  }
}
