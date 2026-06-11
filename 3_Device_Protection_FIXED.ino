void backflowControl(){
  if(output_Mode == 0){bypassEnable = 1;}
  else{
    if(voltageInput > voltageOutput + voltageDropout){bypassEnable = 1;}
    else{bypassEnable = 0;}
  }
  digitalWrite(backflow_MOSFET, bypassEnable);
}

void Device_Protection(){
  // OPRAVA: currentErrorMillis sa musí aktualizovať, inak timer nikdy neprebehne.
  // Pôvodný kód: if(currentErrorMillis - prevErrorMillis >= ...) — currentErrorMillis 
  // zostával 0, takže podmienka (0 - prevErrorMillis) nikdy nebola splnená.
  currentErrorMillis = millis();

  if(currentErrorMillis - prevErrorMillis >= (unsigned long)errorTimeLimit){
    prevErrorMillis = currentErrorMillis;
    if(errorCount < errorCountLimit){
      errorCount = 0;  // Reset počtu chýb ak sú pod limitom
    } else {
      // Príliš veľa opakujúcich sa chýb — pozastav nabíjanie
      // (errorCount zostane vysoký, Charging_Algorithm zastaví buck)
      Serial.println("> WARNING: Persistent error detected, charging paused");
    }
  } 

  // FAULT DETECTION
  ERR = 0;
  backflowControl();

  if(temperature > temperatureMax)
    {OTE = 1; ERR++; errorCount++;} else {OTE = 0;}

  if(currentInput > currentInAbsolute)
    {IOC = 1; ERR++; errorCount++;} else {IOC = 0;}

  if(currentOutput > currentOutAbsolute)
    {OOC = 1; ERR++; errorCount++;} else {OOC = 0;}

  // OPRAVA: Ochranná hranica OOV — voltageBatteryMaxComp namiesto voltageBatteryMax
  // aby teplotná kompenzácia správne fungovala aj v ochrannej logike
  if(voltageOutput > voltageBatteryMaxComp + voltageBatteryThresh)
    {OOV = 1; ERR++; errorCount++;} else {OOV = 0;}

  if(voltageInput < vInSystemMin && voltageOutput < vInSystemMin)
    {FLV = 1; ERR++; errorCount++;} else {FLV = 0;}

  if(output_Mode == 0){
    REC = 0; BNC = 0;
    if(voltageInput < voltageBatteryMaxComp + voltageDropout)
      {IUV = 1; ERR++; errorCount++;} else {IUV = 0;}
  }
  else{
    backflowControl();
    if(voltageOutput < vInSystemMin)
      {BNC = 1; ERR++;} else {BNC = 0;}
    if(voltageInput < voltageBatteryMaxComp + voltageDropout)
      {IUV = 1; ERR++; REC = 1;} else {IUV = 0;}
  }
}
