# Troubleshooting Guide — M5Stack Unit C6L MeshCore

[English](#english) | [Polski](#polski)

---

<a id="english"></a>
## English

This guide provides in-depth diagnostic procedures, root cause analysis for unexpected restarts, antenna matching requirements, release management workflows, and disk footprint explanations for the **M5Stack Unit C6L** running MeshCore.

---

### 1. Diagnosing Unexpected Restarts & Startup Beeps

#### The Symptom
The repeater or companion radio restarts unexpectedly (every 5 minutes, every 10–20 hours, or right after detecting neighbors / sending flood packets). Upon restarting, the device emits a double beep (`2000 Hz` followed by `3000 Hz`).

#### The Audible Beep Source
In `MeshCore/variants/m5stack_unit_c6l/UnitC6LBoard.cpp` (`board.begin()`), the hardware initialization code contains:
```cpp
#ifdef PIN_BUZZER
  pinMode(PIN_BUZZER, OUTPUT);
  tone(PIN_BUZZER, 2000, 80);
  delay(100);
  tone(PIN_BUZZER, 3000, 120);
#endif
```
This is the **chip startup chime**. Hearing this sound during normal operation indicates that the ESP32-C6 has undergone a complete hardware or software reset.

---

### 2. Antenna Mismatch & RF Power Dynamics

#### Can an Incompatible Antenna Cause Reboots?
**Yes, absolutely.** If you connect an antenna that is not tuned to your frequency band (e.g., 433 MHz or 2.4 GHz instead of 868/915 MHz), lacks a proper ground plane, or presents an impedance far from 50 Ω, it can directly trigger device reboots.

#### The Physical & Electrical Mechanism
1. **High VSWR (Voltage Standing Wave Ratio)**:
   - The Semtech SX1262 transmitter is matched for a 50 Ω load at +22 dBm (160 mW RF output).
   - With an unmatched antenna, high VSWR prevents RF power from radiating into the air. Instead, the power is **reflected back into the Power Amplifier (PA)** and matching network.
2. **Current Surge on the 3.3V Rail**:
   - Driving into a high-VSWR load forces the PA transistors into severe saturation, causing instantaneous DC supply current to spike significantly beyond normal levels (often reaching 150–180+ mA).
   - In the M5Stack Unit C6L, the SX1262 is configured in **LDO regulator mode** (`-D SX126X_USE_REGULATOR_LDO=true`). In this mode, the radio draws current directly from the board's 3.3V linear regulator without the efficiency benefits of a switching DC-DC converter.
   - Combined with the ESP32-C6 CPU (60–80 mA) and active LNA amplifier (15 mA), the total 3.3V current spike exceeds 240 mA.
3. **Brownout Reset Trigger (`ESP_RST_BROWNOUT`)**:
   - The sudden current surge drops the 3.3V rail below the ESP32-C6 internal brownout threshold (~2.75V).
   - The hardware brownout detector immediately pulls the reset line, restarting the chip and playing the startup chime.
4. **RF Feedback / Electromagnetic Interference (EMI)**:
   - Near-field RF energy reflected from an unmatched antenna couples into high-impedance PCB traces, including the ESP32-C6 reset line (`CHIP_PU`), causing immediate resets.
5. **Why the Original Antenna Restores Stability**:
   - Switching back to the factory-matched 50 Ω antenna lowers VSWR, allows forward RF energy to radiate efficiently into space, stabilizes the PA current draw, and keeps the 3.3V rail above the brownout threshold.

#### Will `get pwrmgt.bootreason` Say "Antenna Issue"?
**No software command can directly state "antenna issue"** because microcontrollers do not have built-in directional couplers or VSWR detectors. Instead, the diagnostic command reports the **physical consequence** of the antenna mismatch:
- Most commonly: **`> Reset: Brownout (low voltage)`**
- Occasionally: **`> Reset: Panic / exception reset`** or **`External reset`** (from RF EMI coupling into logic pins).

---

### 3. Step-by-Step Diagnostic Procedures

#### Step A: Query the Reset Reason via CLI
Open the MeshCore app or connect via Serial monitor and run:
```text
get pwrmgt.bootreason
```
The device queries `esp_reset_reason()` and outputs the exact hardware reset cause:

| Reported Reason | Underlying Cause | Corrective Action |
|---|---|---|
| `Brownout (low voltage)` | Voltage dip below 2.75V during LoRa TX (+22 dBm) or SPI flash write | Verify antenna match; lower TX power to `20 dBm`; use quality 5V 1A+ power source |
| `Panic / exception reset` | Software crash, illegal memory access, or stack overflow | Check packet lengths; inspect Serial monitor stack trace |
| `Task watchdog reset` | Main loop blocked for > 5 seconds by long operation | Check filesystem or SPI bus lockups |
| `Power-on reset` | Normal startup after physical USB power connection | Expected only on fresh power-up |

#### Step B: Real-Time USB Serial Monitor
Connect the device via USB-C to a PC and run:
```powershell
.\scripts\monitor-serial.bat
```
*(Or use any serial terminal at **115200 baud**).*
- **On Brownout**: The ROM bootloader prints:
  ```text
  Brownout detector was triggered
  ```
- **On Panic**: The ESP-IDF prints:
  ```text
  Guru Meditation Error: Core 0 panic'ed ...
  ```

#### Step C: Packet Logging to Flash
- Start logging: `log start` (logs all RX/TX packets to `/packet_log` in SPIFFS).
- Dump log to screen: `log`
- Stop & erase: `log stop` followed by `log erase`

---

### 4. Mitigating Restarts & Optimizing RF Power

1. **Lower Radio TX Power to 20 dBm**:
   Run via CLI:
   ```text
   set radio.txpower 20
   ```
   *Rationale*: Reduces peak PA current by **~35%** (from ~140 mA down to ~95 mA) with only a 2 dB reduction in output power (negligible effect on real-world range, but eliminates voltage collapse).
2. **Quality Power Supply & Short Cables**:
   Use a short, shielded USB-C cable and a dedicated 5V 1A+ wall charger. Avoid unpowered USB hubs or long extension cables that introduce resistive voltage drops.
3. **Antenna Verification**:
   Always use a tuned 50 Ω LoRa antenna designed specifically for your operating frequency (868 MHz for EU, 915 MHz for US).

---

### 5. Why the Neighbor List is Lost on Reboot

The repeater maintains its neighbor table in volatile RAM:
```cpp
NeighbourInfo neighbours[MAX_NEIGHBOURS];  // in-memory array
```
MeshCore deliberately does not write neighbor discoveries to SPIFFS flash to prevent wearing out flash write cycles. Consequently, **any reset wipes the table**. Eliminating unexpected reboots permanently preserves the neighbor list during normal operation.

---

### 6. Release Tracking & Downloading Specific Compilations

To compare or roll back versions easily, this repository follows standard Git tagging and release archiving:

#### Tagging Standard
Release tags follow the format:
```text
v<MeshCore-ver>-<role>-YYYYMMDD
```
*Example*: `v1.18.0-repeater-20261025` or `v1.18.0-companion-20261025`

#### Rollback via Git
To switch your local workspace to any past compilation:
```bash
git checkout v1.18.0-repeater-20261025
```

#### GitHub Releases & Downloadable Bundles
Whenever a release is tagged on GitHub, standalone ZIP archives are attached to the release:
- `m5stack-unit-c6l-companion-v1.18.0-<date>.zip`
- `m5stack-unit-c6l-repeater-v1.18.0-<date>.zip`

Each bundle contains precompiled `.bin` files (`firmware.bin`, `bootloader.bin`, `partitions.bin`) and one-click flash scripts (`flash-*.bat`), enabling instant deployment of any past release without compiling or installing PlatformIO.

---

### 7. Repository Footprint & Archiving

#### Why is the Folder Over 300–500 MB?
A disk inspection reveals the exact space allocation:

| Directory | Size | Description | Included in Git / GitHub? |
|---|---|---|---|
| `MeshCore/.pio/libdeps/` | ~308 MB | Downloaded toolchain libraries (RadioLib, Adafruit, ed25519) | **No** (ignored by `.gitignore`) |
| `MeshCore/.pio/build/` | ~187 MB | Compiled object files (`.o`), static archives (`.a`), ELF binaries | **No** (ignored by `.gitignore`) |
| `.git/` | ~10 MB | Complete Git history and commits | Stored in Git |
| `firmware/` | ~2.1 MB | Precompiled release binaries ready for flashing | Stored in Git |
| Source code, scripts, docs | < 1 MB | Human-written configuration and code files | Stored in Git |

**Conclusion**: Over **95%** of the disk footprint is local PlatformIO build cache (`.pio`).

#### How to Share a Small ZIP Archive
1. **From GitHub**: Click **Code -> Download ZIP** on GitHub, or download from **Releases**. GitHub automatically excludes `.pio`, producing a lightweight archive (~3–5 MB).
2. **From Local Workspace**: When creating a ZIP manually, exclude the `MeshCore/.pio` directory to keep the archive under **15 MB** (or under **3 MB** if zipping only `firmware/` and `scripts/`).

---

<a id="polski"></a>
## Polski

Przewodnik diagnostyczny, analiza przyczyn restartów, wymagania dopasowania anteny, procedury wydań (releases) oraz wyjaśnienie rozmiaru repozytorium dla **M5Stack Unit C6L** z systemem MeshCore.

---

### 1. Diagnozowanie nieoczekiwanych restartów i dźwięku startowego

#### Objaw
Repeater lub companion restartuje się niespodziewanie (co ok. 5 minut, co 10–20 godzin lub tuż po wykryciu sąsiadów / wysłaniu pakietu flood). Po restarcie urządzenie emituje dwutonowy dźwięk (2000 Hz, a następnie 3000 Hz).

#### Źródło dźwięku („beep”)
W pliku `MeshCore/variants/m5stack_unit_c6l/UnitC6LBoard.cpp` (`board.begin()`) znajduje się kod startowy:
```cpp
#ifdef PIN_BUZZER
  pinMode(PIN_BUZZER, OUTPUT);
  tone(PIN_BUZZER, 2000, 80);
  delay(100);
  tone(PIN_BUZZER, 3000, 120);
#endif
```
Jest to **sygnał startowy procesora**. Usłyszenie tego dźwięku w trakcie normalnej pracy oznacza, że układ ESP32-C6 uległ pełnemu restartowi sprzętowemu lub programowemu.

---

### 2. Wpływ niedopasowanej anteny na stabilność zasilania

#### Czy inna antena może powodować częste restarty?
**Tak, bezwzględnie tak.** Jeśli podłączona zostanie antena niedopasowana do pasma pracy (np. 433 MHz lub 2.4 GHz zamiast 868/915 MHz), o złej impedancji (dalekiej od 50 Ω) lub bez odpowiedniej przeciwwagi (ground plane), może to bezpośrednio wywoływać restarty urządzenia.

#### Mechanizm fizyczno-elektryczny
1. **Wysoki współczynnik fali stojącej (WFS / VSWR)**:
   - Tor nadawczy układu Semtech SX1262 jest fabrycznie dopasowany do obciążenia 50 Ω przy mocy +22 dBm (160 mW mocy w.cz.).
   - Przy niedopasowanej antenie energia wysokiej częstotliwości nie może zostać wypromieniowana w przestrzeń. Zostaje **odbita z powrotem do końcówki mocy (PA)** i układu dopasowania.
2. **Gwałtowny skok prądu na linii 3.3V**:
   - Praca ze zwartym lub niedopasowanym obciążeniem wprowadza tranzystory PA w stan nasycenia, powodując gwałtowny wzrost poboru prądu stałego (nawet do 150–180+ mA).
   - W module M5Stack Unit C6L układ SX1262 pracuje w **trybie regulatora liniowego LDO** (`-D SX126X_USE_REGULATOR_LDO=true`), pobierając prąd bezpośrednio z wewnętrznej linii 3.3V bez przetwornicy DC-DC.
   - W połączeniu z procesorem ESP32-C6 (~60–80 mA) i włączonym wzmacniaczem antenowym LNA (15 mA), całkowity szczytowy pobór prądu przekracza 240 mA.
3. **Zadziałanie detektora Brownout (`ESP_RST_BROWNOUT`)**:
   - Skok poboru prądu wywołuje spadek napięcia na szynie 3.3V poniżej progu zadziałania wewnętrznego komparatora ESP32-C6 (~2.75V).
   - Układ natychmiast wymusza reset sprzętowy procesora i odtwarza sygnał startowy.
4. **Sprzężenie elektromagnetyczne (EMI)**:
   - Niedopasowana antena generuje silne pole bliskie, które indukuje zakłócenia bezpośrednio na ścieżkach płytki drukowanej, w tym na linii resetu procesora (`CHIP_PU`).
5. **Dlaczego powrót do oryginalnej anteny przywrócił stabilność**:
   - Fabryczna antena posiada prawidłową impedancję 50 Ω i niski VSWR. Energia w.cz. jest swobodnie emitowana w przestrzeń, prąd stopnia PA wraca do normy, a napięcie 3.3V nie spada poniżej progu brownout.

#### Czy komenda `get pwrmgt.bootreason` pokaże „problem z anteną”?
**Żaden mikrokontroler nie pokaże wprost komunikatu „antenna issue”**, ponieważ ESP32-C6 nie posiada wbudowanego reflektometru (mostka pomiaru fali odbitej). Oprogramowanie raportuje **fizyczny skutek** niedopasowania anteny:
- Najczęściej: **`> Reset: Brownout (low voltage)`** (spadek napięcia wywołany przeciążeniem końcówki mocy).
- Niekiedy: **`> Reset: Panic / exception reset`** lub **`External reset`** (zakłócenia w.cz. na liniach logicznych).

---

### 3. Procedury diagnostyczne krok po kroku

#### Krok A: Sprawdzenie przyczyny restartu przez CLI
W aplikacji mobilnej MeshCore lub terminalu szeregowym wpisz:
```text
get pwrmgt.bootreason
```
Urządzenie odczyta rejestr `esp_reset_reason()` i wyświetli sprzętową przyczynę:

| Zwrócony powód | Przyczyna | Zalecane działanie |
|---|---|---|
| `Brownout (low voltage)` | Spadek napięcia zasilania poniżej 2.75V podczas nadawania LoRa (+22 dBm) lub zapisu flash | Sprawdź antenę; zmniejsz moc do `20 dBm`; użyj dobrego zasilacza 5V 1A+ |
| `Panic / exception reset` | Błąd programu, nieprawidłowy adres w pamięci lub przepełnienie stosu | Sprawdź długości ramek; zbadaj zrzut rejestrów na porcie szeregowym |
| `Task watchdog reset` | Pętla główna zablokowana na > 5 sekund | Sprawdź operacje na systemie plików SPIFFS |
| `Power-on reset` | Fizyczne włączenie zasilania USB | Prawidłowe tylko przy świeżym podłączeniu |

#### Krok B: Monitorowanie na żywo przez USB Serial
Podłącz urządzenie kablem USB-C do komputera i uruchom:
```powershell
.\scripts\monitor-serial.bat
```
*(lub dowolny terminal szeregowy na prędkości **115200 baud**).*
- **Przy spadku napięcia (Brownout)**:
  ```text
  Brownout detector was triggered
  ```
- **Przy błędzie oprogramowania (Panic)**:
  ```text
  Guru Meditation Error: Core 0 panic'ed ...
  ```

#### Krok C: Logowanie pakietów do pamięci Flash
- Włączenie logowania: `log start` (zapis pakietów do `/packet_log` w SPIFFS).
- Odczyt logu na ekran: `log`
- Zatrzymanie i czyszczenie: `log stop`, a następnie `log erase`

---

### 4. Zapobieganie restartom i optymalizacja zasilania

1. **Obniżenie mocy nadawania do 20 dBm**:
   Wpisz w CLI:
   ```text
   set radio.txpower 20
   ```
   *Uzasadnienie*: Zmniejsza prąd szczytowy PA aż o **~35%** (ze 140 mA do ~95 mA) przy spadku mocy o zaledwie 2 dB (pomijalna różnica w zasięgu, ale całkowity brak ryzyka spadków napięcia).
2. **Dobrej jakości zasilanie i przewód USB**:
   Używaj krótkiego, grubego kabla USB-C oraz zasilacza sieciowego 5V min. 1A. Unikaj pasywnych hubów USB.
3. **Weryfikacja anteny**:
   Stosuj wyłącznie anteny 50 Ω dopasowane do częstotliwości pracy sieci (868 MHz w Europie, 915 MHz w USA).

---

### 5. Dlaczego lista sąsiadów znika po restarcie?

Repeater przechowuje listę sąsiadów wyłącznie w ulotnej pamięci RAM:
```cpp
NeighbourInfo neighbours[MAX_NEIGHBOURS];  // tablica w pamięci RAM
```
Twórcy MeshCore celowo nie zapisują wykrytych sąsiadów do pamięci flash SPIFFS, aby nie doprowadzić do szybkiego zużycia komórek pamięci flash ciągłymi zapisami. W związku z tym **każdy restart czyści tę tablicę**. Zapewnienie stabilności zasilania sprawia, że lista sąsiadów nie jest tracona.

---

### 6. Wersjonowanie wydań (Releases) i pobieranie paczek binarnych

Aby ułatwić porównywanie wersji i powrót do wcześniejszych kompilacji, repozytorium stosuje standardowe tagi Git oraz paczki wydań GitHub:

#### Standard nazewnictwa tagów
```text
v<Wersja-MeshCore>-<rola>-YYYYMMDD
```
*Przykład*: `v1.18.0-repeater-20261025` lub `v1.18.0-companion-20261025`

#### Powrót do poprzedniej wersji w Git
Aby przełączyć kod do dowolnej archiwalnej wersji:
```bash
git checkout v1.18.0-repeater-20261025
```

#### Wydania GitHub (Releases) i pliki ZIP
Dla każdego tagu w zakładce **Releases** na GitHub publikowane są samodzielne archiwa ZIP:
- `m5stack-unit-c6l-companion-v1.18.0-<data>.zip`
- `m5stack-unit-c6l-repeater-v1.18.0-<data>.zip`

Paczka zawiera gotowe pliki `.bin` (`firmware.bin`, `bootloader.bin`, `partitions.bin`) oraz skrypt wgrywający (`flash-*.bat`), co pozwala na wgranie dowolnej wersji jednym kliknięciem bez potrzeby posiadania środowiska PlatformIO.

---

### 7. Rozmiar repozytorium i tworzenie małych plików ZIP

#### Dlaczego folder ma ponad 300–500 MB?
Szczegółowa analiza zajętości dysku:

| Katalog | Rozmiar | Opis | Czy trafia do Git / GitHub? |
|---|---|---|---|
| `MeshCore/.pio/libdeps/` | ~308 MB | Pobrane biblioteki zewnętrzne (RadioLib, Adafruit, ed25519) | **Nie** (ignorowane przez `.gitignore`) |
| `MeshCore/.pio/build/` | ~187 MB | Skompilowane pliki obiektowe (`.o`), archiwa (`.a`), binaria ELF | **Nie** (ignorowane przez `.gitignore`) |
| `.git/` | ~10 MB | Pełna historia commitów Git | Zapisane w Git |
| `firmware/` | ~2.1 MB | Gotowe, skompilowane binaria do wgrania | Zapisane w Git |
| Pliki źródłowe, skrypty, docs | < 1 MB | Konfiguracje, skrypty i dokumentacja | Zapisane w Git |

**Wniosek**: Ponad **95%** rozmiaru na dysku to lokalna pamięć podręczna kompilatora PlatformIO (`.pio`).

#### Jak utworzyć mały plik ZIP do przesłania:
1. **Pobieranie z GitHub**: Użyj przycisku **Code -> Download ZIP** lub pobierz paczkę z **Releases**. GitHub automatycznie pomija folder `.pio`, dzięki czemu plik ZIP zajmuje jedynie **~3–5 MB**.
2. **Pakowanie lokalne**: Pakując folder ręcznie, pomiń katalog `MeshCore/.pio`. Spakowane repozytorium bez pamięci podręcznej zajmuje zaledwie **~12–15 MB** (lub poniżej **3 MB**, jeśli pakujesz tylko gotowe pliki z `firmware/` i `scripts/`).
