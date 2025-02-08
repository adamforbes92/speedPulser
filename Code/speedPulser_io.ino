void basicInit() {
#if serialDebug
  Serial.begin(baudSerial);
  Serial.println(F("Initialising SpeedPulser..."));
#endif

#if serialDebug
  Serial.println(F("Initialising PWM..."));
#endif
  digitalWrite(pinMotOutput, HIGH);                          // force pin high before assigning - should stop motor jerking on boot?
  pinMode(pinMotOutput, OUTPUT);                             // now assign, should already be high
  PWM_Instance = new AVR_PWM(pinMotOutput, frequency, 100);  // begin PWM with a base freq. of 10kHz @ 100% duty (mot. off)

#if serialDebug
  Serial.println(F("Initialised PWM!"));
#endif

#if serialDebug
  Serial.println(F("Setting up pins..."));
#endif

  attachInterrupt(digitalPinToInterrupt(pinSpeedOutput), incomingHz, RISING);  //setup interrupt to toggle pin on change.  Consider changing to just a loop to capture pulses?

#if serialDebug
  Serial.println(F("Pins setup!"));
  Serial.println(F("Initialised SpeedPulser!"));
#endif
}

void testSpeed() {
  if (tempDuty > 0) {
    PWM_Instance->setPWM(pinMotOutput, frequency, tempDuty);
  } else {
    for (int i = motorUpperLimit; i >= motorLowerLimit; i--) {
      Serial.println(i);
      PWM_Instance->setPWM(pinMotOutput, frequency, i);
      delay(200);
    }

    for (int i = motorLowerLimit; i <= motorUpperLimit; i++) {
      Serial.println(i);
      PWM_Instance->setPWM(pinMotOutput, frequency, i);
      delay(200);
    }
  }
}