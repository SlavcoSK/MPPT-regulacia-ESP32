# 🌞 MPPT Solar Charge Controller V1.11
## Optimalizovaný firmware pre nabíjanie olovených batérií zo solárnych panelov

[![ESP32](https://img.shields.io/badge/ESP32-WROOM32-blue)](https://www.espressif.com/en/products/socs/esp32)
[![License](https://img.shields.io/badge/license-Open%20Source-green)](LICENSE)
[![Firmware](https://img.shields.io/badge/firmware-V1.11-orange)](CHANGELOG.md)
[![Status](https://img.shields.io/badge/status-Stable-brightgreen)](README.md)

---

## 📋 O Projekte

Toto je **DIY 1kW MPPT solárny regulátor** s open-source firmware pre nabíjanie **olovených batérií** zo solárnych panelov. Firmware bol pôvodne vyvinutý ako **Project Fugu** spoločnosťou TechBuilder a bol ďalej vylepšený o špecifické funkcie pre olovené batérie.

### Verzia V1.11 — Čo Je Nové?
- ✨ **Trojfázové nabíjanie** (Bulk → Absorption → Float)
- 🌡️ **Teplotná kompenzácia** napätia (-3mV/°C/článok)
- 🎯 **MPPT hysteréza** — eliminácia oscilácie pri MPP
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
| **LCD Display** | 16x2 I2C | Menu s 5 obrazovkami + settings |
| **Teplota senzor** | NTC 9kΩ | Termoistorový senzor |

### Výkon a Limity
| Parameter | Hodnota |
|-----------|---------|
| **Vstupné napätie** | 10-60V (napáť. panely) |
| **Výstupné napätie** | 12-48V (batérie) |
| **Max. prúd vstup** | 30A (ACS712) |
| **Max. prúd výstup** | 30A (nastaviteľný) |
| **Max. výkon** | 1000W (teoreticky) |
| **Účinnosť** | 90-97% (synchronný buck) |
| **PWM frekvencia** | 1.2kHz - 312kHz |

---

## 🔋 Podpora Batérií

### ✅ Optimalizované pre:
- **Olovenokyslé batérie** (WET, AGM, GEL)
  - 12V systémy (6 článkov)
  - 24V systémy (12 články)
  - 48V systémy (24 články)
  
### 🔄 Trojfázový Nabíjací Režim
```
FÁZA 0: BULK (Constant Current)
├─ Maximálny nabíjací prúd (30A default)
├─ MPPT algoritmus hľadá optimálny bod panelu
└─ Trvá až do dosiahnutia absorpčného napätia

FÁZA 1: ABSORPTION (Constant Voltage)
├─ Konštantné napätie = 28.80V (24V systém)
├─ Prúd prirodzene klesá
├─ Končí keď I < 1A alebo po 2 hodinách
└─ Bez MPPT — CV riadenie

FÁZA 2: FLOAT (Trickle Charging)
├─ Konštantné udržiavacie napätie = 27.20V
├─ Zabraňuje prebitiu počas dlhého slnka
└─ Návrat do BULK ak U < 26.70V
```

### 🌡️ Teplotná Kompenzácia
Štandardný parameter pre olovenokyslé batérie:
```
V_compensation = -3 mV / °C / článok

Príklad (24V systém pri 35°C):
  V_absorption = 28.80V - (12 × 0.003 × 10°C) = 28.44V
  V_float = 27.20V - 0.36V = 26.84V
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
Otvor `ARDUINO_MPPT_FIRMWARE_V1.1.ino` a nastav:

#### WiFi (voliteľné)
```cpp
char auth[] = "Tvoj_Blynk_Token";    // z email-u po registrácii
char ssid[] = "Moja_WiFi_SSID";
char pass[] = "Heslo_WiFi";
```

#### Typ Batérie

**Pre 24V systém (aktívny default):**
```cpp
voltageBatteryMax       = 28.80;   // Absorption (2.40V/článok)
voltageFloat            = 27.20;   // Float (2.267V/článok)
voltageBatteryMin       = 23.60;   // Minimum
batteryNumCells         = 12;      // 12 článkov
```

**Pre 12V systém (odkomentuj v kóde):**
```cpp
voltageBatteryMax       = 14.40;   // 2.40V/článok
voltageFloat            = 13.60;   // 2.267V/článok
voltageBatteryMin       = 11.80;   // Minimum
batteryNumCells         = 6;       // 6 články
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
OBRAZOVKA 1: Výkon & Energia
├─ Výkon: 500W
├─ Energia: 12.34 kWh
├─ Dní: 5
└─ SOC: 85%

OBRAZOVKA 2: Napätia & Prúdy
├─ V_input: 48.5V
├─ V_output: 28.7V
├─ I_input: 10.2A
└─ I_output: 17.5A

OBRAZOVKA 3: Energia & Battery Bar
├─ Energia: 12.34 kWh
└─ [████████░░░░░░░░] 85%

OBRAZOVKA 4: Teplota
├─ Teplota: 42°C
└─ Chladič: ON

OBRAZOVKA 5: Stav Nabíjania (NOVÉ)
├─ BULK CHARGING
└─ Vc: 28.80V

OBRAZOVKA 6: SETTINGS
└─ Tlač SELECT
```

### Tlačítka
| Tlačítko | Funkcia |
|----------|---------|
| **←/→** | Listovanie cez obrazovky / zmena hodnôt |
| **SELECT** | Vstup do Settings / Potvrdzenie |
| **BACK** | Návrat na obrazovku 1 / Zrušenie |

### Settings Menu (12 položiek)
1. **Supply Algorithm** — MPPT vs CC-CV
2. **Charger/PSU Mode** — Režim nabíjača vs zdroja
3. **Absorption Voltage** — Max nabíjacie napätie (28.80V)
4. **Min Battery Voltage** — Spodný limit (23.60V)
5. **Float Voltage** (NOVÉ) — Udržiavacie napätie (27.20V)
6. **Charging Current** — Max nabíjací prúd (30A)
7. **Cooling Fan** — Chladiaci ventilátor (ON/OFF)
8. **Fan Trigger Temp** — Teplota zapnutia (60°C)
9. **Shutdown Temp** — Teplota vypnutia (90°C)
10. **WiFi Feature** — Pripojenie WiFi
11. **Autoload Feature** — Načítanie uložených nastavení
12. **Backlight Sleep** — Režim spánku LCD (8 možností)
13. **Factory Reset** — Reset do výrobných hodnôt
14. **Firmware Version** — Info o firmware

---

## 📡 Telemetria

### Serial Monitor (USB)
```
Nastav: 115200 baud, 8N1
serialTelemMode = 1 (All Data) / 2 (Essential) / 3 (Numbers Only)

Príklad výstupu (Mode 1):
 ERR:0 FLV:0 BNC:0 IUV:0 IOC:0 OOV:0 OOC:0 OTE:0 REC:0
 MPPTA:1 CM:1 BYP:1 EN:1 FAN:0 WiFi:1
 PHASE:BULK PI:500 PO:485 PWM:156 PPWM:140
 VI:48.5 VO:28.7 CI:10.2 CO:17.5 Wh:12340 Temp:42
 VmaxC:28.80 VfltC:27.20 SOC:85% T:28500 LoopT:5.230ms
```

### WiFi/Blynk App (Voliteľné)
```
Virtuálne piny na Blynk aplikácii:

V1  — Vstupný výkon (W)
V2  — Stav nabíjania (%)
V3  — Vstupné napätie (V)
V4  — Vstupný prúd (A)
V5  — Výstupné napätie (V)
V6  — Výstupný prúd (A)
V7  — Teplota (°C)
V8  — Energia (kWh)
V9  — Úspory ($)
V10-13 — LED indikátory
V14 — Min Battery Voltage
V15 — Max Battery Voltage (teplotne komp.)
V16 — Charging Current
V17 — Electrical Price
V18 — Float Voltage (NOVÉ)
V19 — Charging Phase (NOVÉ)
V20 — Float Phase LED (NOVÉ)
```

---

## 🔧 Opravy V1.11

### Kritické Bug-fixy (6x)

#### 1. **Timer Bug v Device_Protection** ⚠️
```cpp
// CHYBNE (pôvodný kód):
if(currentErrorMillis - prevErrorMillis >= errorTimeLimit) {
  // currentErrorMillis je vždy 0 — podmienka nikdy pravdivá!
}

// OPRAVENÉ:
currentErrorMillis = millis();  // Pridané na začiatku funkcie
if(currentErrorMillis - prevErrorMillis >= errorTimeLimit) {
  // Teraz funguje správne
}
```

#### 2. **LoopTime Stopky Obrátené** ⚠️
```cpp
// CHYBNE (pôvodný kód):
loopTimeStart = micros();
loopTime = (loopTimeStart - loopTimeEnd) / 1000.0;  // Vždy 0 alebo záporné!
loopTimeEnd = micros();

// OPRAVENÉ:
loopTimeEnd = micros();
loopTime = (loopTimeEnd - loopTimeStart) / 1000.000;
loopTimeStart = micros();
```

#### 3. **EEPROM Truncation Float Hodnôt** ⚠️
```cpp
// CHYBNE (pôvodný kód):
EEPROM.write(1, voltageBatteryMax);  // 28.80 → 28 (stratas 0.80!)

// OPRAVENÉ:
conv1 = (int)(voltageBatteryMax * 100);   // 28.80 → 2880
EEPROM.write(1, (uint8_t)(conv1 / 100));  // Celá: 28
EEPROM.write(2, (uint8_t)(conv1 % 100));  // Desatinná: 80
```

#### 4. **Výstupný Prúd Ignoruje Účinnosť** ⚠️
```cpp
// CHYBNE:
currentOutput = (voltageInput * currentInput) / voltageOutput;  // Ignoruje η

// OPRAVENÉ:
currentOutput = (voltageInput * currentInput * efficiencyRate) / voltageOutput;
```

#### 5. **Nesprávna Kalibrácia Prúdového Senzora** ⚠️
```cpp
// OPRAVENÉ: Kalibrácia len keď je voltageInput < 3.0V
if(buckEnable == 0 && FLV == 0 && OOV == 0 && voltageInput < 3.0){
  currentMidPoint = ((CSI/avgCountCS)*1.3300)-0.003;
}
```

#### 6. **LCD Battery Bar Presnosť** ⚠️
```cpp
// CHYBNE:
int batteryPercentBars = batteryPercent / 6.18;  // Nepresné

// OPRAVENÉ:
int batteryPercentBars = batteryPercent / 6.25;  // 100% / 16 blokov = 6.25
```

---

## 🧪 Testovanie

### Bench Test (Predtým ako slnko!)

```
VYBAVENIE:
- Bench PSU (min. 30V, 5A)
- Multimeter
- Sériový monitor v Arduino IDE

PROCEDÚRA:
1. Nastav PSU na 30V, prúdový limit 3A
2. Pripoj výstupy do MPPTu (bez batérie!)
3. Otvor Serial Monitor (115200 baud)
4. Skontroluj inicializáciu:
   - "Serial Initialized"
   - "FLASH MEMORY LOADED"
   - "MPPT HAS INITIALIZED"

5. BULK FÁZA TEST:
   - Sleduj ako napätie stúpa
   - Prúd by mal byť blízko 3A (limit PSU)
   - PWM by sa mal zvyšovať

6. ABSORPTION FÁZA TEST:
   - Keď U dosahne 28.80V, malý prúd (~1A)
   - V_maxC by malo byť 28.80V
   - Fáza by mala byť: ABSORPTION

7. FLOAT FÁZA TEST:
   - Nižší prúd (<0.5A)
   - V_output by malo klesnúť na 27.20V
   - Fáza by mala byť: FLOAT

8. TEPLOTNÁ KOMPENZÁCIA TEST:
   - Meň NTC teplotu (ohrievaním)
   - Sleduj VmaxC a VfltC ako klesajú
   - Kompenzácia: -3mV/°C/článok (12 × -0.003 = -0.036V za °C)

9. SÉRIOVÝ VÝSTUP:
   - Všetky prúdy/napätia sú reálne
   - LoopTime ~ 5-10ms
   - Žiadne ERROR flagi
```

---

## 🚀 Prvý Start So Slnečnými Panelmi

```
PREDPOKLADY:
- Bench test úspešný ✓
- Batéria je nabitá min. na 50%
- Panel ma min. 100W (pre 24V systém)
- Slnečný deň

PROCEDÚRA:
1. Odpoj PSU, pripoj solárny panel
2. Sleduj sériový monitor:
   - InputSource by malo zmeniť na 1 (PV source)
   - V_input by malo stúpať
   - PWM by sa malo začať meniť (MPPT)
   
3. Keď V_output dosahne 28.80V → ABSORPTION
   
4. Sleduj Screen #5:
   - Má zobrazovať aktuálnu fázu
   - Teplotne kompenzované napätia
   
5. Počas dňa:
   - Ak ostane slnko → FLOAT fáza
   - LCD by malo spať po časovači (configurable)
   
6. Voľný WiFi (Blynk):
   - Ak máš WiFi, sleduj app
   - Virtuálne piny fungujú?
   
ÚSPECH! 🎉
```

---

## ⚙️ Pokročilé Nastavenia

### Zmena MPPT Hysterézy
```cpp
// V 4_Charging_Algorithm.ino:
int mpptCoarseStep = 3;              // Veľký krok ďaleko od MPP
int mpptFineStep   = 1;              // Malý krok blízko MPP
float mpptPowerHysteresis = 0.5;     // Prahový filter (W)

// Vyššie hodnoty → rýchlejšie, ale viac oscilácie
// Nižšie hodnoty → pomalšie, ale stabilnejšie
```

### Zmena Teplotnej Kompenzácie
```cpp
// V ARDUINO_MPPT_FIRMWARE_V1.1.ino:
bool enableTempCompensation = 1;   // 0 = vypnúť pre LiFePO4
float tempCompCoeff = -0.003;      // V/°C/článok
float tempCompRef = 25.0;          // Referenčná teplota

// Pre vlastný typ batérie:
// - Skontroluj datasheert
// - Typicky: -3mV pre olovenokyslé, +3mV pre LiFePO4
```

### Zmena LCD Sleep Timer
```
LCD Menu → Settings → Backlight Sleep
Možnosti: Never / 10s / 5min / 1h / 6h / 12h / 1day / 3days / 1week / 1month
```

### Serial Telemetria Režimy
```cpp
// V hlavnom súbore:
int serialTelemMode = 1;  // 0=Off, 1=All, 2=Essential, 3=Numbers only
int millisSerialInterval = 1;  // Frekvencia (ms) — 1ms = 1000 liniek/sec
```

---

## 🐛 Troubleshooting

### Problém: LCD Nezobrazuje Nič
```
Riešenie:
1. Skontroluj I2C adresu (default 0x27)
   - Spusti I2C scanner sketch
   - Ak je inakšia, zmeň v kóde: LiquidCrystal_I2C lcd(0x27,16,2);
   
2. Skontroluj kábel SDA/SCL
   
3. Restart board
```

### Problém: Prúdový Senzor Nemerá Správne
```
Riešenie:
1. V LCD Menu: Settings → Charging Current
2. Pustí bez panelu s bench PSU (Vin=30V, Iout=0A)
3. Senzor sa autokalibruje keď:
   - buckEnable == 0
   - voltageInput < 3.0V
   - Žiadne chyby (ERR == 0)

4. Ak stále nemerá, skontroluj:
   - Polaritu ACS712 (OUT musí byť na ADC)
   - Kondenzátor na výstupe ACS712 (100nF + 1µF)
```

### Problém: MPPT Nesvišťí
```
Riešenie:
1. Skontroluj či je voltageInput > 15V
   (MPPT sa aktivuje len keď je dostatočné napätie)
   
2. Skontroluj či je MPPT_Mode = 1 v Settings
   
3. Ak napätie stúpa lineárne ale PWM sa nejde,
   problém je v PWM driveri (IR2104)
```

### Problém: Batéria sa Nepláni
```
Riešenie:
1. Skontroluj:
   - Sú tam chyby? (Serial monitor: ERR:0?)
   - Je backflow MOSFET otvorený? (Blynk: BYP:1?)
   - Je buck Enable? (Blynk: EN:1?)

2. Skontroluj Charging Phase:
   - Ak je BULK a voltageOutput < 28.80V → malo by plniť
   - Ak je ABS/FLOAT → zvyšok nabíjania
   
3. Ak prúd je nulový:
   - Skontroluj voltáž panelu vs batérie
   - Potrebná je rozdiel > 1.5V (voltageDropout)
```

### Problém: Zateplenie (fan sa zapína)
```
Riešenie:
1. Skontroluj NTC teplotu (obrazovka 4)
2. Zvýš fan trigger teplotu (Settings: Fan Trigger Temp)
3. Ulož nastavenia (Select v Settings)
4. Ak fan stále běží:
   - Skontroluj IR2104 driver — nemusí mať radiátor
   - Ulož PWM rezistor pre menší PWM
```

---

## 📚 Literatúra a Zdroje

### Originálny Projekt
- **Project FUGU**: [YouTube Tutorial](https://www.youtube.com/watch?v=ShXNJM6uHLM)
- **Autor**: TechBuilder (Angelo Casimiro)
- **GitHub**: [AngeloCasi/FUGU-ARDUINO-MPPT-FIRMWARE](https://github.com/AngeloCasi/FUGU-ARDUINO-MPPT-FIRMWARE)

### Štandardy Nabíjania Olovených Batérií
- IEC 60896-1: Stacionárne batérie — Bezpečnosť
- IEEE 1188: Odporúčania pre nabíjanie olovenokyslých batérií
- DIN 41249: Nabíjacie profily (3-Stage: Bulk-Absorption-Float)

### Komponenty
- [ADS1115 Datasheet](https://ti.com/lit/ds/symlink/ads1115.pdf)
- [ACS712 Datasheet](https://www.allegromicro.com/en/products/sense/current-sensor-ics/zero-to-fifty-amp-integrated-conductor-sensor-ics/acs712)
- [IR2104 Driver](https://www.infineon.com/cms/en/product/power/gate-driver-ics/ir2104/)

---

## 📝 Licencia

Tento projekt je **Open Source** a vychádza z pôvodného kódu TechBuilder-a.

**Licencia**: Creative Commons BY-SA 4.0  
**Autori**: TechBuilder (originál) + Slavko (V1.11 optimalizácie)

Môžeš:
- ✅ Používať pre osobné účely
- ✅ Modifikovať kód
- ✅ Zdieľať s ostatnými
- ⚠️ Uviesť pôvodného autora

---

## 🤝 Príspevky

Máš nápad na vylepšenie? Otvor **Issue** alebo **Pull Request**:

```bash
git checkout -b feature/nova-funkcionalita
git commit -m "Feat: Nová funkcionalita"
git push origin feature/nova-funkcionalita
```

### Oblasti na Vylepšenie
- [ ] Bluetooth telemetria (BLE)
- [ ] CAN bus podpora (externé BMS)
- [ ] Data logging na SD kartu
- [ ] OTA firmware update
- [ ] Support pre ďalšie typy batérií (LiFePO4, NiMH)
- [ ] Webový dashboard

---

## 📞 Kontakt a Podpora

| Kontakt | Info |
|---------|------|
| **GitHub Issues** | [SlavcoSK/MPPT-regulacia-ESP32/issues](https://github.com/SlavcoSK/MPPT-regulacia-ESP32/issues) |
| **Diskusie** | [GitHub Discussions](https://github.com/SlavcoSK/MPPT-regulacia-ESP32/discussions) |
| **Email** | Vía GitHub Issues |

---

## 🎓 Poznámky

### Pre Začiatočníkov
Ak si úplne nový v tejto oblasti:
1. Prečítaj si YouTube tutoriál TechBuilder-a
2. Pochop princípy MPPT a nabíjania batérií
3. Staň sa s palacom dobrovoľne — nie je to neoddelateľ!
4. Testuj na bench PSU, **nikdy** priamo so slnkom

### Pre Pokročilých
- Firmware je modulárny — ľahko sa rozširuje
- Všetky funkcie sú v komentároch — ľahko sa modifikujú
- MPPT algoritmus je základný P&O — môžeš implementovať iné (dP/dV, etc.)
- WiFi telemetria je cez Blynk — ľahko zameniť za MQTT/REST API

### Výkon a Energetika
- Loop je optimalizovaný na ~5-10ms (čím viac, tým lepšie)
- Viac ADC vzoriek = presnosť ale pomalšie
- WiFi komunikácia je asynchrónna — nBlockuje main loop
- LCD update každý 1 sekunda (nastaviteľné)

---

## 🏆 Credits

```
ORIGINÁLNY FIRMWARE:
└─ TechBuilder (Angelo Casimiro)
   ├─ YouTube: www.youtube.com/TechBuilder
   ├─ GitHub: www.github.com/AngeloCasi
   └─ Email: casithebuilder@gmail.com

OPTIMALIZÁCIA PRE OLOVENÉ BATÉRIE (V1.11):
├─ Trojfázové nabíjanie
├─ Teplotná kompenzácia
├─ MPPT hysteréza
├─ 6 kritických bug-fixov
└─ Autor: Slavko

TECHNICKÁ PODPORA:
└─ Claude AI (analýza, refaktorovanie, dokumentácia)
```

---

## 📊 Status Projektu

```
Verzionácia: V1.11 STABLE
Posledná Aktualizácia: 12. júna 2026
Testované Na: ESP32 WROOM32 + ADS1015 + ACS712
Kompatibilita: 100% s pôvodným hardwarom TechBuilder
Status: ✓ PRODUCTION READY
```

**Ďakujeme za používanie! ☀️⚡🔋**

---

*Ak sa ti firmware páči, daj mu ⭐ na GitHub!*

