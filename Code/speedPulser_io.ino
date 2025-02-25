void basicInit() {
  DEBUG("Initialising PWM...");
  pinMode(pinMotOutput, OUTPUT);                                          // now assign, should already be high
  digitalWrite(pinMotOutput, HIGH);                                       // force pin high before assigning - should stop motor jerking on boot?
  PWM_Instance = new ESP32_FAST_PWM(pinMotOutput, frequency, dutyCycle);  // begin PWM with a base freq. of 10kHz @ 100% duty (mot. off)
  DEBUG("Initialised PWM...");

  DEBUG("Setting up pins...");
  attachInterrupt(digitalPinToInterrupt(pinSpeedOutput), incomingHz, RISING);  //setup interrupt to toggle pin on change.  Consider changing to just a loop to capture pulses?
  DEBUG("Pins setup!");

  DEBUG("Initialised SpeedPulser!");
}

void testSpeed() {
  DEBUG("PWM: %d", tempDuty);
  while (tempDuty < 100) {
    tempDuty += 0.01;
    uint8_t dutyCycle = map(tempDuty, 0, 100, motorLowerLimit, motorUpperLimit);  // map incoming range to this codes range.  Should match, but sense check...
    PWM_Instance->setPWM_manual(pinMotOutput, tempDuty);
    delay(sweepSpeed);
  }
}