void setupUI() {
  //Turn off verbose debugging
  ESPUI.setVerbosity(Verbosity::Quiet);
  ESPUI.sliderContinuous = true;

  // create basic tab
  auto tabBasic = ESPUI.addControl(Tab, "", "Basic Controls");
  ESPUI.addControl(Separator, "Needle Sweep", "", Dark, tabBasic);
  bool_NeedleSweep = ESPUI.addControl(Switcher, "Needle Sweep", String(hasNeedleSweep), Dark, tabBasic, generalCallback);
  int16_sweepSpeed = ESPUI.addControl(Slider, "Rate of Change (ms)", String(sweepSpeed), Dark, tabBasic, generalCallback);
  ESPUI.addControl(Min, "", "0", Dark, int16_sweepSpeed);
  ESPUI.addControl(Max, "", "50", Dark, int16_sweepSpeed);
  ESPUI.addControl(Button, "Test Needle Sweep", "Test", Dark, tabBasic, extendedCallback, (void *)11);

  // create advanced tab
  auto tabAdvanced = ESPUI.addControl(Tab, "", "Advanced Controls");
  ESPUI.addControl(Separator, "Testing", "", Dark, tabAdvanced);
  bool_testSpeedo = ESPUI.addControl(Switcher, "Test Speedo", "", Dark, tabAdvanced, generalCallback);
  int16_tempSpeed = ESPUI.addControl(Slider, "Go to Speed", String(tempSpeed), Dark, tabAdvanced, generalCallback);
  ESPUI.addControl(Min, "", "0", Dark, int16_tempSpeed);
  ESPUI.addControl(Max, "", "200", Dark, int16_tempSpeed);

  ESPUI.addControl(Separator, "Speed Offsets", "", Dark, tabAdvanced);
  bool_positiveOffset = ESPUI.addControl(Switcher, "Positive Offset", String(speedOffsetPositive), Dark, tabAdvanced, generalCallback);
  int16_speedOffset = ESPUI.addControl(Slider, "Speed Offset", String(speedOffset), Dark, tabAdvanced, generalCallback);
  ESPUI.addControl(Min, "", "0", Dark, int16_speedOffset);
  ESPUI.addControl(Max, "", "50", Dark, int16_speedOffset);

  ESPUI.addControl(Separator, "Speed Limits:", "", Dark, tabAdvanced);
  int16_minSpeed = ESPUI.addControl(Slider, "Minimum Speed", String(minSpeed), Dark, tabAdvanced, generalCallback);
  int16_maxSpeed = ESPUI.addControl(Slider, "Maximum Speed", String(maxSpeed), Dark, tabAdvanced, generalCallback);
  ESPUI.addControl(Min, "", "0", Dark, int16_minSpeed);
  ESPUI.addControl(Max, "", "400", Dark, int16_maxSpeed);
  ESPUI.addControl(Button, "Reset", "Reset", Dark, tabAdvanced, extendedCallback, (void *)12);

  ESPUI.addControl(Separator, "Hall Incoming Freq:", "", Dark, tabAdvanced);
  int16_minHall = ESPUI.addControl(Slider, "Minimum Hall", String(minFreqHall), Dark, tabAdvanced, generalCallback);
  int16_maxHall = ESPUI.addControl(Slider, "Maximum Hall", String(maxFreqHall), Dark, tabAdvanced, generalCallback);
  ESPUI.addControl(Min, "", "0", Dark, int16_minHall);
  ESPUI.addControl(Max, "", "400", Dark, int16_maxHall);
  ESPUI.addControl(Button, "Reset", "Reset", Dark, tabAdvanced, extendedCallback, (void *)13);

  ESPUI.addControl(Separator, "CAN Incoming Freq:", "", Dark, tabAdvanced);
  int16_minCAN = ESPUI.addControl(Slider, "Minimum CAN", String(minFreqCAN), Dark, tabAdvanced, generalCallback);
  int16_maxCAN = ESPUI.addControl(Slider, "Maximum CAN", String(maxFreqCAN), Dark, tabAdvanced, generalCallback);
  ESPUI.addControl(Min, "", "0", Dark, int16_minCAN);
  ESPUI.addControl(Max, "", "400", Dark, int16_maxCAN);
  ESPUI.addControl(Button, "Reset", "Reset", Dark, tabAdvanced, extendedCallback, (void *)14);

  ESPUI.addControl(Separator, "Incoming Speed:", "", Dark, tabAdvanced);
  label_speed = ESPUI.addControl(Label, "", "0", Dark, tabAdvanced, generalCallback);

  //Finally, start up the UI.
  //This should only be called once we are connected to WiFi.
  ESPUI.begin(wifiHostName);
}

void updateCallback(Control *sender, int type) {
  updates = (sender->value.toInt() > 0);
}

void getTimeCallback(Control *sender, int type) {
  if (type == B_UP) {
    ESPUI.updateTime(mainTime);
  }
}

void graphAddCallback(Control *sender, int type) {
  if (type == B_UP) {
    ESPUI.addGraphPoint(graph, random(1, 50));
  }
}

void graphClearCallback(Control *sender, int type) {
  if (type == B_UP) {
    ESPUI.clearGraph(graph);
  }
}

void generalCallback(Control *sender, int type) {
#ifdef serialDebugWifi
  Serial.print("CB: id(");
  Serial.print(sender->id);
  Serial.print(") Type(");
  Serial.print(type);
  Serial.print(") '");
  Serial.print(sender->label);
  Serial.print("' = ");
  Serial.println(sender->value);
#endif

  uint8_t tempID = int(sender->id);
  switch (tempID) {
    case 3:
      hasNeedleSweep = sender->value.toInt();
      break;
    case 4:
      sweepSpeed = sender->value.toInt();
      break;
    case 10:
      testSpeedo = sender->value.toInt();
      break;
    case 11:
      tempSpeed = sender->value.toInt();
      break;
    case 15:
      speedOffsetPositive = sender->value.toInt();
      break;
    case 16:
      speedOffset = sender->value.toInt();
      break;
    case 20:
      minSpeed = sender->value.toInt();
      break;
    case 21:
      maxSpeed = sender->value.toInt();
      break;
    case 26:
      minFreqHall = sender->value.toInt();
      break;
    case 27:
      maxFreqHall = sender->value.toInt();
      break;
    case 32:
      minFreqCAN = sender->value.toInt();
      break;
    case 33:
      maxFreqCAN = sender->value.toInt();
      break;
  }
}

void extendedCallback(Control *sender, int type, void *param) {
#ifdef serialDebugWifi
  Serial.print("CB: id(");
  Serial.print(sender->id);
  Serial.print(") Type(");
  Serial.print(type);
  Serial.print(") '");
  Serial.print(sender->label);
  Serial.print("' = ");
  Serial.println(sender->value);
  Serial.print("param = ");
  Serial.println((long)param);
#endif

  uint8_t tempID = int(sender->id);
  switch (tempID) {
    case 7:
      if (type == B_UP) {
        tempNeedleSweep = true;
      }
      break;
    case 24:
      if (type == B_UP) {
        minSpeed = 0;
        maxSpeed = 200;
        ESPUI.updateSlider(int16_minSpeed, minSpeed);
        ESPUI.updateSlider(int16_maxSpeed, maxSpeed);
      }
      break;
    case 30:
      if (type == B_UP) {
        minFreqHall = 0;
        maxFreqHall = 200;
        ESPUI.updateSlider(int16_minHall, minFreqHall);
        ESPUI.updateSlider(int16_maxHall, maxFreqHall);
      }
      break;
    case 36:
      if (type == B_UP) {
        minFreqCAN = 0;
        maxFreqCAN = 200;
        ESPUI.updateSlider(int16_minCAN, minFreqCAN);
        ESPUI.updateSlider(int16_maxCAN, maxFreqCAN);
      }
      break;
  }
}

void connectWifi() {
  int connect_timeout;

  WiFi.setHostname(wifiHostName);
  Serial.println("Begin wifi...");

  Serial.println("\nCreating access point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(wifiHostName);

  connect_timeout = 20;
  do {
    delay(250);
    Serial.print(",");
    connect_timeout--;
  } while (connect_timeout);
}

void textCallback(Control *sender, int type) {
  //This callback is needed to handle the changed values, even though it doesn't do anything itself.
}

void randomString(char *buf, int len) {
  for (auto i = 0; i < len - 1; i++)
    buf[i] = random(0, 26) + 'A';
  buf[len - 1] = '\0';
}

void disconnectWifi() {
  DEBUG_PRINTF("Number of connections: ");
  DEBUG_PRINTLN(WiFi.softAPgetStationNum());

  if (WiFi.softAPgetStationNum() == 0) {
    DEBUG_PRINTLN("No connections, turning off");
    WiFi.disconnect(true, false);
    WiFi.mode(WIFI_OFF);
  }
}
