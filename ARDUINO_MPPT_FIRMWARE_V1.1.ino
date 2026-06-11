/*  PROJECT FUGU FIRMWARE V1.10  (DIY 1kW Open Source MPPT Solar Charge Controller)
 *  By: TechBuilder (Angelo Casimiro)
 *  Upravené pre nabíjanie olovených batérií zo solárnych panelov
 *  OPRAVY: Slavko / Claude
 *  -----------------------------------------------------------------------------------------------------------
 *  ZOZNAM OPRÁV:
 *  1. Pridaná teplotná kompenzácia napätia pre olovené batérie (-3mV/článok/°C)
 *  2. Pridaná trojfázová nabíjacia logika: Bulk → Absorption → Float
 *  3. Opravený pretečenie unsigned long pre backlight timer (1 mesiac)
 *  4. Opravený výpočet loopTime (zamenené start/end)
 *  5. Opravená chyba časovača v Device_Protection (currentErrorMillis sa neaktualizoval)
 *  6. Pridaná hysteréza MPPT algoritmu (eliminácia PWM oscilácie pri MPP)
 *  7. Opravená chyba ukladania float hodnôt do EEPROM
 *  8. Pridané hodnoty pre 12V a 24V olovenokyslé systémy
 *  -----------------------------------------------------------------------------------------------------------
 */

//================================ MPPT FIRMWARE LCD MENU INFO =====================================//
String 
firmwareInfo      = "V1.11   ",
firmwareDate      = "12/06/26",
firmwareContactR1 = "Lead-Acid MPPT  ",  
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
Adafruit_ADS1015 ads;               // Použi ADS1015 (12-bit)
//Adafruit_ADS1115 ads;             // Odkomentuj pre ADS1115 (16-bit)

//====================================== USER PARAMETERS ===========================================//
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

//========================================= WiFi SSID ==============================================//
char 
auth[] = "InputBlynkAuthenticationToken",
ssid[] = "InputWiFiSSID",
pass[] = "InputWiFiPassword";

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
enableDynamicCooling    = 0,
// NOVÉ: Teplotná kompenzácia napätia pre olovené batérie
enableTempCompensation  = 1;

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
baudRate                = 500000,
// NOVÉ: Počet článkov batérie pre teplotnú kompenzáciu
batteryNumCells         = 12;        // 12V systém = 6 článkov (6x2V), 24V = 12 článkov

// NOVÉ: Fázy nabíjania olovených batérií
// 0 = Bulk (CC), 1 = Absorption (CV), 2 = Float
int chargingPhase       = 0;

float 
// ===== HODNOTY PRE 12V OLOVENÝ SYSTÉM =====
// voltageBatteryMax    = 14.40,  // Absorption voltage (12V / 6-článkový)
// voltageBatteryMin    = 11.80,  // Minimum (vybitá)
// voltageFloat         = 13.60,  // Float voltage

// ===== HODNOTY PRE 24V OLOVENÝ SYSTÉM (aktuálne aktívne) =====
voltageBatteryMax       = 28.80,   // Absorption voltage (24V / 12-článkový, 2.40V/článok)
voltageBatteryMin       = 23.60,   // Minimum (vybitá)
voltageFloat            = 27.20,   // Float voltage (24V, 2.267V/článok)
// Absorpčná fáza: čas alebo prúdový limit ukončenia
absorptionCurrentEnd    = 1.00,    // Ukončenie absorption fázy keď prúd klesne pod X A
currentCharging         = 30.0000,
electricalPrice         = 9.5000;


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
vOutSystemMax           = 50.0000,
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
vInSystemMin            = 10.000,
// NOVÉ: Teplotná kompenzácia -3mV/článok/°C (štandard pre olovené batérie)
tempCompCoeff           = -0.003,   // V/článok/°C
tempCompRef             = 25.0;     // Referenčná teplota (°C)


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
intTemp               = 0;

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
// NOVÉ: Teplotne kompenzované napätie
voltageBatteryMaxComp = 0.0000,  // Teplotne kompenzovaná absorption voltage
voltageFloatComp      = 0.0000;  // Teplotne kompenzovaná float voltage

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
// NOVÉ: Časovač absorpčnej fázy
absorptionStartMillis = 0,
absorptionTimeLimit   = 7200000; // Max 2 hodiny absorption fázy (ms)

//====================================== MAIN PROGRAM =============================================//

//================= CORE0: SETUP (DUAL CORE MODE) =====================//
void coreTwo(void * pvParameters){
  setupWiFi();
  while(1){
    Wireless_Telemetry();
  }
}

//================== CORE1: SETUP (DUAL CORE MODE) ====================//
void setup() { 
  Serial.begin(baudRate);
  Serial.println("> Serial Initialized");
  
  pinMode(backflow_MOSFET, OUTPUT);                          
  pinMode(buck_EN, OUTPUT);
  pinMode(LED, OUTPUT); 
  pinMode(FAN, OUTPUT);
  // OPRAVA: 'TS' je float premenná, nie GPIO číslo — správny pin je TempSensor
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
    lcd.print("MPPT INITIALIZED");
    lcd.setCursor(0, 1);
    lcd.print("FIRMWARE ");
    lcd.print(firmwareInfo);    
    delay(1500);
    lcd.clear();
  }

  Serial.println("> MPPT HAS INITIALIZED");
}

//================== CORE1: LOOP (DUAL CORE MODE) ======================//
void loop() {
  Read_Sensors();
  Device_Protection();
  System_Processes();
  Charging_Algorithm();
  Onboard_Telemetry();
  LCD_Menu();
}
