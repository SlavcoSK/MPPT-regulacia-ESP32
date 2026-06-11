/*  PROJECT FUGU FIRMWARE V1.12  (DIY 1kW Open Source MPPT Solar Charge Controller)
 *  By: TechBuilder (Angelo Casimiro)
 *  Upravené pre nabíjanie olovených A LiFePO4 batérií zo solárnych panelov
 *  OPRAVY: Slavko / Claude
 *  -----------------------------------------------------------------------------------------------------------
 *  ČÍSLOVANIE BATÉRIÍ:
 *  0 = Lead-Acid (trojfázové: BULK → ABSORPTION → FLOAT)
 *  1 = LiFePO4 (CC-CV: Constant Current → Constant Voltage)
 *  2 = NiMH (CC-CV s nižšími napätiami)
 *  3 = Custom (vlastné nastavenia)
 *  -----------------------------------------------------------------------------------------------------------
 */

//================================ MPPT FIRMWARE LCD MENU INFO =====================================//
String 
firmwareInfo      = "V1.12   ",
firmwareDate      = "12/06/26",
firmwareContactR1 = "Multi-Type MPPT ",  
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

//====================================== VÝBER BATÉRIE =============================================//
// ZMEŇ TOTO ČÍSLO NA VÝBER TYPU BATÉRIE:
// 0 = Lead-Acid (trojfázové nabíjanie)
// 1 = LiFePO4 (CC-CV s teplotnou kompenzáciou +3mV/°C)
// 2 = NiMH (CC-CV, nižšie napätia)
// 3 = Custom (vlastné nastavenia)
#define BATTERY_TYPE 0  // <<< ZMEŇ TOTO PRE ZMENU TYPU BATÉRIE

//====================================== KONFIGURÁCIA PODĽA TYPU BATÉRIE ============================//

#if BATTERY_TYPE == 0  // ===== LEAD-ACID (TROJFÁZOVÉ) =====
  #define BATTERY_NAME "Lead-Acid"
  #define USE_THREE_PHASE_CHARGING 1
  #define USE_TEMP_COMPENSATION 1
  #define TEMP_COMPENSATION_COEFF -0.003  // -3mV/°C/článok
  
  // PRE 24V SYSTÉM (12 články)
  #define BATTERY_NUM_CELLS 12
  #define VOLTAGE_ABSORPTION 28.80
  #define VOLTAGE_FLOAT 27.20
  #define VOLTAGE_MIN 23.60
  #define ABSORPTION_TIME_LIMIT 7200000  // 2 hodiny
  #define ABSORPTION_CURRENT_END 1.00    // 1A
  
  // PRE 12V SYSTÉM — ODKOMENTUJ A ZMEŇ NA:
  // #define BATTERY_NUM_CELLS 6
  // #define VOLTAGE_ABSORPTION 14.40
  // #define VOLTAGE_FLOAT 13.60
  // #define VOLTAGE_MIN 11.80

#elif BATTERY_TYPE == 1  // ===== LiFePO4 (CC-CV) =====
  #define BATTERY_NAME "LiFePO4"
  #define USE_THREE_PHASE_CHARGING 0
  #define USE_TEMP_COMPENSATION 1
  #define TEMP_COMPENSATION_COEFF +0.003  // +3mV/°C/článok (opakom ako Lead-Acid)
  
  // PRE 24V SYSTÉM (8 články × 3.2V)
  #define BATTERY_NUM_CELLS 8
  #define VOLTAGE_ABSORPTION 25.60  // 8 × 3.20V (max safe voltage)
  #define VOLTAGE_FLOAT 25.00       // Float 3.125V/článok
  #define VOLTAGE_MIN 20.00         // 2.5V/článok (vybitá)
  #define ABSORPTION_TIME_LIMIT 3600000  // 1 hodina (kratší čas)
  #define ABSORPTION_CURRENT_END 0.50    // 0.5A (nižší prúd)
  
  // PRE 12V SYSTÉM — ODKOMENTUJ A ZMEŇ NA:
  // #define BATTERY_NUM_CELLS 4
  // #define VOLTAGE_ABSORPTION 12.80
  // #define VOLTAGE_FLOAT 12.50
  // #define VOLTAGE_MIN 10.00

#elif BATTERY_TYPE == 2  // ===== NiMH (CC-CV) =====
  #define BATTERY_NAME "NiMH"
  #define USE_THREE_PHASE_CHARGING 0
  #define USE_TEMP_COMPENSATION 1
  #define TEMP_COMPENSATION_COEFF -0.002  // -2mV/°C/článok
  
  // PRE 24V SYSTÉM (20 články × 1.2V)
  #define BATTERY_NUM_CELLS 20
  #define VOLTAGE_ABSORPTION 28.80  // 20 × 1.44V (nabíjacie napätie)
  #define VOLTAGE_FLOAT 27.60       // Float 1.38V/článok
  #define VOLTAGE_MIN 22.00         // Min 1.1V/článok
  #define ABSORPTION_TIME_LIMIT 5400000  // 1.5 hodiny
  #define ABSORPTION_CURRENT_END 0.30    // 0.3A
  
  // PRE 12V SYSTÉM — ODKOMENTUJ A ZMEŇ NA:
  // #define BATTERY_NUM_CELLS 10
  // #define VOLTAGE_ABSORPTION 14.40
  // #define VOLTAGE_FLOAT 13.80
  // #define VOLTAGE_MIN 11.00

#elif BATTERY_TYPE == 3  // ===== CUSTOM (VLASTNÉ NASTAVENIA) =====
  #define BATTERY_NAME "Custom"
  #define USE_THREE_PHASE_CHARGING 1  // 1 = trojfázové, 0 = CC-CV
  #define USE_TEMP_COMPENSATION 1     // 1 = zapnúť, 0 = vypnúť
  #define TEMP_COMPENSATION_COEFF -0.003  // V/°C/článok (tvoja hodnota)
  
  // ZMEŇ TIETO HODNOTY NA SVOJE:
  #define BATTERY_NUM_CELLS 12
  #define VOLTAGE_ABSORPTION 28.80   // Absorption napätie (V)
  #define VOLTAGE_FLOAT 27.20        // Float napätie (V)
  #define VOLTAGE_MIN 23.60          // Minimálne napätie (V)
  #define ABSORPTION_TIME_LIMIT 7200000  // Čas absorpcie (ms)
  #define ABSORPTION_CURRENT_END 1.00    // Prúd ukončenia absorpcie (A)

#else
  #error "BATTERY_TYPE musí byť 0, 1, 2 alebo 3"
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
// ===== DYNAMICKY PRIRADENÉ Z KONFIGURÁCIE =====
voltageBatteryMax       = VOLTAGE_ABSORPTION,
voltageBatteryMin       = VOLTAGE_MIN,
voltageFloat            = VOLTAGE_FLOAT,
currentCharging         = 30.0000,   // Nastaviteľný v LCD Menu
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
chargingPhase         = 0;  // 0=BULK, 1=ABSORPTION, 2=FLOAT

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
absorptionStartMillis = 0,
absorptionTimeLimit   = ABSORPTION_TIME_LIMIT;

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
  Serial.print("> MPPT Initialized for Battery Type: ");
  Serial.println(BATTERY_NAME);
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

//================== CORE1: LOOP (DUAL CORE MODE) ======================//
void loop() {
  Read_Sensors();
  Device_Protection();
  System_Processes();
  Charging_Algorithm();
  Onboard_Telemetry();
  LCD_Menu();
}
