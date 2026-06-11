void Onboard_Telemetry(){    

  currentSerialMillis = millis();
  if(currentSerialMillis - prevSerialMillis >= millisSerialInterval){
    prevSerialMillis = currentSerialMillis;

    if(serialTelemMode == 0){}

    else if(serialTelemMode == 1){  // 1 - Display All
      Serial.print(" ERR:");    Serial.print(ERR);
      Serial.print(" FLV:");    Serial.print(FLV);  
      Serial.print(" BNC:");    Serial.print(BNC);  
      Serial.print(" IUV:");    Serial.print(IUV); 
      Serial.print(" IOC:");    Serial.print(IOC); 
      Serial.print(" OOV:");    Serial.print(OOV); 
      Serial.print(" OOC:");    Serial.print(OOC);
      Serial.print(" OTE:");    Serial.print(OTE); 
      Serial.print(" REC:");    Serial.print(REC);
      Serial.print(" MPPTA:");  Serial.print(MPPT_Mode);     
      Serial.print(" CM:");     Serial.print(output_Mode);
      
      Serial.print(" ");
      Serial.print(" BYP:");    Serial.print(bypassEnable);
      Serial.print(" EN:");     Serial.print(buckEnable);
      Serial.print(" FAN:");    Serial.print(fanStatus);    
      Serial.print(" WiFi:");   Serial.print(WIFI);

      // NOVÉ: Fáza nabíjania olovenej batérie
      Serial.print(" PHASE:");
      if(chargingPhase == 0)     Serial.print("BULK");
      else if(chargingPhase == 1)Serial.print("ABS");
      else if(chargingPhase == 2)Serial.print("FLT");
      
      Serial.print(" ");  
      Serial.print(" PI:");     Serial.print(powerInput, 0); 
      Serial.print(" PO:");     Serial.print(powerOutput, 0);  // NOVÉ: výstupný výkon
      Serial.print(" PWM:");    Serial.print(PWM); 
      Serial.print(" PPWM:");   Serial.print(PPWM); 
      Serial.print(" VI:");     Serial.print(voltageInput, 1); 
      Serial.print(" VO:");     Serial.print(voltageOutput, 1); 
      Serial.print(" CI:");     Serial.print(currentInput, 2); 
      Serial.print(" CO:");     Serial.print(currentOutput, 2); 
      Serial.print(" Wh:");     Serial.print(Wh, 2); 
      Serial.print(" Temp:");   Serial.print(temperature, 1);

      // NOVÉ: Teplotne kompenzované napätia
      Serial.print(" VmaxC:");  Serial.print(voltageBatteryMaxComp, 2);
      Serial.print(" VfltC:");  Serial.print(voltageFloatComp, 2);
      
      Serial.print(" ");
      Serial.print(" CSMPV:");  Serial.print(currentMidPoint, 3);  
      Serial.print(" CSV:");    Serial.print(CSI_converted, 3);   
      Serial.print(" VO%Dev:"); Serial.print(outputDeviation, 1);   
      Serial.print(" SOC:");    Serial.print(batteryPercent); Serial.print("%"); 
      Serial.print(" T:");      Serial.print(secondsElapsed); 
      Serial.print(" LoopT:");  Serial.print(loopTime, 3); Serial.print("ms");  
      Serial.println("");    
    }

    else if(serialTelemMode == 2){  // 2 - Display Essential Data
      Serial.print(" PI:");    Serial.print(powerInput, 0); 
      Serial.print(" PWM:");   Serial.print(PWM); 
      Serial.print(" PPWM:");  Serial.print(PPWM); 
      Serial.print(" VI:");    Serial.print(voltageInput, 1); 
      Serial.print(" VO:");    Serial.print(voltageOutput, 1); 
      Serial.print(" CI:");    Serial.print(currentInput, 2); 
      Serial.print(" CO:");    Serial.print(currentOutput, 2); 
      Serial.print(" Wh:");    Serial.print(Wh, 2); 
      Serial.print(" Temp:");  Serial.print(temperature, 1);  
      Serial.print(" EN:");    Serial.print(buckEnable);
      Serial.print(" FAN:");   Serial.print(fanStatus);
      Serial.print(" PH:");    Serial.print(chargingPhase);  // NOVÉ: fáza nabíjania
      Serial.print(" SOC:");   Serial.print(batteryPercent); Serial.print("%"); 
      Serial.print(" T:");     Serial.print(secondsElapsed); 
      Serial.print(" LoopT:"); Serial.print(loopTime, 3); Serial.print("ms");  
      Serial.println("");    
    }  

    else if(serialTelemMode == 3){  // 3 - Display Numbers Only 
      Serial.print(" "); Serial.print(powerInput, 0); 
      Serial.print(" "); Serial.print(voltageInput, 1); 
      Serial.print(" "); Serial.print(voltageOutput, 1); 
      Serial.print(" "); Serial.print(currentInput, 2); 
      Serial.print(" "); Serial.print(currentOutput, 2);   
      Serial.print(" "); Serial.print(Wh, 2); 
      Serial.print(" "); Serial.print(temperature, 1);  
      Serial.print(" "); Serial.print(buckEnable);
      Serial.print(" "); Serial.print(fanStatus);   
      Serial.print(" "); Serial.print(batteryPercent);
      Serial.print(" "); Serial.print(chargingPhase);  // NOVÉ
      Serial.print(" "); Serial.print(secondsElapsed); 
      Serial.print(" "); Serial.print(loopTime, 3);
      Serial.println("");    
    }  
  } 
}
