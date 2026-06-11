void setupWiFi(){
  if(enableWiFi == 1){
    Blynk.begin(auth, ssid, pass);
    WIFI = 1;
  }
}

void Wireless_Telemetry(){
  
  ////////// WIFI TELEMETRY //////////
  if(enableWiFi == 1){
    int LED1, LED2, LED3, LED4, LED5;
    if(buckEnable == 1)      {LED1 = 200;} else {LED1 = 0;}   // NABÍJANIE AKTÍVNE
    if(batteryPercent >= 99) {LED2 = 200;} else {LED2 = 0;}   // PLNÁ BATÉRIA
    if(batteryPercent <= 10) {LED3 = 200;} else {LED3 = 0;}   // NÍZKA BATÉRIA
    if(IUV == 0)             {LED4 = 200;} else {LED4 = 0;}   // SOLÁRNY PANEL PRÍTOMNÝ
    // NOVÉ: LED indikátor fázy nabíjania
    if(chargingPhase == 2)   {LED5 = 200;} else {LED5 = 0;}   // FLOAT FÁZA (plná batéria)

    Blynk.run();  
    Blynk.virtualWrite(1,  powerInput); 
    Blynk.virtualWrite(2,  batteryPercent);
    Blynk.virtualWrite(3,  voltageInput);    
    Blynk.virtualWrite(4,  currentInput);   
    Blynk.virtualWrite(5,  voltageOutput); 
    Blynk.virtualWrite(6,  currentOutput); 
    Blynk.virtualWrite(7,  temperature); 
    Blynk.virtualWrite(8,  Wh / 1000); 
    Blynk.virtualWrite(9,  energySavings);       
    Blynk.virtualWrite(10, LED1);   // LED - Battery Charging Status
    Blynk.virtualWrite(11, LED2);   // LED - Full Battery Charge Status
    Blynk.virtualWrite(12, LED3);   // LED - Low Battery Charge Status
    Blynk.virtualWrite(13, LED4);   // LED - PV Harvesting

    Blynk.virtualWrite(14, voltageBatteryMin);       // Minimum Battery Voltage
    Blynk.virtualWrite(15, voltageBatteryMaxComp);   // OPRAVA: teplotne kompenzované max napätie
    Blynk.virtualWrite(16, currentCharging);         // Charging Current
    Blynk.virtualWrite(17, electricalPrice);         // Electrical Price
    // NOVÉ: Float napätie a fáza nabíjania
    Blynk.virtualWrite(18, voltageFloatComp);        // Float Voltage (teplotne komp.)
    Blynk.virtualWrite(19, chargingPhase);           // Fáza nabíjania (0=Bulk,1=Abs,2=Float)
    Blynk.virtualWrite(20, LED5);                    // LED - Float Phase
  }

  ////////// BLUETOOTH TELEMETRY //////////
  if(enableBluetooth == 1){
    // Budúca implementácia BLE
  }
}
