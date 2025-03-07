void basicInit() {
  DEBUG_PRINTLN("Initialising SpeedPulser...");

  DEBUG_PRINTLN("Setting up PWM...");
  motorPWM = new ESP32_FAST_PWM(pinMotorOutput, pwmFrequency, dutyCycle, 0, pwmResolution);  // begin PWM with a base freq. of 10kHz @ 100% duty (mot. off)
  DEBUG_PRINTLN("Set up PWM!");

  DEBUG_PRINTLN("Setting up speed interrupt...");
  attachInterrupt(digitalPinToInterrupt(pinSpeedInput), incomingHz, FALLING);  //setup interrupt to toggle pin on change.  Consider changing to just a loop to capture pulses?
  DEBUG_PRINTLN("Set up speed interrupt!");

  DEBUG_PRINTLN("Initialised SpeedPulser!");
}

void testSpeed() {
  DEBUG_PRINTF("PWM: %d", tempDuty);
  tempDuty = 0;
  motorPWM->setPWM_manual(pinMotorOutput, tempDuty);
  delay(sweepSpeed * 200);
}