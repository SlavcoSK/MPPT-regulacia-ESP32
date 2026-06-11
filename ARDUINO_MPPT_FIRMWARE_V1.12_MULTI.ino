/*  PROJECT FUGU FIRMWARE V1.12  (DIY 1kW Open Source MPPT Solar Charge Controller)
 *  By: TechBuilder (Angelo Casimiro)
 *  Upravené pre nabíjanie Lead-Acid, LiFePO4, NiMH batérií (12V/24V/48V)
 *  OPRAVY: Slavko / Claude
 *  -----------------------------------------------------------------------------------------------------------
 *  V1.12 NOVÉ FUNKCIE:
 *  - Podpora 4 typov batérií × 3 napäťové systémy
 *  - Flexibilné nastavenie všetkých parametrov z LCD MENU
 *  - Odporúčané hodnoty pre všetky kombinácie
 *  - Teplotná kompenzácia s opačnými znamienkami podľa typu
 *  - Trojfázové (Lead-Acid) alebo CC-CV (LiFePO4, NiMH) režimy
 *  -----------------------------------------------------------------------------------------------------------
 */

//================================ MPPT FIRMWARE LCD MENU INFO =====================================//
String 
firmwareInfo      = "V1.12   ",
firmwareDate      = "12/06/26",
firmwareContactR1 = "Multi-Batt MPPT ",  
firmwareContactR2 = "Solar Charger   ";        
           
//====================== ARDUINO LIBRARIES =========================================================//
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_ADS1X15.h>

LiquidCrystal_I2C lcd(0x27,16,2);
TaskHandle_t Core2;
Adafruit_ADS1015 ads;

//====================================== GPIO PINS ==================================================//
#define backflow_MOSFET 27
#define buck_IN         33
#define buck_EN         32
#define LED             2
#define FAN             16
#define ADC_ALERT       34
#define TempSensor      35
#define buttonLeft      18
#define buttonRight     17
#define buttonBack      19
#define buttonSelect    23

//========================================= WiFi CREDENTIALS ========================================//
char 
auth[] = "InputBlynkAuthenticationToken",
ssid[] = "InputWiFiSSID",
pass[] = "InputWiFiPassword";

//====================================== VÝBER BATÉRIE A SYSTÉMU ===================================//
// ZMEŇ TIETO DVE HODNOTY NA VÝBER TYPU BATÉRIE A NAPÄTIA:
#define BATTERY_TYPE 0        // 0=Lead-Acid, 1=LiFePO4, 2=NiMH, 3=Custom
#define VOLTAGE_SYSTEM 24     // 12, 24, alebo 48 (Volty)

//====================================== DYNAMICKÉ KONFIGURÁCIE BATÉRIÍ ============================//

#if BATTERY_TYPE == 0  // ===== LEAD-ACID (TROJFÁZOVÉ) =====

  #define BATTERY_NAME "Lead-Acid"
  #define USE_THREE_PHASE_CHARGING 1
  #define TEMP_COMPENSATION_COEFF -0.003
  
  #if VOLTAGE_SYSTEM == 12
    #define DEFAULT_V_ABS 14.40
    #define DEFAULT_V_FLOAT 13.60
    #define DEFAULT_V_MIN 11.80
    #define DEFAULT_I_CHARGE 10.0
    #define BATTERY_NUM_CELLS 6
  #elif VOLTAGE_SYSTEM == 24
    #define DEFAULT_V_ABS 28.80
    #define DEFAULT_V_FLOAT 27.20
    #define DEFAULT_V_MIN 23.60
    #define DEFAULT_I_CHARGE 20.0
    #define BATTERY_NUM_CELLS 12
  #elif VOLTAGE_SYSTEM == 48
    #define DEFAULT_V_ABS 57.60
    #define DEFAULT_V_FLOAT 54.40
    #define DEFAULT_V_MIN 47.20
    #define DEFAULT_I_CHARGE 30.0
    #define BATTERY_NUM_CELLS 24
  #endif
  #define DEFAULT_ABS_TIME 7200000
  #define DEFAULT_ABS_CURRENT_END 1.00

#elif BATTERY_TYPE == 1  // ===== LiFePO4 (CC-CV BEZ TROJFÁZÍ!) =====

  #define BATTERY_NAME "LiFePO4"
  #define USE_THREE_PHASE_CHARGING 0
  #define TEMP_COMPENSATION_COEFF +0.003
  
  #if VOLTAGE_SYSTEM == 12
    #define DEFAULT_V_ABS 12.80
    #define DEFAULT_V_FLOAT 12.50
    #define DEFAULT_V_MIN 10.00
    #define DEFAULT_I_CHARGE 15.0
    #define BATTERY_NUM_CELLS 4
  #elif VOLTAGE_SYSTEM == 24
    #define DEFAULT_V_ABS 25.60
    #define DEFAULT_V_FLOAT 25.00
    #define DEFAULT_V_MIN 20.00
    #define DEFAULT_I_CHARGE 25.0
    #define BATTERY_NUM_CELLS 8
  #elif VOLTAGE_SYSTEM == 48
    #define DEFAULT_V_ABS 51.20
    #define DEFAULT_V_FLOAT 50.00
    #define DEFAULT_V_MIN 40.00
    #define DEFAULT_I_CHARGE 30.0
    #define BATTERY_NUM_CELLS 16
  #endif
  #define DEFAULT_ABS_TIME 3600000
  #define DEFAULT_ABS_CURRENT_END 0.50

#elif BATTERY_TYPE == 2  // ===== NiMH (CC-CV) =====

  #define BATTERY_NAME "NiMH"
  #define USE_THREE_PHASE_CHARGING 0
  #define TEMP_COMPENSATION_COEFF -0.002
  
  #if VOLTAGE_SYSTEM == 12
    #define DEFAULT_V_ABS 14.40
    #define DEFAULT_V_FLOAT 13.80
    #define DEFAULT_V_MIN 11.00
    #define DEFAULT_I_CHARGE 10.0
    #define BATTERY_NUM_CELLS 10
  #elif VOLTAGE_SYSTEM == 24
    #define DEFAULT_V_ABS 28.80
    #define DEFAULT_V_FLOAT 27.60
    #define DEFAULT_V_MIN 22.00
    #define DEFAULT_I_CHARGE 20.0
    #define BATTERY_NUM_CELLS 20
  #elif VOLTAGE_SYSTEM == 48
    #define DEFAULT_V_ABS 57.60
    #define DEFAULT_V_FLOAT 55.20
    #define DEFAULT_V_MIN 44.00
    #define DEFAULT_I_CHARGE 30.0
    #define BATTERY_NUM_CELLS 40
  #endif
  #define DEFAULT_ABS_TIME 5400000
  #define DEFAULT_ABS_CURRENT_END 0.30

#elif BATTERY_TYPE == 3  // ===== CUSTOM (VLASTNÉ) =====

  #define BATTERY_NAME "Custom"
  #define USE_THREE_PHASE_CHARGING 1
  #define TEMP_COMPENSATION_COEFF -0.003
  
  // ZMEŇ TIETO HODNOTY NA SVOJE:
  #define DEFAULT_V_ABS 28.80
  #define DEFAULT_V_FLOAT 27.20
  #define DEFAULT_V_MIN 23.60
  #define DEFAULT_I_CHARGE 20.0
  #define BATTERY_NUM_CELLS 12
  #define DEFAULT_ABS_TIME 7200000
  #define DEFAULT_ABS_CURRENT_END 1.00

#endif

//====================================== USER PARAMETERS ===========================================//
bool                                  
MPPT_Mode               = 1,
output_Mode             = 1,
disableFlashAutoLoad    = 0,
enablePPWM              = 1,
enableWiFi              = 1,
enableFan               = 1,
enableBluetooth         = 1,
enableLCD               = 1,
enableLCDBacklight      = 1,
overrideFan             = 0,
enableDynamicCooling    = 0;

int
serialTelemMode         = 1,
pwmResolution           = 11,
pwmFrequency            = 39000,
temperatureFan          = 60,
temperatureMax          = 90,
telemCounterReset       = 0,
errorTimeLimit          = 1000,
errorCountLimit         = 5,
millisRoutineInterval   = 250,
millisSerialInterval    = 1,
millisLCDInterval       = 1000,
millisWiFiInterval      = 2000,
millisLCDBackLInterval  = 2000,
backlightSleepMode      = 0,
baudRate                = 500000;

float 
// ===== DYNAMICKY INICIALIZOVANÉ Z DEFAULT HODNÔT =====
voltageBatteryMax       = DEFAULT_V_ABS,
voltageBatteryMin       = DEFAULT_V_MIN,
voltageFloat            = DEFAULT_V_FLOAT,
currentCharging         = DEFAULT_I_CHARGE,
electricalPrice         = 9.5000,
absorptionTimeLimit     = DEFAULT_ABS_TIME,
absorptionCurrentEnd    = DEFAULT_ABS_CURRENT_END;

//================================== CALIBRATION PARAMETERS =======================================//
bool
ADS1015_Mode            = 1;
int
ADC_GainSelect          = 2,
avgCountVS              = 3,
avgCountCS              = 4,
avgCountTS              = 500;
float
inVoltageDivRatio       = 40.2156,
outVoltageDivRatio      = 24.5000,
vOutSystemMax           = 60.0000,
cOutSystemMax           = 50.0000,
ntcResistance           = 9000.00,
voltageDropout          = 1.0000,
voltageBatteryThresh    = 1.5000,
currentInAbsolute       = 31.0000,
currentOutAbsolute      = 50.0000,
PPWM_margin             = 99.5000,
PWM_MaxDC               = 97.0000,
efficiencyRate          = 1.0000,
currentMidPoint         = 2.5250,
currentSens             = 0.0000,
currentSensV            = 0.0660,
vInSystemMin            = 10.000;

//===================================== SYSTEM PARAMETERS =========================================//
bool
buckEnable            = 0,
fanStatus             = 0,
bypassEnable          = 0,
chargingPause         = 0,
lowPowerMode          = 0,
buttonRightStatus     = 0,
buttonLeftStatus      = 0,
buttonBackStatus      = 0,
buttonSelectStatus    = 0,
buttonRightCommand    = 0,
buttonLeftCommand     = 0,
buttonBackCommand     = 0,
buttonSelectCommand   = 0,
settingMode           = 0,
setMenuPage           = 0,
boolTemp              = 0,
flashMemLoad          = 0,
confirmationMenu      = 0,
WIFI                  = 0,
BNC                   = 0,
REC                   = 0,
FLV                   = 0,
IUV                   = 0,
IOV                   = 0,
IOC                   = 0,
OUV                   = 0,
OOV                   = 0,
OOC                   = 0,
OTE                   = 0;

int
inputSource           = 0,
avgStoreTS            = 0,
temperature           = 0,
sampleStoreTS         = 0,
pwmMax                = 0,
pwmMaxLimited         = 0,
PWM                   = 0,
PPWM                  = 0,
pwmChannel            = 0,
batteryPercent        = 0,
errorCount            = 0,
menuPage              = 0,
subMenuPage           = 0,
ERR                   = 0,
conv1                 = 0,
conv2                 = 0,
intTemp               = 0,
chargingPhase         = 0;

float
VSI                   = 0.0000,
VSO                   = 0.0000,
CSI                   = 0.0000,
CSI_converted         = 0.0000,
TS                    = 0.0000,
powerInput            = 0.0000,
powerInputPrev        = 0.0000,
powerOutput           = 0.0000,
energySavings         = 0.0000,
voltageInput          = 0.0000,
voltageInputPrev      = 0.0000,
voltageOutput         = 0.0000,
currentInput          = 0.0000,
currentOutput         = 0.0000,
TSlog                 = 0.0000,
ADC_BitReso           = 0.0000,
daysRunning           = 0.0000,
Wh                    = 0.0000,
kWh                   = 0.0000,
MWh                   = 0.0000,
loopTime              = 0.0000,
outputDeviation       = 0.0000,
buckEfficiency        = 0.0000,
floatTemp             = 0.0000,
vOutSystemMin         = 0.0000,
voltageBatteryMaxComp = 0.0000,
voltageFloatComp      = 0.0000;

unsigned long 
currentErrorMillis    = 0,
currentButtonMillis   = 0,
currentSerialMillis   = 0,
currentRoutineMillis  = 0,
currentLCDMillis      = 0,
currentLCDBackLMillis = 0,
currentWiFiMillis     = 0,
currentMenuSetMillis  = 0,
prevButtonMillis      = 0,
prevSerialMillis      = 0,
prevRoutineMillis     = 0,
prevErrorMillis       = 0,
prevWiFiMillis        = 0,
prevLCDMillis         = 0,
prevLCDBackLMillis    = 0,
timeOn                = 0,
loopTimeStart         = 0,
loopTimeEnd           = 0,
secondsElapsed        = 0,
absorptionStartMillis = 0;

//====================================== MAIN PROGRAM =============================================//

void coreTwo(void * pvParameters){
  setupWiFi();
  while(1){
    Wireless_Telemetry();
  }
}

void setup() { 
  Serial.begin(baudRate);
  Serial.print("> MPPT Initialized for: ");
  Serial.print(BATTERY_NAME);
  Serial.print(" (");
  Serial.print(VOLTAGE_SYSTEM);
  Serial.println("V)");
  Serial.println("> Serial Initialized");
  
  pinMode(backflow_MOSFET, OUTPUT);                          
  pinMode(buck_EN, OUTPUT);
  pinMode(LED, OUTPUT); 
  pinMode(FAN, OUTPUT);
  pinMode(TempSensor, INPUT); 
  pinMode(ADC_ALERT, INPUT);
  pinMode(buttonLeft, INPUT); 
  pinMode(buttonRight, INPUT); 
  pinMode(buttonBack, INPUT); 
  pinMode(buttonSelect, INPUT); 
  
  ledcSetup(pwmChannel, pwmFrequency, pwmResolution);
  ledcAttachPin(buck_IN, pwmChannel);
  ledcWrite(pwmChannel, PWM);
  pwmMax = pow(2, pwmResolution) - 1;
  pwmMaxLimited = (PWM_MaxDC * pwmMax) / 100.000;
  
  ADC_SetGain();
  ads.begin();
  
  buck_Disable();

  xTaskCreatePinnedToCore(coreTwo, "coreTwo", 10000, NULL, 0, &Core2, 0);
  
  EEPROM.begin(512);
  Serial.println("> FLASH MEMORY: STORAGE INITIALIZED");
  initializeFlashAutoload();
  Serial.println("> FLASH MEMORY: SAVED DATA LOADED");

  if(enableLCD == 1){
    lcd.begin();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("MPPT V1.12");
    lcd.setCursor(0, 1);
    lcd.print(BATTERY_NAME);
    delay(1500);
    lcd.clear();
  }

  Serial.println("> MPPT HAS INITIALIZED");
}

void loop() {
  Read_Sensors();
  Device_Protection();
  System_Processes();
  Charging_Algorithm();
  Onboard_Telemetry();
  LCD_Menu();
}
