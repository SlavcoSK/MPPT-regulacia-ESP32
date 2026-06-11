# 🌞 MPPT Solar Charge Controller V1.12
## Univerzálny firmware pre nabíjanie viacerých typov batérií zo solárnych panelov

[![ESP32](https://img.shields.io/badge/ESP32-WROOM32-blue)](https://www.espressif.com/en/products/socs/esp32)
[![License](https://img.shields.io/badge/license-Open%20Source-green)](LICENSE)
[![Firmware](https://img.shields.io/badge/firmware-V1.12-orange)](CHANGELOG.md)
[![Status](https://img.shields.io/badge/status-Stable-brightgreen)](README.md)

---

## 📋 O Projekte

Toto je **DIY 1kW MPPT solárny regulátor** s open-source firmware pre nabíjanie **viacerých typov batérií** (Lead-Acid, LiFePO4, NiMH) v **rôznych napäťových systémoch** (12V, 24V, 48V) zo solárnych panelov. Firmware umožňuje **flexibilné nastavenie všetkých parametrov** priamo z LCD menu.

### Čo Je Nové v V1.12?
- ✨ **Podpora 4 typov batérií** × **3 napäťové systémy** = 12 konfigurácií
- 🎛️ **Nastaviteľné napätia a prúdy** z LCD menu (bez flashovania!)
- 📊 **Odporúčané hodnoty** pre všetky kombinácie
- 🔄 **Adaptívne nabíjanie**: Trojfázové (Lead-Acid) alebo CC-CV (LiFePO4, NiMH)
- 🌡️ **Teplotná kompenzácia** s opačnými znamienkami
- 🔧 **6 kritických bug-fixov** z pôvodného kódu

---

## 🔌 Technické Špecifikácie

### Hardware
| Komponenta | Model | Popis |
|-----------|-------|-------|
| **Mikroriadi č** | ESP32 WROOM32 | Dual-core, 240MHz, WiFi + BLE |
| **ADC** | ADS1015 / ADS1115 | 12-bit / 16-bit I2C ADC (auto-detekcia) |
| **Prúdový senzor** | ACS712-30A | 0-30A, galvanicky izolovaný |
| **MOSFET Driver** | IR2104 | Synchronný buck konvertor |
| **LCD Display** | 16x2 I2C | Menu s 6 obrazovkami + settings |
| **Teplota senzor** | NTC 9kΩ | Termoistorový senzor |

### Výkon a Limity
| Parameter | Hodnota |
|-----------|---------|
| **Vstupné napätie** | 10-60V (slnečné panely) |
| **Výstupné napätie** | 12-48V (batérie) |
| **Max. prúd vstup** | 30A (ACS712) |
| **Max. prúd výstup** | 30A (nastaviteľný) |
| **Max. výkon** | 1000W (teoreticky) |
| **Účinnosť** | 90-97% (synchronný buck) |

---

## 🔋 Podpora Batérií — Odporúčané Parametre

### ✅ LEAD-ACID (trojfázové nabíjanie)

#### Trojfázový Nabíjací Režim (iba Lead-Acid)
```
FÁZA 0: BULK (Constant Current)
├─ Nabíjací prúd = nastaviteľný (rekomendácia: max 0.1C × kapacita)
├─ MPPT algoritmus aktívny
└─ Trvá až do V_absorption

FÁZA 1: ABSORPTION (Constant Voltage)
├─ Konštantné napätie = V_absorption (teplotne kompenzované)
├─ Prúd prirodzene klesá
├─ Končí keď: I < absorptionCurrentEnd ALEBO čas > absorptionTimeLimit
└─ CV riadenie bez MPPT

FÁZA 2: FLOAT (Trickle Charging)
├─ Napätie = V_float (teplotne kompenzované)
├─ Minimálny udržiavací prúd
└─ Návrat do BULK ak U klesne príliš
```

#### 12V Lead-Acid Systém (6 článkov × 2V)
```
KONFIGURÁCIA:
#define BATTERY_TYPE 0
#define VOLTAGE_SYSTEM 12

ODPORÚČANÉ HODNOTY (v LCD MENU):
Absorption Voltage:        14.40 V  (2.40 V/článok)
Float Voltage:             13.60 V  (2.27 V/článok)
Min Battery Voltage:       11.80 V  (~1.97 V/článok)
Charging Current:          10.0  A  (C/10 rate pre 100Ah)
Absorption Time Limit:     2 hodiny
Absorption Current End:    1.0  A
Temp Compensation:         -3 mV/°C/článok
```

#### 24V Lead-Acid Systém (12 články × 2V)
```
KONFIGURÁCIA:
#define BATTERY_TYPE 0
#define VOLTAGE_SYSTEM 24

ODPORÚČANÉ HODNOTY (v LCD MENU):
Absorption Voltage:        28.80 V  (2.40 V/článok)
Float Voltage:             27.20 V  (2.27 V/článok)
Min Battery Voltage:       23.60 V  (~1.97 V/článok)
Charging Current:          20.0  A  (C/10 rate pre 200Ah)
Absorption Time Limit:     2 hodiny
Absorption Current End:    1.0  A
Temp Compensation:         -3 mV/°C/článok
```

#### 48V Lead-Acid Systém (24 články × 2V)
```
KONFIGURÁCIA:
#define BATTERY_TYPE 0
#define VOLTAGE_SYSTEM 48

ODPORÚČANÉ HODNOTY (v LCD MENU):
Absorption Voltage:        57.60 V  (2.40 V/článok)
Float Voltage:             54.40 V  (2.27 V/článok)
Min Battery Voltage:       47.20 V  (~1.97 V/článok)
Charging Current:          30.0  A  (C/10 rate pre 300Ah)
Absorption Time Limit:     2 hodiny
Absorption Current End:    1.0  A
Temp Compensation:         -3 mV/°C/článok
```

---

### ✅ LiFePO4 (CC-CV nabíjanie — BEZ trojfází!)

#### CC-CV Nabíjací Režim (INÝ ako Lead-Acid!)
```
FÁZA 0: CC (Constant Current)
├─ Nabíjací prúd = nastaviteľný (rekomendácia: 0.3C - 0.5C)
├─ MPPT algoritmus aktívny
└─ Trvá až do V_absorption

FÁZA 1: CV (Constant Voltage)
├─ Konštantné napätie = V_absorption
├─ Prúd klesá exponenciálne
└─ BEZ FLOAT FÁZY! (LiFePO4 to nevyžaduje)

⚠️ POZOR: LiFePO4 má OPAČNÚ teplotú kompenzáciu! (+3mV, nie -3mV)
```

#### 12V LiFePO4 Systém (4 články × 3.2V)
```
KONFIGURÁCIA:
#define BATTERY_TYPE 1
#define VOLTAGE_SYSTEM 12

ODPORÚČANÉ HODNOTY (v LCD MENU):
Absorption Voltage:        12.80 V  (3.20 V/článok - MAX!)
Float Voltage:             12.50 V  (3.125 V/článok - FLOAT, ale nie je kritický)
Min Battery Voltage:       10.00 V  (2.50 V/článok - vybitá)
Charging Current:          15.0  A  (0.5C pre 30Ah)
Absorption Time Limit:     1 hodina (kratšie ako Lead-Acid!)
Absorption Current End:    0.5  A   (nižšie ako Lead-Acid!)
Temp Compensation:         +3 mV/°C/článok (OPAK!)
```

#### 24V LiFePO4 Systém (8 články × 3.2V)
```
KONFIGURÁCIA:
#define BATTERY_TYPE 1
#define VOLTAGE_SYSTEM 24

ODPORÚČANÉ HODNOTY (v LCD MENU):
Absorption Voltage:        25.60 V  (3.20 V/článok - MAX!)
Float Voltage:             25.00 V  (3.125 V/článok)
Min Battery Voltage:       20.00 V  (2.50 V/článok - vybitá)
Charging Current:          25.0  A  (0.5C pre 50Ah)
Absorption Time Limit:     1 hodina
Absorption Current End:    0.5  A
Temp Compensation:         +3 mV/°C/článok (OPAK!)
```

#### 48V LiFePO4 Systém (16 články × 3.2V)
```
KONFIGURÁCIA:
#define BATTERY_TYPE 1
#define VOLTAGE_SYSTEM 48

ODPORÚČANÉ HODNOTY (v LCD MENU):
Absorption Voltage:        51.20 V  (3.20 V/článok - MAX!)
Float Voltage:             50.00 V  (3.125 V/článok)
Min Battery Voltage:       40.00 V  (2.50 V/článok - vybitá)
Charging Current:          30.0  A  (0.5C pre 60Ah)
Absorption Time Limit:     1 hodina
Absorption Current End:    0.5  A
Temp Compensation:         +3 mV/°C/článok (OPAK!)
```

---

### ✅ NiMH (CC-CV nabíjanie)

#### CC-CV Nabíjací Režim (podobný LiFePO4, ale iné napätia)
```
FÁZA 0: CC (Constant Current)
├─ Nabíjací prúd = nastaviteľný (0.1C - 0.2C)
├─ MPPT algoritmus aktívny
└─ Až do V_absorption

FÁZA 1: CV (Constant Voltage)
├─ Konštantné napätie = V_absorption
└─ BEZ FLOAT FÁZY
```

#### 12V NiMH Systém (10 články × 1.2V)
```
KONFIGURÁCIA:
#define BATTERY_TYPE 2
#define VOLTAGE_SYSTEM 12

ODPORÚČANÉ HODNOTY (v LCD MENU):
Absorption Voltage:        14.40 V  (1.44 V/článok - nabíjacie)
Float Voltage:             13.80 V  (1.38 V/článok)
Min Battery Voltage:       11.00 V  (1.10 V/článok - vybitá)
Charging Current:          10.0  A  (0.1C pre 100Ah)
Absorption Time Limit:     1.5 hodiny
Absorption Current End:    0.3  A   (nízky, aby sa nepoškodili)
Temp Compensation:         -2 mV/°C/článok (medzi Lead-Acid a LiFePO4)
```

#### 24V NiMH Systém (20 články × 1.2V)
```
KONFIGURÁCIA:
#define BATTERY_TYPE 2
#define VOLTAGE_SYSTEM 24

ODPORÚČANÉ HODNOTY (v LCD MENU):
Absorption Voltage:        28.80 V  (1.44 V/článok)
Float Voltage:             27.60 V  (1.38 V/článok)
Min Battery Voltage:       22.00 V  (1.10 V/článok)
Charging Current:          20.0  A  (0.1C pre 200Ah)
Absorption Time Limit:     1.5 hodiny
Absorption Current End:    0.3  A
Temp Compensation:         -2 mV/°C/článok
```

#### 48V NiMH Systém (40 články × 1.2V)
```
KONFIGURÁCIA:
#define BATTERY_TYPE 2
#define VOLTAGE_SYSTEM 48

ODPORÚČANÉ HODNOTY (v LCD MENU):
Absorption Voltage:        57.60 V  (1.44 V/článok)
Float Voltage:             55.20 V  (1.38 V/článok)
Min Battery Voltage:       44.00 V  (1.10 V/článok)
Charging Current:          30.0  A  (0.1C pre 300Ah)
Absorption Time Limit:     1.5 hodiny
Absorption Current End:    0.3  A
Temp Compensation:         -2 mV/°C/článok
```

---

### 📊 TABUĽKA VŠETKÝCH ODPORÚČANÝCH PARAMETROV

#### LEAD-ACID (Trojfázové)

| Systém | V_Abs | V_Float | V_Min | I_Charge | Abs_Time | I_End | Temp_Coeff |
|--------|-------|---------|-------|----------|----------|-------|------------|
| **12V** | 14.40 | 13.60 | 11.80 | 10.0A | 2h | 1.0A | -3mV |
| **24V** | 28.80 | 27.20 | 23.60 | 20.0A | 2h | 1.0A | -3mV |
| **48V** | 57.60 | 54.40 | 47.20 | 30.0A | 2h | 1.0A | -3mV |

#### LiFePO4 (CC-CV bez trojfází)

| Systém | V_Abs | V_Float | V_Min | I_Charge | Abs_Time | I_End | Temp_Coeff |
|--------|-------|---------|-------|----------|----------|-------|------------|
| **12V** | 12.80 | 12.50 | 10.00 | 15.0A | 1h | 0.5A | **+3mV** |
| **24V** | 25.60 | 25.00 | 20.00 | 25.0A | 1h | 0.5A | **+3mV** |
| **48V** | 51.20 | 50.00 | 40.00 | 30.0A | 1h | 0.5A | **+3mV** |

#### NiMH (CC-CV, medzi Lead-Acid a LiFePO4)

| Systém | V_Abs | V_Float | V_Min | I_Charge | Abs_Time | I_End | Temp_Coeff |
|--------|-------|---------|-------|----------|----------|-------|------------|
| **12V** | 14.40 | 13.80 | 11.00 | 10.0A | 1.5h | 0.3A | -2mV |
| **24V** | 28.80 | 27.60 | 22.00 | 20.0A | 1.5h | 0.3A | -2mV |
| **48V** | 57.60 | 55.20 | 44.00 | 30.0A | 1.5h | 0.3A | -2mV |

---

### 🌡️ Teplotná Kompenzácia Podľa Typu

**LEAD-ACID: -3 mV/°C/článok** (znižuje sa pri teplote)
```
Pri 25°C:  V_abs = 28.80V (24V)
Pri 35°C:  V_abs = 28.80 - (12 × 0.003 × 10) = 28.44V
Pri 45°C:  V_abs = 28.80 - (12 × 0.003 × 20) = 28.08V

Logika: Vyšší teplota → nižšie bezpečné nabíjacie napätie
```

**LiFePO4: +3 mV/°C/článok** (zvyšuje sa pri teplote! — OPAK!)
```
Pri 25°C:  V_abs = 25.60V (24V)
Pri 35°C:  V_abs = 25.60 + (8 × 0.003 × 10) = 25.84V
Pri 45°C:  V_abs = 25.60 + (8 × 0.003 × 20) = 26.08V

Logika: LiFePO4 potrebuje VYŠŠIE napätie pri vyšších teplotách!
```

**NiMH: -2 mV/°C/článok** (kompromis medzi Lead-Acid a LiFePO4)
```
Pri 25°C:  V_abs = 28.80V (24V)
Pri 35°C:  V_abs = 28.80 - (20 × 0.002 × 10) = 28.40V
Pri 45°C:  V_abs = 28.80 - (20 × 0.002 × 20) = 28.00V
```

---

## 📦 Inštalácia

### 1. Požiadavky
```bash
# Arduino IDE > 1.8.0
# ESP32 Board Definition > 1.0.6
# Potrebné knižnice:
  - Adafruit_ADS1X15 (ADS1015/ADS1115)
  - LiquidCrystal_I2C (16x2 LCD)
  - BlynkSimpleEsp32 (WiFi telemetria - voliteľné)
```

### 2. Inštalácia Knižníc
V Arduino IDE: **Sketch → Include Library → Manage Libraries**
```
Hľadaj a inštaluj:
1. Adafruit ADS1X15
2. LiquidCrystal I2C
3. Blynk (Legacy) - voliteľné
```

### 3. Stiahnutie Firmware
```bash
git clone https://github.com/SlavcoSK/MPPT-regulacia-ESP32.git
cd MPPT-regulacia-ESP32
```

### 4. Konfigurácia Firmware

#### Typ Batérie (POČAS FLASHOVANIA)
Otvor `ARDUINO_MPPT_FIRMWARE_V1.12.ino`:
```cpp
#define BATTERY_TYPE 0        // 0=Lead-Acid, 1=LiFePO4, 2=NiMH, 3=Custom
#define VOLTAGE_SYSTEM 24     // 12, 24, alebo 48 (V)
```

#### WiFi (voliteľné)
```cpp
char auth[] = "Tvoj_Blynk_Token";
char ssid[] = "Moja_WiFi_SSID";
char pass[] = "Heslo_WiFi";
```

### 5. Flashovanie
1. Vyber dosku: **Tools → Board → ESP32 Dev Module**
2. Vyber port: **Tools → Port → COM*X***
3. Flashuj: **Sketch → Upload**

---

## 🖥️ Obsluha — LCD MENU

### Hlavné Obrazovky (6)

```
OBRAZOVKA 0: Výkon & Energia
├─ Výkon: 500W
├─ Energia: 12.34 kWh
├─ Dní: 5
├─ SOC: 85%
├─ V_out: 28.7V
└─ I_out: 17.5A

OBRAZOVKA 1: Napätia & Prúdy
├─ V_input: 48.5V
├─ V_output: 28.7V
├─ I_input: 10.2A
└─ I_output: 17.5A

OBRAZOVKA 2: Energia & Stupeň Nabití
├─ Energia: 12.34 kWh
└─ [████████░░░░░░░░] 85%

OBRAZOVKA 3: Teplota
├─ Teplota: 42°C
└─ Chladič: ON

OBRAZOVKA 4: Stav Nabíjania
├─ BULK CHARGING (alebo ABSORPTION, FLOAT)
├─ Vc: 28.80V (teplotne kompenzované)
└─ Phase Timer: 01:23:45

OBRAZOVKA 5: SETTINGS
└─ Tlač SELECT pre vstup do menu
```

### Settings Menu — 16 Položiek

Všetky tieto sa dajú nastavovať z LCD bez flashovania!

| # | Položka | Typ | Rozsah | Jednotka |
|---|---------|-----|--------|----------|
| 1 | **Supply Algorithm** | MPPT vs CC-CV | Bool | - |
| 2 | **Charger/PSU Mode** | Nabíjač vs Zdroj | Bool | - |
| 3 | **Absorption Voltage** | Nabíjacie napätie | 10.00-60.00 | V |
| 4 | **Min Battery Voltage** | Spodný limit | 5.00-50.00 | V |
| 5 | **Float Voltage** | Udržiavacie napätie | 5.00-60.00 | V |
| 6 | **Charging Current** | Max prúd | 1.0-30.0 | A |
| 7 | **Cooling Fan** | Ventilátor ON/OFF | Bool | - |
| 8 | **Fan Trigger Temp** | Zapnutie ventilátora | 30-80 | °C |
| 9 | **Shutdown Temp** | Vypnutie systému | 50-120 | °C |
| 10 | **Absorption Time Limit** | Max čas absorpcie | 30-300 | min |
| 11 | **Absorption Current End** | Prúd ukončenia | 0.1-5.0 | A |
| 12 | **WiFi Feature** | WiFi pripojenie | Bool | - |
| 13 | **Autoload Feature** | Načítanie nastavení | Bool | - |
| 14 | **Backlight Sleep** | Režim spánku LCD | 0-9 | možností |
| 15 | **Factory Reset** | Reset do pôvodného | - | - |
| 16 | **Firmware Version** | Info o firmware | - | - |

### Navigácia
| Tlačítko | Funkcia |
|----------|---------|
| **←/→** | Listovanie obrazoviek / Zmena hodnôt |
| **SELECT** | Vstup do Settings / Potvrdzenie zmeny |
| **BACK** | Návrat na obrazovku 0 / Zrušenie |

---

## ⚙️ RÝCHLY ŠTART — Zmena Typu A Napätia Batérie

### MOŽNOSŤ 1: Zmena Cez LCD Menu (BEZ FLASHOVANIA!)

```
1. LCD Menu → Settings (Obrazovka 5, tlač SELECT)
2. Zmeniť tieto hodnoty:
   ├─ Absorption Voltage      (nastav podľa tabuľky)
   ├─ Float Voltage           (nastav podľa tabuľky)
   ├─ Min Battery Voltage     (nastav podľa tabuľky)
   ├─ Charging Current        (nastav podľa tabuľky)
   ├─ Absorption Time Limit   (nastav podľa tabuľky)
   └─ Absorption Current End  (nastav podľa tabuľky)
3. Settings → Save
4. HOTOVO! — Zmeny sú uložené v EEPROM
```

### MOŽNOSŤ 2: Zmena Cez Kód (POČAS FLASHOVANIA)

```cpp
// V ARDUINO_MPPT_FIRMWARE_V1.12.ino na začiatku:

#define BATTERY_TYPE 0        // 0=Lead-Acid, 1=LiFePO4, 2=NiMH, 3=Custom
#define VOLTAGE_SYSTEM 24     // 12 V pre 12V syst., 24 V pre 24V, 48 V pre 48V

// Potom flashuj normálne
```

---

## 📊 TABUĽKA NA RÝCHLY VÝBER

### "Ktoré Hodnoty Si Mám Zvoliť?"

**Mám 24V Lead-Acid batériu (100Ah):**
```
Absorption Voltage:      28.80 V
Float Voltage:           27.20 V
Min Battery Voltage:     23.60 V
Charging Current:        10-20 A (podľa kapacity)
Absorption Time Limit:   2 hodiny
Absorption Current End:  1.0 A
```

**Mám 24V LiFePO4 batériu (100Ah):**
```
Absorption Voltage:      25.60 V   (NIE 28.80!)
Float Voltage:           25.00 V   (NIE 27.20!)
Min Battery Voltage:     20.00 V   (NIE 23.60!)
Charging Current:        25-30 A   (0.3-0.5C)
Absorption Time Limit:   1 hodina  (nie 2!)
Absorption Current End:  0.5 A     (nie 1!)
```

**Mám 12V Lead-Acid batériu (100Ah):**
```
Absorption Voltage:      14.40 V
Float Voltage:           13.60 V
Min Battery Voltage:     11.80 V
Charging Current:        10 A
Absorption Time Limit:   2 hodiny
Absorption Current End:  1.0 A
```

**Mám 12V LiFePO4 batériu (100Ah):**
```
Absorption Voltage:      12.80 V   (NIE 14.40!)
Float Voltage:           12.50 V
Min Battery Voltage:     10.00 V   (NIE 11.80!)
Charging Current:        15-20 A
Absorption Time Limit:   1 hodina
Absorption Current End:  0.5 A
```

---

## 🧪 Testovanie

### Bench Test (ODPORÚČANÉ pred slnkom!)

```
PROCEDÚRA:

1. INICIALIZÁCIA:
   ✓ "MPPT Initialized for Battery Type: Lead-Acid/LiFePO4/NiMH"
   ✓ "Serial Initialized"
   ✓ "FLASH MEMORY LOADED"
   ✓ "MPPT HAS INITIALIZED"

2. LEAD-ACID (trojfázové):
   → BULK: V rastie, I ~ max, PWM rastie
   → ABSORPTION: V = 28.80V, I klesá
   → FLOAT: V = 27.20V, I malý

3. LiFePO4 (CC-CV):
   → CC: V rastie, I = max, PWM rastie
   → CV: V = 25.60V, I klesá exponenciálne
   → KONIEC (bez FLOAT!)

4. NiMH (CC-CV):
   → CC: V rastie, I = max
   → CV: V = 28.80V, I klesá
   → KONIEC

5. TEPLOTNÁ KOMPENZÁCIA:
   Lead-Acid: V_abs KLESÁ pri zvýšení teploty (-3mV)
   LiFePO4: V_abs RASTIE pri zvýšení teploty (+3mV)
```

---

## 🔧 Všetky Opravy V1.12

- ✅ Bug-fix: Timer v Device_Protection (currentErrorMillis)
- ✅ Bug-fix: LoopTime stopky (zamenené start/end)
- ✅ Bug-fix: EEPROM truncation (float → uint8_t)
- ✅ Bug-fix: Výstupný prúd (teraz s efficiencyRate)
- ✅ Bug-fix: Kalibrácia prúdového senzora
- ✅ Bug-fix: LCD battery bar (6.18 → 6.25)
- ✅ NOVÁ: Multi-type battery support (Lead-Acid, LiFePO4, NiMH, Custom)
- ✅ NOVÁ: Flexibilné nastavenie všetkých parametrov z LCD
- ✅ NOVÁ: Odporúčané hodnoty pre 12V/24V/48V systémy

---

## 📞 Podpora

| Kontakt | Info |
|---------|------|
| **GitHub Issues** | [Issues](https://github.com/SlavcoSK/MPPT-regulacia-ESP32/issues) |
| **Diskusie** | [Discussions](https://github.com/SlavcoSK/MPPT-regulacia-ESP32/discussions) |

---

## 🏆 Credits

```
ORIGINÁLNY FIRMWARE:
└─ TechBuilder (Angelo Casimiro)

VYLEPŠENIA:
├─ V1.11 (Bug-fixy): Slavko
└─ V1.12 (Multi-type, flexibilné parametre): Slavko + Claude AI
```

---

**Ďakujeme za používanie! ☀️⚡🔋**

