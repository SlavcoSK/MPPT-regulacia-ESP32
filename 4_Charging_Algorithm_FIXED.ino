void buck_Enable(){
  buckEnable = 1;
  digitalWrite(buck_EN, HIGH);
  digitalWrite(LED, HIGH);
}

void buck_Disable(){
  buckEnable = 0; 
  digitalWrite(buck_EN, LOW);
  digitalWrite(LED, LOW);
  PWM = 0;
}

void predictivePWM(){
  if(voltageInput <= 0){PPWM = 0;}
  else{PPWM = (PPWM_margin * pwmMax * voltageOutput) / (100.00 * voltageInput);}
  PPWM = constrain(PPWM, 0, pwmMaxLimited);
}   

void PWM_Modulation(){
  if(output_Mode == 0){
    PWM = constrain(PWM, 0, pwmMaxLimited);
  }
  else{
    predictivePWM();
    PWM = constrain(PWM, PPWM, pwmMaxLimited);
  } 
  ledcWrite(pwmChannel, PWM);
  buck_Enable();
}

// =====================================================================
// TROJFÁZOVÝ NABÍJACÍ ALGORITMUS PRE OLOVENÉ BATÉRIE
// =====================================================================
// Fáza 0 - BULK (CC):        Maximálny prúd, napätie stúpa k absorpčnej hodnote
// Fáza 1 - ABSORPTION (CV):  Konštantné napätie = voltageBatteryMaxComp,
//                             prúd prirodzene klesá, končí keď I < absorptionCurrentEnd
//                             alebo po absorptionTimeLimit ms
// Fáza 2 - FLOAT:            Udržiavacie napätie = voltageFloatComp,
//                             zabraňuje prebitiu batérie počas dlhého slnečného dňa
// =====================================================================

void updateChargingPhase(){
  // Prechod BULK → ABSORPTION
  if(chargingPhase == 0){
    if(voltageOutput >= voltageBatteryMaxComp){
      chargingPhase = 1;
      absorptionStartMillis = millis();
      Serial.println("> CHARGING PHASE: Bulk → Absorption");
    }
  }
  // Prechod ABSORPTION → FLOAT
  else if(chargingPhase == 1){
    unsigned long absorptionElapsed = millis() - absorptionStartMillis;
    bool timeExpired    = (absorptionElapsed >= absorptionTimeLimit);
    bool currentDropped = (currentOutput <= absorptionCurrentEnd && currentOutput > 0);
    if(timeExpired || currentDropped){
      chargingPhase = 2;
      Serial.print("> CHARGING PHASE: Absorption → Float (reason: ");
      if(timeExpired)    Serial.print("timeout)");
      if(currentDropped) Serial.print("current threshold)");
      Serial.println("");
    }
  }
  // Float → Bulk: ak batéria klesne výrazne pod float napätie (napr. nočná spotreba)
  else if(chargingPhase == 2){
    if(voltageOutput < voltageFloatComp - 0.50){
      chargingPhase = 0;
      Serial.println("> CHARGING PHASE: Float → Bulk (battery discharged)");
    }
  }
}

// =====================================================================
// MPPT ALGORITMUS S HYSTERÉZOU
// Pôvodný kód inkrementoval/dekrementoval PWM o 1 krok každý cyklus —
// pri dosiahnutí MPP PWM osciloval +1/-1 nepretržite.
// Oprava: sledujeme či sme už raz prekonali MPP a použijeme
// menší krok (fineStep) pre jemné doladenie, väčší (coarseStep) pre hrubé.
// =====================================================================
int mpptCoarseStep = 3;   // Hrubý krok ďaleko od MPP
int mpptFineStep   = 1;   // Jemný krok blízko MPP
float mpptPowerHysteresis = 0.5; // Minimálna zmena výkonu (W) považovaná za skutočnú zmenu

void Charging_Algorithm(){
  if(ERR > 0 || chargingPause == 1){
    buck_Disable();
    // Reset fázy nabíjania pri poruche
    if(ERR > 0) chargingPhase = 0;
    return;
  }

  if(REC == 1){
    REC = 0;
    buck_Disable();
    lcd.setCursor(0, 0); lcd.print("POWER SOURCE    ");
    lcd.setCursor(0, 1); lcd.print("DETECTED        ");
    Serial.println("> Solar Panel Detected");
    Serial.print("> Computing For Predictive PWM ");
    for(int i = 0; i < 40; i++){Serial.print("."); delay(30);}
    Serial.println("");
    Read_Sensors();
    predictivePWM();
    PWM = PPWM;
    lcd.clear();
    return;
  }

  // ==== CC-CV PSU REŽIM (bez MPPT, bez fáz nabíjania) ====
  if(MPPT_Mode == 0){
    if(currentOutput > currentCharging)          {PWM--;}
    else if(voltageOutput > voltageBatteryMaxComp){PWM--;}
    else if(voltageOutput < voltageBatteryMaxComp){PWM++;}
    PWM_Modulation();
    return;
  }

  // ==== MPPT + TROJFÁZOVÉ NABÍJANIE PRE OLOVENÉ BATÉRIE ====

  // Aktualizuj fázu nabíjania
  updateChargingPhase();

  // --- FÁZA 2: FLOAT ---
  if(chargingPhase == 2){
    // Udržuj výstupné napätie na úrovni float napätia
    if(currentOutput > currentCharging)         {PWM--;}
    else if(voltageOutput > voltageFloatComp)    {PWM--;}
    else if(voltageOutput < voltageFloatComp - 0.10){PWM++;}
    // V float fáze nepotrebujeme MPPT — jednoduché CV riadenie
    PWM_Modulation();
    return;
  }

  // --- FÁZA 1: ABSORPTION (CV pri voltageBatteryMaxComp) ---
  if(chargingPhase == 1){
    if(currentOutput > currentCharging)              {PWM--;}
    else if(voltageOutput > voltageBatteryMaxComp)    {PWM--;}
    else if(voltageOutput < voltageBatteryMaxComp - 0.05){PWM++;}
    // V absorption fáze tiež nepotrebujeme MPPT — CV riadenie
    PWM_Modulation();
    return;
  }

  // --- FÁZA 0: BULK (CC + MPPT) ---
  // Prúdový limit má prioritu
  if(currentOutput > currentCharging){
    PWM--;
    PWM_Modulation();
    return;
  }
  // Napäťový limit (prechod do absorption)
  if(voltageOutput >= voltageBatteryMaxComp){
    PWM_Modulation();
    return;
  }

  // MPPT ALGORITMUS S HYSTERÉZOU
  float powerDelta   = powerInput - powerInputPrev;
  float voltageDelta = voltageInput - voltageInputPrev;

  // Ignoruj zmeny výkonu menšie ako hysteréza (šum)
  if(abs(powerDelta) > mpptPowerHysteresis){
    int step = (abs(powerDelta) > 5.0) ? mpptCoarseStep : mpptFineStep;

    if(powerDelta > 0 && voltageDelta > 0)     {PWM -= step;}  // ↑P ↑V → sme za MPP → znižuj DC
    else if(powerDelta > 0 && voltageDelta < 0){PWM += step;}  // ↑P ↓V → blížime sa k MPP → zvyšuj DC
    else if(powerDelta < 0 && voltageDelta > 0){PWM += step;}  // ↓P ↑V → sme za MPP → zvyšuj DC
    else if(powerDelta < 0 && voltageDelta < 0){PWM -= step;}  // ↓P ↓V → ideme preč od MPP → znižuj DC
  }
  else{
    // Sme blízko MPP — jemné doladenie smerom k nabíjaciemu napätiu
    if(voltageOutput < voltageBatteryMaxComp){PWM += mpptFineStep;}
  }

  powerInputPrev   = powerInput;
  voltageInputPrev = voltageInput;
  PWM_Modulation();
}
