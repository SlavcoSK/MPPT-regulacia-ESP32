void System_Processes(){
  ///////////////// FAN COOLING /////////////////
  if(enableFan == true){
    if(enableDynamicCooling == false){
      if(overrideFan == true)              {fanStatus = true;}
      else if(temperature >= temperatureFan){fanStatus = 1;}
      else if(temperature < temperatureFan) {fanStatus = 0;}
      digitalWrite(FAN, fanStatus);
    }
    else{}  // DYNAMIC PWM COOLING MODE (budúca implementácia)
  }
  else{digitalWrite(FAN, LOW);}
  
  // ============================================================
  // OPRAVA: Stopky loopTime — pôvodný kód mal zamenené
  // loopTimeStart a loopTimeEnd, čo dávalo vždy záporný alebo
  // nulový čas (loopTimeEnd bol vždy 0 pri prvom prechode).
  // Správne: najprv zaznamenaj koniec, potom vypočítaj rozdiel
  // od predchádzajúceho štartu, nakoniec aktualizuj štart.
  // ============================================================
  loopTimeEnd   = micros();
  loopTime      = (loopTimeEnd - loopTimeStart) / 1000.000;
  loopTimeStart = micros();

  ///////////// AUTO DATA RESET /////////////
  if(telemCounterReset == 0){}
  else if(telemCounterReset == 1 && daysRunning > 1)  {resetVariables();}
  else if(telemCounterReset == 2 && daysRunning > 7)  {resetVariables();}
  else if(telemCounterReset == 3 && daysRunning > 30) {resetVariables();}
  else if(telemCounterReset == 4 && daysRunning > 365){resetVariables();}

  ///////////// LOW POWER MODE /////////////
  if(lowPowerMode == 1){}   
  else{}      
}

// =====================================================================
// OPRAVA: factoryReset — hodnoty upresnené pre 24V olovený systém
// =====================================================================
void factoryReset(){
  EEPROM.write(0, 1);   // STORE: Charging Algorithm (1 = MPPT Mode)
  EEPROM.write(12, 1);  // STORE: Charger/PSU Mode Selection (1 = Charger Mode)
  // 28.80V → celá časť = 28, desatinná = 80
  EEPROM.write(1, 28);  // STORE: Max Battery Voltage (whole) — 28.80V absorption
  EEPROM.write(2, 80);  // STORE: Max Battery Voltage (decimal)
  // 23.60V → celá časť = 23, desatinná = 60
  EEPROM.write(3, 23);  // STORE: Min Battery Voltage (whole)
  EEPROM.write(4, 60);  // STORE: Min Battery Voltage (decimal)
  EEPROM.write(5, 30);  // STORE: Charging Current (whole)
  EEPROM.write(6, 0);   // STORE: Charging Current (decimal)
  EEPROM.write(7, 1);   // STORE: Fan Enable (Bool)
  EEPROM.write(8, 60);  // STORE: Fan Temp (Integer)
  EEPROM.write(9, 90);  // STORE: Shutdown Temp (Integer)
  EEPROM.write(10, 1);  // STORE: Enable WiFi (Boolean)
  EEPROM.write(11, 1);  // STORE: Enable autoload (on by default)
  EEPROM.write(13, 0);  // STORE: LCD backlight sleep timer (default: 0 = never)
  // Float voltage: 27.20V → 27 a 20
  EEPROM.write(14, 27); // STORE: Float Voltage (whole) — NOVÉ
  EEPROM.write(15, 20); // STORE: Float Voltage (decimal) — NOVÉ
  EEPROM.commit();
  loadSettings();
}

void loadSettings(){
  MPPT_Mode          = EEPROM.read(0);
  output_Mode        = EEPROM.read(12);
  voltageBatteryMax  = EEPROM.read(1) + (EEPROM.read(2) * 0.01);
  voltageBatteryMin  = EEPROM.read(3) + (EEPROM.read(4) * 0.01);
  currentCharging    = EEPROM.read(5) + (EEPROM.read(6) * 0.01);
  enableFan          = EEPROM.read(7);
  temperatureFan     = EEPROM.read(8);
  temperatureMax     = EEPROM.read(9);
  enableWiFi         = EEPROM.read(10);
  flashMemLoad       = EEPROM.read(11);
  backlightSleepMode = EEPROM.read(13);
  // NOVÉ: Načítaj float napätie
  voltageFloat       = EEPROM.read(14) + (EEPROM.read(15) * 0.01);
  // Ak float napätie nie je uložené (nová EEPROM), nastav rozumný default
  if(voltageFloat < 10.0 || voltageFloat > vOutSystemMax){
    voltageFloat = voltageBatteryMax - 1.60; // Aproximácia: float ≈ absorption - 1.6V pre 24V
  }
}

void saveSettings(){
  EEPROM.write(0, MPPT_Mode);
  EEPROM.write(12, output_Mode);

  // =====================================================================
  // OPRAVA: Pôvodný kód: EEPROM.write(1, voltageBatteryMax)
  // EEPROM.write() prijíma uint8_t — float sa implicitne truncuje na celé číslo!
  // Správne: celú a desatinnú časť uložiť zvlášť.
  // =====================================================================
  conv1 = (int)(voltageBatteryMax * 100);   // Napr. 28.80 → 2880
  EEPROM.write(1, (uint8_t)(conv1 / 100));  // Celá časť: 28
  EEPROM.write(2, (uint8_t)(conv1 % 100));  // Desatinná: 80

  conv1 = (int)(voltageBatteryMin * 100);
  EEPROM.write(3, (uint8_t)(conv1 / 100));
  EEPROM.write(4, (uint8_t)(conv1 % 100));

  conv1 = (int)(currentCharging * 100);
  EEPROM.write(5, (uint8_t)(conv1 / 100));
  EEPROM.write(6, (uint8_t)(conv1 % 100));

  EEPROM.write(7, enableFan);
  EEPROM.write(8, temperatureFan);
  EEPROM.write(9, temperatureMax);
  EEPROM.write(10, enableWiFi);
  EEPROM.write(13, backlightSleepMode);

  // NOVÉ: Uloženie float napätia
  conv1 = (int)(voltageFloat * 100);
  EEPROM.write(14, (uint8_t)(conv1 / 100));
  EEPROM.write(15, (uint8_t)(conv1 % 100));

  EEPROM.commit();
}

void saveAutoloadSettings(){
  EEPROM.write(11, flashMemLoad);
  EEPROM.commit();
}

void initializeFlashAutoload(){
  if(disableFlashAutoLoad == 0){
    flashMemLoad = EEPROM.read(11);
    if(flashMemLoad == 1){loadSettings();}
  }
  // Po načítaní nastavení vždy inicializuj teplotnú kompenzáciu
  computeTempCompensation();
}
