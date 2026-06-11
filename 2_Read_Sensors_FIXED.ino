void ADC_SetGain(){
  if(ADS1015_Mode == true){
    if(ADC_GainSelect == 0){ads.setGain(GAIN_TWOTHIRDS); ADC_BitReso = 3.0000;}
    else if(ADC_GainSelect == 1){ads.setGain(GAIN_ONE);  ADC_BitReso = 2.0000;}
    else if(ADC_GainSelect == 2){ads.setGain(GAIN_TWO);  ADC_BitReso = 1.0000;}
  }
  else{
    if(ADC_GainSelect == 0){ads.setGain(GAIN_TWOTHIRDS); ADC_BitReso = 0.1875;}
    else if(ADC_GainSelect == 1){ads.setGain(GAIN_ONE);  ADC_BitReso = 0.125;}
    else if(ADC_GainSelect == 2){ads.setGain(GAIN_TWO);  ADC_BitReso = 0.0625;}
  }
}

void resetVariables(){
  secondsElapsed = 0;
  energySavings  = 0; 
  daysRunning    = 0; 
  timeOn         = 0; 
}

// =====================================================================
// NOVÉ: Teplotná kompenzácia nabíjacieho napätia pre olovené batérie
// Štandard: -3mV/článok/°C oproti referenčnej teplote 25°C
// Pre 24V systém (12 článkov): korekcia = 12 * (-0.003) * (T - 25)
// =====================================================================
void computeTempCompensation(){
  if(enableTempCompensation && temperature > -40 && temperature < 80){
    float deltaT = (float)temperature - tempCompRef;
    float correction = (float)batteryNumCells * tempCompCoeff * deltaT;
    // Obmedzenie kompenzácie na rozumný rozsah (±2V pre 24V systém)
    correction = constrain(correction, -2.0, 2.0);
    voltageBatteryMaxComp = voltageBatteryMax + correction;
    voltageFloatComp      = voltageFloat      + correction;
  } else {
    // Ak teplota nie je k dispozícii alebo mimo rozsahu, použij nekompenzované hodnoty
    voltageBatteryMaxComp = voltageBatteryMax;
    voltageFloatComp      = voltageFloat;
  }
}

void Read_Sensors(){

  /////////// TEMPERATURE SENSOR /////////////
  if(sampleStoreTS <= avgCountTS){
    TS = TS + analogRead(TempSensor);
    sampleStoreTS++;   
  }
  else{
    TS = TS / sampleStoreTS;
    // Ochrana pred delením nulou a neplatnou hodnotou ADC
    if(TS > 0){
      float tsRatio = 4095.00 / TS - 1.00;
      if(tsRatio > 0){
        TSlog = log(ntcResistance * tsRatio);
        temperature = (1.0 / (1.009249522e-03 + 2.378405444e-04 * TSlog + 2.019202697e-07 * TSlog * TSlog * TSlog)) - 273.15;
        // Obmedzenie na reálny rozsah teplôt
        temperature = constrain(temperature, -40, 120);
      }
    }
    sampleStoreTS = 0;
    TS = 0;
    // Prepočítaj teplotnú kompenzáciu po každom meraní teploty
    computeTempCompensation();
  }

  /////////// VOLTAGE & CURRENT SENSORS /////////////
  VSI = 0.0000;
  VSO = 0.0000;
  CSI = 0.0000;

  for(int i = 0; i < avgCountVS; i++){
    VSI = VSI + ads.computeVolts(ads.readADC_SingleEnded(3));
    VSO = VSO + ads.computeVolts(ads.readADC_SingleEnded(1));
  }
  voltageInput  = (VSI / avgCountVS) * inVoltageDivRatio; 
  voltageOutput = (VSO / avgCountVS) * outVoltageDivRatio;
  // Ochrana pred záporným napätím (šum ADC)
  if(voltageInput  < 0) voltageInput  = 0;
  if(voltageOutput < 0) voltageOutput = 0;

  for(int i = 0; i < avgCountCS; i++){
    CSI = CSI + ads.computeVolts(ads.readADC_SingleEnded(2));
  }
  CSI_converted = (CSI / avgCountCS) * 1.3300;
  currentInput  = ((CSI_converted - currentMidPoint) * -1) / currentSensV;  
  if(currentInput < 0){currentInput = 0.0000;}

  // OPRAVA: Pôvodný kód počítal currentOutput zo vstupných veličín (chybne).
  // Správny výpočet: ak máme len jeden senzor prúdu na vstupe, odvodíme výstupný 
  // prúd z bilancie výkonu cez účinnosť (P_out = P_in * η → I_out = P_in * η / V_out).
  // Ak máš druhý prúdový senzor na výstupe, zmeraj ho priamo na kanáli 0 ADS.
  if(voltageOutput > 1.0){
    currentOutput = (voltageInput * currentInput * efficiencyRate) / voltageOutput;
  } else {
    currentOutput = 0.0000;
  }
  if(currentOutput < 0) currentOutput = 0.0000;

  //POWER SOURCE DETECTION
  if(voltageInput <= 3 && voltageOutput <= 3){inputSource = 0;}
  else if(voltageInput > voltageOutput)      {inputSource = 1;}
  else if(voltageInput < voltageOutput)      {inputSource = 2;}
  
  //////// AUTOMATIC CURRENT SENSOR CALIBRATION ////////
  // OPRAVA: Kalibrácia len keď buck je vypnutý, nie sú žiadne chyby, 
  // a vstupné napätie je minimálne (zabráni kalibrácii na nesprávnu hodnotu)
  if(buckEnable == 0 && FLV == 0 && OOV == 0 && voltageInput < 3.0){
    currentMidPoint = ((CSI / avgCountCS) * 1.3300) - 0.003;
  }
  
  //POWER COMPUTATION
  powerInput      = voltageInput  * currentInput;
  powerOutput     = voltageOutput * currentOutput;  // OPRAVA: výkon na výstupe = V_out * I_out
  outputDeviation = (voltageOutput / voltageBatteryMaxComp) * 100.000;

  // STATE OF CHARGE pre olovené batérie (lineárna aproximácia - pre presnosť použiť OCV tabuľku)
  batteryPercent  = ((voltageOutput - voltageBatteryMin) / (voltageBatteryMax - voltageBatteryMin)) * 101;
  batteryPercent  = constrain(batteryPercent, 0, 100);

  //TIME DEPENDENT SENSOR DATA COMPUTATION
  currentRoutineMillis = millis();
  if(currentRoutineMillis - prevRoutineMillis >= millisRoutineInterval){
    prevRoutineMillis = currentRoutineMillis;
    Wh = Wh + (powerInput / (3600.000 * (1000.000 / millisRoutineInterval)));
    kWh = Wh / 1000.000;
    MWh = Wh / 1000000.000;
    daysRunning = timeOn / (86400.000 * (1000.000 / millisRoutineInterval));
    timeOn++;
  } 

  secondsElapsed = millis() / 1000;
  energySavings  = electricalPrice * (Wh / 1000.0000);
}
