void basicInit() {
  DEBUG_PRINTLN("Initialising SpeedPulser...");

  DEBUG_PRINTLN("Setting up PWM...");
  motorPWM = new ESP32_FAST_PWM(pinMotorOutput, pwmFrequency, dutyCycle, 0, pwmResolution);  // begin PWM with a base freq. of 10kHz @ 100% duty (mot. off)
  DEBUG_PRINTLN("Set up PWM!");

  DEBUG_PRINTLN("Setting up speed interrupt...");
  attachInterrupt(digitalPinToInterrupt(pinSpeedInput), incomingHz, RISING);  //setup interrupt to toggle pin on change.  Consider changing to just a loop to capture pulses?
  DEBUG_PRINTLN("Set up speed interrupt!");

  DEBUG_PRINTLN("Initialised SpeedPulser!");
}

void testSpeed() {
  DEBUG_PRINTF("PWM: %d", tempDuty);
  while (tempDuty < 100) {
    tempDuty += 0.001;
    uint8_t dutyCycle = map(tempDuty, 0, 100, motorLowerLimit, motorUpperLimit);  // map incoming range to this codes range.  Should match, but sense check...
    motorPWM->setPWM_manual(pinMotorOutput, tempDuty);
    delayMicroseconds(1);
  }
  tempDuty = 100;
  while (tempDuty > 0) {
    tempDuty -= 0.001;
    uint8_t dutyCycle = map(tempDuty, 0, 100, motorLowerLimit, motorUpperLimit);  // map incoming range to this codes range.  Should match, but sense check...
    motorPWM->setPWM_manual(pinMotorOutput, tempDuty);
    delayMicroseconds(1);
  }
  tempDuty = 0;
  motorPWM->setPWM_manual(pinMotorOutput, tempDuty);
  delay(sweepSpeed * 200);
}