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
  if (tempSpeed > 0) {
    dutyCycle = findClosestMatch(tempSpeed);  // find the closest final duty based on the incoming duty (use motor perfomance)
    DEBUG_PRINTF("Speed: %d", tempSpeed);
    DEBUG_PRINTLN("");

    motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);  // set the duty of the motor from the calculations
    delay(sweepSpeed * 10);
  } else {
    for (uint8_t i = 0; i < maxSpeed; i++) { // run through all available speeds and drive the motor
      dutyCycle = findClosestMatch(i);  // find the closest final duty based on the incoming duty (use motor perfomance)
      DEBUG_PRINTF("Speed: %d", i);
      DEBUG_PRINTLN("");

      motorPWM->setPWM_manual(pinMotorOutput, dutyCycle);  // set the duty of the motor from the calculations
      delay(sweepSpeed);
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
  motorPWM->setPWM_manual(pinMotorOutput, 0);
}