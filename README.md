# 🌞 MPPT Solar Charge Controller V1.12
## Univerzálny firmware pre nabíjanie viacerých typov batérií zo solárnych panelov

[![ESP32](https://img.shields.io/badge/ESP32-WROOM32-blue)](https://www.espressif.com/en/products/socs/esp32)
[![License](https://img.shields.io/badge/license-Open%20Source-green)](LICENSE)
[![Firmware](https://img.shields.io/badge/firmware-V1.12-orange)](CHANGELOG.md)
[![Status](https://img.shields.io/badge/status-Stable-brightgreen)](README.md)

---

## 📋 O Projekte

Toto je **DIY 1kW MPPT solárny regulátor** s open-source firmware pre nabíjanie **viacerých typov batérií** zo solárnych panelov. Firmware bol pôvodne vyvinutý ako **Project Fugu** spoločnosťou TechBuilder a bol ďalej vylepšený o podporu rôznych chemických typov batérií.

### Čo Je Nové v V1.12?
- ✨ **Podpora 4 typov batérií**: Lead-Acid, LiFePO4, NiMH, Custom
- 🔄 **Adaptívne nabíjanie**: Trojfázové (Lead-Acid) alebo CC-CV (LiFePO4, NiMH)
- 🌡️ **Teplotná kompenzácia** (s opačnými znamienkami podľa typu!)
- ⚙️ **Nastaviteľné nabíjacie prúdy** — v LCD Menu
- 🔧 **6 kritických bug-fixov** z pôvodného kódu
- 📊 **Vylepšená telemetria** (Serial, WiFi, LCD)

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
| **PWM frekvencia** | 1.2kHz - 312kHz |

---

## 🔋 Podpora Batérií

### ✅ Podporované Typy

#### 🟦 LEAD-ACID (trojfázové nabíjanie)
```
KONFIGURÁCIA V KÓDE:
#define BATTERY_TYPE 0
#define VOLTAGE_ABSORPTION 28.80V (24V) alebo 14.40V (12V)
#define VOLTAGE_FLOAT 27.20V (24V) alebo 13.60V (12V)
#define VOLTAGE_MIN 23.60V (24V) alebo 11.80V (12V)
#define TEMP_COMPENSATION_COEFF -0.003  (-3mV/°C/článok)
#define USE_THREE_PHASE_CHARGING 1
#define ABSORPTION_TIME_LIMIT 7200000  (2 hodiny)
#define ABSORPTION_CURRENT_END 1.00    (1 Ampér)
```

**Trojfázový Nabíjací Režim (iba pre olovenokyslé):**

```
FÁZA 0: BULK (Constant Current)
├─ Nabíjací prúd = currentCharging (nastaviteľný v LCD, default 30A)
├─ MPPT algoritmus hľadá bod max. výkonu slnečného panelu
├─ Bez limitu napätia
└─ Trvá až do dosiahnutia V_absorption = 28.80V

FÁZA 1: ABSORPTION (Constant Voltage)
├─ Konštantné napätie = 28.80V (teplotne kompenzované!)
├─ Prúd prirodzene klesá keď sa batéria plní
├─ Končí keď:
│  ├─ Prúd klesne pod 1A (nastaviteľný: absorptionCurrentEnd), ALEBO
│  └─ Čas presáhne 2 hodiny (nastaviteľný: absorptionTimeLimit)
└─ Bez MPPT — čisté CV (Constant Voltage) riadenie

FÁZA 2: FLOAT (Trickle Charging)
├─ Konštantné udržiavacie napätie = 27.20V (teplotne kompenzované!)
├─ Minimálny tenkový nabíjací prúd
├─ Udržiava batériu plne nabitú bez poškodenia
├─ Ideálne počas dlhého slnečného dňa
└─ Automatický návrat do BULK ak napätie klesne pod 26.70V
```

#### 🟩 LiFePO4 (CC-CV bez trojfází)
```
KONFIGURÁCIA V KÓDE:
#define BATTERY_TYPE 1
#define VOLTAGE_ABSORPTION 25.60V (24V, 8 články × 3.2V)
#define VOLTAGE_FLOAT 25.00V (3.125V/článok)
#define VOLTAGE_MIN 20.00V (2.5V/článok = prázdna)
#define TEMP_COMPENSATION_COEFF +0.003  (+3mV/°C/článok! OPAK!)
#define USE_THREE_PHASE_CHARGING 0  (NO THREE PHASES!)
#define ABSORPTION_TIME_LIMIT 3600000  (iba 1 hodina, nie 2!)
#define ABSORPTION_CURRENT_END 0.50    (0.5A, nie 1A!)
```

**CC-CV Nabíjací Režim (bezpečnejší pre LiFePO4):**

```
FÁZA 0: CC (Constant Current)
├─ Nabíjací prúd = currentCharging (limit príslušný pre LiFePO4)
├─ MPPT algoritmus aktívny
├─ Bez limitu napätia
└─ Trvá až do V_absorption

FÁZA 1: CV (Constant Voltage)
├─ Konštantné napätie = 25.60V (teplotne komp. s +3mV!)
├─ Prúd klesá exponenciálne (nie lineárne ako Lead-Acid!)
└─ Bez ďalšej fázy — priamo saturácia

❌ BEZ FLOAT FÁZY — LiFePO4 to nevyžaduje a je to nebezpečné!
```

**Prečo iné parametre?**
- LiFePO4 má nižšie napätia (3.2V vs 2.4V Lead-Acid)
- Teplotná kompenácia je OPAČNÁ: +3mV/°C (zvyšuje sa s teplotou!)
- Absorption čas je kratší (1h vs 2h)
- Absorption prúd je nižší (0.5A vs 1A)

#### 🟪 NiMH (CC-CV s vlastnými parametrami)
```
KONFIGURÁCIA V KÓDE:
#define BATTERY_TYPE 2
#define VOLTAGE_ABSORPTION 28.80V (20 články × 1.44V)
#define VOLTAGE_FLOAT 27.60V (1.38V/článok)
#define VOLTAGE_MIN 22.00V (1.1V/článok)
#define TEMP_COMPENSATION_COEFF -0.002  (-2mV/°C/článok)
#define USE_THREE_PHASE_CHARGING 0
#define ABSORPTION_TIME_LIMIT 5400000  (1.5 hodiny)
#define ABSORPTION_CURRENT_END 0.30    (0.3A — najnižší)
```

#### 🔧 CUSTOM (vlastné batérie)
```
KONFIGURÁCIA V KÓDE:
#define BATTERY_TYPE 3

Potom odkomentuj a nastav VŠETKY tieto hodnoty:
#define BATTERY_NUM_CELLS XX          (počet článkov)
#define VOLTAGE_ABSORPTION XX.XX      (napätie nabíjania)
#define VOLTAGE_FLOAT XX.XX           (float napätie)
#define VOLTAGE_MIN XX.XX             (minimálne napätie)
#define TEMP_COMPENSATION_COEFF ±0.00X (teplotný koeficient)
#define USE_THREE_PHASE_CHARGING 0/1  (trojfázové alebo nie)
#define ABSORPTION_TIME_LIMIT XXXXX00 (čas absorpcie v ms)
#define ABSORPTION_CURRENT_END X.XX   (prúd ukončenia)
```

### 🌡️ Teplotná Kompenzácia (DÔLEŽITÉ!)

**LEAD-ACID: -3 mV/°C/artikel (ZNIŽUJE SA pri vzraste teploty)**
```
Pri 25°C:  V_abs = 28.80V
Pri 35°C:  V_abs = 28.80V - (12 × 0.003 × 10) = 28.44V
Pri 45°C:  V_abs = 28.80V - (12 × 0.003 × 20) = 28.08V

Logika: Vyšší teplota → nižšie bezpečné nabíjacie napätie
```

**LiFePO4: +3 mV/°C/artikl (ZVYŠUJE SA pri vzraste teploty) — OPAK!**
```
Pri 25°C:  V_abs = 25.60V
Pri 35°C:  V_abs = 25.60V + (8 × 0.003 × 10) = 25.84V
Pri 45°C:  V_abs = 25.60V + (8 × 0.003 × 20) = 26.08V

Logika: LiFePO4 vyžaduje VYŠŠIE napätie pri vyšších teplotách!
```

**NiMH: -2 mV/°C/articles (MEDZI Lead-Acid a LiFePO4)**
```
-2mV je compromise — NiMH je medzi LiFePO4 a Lead-Acid
```

### 📊 Porovnanie Nabíjacích Režimov

| Vlastnosť | Lead-Acid | LiFePO4 | NiMH |
|-----------|-----------|---------|------|
| **Nabíjací Režim** | Trojfázový (Bulk→Abs→Float) | CC-CV (bez Float) | CC-CV (bez Float) |
| **V_absorption** | 2.40V/čl | 3.20V/čl | 1.44V/čl |
| **V_float** | 2.27V/čl | - (N/A) | - (N/A) |
| **Teplotný Koeff** | -3mV/°C/čl | +3mV/°C/čl | -2mV/°C/čl |
| **Abs. Čas** | 2 hodiny | 1 hodina | 1.5 hodiny |
| **Abs. Prúd End** | 1.0A | 0.5A | 0.3A |
| **MPPT v Abs** | ✅ Áno | ❌ Nie (CV riadenie) | ❌ Nie (CV riadenie) |
| **Float Fáza** | ✅ Áno | ❌ Nie (bezpečnostne) | ❌ Nie |
| **3-Phase** | ✅ Áno | ❌ Nie | ❌ Nie |

### ⚙️ Ako Zmeniť Typ Batérie

**KROCI:**

1. **Otvor hlavný súbor** `ARDUINO_MPPT_FIRMWARE_V1.12.ino`

2. **Nájdi tento riadok na začiatku** (okolo riadku 55):
```cpp
#define BATTERY_TYPE 0  // <<< ZMEŇ TOTO
// 0 = Lead-Acid
// 1 = LiFePO4
// 2 = NiMH
// 3 = Custom
```

3. **Zmeň číslo**:
   - Pre olovenokyslé: `#define BATTERY_TYPE 0`
   - Pre LiFePO4: `#define BATTERY_TYPE 1`
   - Pre NiMH: `#define BATTERY_TYPE 2`
   - Pre vlastné: `#define BATTERY_TYPE 3`

4. **Ak potrebuješ iné napätia (12V, 48V)**:
   - Nájdi sekciu: `#if BATTERY_TYPE == 0` (alebo ktorý typ)
   - Odkomentuj komentáre s tvojimi hodnotami
   - Príklad:
   ```cpp
   // PRE 12V SYSTÉM — ODKOMENTUJ A ZMEŇ NA:
   // #define BATTERY_NUM_CELLS 6
   // #define VOLTAGE_ABSORPTION 14.40
   // #define VOLTAGE_FLOAT 13.60
   // #define VOLTAGE_MIN 11.80
   ```

5. **Flashuj do ESP32** a hotovo!

**ALTERNATÍVA: V LCD MENU**

Ak nechceš meniť kód:
- LCD Menu → Settings → Absorption Voltage (zmeň na 25.60 pre LiFePO4)
- LCD Menu → Settings → Float Voltage (zmeň podľa potreby)
- LCD Menu → Settings → Charging Current (zmeň prúd)
- LCD Menu → Settings → Save (ulož zmeny)

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

#### Typ Batérie (DÔLEŽITÉ!)
Otvor `ARDUINO_MPPT_FIRMWARE_V1.12.ino` a:
```cpp
#define BATTERY_TYPE 0  // 0=Lead-Acid, 1=LiFePO4, 2=NiMH, 3=Custom
```

#### WiFi (voliteľné)
```cpp
char auth[] = "Tvoj_Blynk_Token";    // z email-u po registrácii
char ssid[] = "Moja_WiFi_SSID";
char pass[] = "Heslo_WiFi";
```

#### ADC Typ
```cpp
bool ADS1015_Mode = 1;  // 1 = ADS1015 (12-bit), 0 = ADS1115 (16-bit)
```

### 5. Flashovanie
1. Vyber dosku: **Tools → Board → ESP32 Dev Module**
2. Vyber port: **Tools → Port → COM*X***
3. Flashuj: **Sketch → Upload**

---

## 🖥️ Obsluha

### LCD Menu
```
OBRAZOVKA 0: Výkon & Energia
├─ Výkon: 500W | Energia: 12.34 kWh | Dní: 5
└─ SOC: 85% | V_out: 28.7V | I_out: 17.5A

OBRAZOVKA 1: Napätia & Prúdy
├─ V_input: 48.5V | V_output: 28.7V
└─ I_input: 10.2A | I_output: 17.5A

OBRAZOVKA 2: Energia & Battery Bar
├─ Energia: 12.34 kWh
└─ [████████░░░░░░░░] 85%

OBRAZOVKA 3: Teplota
├─ Teplota: 42°C
└─ Chladič: ON

OBRAZOVKA 4: Stav Nabíjania (NOVÉ)
├─ BULK CHARGING (alebo ABSORPTION, FLOAT)
└─ Vc: 28.80V (teplotne kompenzované)

OBRAZOVKA 5: SETTINGS
└─ Tlač SELECT
```

### Tlačítka
| Tlačítko | Funkcia |
|----------|---------|
| **←/→** | Listovanie / Zmena hodnôt |
| **SELECT** | Vstup do Settings / Potvrdzenie |
| **BACK** | Návrat / Zrušenie |

### Settings (14 položiek)
1. **Supply Algorithm** — MPPT vs CC-CV
2. **Charger/PSU Mode** — Nabíjač vs zdroj
3. **Absorption Voltage** — Nabíjacie napätie
4. **Min Battery Voltage** — Spodný limit
5. **Float Voltage** — Udržiavacie napätie
6. **Charging Current** — Max nabíjací prúd
7. **Cooling Fan** — Chladiaci ventilátor
8. **Fan Trigger Temp** — Zapnutie ventilátora
9. **Shutdown Temp** — Vypnutie systému
10. **WiFi Feature** — WiFi pripojenie
11. **Autoload Feature** — Načítanie nastavení
12. **Backlight Sleep** — Režim spánku LCD
13. **Factory Reset** — Reset do pôvodného
14. **Firmware Version** — Info o firmware

---

## 🧪 Testovanie

### Bench Test (ODPORÚČANÉ pred slnkom!)

```
VYBAVENIE:
- Bench PSU (min. 30V, 5A)
- Multimeter
- Sériový monitor v Arduino IDE (115200 baud)

PROCEDÚRA:

1. INICIALIZÁCIA:
   - Napoj USB do ESP32
   - Otvor Serial Monitor
   - Skontroluj správy:
     ✓ "MPPT Initialized for Battery Type: Lead-Acid"
     ✓ "Serial Initialized"
     ✓ "FLASH MEMORY LOADED"
     ✓ "MPPT HAS INITIALIZED"

2. BULK FÁZA (Lead-Acid):
   - PSU: 30V, limit 3A
   - Pozoruj: napätie stúpa, prúd ~3A
   - PWM rastie
   - PHASE = BULK

3. ABSORPTION FÁZA (Lead-Acid):
   - Keď U dosahne 28.80V
   - Prúd klesá (napr. 2A → 1A)
   - Keď I < 1A → prechod do FLOAT
   - PHASE = ABSORPTION

4. FLOAT FÁZA (Lead-Acid):
   - U = 27.20V
   - I < 0.5A
   - PHASE = FLOAT

5. PRE LiFePO4:
   - Nie sú 3 fázy — iba CC→CV
   - U_abs = 25.60V (nie 28.80V!)
   - Krátší čas absorpcie (1h)

6. TEPLOTNÁ KOMPENZÁCIA:
   - Sleduj VmaxC a VfltC
   - Lead-Acid: klesajú pri zvýšení teploty (-3mV)
   - LiFePO4: rastú pri zvýšení teploty (+3mV)

7. SÉRIOVÝ VÝSTUP:
   - Všetky prúdy/napätia reálne
   - LoopTime ~ 5-10ms
   - Žiadne ERROR flagi (ERR:0)
   - PHASE meníka sa správne
```

### Prvý Start So Slnečnými Panelmi

```
PREDPOKLADY:
- Bench test ÚSPEŠNÝ ✓
- Batéria min. 50% nabitá
- Slnečný deň
- Panel min. 100W

PROCEDÚRA:
1. Odpoj PSU
2. Pripoj slnečný panel
3. Sleduj Serial Monitor:
   - InputSource → 1 (PV zdroj)
   - V_input rastie
   - PWM sa mení (MPPT)

4. Keď V_output dosáhne V_abs → ABSORPTION fáza
5. Keď I < threshold → FLOAT fáza
6. WiFi/LCD: zobraz status

ÚSPECH! ☀️
```

---

## 🔧 Opravy V1.12

Všetky kritické bug-fixy z V1.11 sú zachované + **NOVÁ: podpora viacerých typov batérií!**

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

VYLEPŠENIA V1.11 (Bug-fixy):
└─ Slavko

VYLEPŠENIA V1.12 (Multi-type batérie):
└─ Slavko + Claude AI

TECHNICKÁ PODPORA:
└─ Claude AI
```

---

**Ďakujeme za používanie! ☀️⚡🔋**

