void basicInit() {
  DEBUG_PRINTLN("Initialising SpeedPulser...");

  DEBUG_PRINTLN("Setting up LED Output...");
  pinMode(pinOnboardLED, OUTPUT);
  digitalWrite(pinOnboardLED, ledOnboard);
  DEBUG_PRINTLN("Set up LED Output!");

  DEBUG_PRINTLN("Setting up PWM...");
  motorPWM = new ESP32_FAST_PWM(pinMotorOutput, pwmFrequency, dutyCycle, 0, pwmResolution);  // begin PWM with a base freq. of 10kHz @ 100% duty (mot. off)
  DEBUG_PRINTLN("Set up PWM!");

  DEBUG_PRINTLN("Setting up Speed Interrupt...");
  attachInterrupt(digitalPinToInterrupt(pinSpeedInput), incomingHz, FALLING);  //setup interrupt to toggle pin on change
  DEBUG_PRINTLN("Set up speed interrupt!");

  DEBUG_PRINTLN("Initialised SpeedPulser!");
}

void testSpeed() {
  // check to see if tempSpeed has a value.  IF it does (>0), set the speed using the 'find closest match' as a duty cycle
  if (tempSpeed > 0) {
    DEBUG_PRINTF("Speed: %d", tempSpeed);
    if (speedOffsetPositive) {
      dutyCycle = tempSpeed + speedOffset;
      dutyCycle = dutyCycle * speedMultiplier;
      if (convertToMPH) {
        dutyCycle = dutyCycle * mphFactor;
      }
      dutyCycle = findClosestMatch(dutyCycle);
      motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
    } else {
      if (tempSpeed - speedOffset > 0) {
        dutyCycle = tempSpeed - speedOffset;
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
    DEBUG_PRINTF(".  Adjusted Speed: %d", dutyCycle);
    DEBUG_PRINTLN("");
  } else {
    // tempSpeed == 0, therefore run through every single duty with a long delay to give you time to go between IDE & cluster and write down...
    for (uint16_t i = 14; i < 385; i++) {  // run through all available speeds and drive the motor
      DEBUG_PRINTF("Duty: %d", i);
      DEBUG_PRINTLN("");

      if (tempSpeed > 0) {
        i = 385;
      }

      motorPWM->setPWM_manual(pinMotorOutput, i);  // set the duty of the motor from the calculations
      delay(sweepSpeed * 500);
    }
  }
}

void needleSweep() {
  // ramp up
  for (int i = 0; i < maxSpeed; i++) {
    DEBUG_PRINTF("Speed: %d", i);
    DEBUG_PRINTLN("");

    dutyCycle = findClosestMatch(i);
    motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
    delay(sweepSpeed);
  }

  // pause
  delay(sweepSpeed);

  // ramp down
  for (int i = maxSpeed; i > 0; i--) {  // set at >0 to stop the needle 'bouncing' when it returns to zero
    DEBUG_PRINTF("Speed: %d", i);
    DEBUG_PRINTLN("");

    dutyCycle = findClosestMatch(i);
    motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);
    delay(sweepSpeed);
  }
  delay(sweepSpeed);
  dutyCycle = 0;
  motorPWM->setPWM_manual(pinMotorOutput, 0);
}