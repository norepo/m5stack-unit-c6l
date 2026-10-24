# M5Stack Unit C6L — MeshCore Firmware & Tools

[English](#english) | [Polski](#polski)

[![Hardware](https://img.shields.io/badge/Hardware-M5Stack%20Unit%20C6L-orange.svg)](https://m5stack.com)
[![MCU](https://img.shields.io/badge/MCU-ESP32--C6%20(RISC--V)-blue.svg)](https://espressif.com)
[![Radio](https://img.shields.io/badge/LoRa-Semtech%20SX1262%20(+22dBm)-green.svg)](https://semtech.com)
[![MeshCore](https://img.shields.io/badge/MeshCore-v1.18.0-purple.svg)](https://github.com/meshcore-dev/MeshCore)

---

<a id="english"></a>
## English

Project repository containing precompiled **MeshCore 1.18** firmware, flashing tools, hardware configurations, and deployment guides for the **M5Stack Unit C6L**.

### Directory Organization

```
m5stack-unit-c6l/
├── firmware/
│   ├── 01-companion-radio-ble/          # Bluetooth & Wi-Fi Companion Radio (Phone/Network App)
│   │   ├── firmware.bin                 # Main application image (~2,098 KB)
│   │   ├── bootloader.bin               # ESP32-C6 bootloader
│   │   ├── partitions.bin               # Partition table (huge_app 3.0 MB)
│   │   └── README.md                    # Bilingual companion documentation
│   └── 02-repeater/                     # Standalone Relay / Repeater
│       ├── firmware.bin                 # Main repeater image (~544 KB)
│       ├── bootloader.bin               # ESP32-C6 bootloader
│       ├── partitions.bin               # Partition table
│       └── README.md                    # Bilingual repeater documentation
├── scripts/
│   ├── flash-companion.bat / .ps1       # Standard flash Companion Radio
│   ├── flash-companion-clean.bat        # Clean flash Companion (wipes SPIFFS & old data)
│   ├── flash-repeater.bat / .ps1        # Standard flash Repeater
│   ├── flash-repeater-clean.bat         # Clean flash Repeater (wipes SPIFFS & old data)
│   ├── erase-flash.bat / .ps1           # Full chip flash erase tool
│   └── monitor-serial.bat               # 115200 baud serial log monitor
├── config/
│   ├── platformio.ini                   # Build environment configuration
│   ├── target.h                         # Target definition & radio driver bindings
│   ├── UnitC6LBoard.cpp                 # Expander & peripheral initialization
│   ├── UnitC6LBoard.h                   # Board class header
│   └── UnitC6LDisplay.h                 # OLED driver (rotation & drawXBitmap)
├── patches/
│   └── 01-meshcore-uitask-and-ble-pin.patch # Upstream MeshCore source patch
└── docs/
    ├── hardware-pinout.md               # Complete GPIO & schematic pinout
    └── dual-boot-ota.md                 # Dual-boot slot switching guide
```

### Firmware Roles

#### 1. Companion Radio (BLE + Wi-Fi)
- **Folder**: [`firmware/01-companion-radio-ble`](firmware/01-companion-radio-ble)
- **Role**: Connects to the MeshCore mobile application via Bluetooth Low Energy or local Wi-Fi network (TCP port 5000) with dynamic random PIN pairing for BLE.
- **Display**: 64x48 OLED with scrolling status bar, message counter, real-time connection status (BLE/USB/Wi-Fi), dedicated Wi-Fi toggle screen, and RF telemetry.
- **Memory**: 58.6% RAM (192 KB / 328 KB), 66.7% Flash (2,098 KB / 3,146 KB with `huge_app.csv`).

#### 2. Repeater (Relay)
- **Folder**: [`firmware/02-repeater`](firmware/02-repeater)
- **Role**: Autonomous solar or USB powered packet relay. Extends mesh range without phone connection.
- **Mode**: Headless (Zero-UI overhead), reserving maximal RAM for routing tables and packet buffering.
- **Memory**: 9.1% RAM (29.7 KB / 328 KB), 27.7% Flash (544 KB / 1,966 KB).
- **Default Advert**: `"Unit C6L Repeater"` / Admin Pass: `"password"`

### Quick Start — Flashing

#### Flash Companion Radio (BLE + Wi-Fi)
- **Clean Flash (Recommended)**: Double-click `scripts\flash-companion-clean.bat` or run:
  ```powershell
  .\scripts\flash-companion.ps1 -Erase
  ```
- **Standard Flash (Preserves settings)**: Double-click `scripts\flash-companion.bat` or run:
  ```powershell
  .\scripts\flash-companion.ps1
  ```

#### Flash Repeater
- **Clean Flash (Recommended)**: Double-click `scripts\flash-repeater-clean.bat` or run:
  ```powershell
  .\scripts\flash-repeater.ps1 -Erase
  ```
- **Standard Flash**: Double-click `scripts\flash-repeater.bat` or run:
  ```powershell
  .\scripts\flash-repeater.ps1
  ```

#### Full Chip Erase
To wipe all flash partitions completely, double-click `scripts\erase-flash.bat` or run:
```powershell
.\scripts\erase-flash.ps1
```

### Key Hardware & UI Enhancements
- **Wi-Fi 6 TCP Interface & Dedicated Screen**: Integrated MeshCore Wi-Fi interface listening on TCP port `5000` with dynamic CLI configuration (`set wifi.ssid`, `set wifi.pwd`, `set wifi.enabled`). Added a dedicated 64x48 OLED screen with an upright Wi-Fi icon, connection telemetry (SSID and auto-wrapped IP address), and long-press toggle (`HOLD: ON/OFF`). Enables controlling and managing MeshCore companion device over home Wi-Fi outside Bluetooth range via port 5000.
- **Partition Table Expansion (`huge_app.csv`)**: Switched to a 3.0 MB application partition (`huge_app.csv`) to comfortably accommodate both BLE 5.3 and Wi-Fi 6 stacks simultaneously.
- **Phantom GPS Screen Eliminated (`-UENV_INCLUDE_GPS`)**: Removed inherited GPS configuration that produced scrambled `Gps ' df 1 cces` text on the GPS-less Unit C6L.
- **Prominent Random BLE PIN**: The 6-digit session PIN (`PIN:%06d`) is generated via `StdRNG` and displayed directly on line 2 of the main screen upon boot.
- **Battery Info Relocation**: Removed `BAT: 0.00V` from the first screen and moved it to line 5 of the `RADIO` screen (`BAT:%.2fV`), keeping it accessible for external power diagnostics without wasting space on the primary screen.
- **Accurate Connection State Detection**: Differentiates between USB serial connection, active Bluetooth pairing, and Wi-Fi network link; displays `BLE Ready`, `Connected`, `USB Link`, `BLE: OFF`, and `WIFI:CONN`.
- **Upright XBM Graphics (`drawXBitmap`)**: Corrected LSB-first bit decoding for the MeshCore splash logo, Bluetooth icon, Wi-Fi icon, broadcast waves, and power button.
- **Explicit Action Labels**: Added `HOLD: SEND` on the Advert screen, `HOLD: OFF` on the Shutdown screen, and `HOLD: ON/OFF` on the Wi-Fi screen.
- **Dual-Button Navigation**: Front expander button (`P0`) and side button (`GPIO 9`) both wake the display and cycle menu pages.
- **SX1262 LDO Regulator Mode**: Configured `-D SX126X_USE_REGULATOR_LDO=true` to match the Stamp C6LoRa power topology.

---

<a id="polski"></a>
## Polski

Repozytorium projektu zawierające skompilowane oprogramowanie **MeshCore 1.18**, narzędzia do wgrywania, konfiguracje sprzętowe oraz instrukcje wdrożeniowe dla **M5Stack Unit C6L**.

### Struktura Katalogów

```
m5stack-unit-c6l/
├── firmware/
│   ├── 01-companion-radio-ble/          # Radio towarzyszące Bluetooth i Wi-Fi (aplikacja na telefon/sieć)
│   │   ├── firmware.bin                 # Obraz aplikacji (~2 098 KB)
│   │   ├── bootloader.bin               # Bootloader ESP32-C6
│   │   ├── partitions.bin               # Tabela partycji (huge_app 3.0 MB)
│   │   └── README.md                    # Dwujęzyczna dokumentacja radia towarzyszącego
│   └── 02-repeater/                     # Autonomiczny przekaźnik / repeater
│       ├── firmware.bin                 # Obraz aplikacji przekaźnika (~544 KB)
│       ├── bootloader.bin               # Bootloader ESP32-C6
│       ├── partitions.bin               # Tabela partycji
│       └── README.md                    # Dwujęzyczna dokumentacja repeatera
├── scripts/
│   ├── flash-companion.bat / .ps1       # Standardowe wgrywanie radia towarzyszącego
│   ├── flash-companion-clean.bat        # Czyste wgrywanie radia (usuwa stary SPIFFS i konfigurację)
│   ├── flash-repeater.bat / .ps1        # Standardowe wgrywanie przekaźnika
│   ├── flash-repeater-clean.bat         # Czyste wgrywanie przekaźnika (usuwa stary SPIFFS)
│   ├── erase-flash.bat / .ps1           # Narzędzie pełnego czyszczenia pamięci Flash
│   └── monitor-serial.bat               # Monitor logów szeregowych (115200 baud)
├── config/
│   ├── platformio.ini                   # Konfiguracja środowiska kompilacji
│   ├── target.h                         # Definicje sprzętowe i sterownik radia
│   ├── UnitC6LBoard.cpp                 # Inicjalizacja ekspandera i peryferiów
│   ├── UnitC6LBoard.h                   # Nagłówek klasy płytki
│   └── UnitC6LDisplay.h                 # Sterownik OLED (obrót i funkcja drawXBitmap)
├── patches/
│   └── 01-meshcore-uitask-and-ble-pin.patch # Łatka z kodem źródłowym MeshCore
└── docs/
    ├── hardware-pinout.md               # Pełna rozpiska wyprowadzeń GPIO i schematów
    └── dual-boot-ota.md                 # Poradnik konfiguracji Dual-Boot
```

### Role Oprogramowania

#### 1. Radio Towarzyszące — Companion Radio (BLE + Wi-Fi)
- **Katalog**: [`firmware/01-companion-radio-ble`](firmware/01-companion-radio-ble)
- **Rola**: Łączy się z aplikacją mobilną MeshCore przez Bluetooth Low Energy lub lokalną sieć domową Wi-Fi (port TCP 5000) z wykorzystaniem dynamicznie generowanego kodu PIN dla BLE.
- **Wyświetlacz**: Ekran 64x48 OLED z przewijanym paskiem statusu, licznikiem wiadomości, statusem połączenia (BLE/USB/Wi-Fi), dedykowanym ekranem przełączania Wi-Fi oraz parametrami radiowymi.
- **Pamięć**: 58.6% RAM (192 KB / 328 KB), 66.7% Flash (2 098 KB / 3 146 KB z `huge_app.csv`).

#### 2. Przekaźnik — Repeater (Relay)
- **Katalog**: [`firmware/02-repeater`](firmware/02-repeater)
- **Rola**: Autonomiczny przekaźnik pakietów zasilany z portu USB lub panelu słonecznego. Rozszerza zasięg sieci mesh bez potrzeby połączenia z telefonem.
- **Tryb**: Headless (brak narzutu interfejsu graficznego), co zapewnia maksymalną ilość wolnego RAM-u na tablice routingu.
- **Pamięć**: 9.1% RAM (29.7 KB / 328 KB), 27.7% Flash (544 KB / 1 966 KB).
- **Domyślna nazwa Advert**: `"Unit C6L Repeater"` / Hasło administratora: `"password"`

### Szybki Start — Wgrywanie Oprogramowania

#### Wgrywanie Radia Towarzyszącego (BLE + Wi-Fi)
- **Czyste Wgrywanie (Zalecane)**: Kliknij dwukrotnie `scripts\flash-companion-clean.bat` lub uruchom w PowerShell:
  ```powershell
  .\scripts\flash-companion.ps1 -Erase
  ```
- **Standardowe Wgrywanie (Zachowuje dane)**: Kliknij dwukrotnie `scripts\flash-companion.bat` lub uruchom:
  ```powershell
  .\scripts\flash-companion.ps1
  ```

#### Wgrywanie Przekaźnika (Repeater)
- **Czyste Wgrywanie (Zalecane)**: Kliknij dwukrotnie `scripts\flash-repeater-clean.bat` lub uruchom:
  ```powershell
  .\scripts\flash-repeater.ps1 -Erase
  ```
- **Standardowe Wgrywanie**: Kliknij dwukrotnie `scripts\flash-repeater.bat` lub uruchom:
  ```powershell
  .\scripts\flash-repeater.ps1
  ```

#### Pełne Czyszczenie Pamięci Urządzenia
Aby całkowicie wymazać wszystkie partycje pamięci Flash, kliknij dwukrotnie `scripts\erase-flash.bat` lub uruchom:
```powershell
.\scripts\erase-flash.ps1
```

### Kluczowe Usprawnienia Sprzętowe i Graficzne
- **Obsługa Sieci Wi-Fi i Dedykowany Ekran**: Zintegrowano interfejs sieciowy MeshCore TCP na porcie `5000` z dynamiczną konfiguracją przez CLI (`set wifi.ssid`, `set wifi.pwd`, `set wifi.enabled`). Dodano dedykowany ekran 64x48 OLED z czytelną ikoną Wi-Fi, statusem połączenia (nazwa SSID oraz zawijany adres IP) i przełączaniem długim wciśnięciem (`HOLD: ON/OFF`). Umożliwia to zarządzanie urządzeniem poza zasięgiem Bluetooth w domowej sieci Wi-Fi.
- **Rozszerzona Tabela Partycji (`huge_app.csv`)**: Zastosowano partycję aplikacji 3.0 MB (`huge_app.csv`), aby bez problemu pomieścić jednoczesną obsługę stosów BLE 5.3 oraz Wi-Fi 6.
- **Usunięcie Nieprawidłowego Ekranu GPS (`-UENV_INCLUDE_GPS`)**: Wyeliminowano odziedziczoną konfigurację GPS, która powodowała nakładanie się napisów i tworzenie nieczytelnego ciągu `Gps ' df 1 cces` na urządzeniu bez modułu GPS.
- **Widoczny Losowy Kod PIN BLE**: 6-cyfrowy kod PIN (`PIN:%06d`) jest generowany za pomocą `StdRNG` i wyświetlany w 2. linii ekranu głównego zaraz po uruchomieniu.
- **Przeniesienie Odczytu Baterii**: Usunięto odczyt `BAT: 0.00V` z pierwszego ekranu i przeniesiono go do 5. linii ekranu radiowego (`BAT:%.2fV`), dzięki czemu nie zajmuje cennego miejsca na ekranie głównym.
- **Precyzyjna Detekcja Stanu Połączenia**: Urządzenie rozróżnia połączenie przewodowe USB, aktywne połączenie Bluetooth oraz łącze Wi-Fi; wyświetla stany `BLE Ready`, `Connected`, `USB Link`, `BLE: OFF` oraz `WIFI:CONN`.
- **Prawidłowe Renderowanie Ikon XBM (`drawXBitmap`)**: Zastosowano dekodowanie LSB-first, usuwając efekt lustrzanego odbicia logo powitalnego, ikony Bluetooth, ikony Wi-Fi, fal nadawania i przycisku zasilania.
- **Czytelne Etykiety Akcji Pod Ikonami**: Dodano podpisy `HOLD: SEND` na ekranie rozgłaszania, `HOLD: OFF` na ekranie wyłączania oraz `HOLD: ON/OFF` na ekranie Wi-Fi.
- **Nawigacja Dwoma Przyciskami**: Zarówno przedni przycisk ekspandera (`P0`), jak i boczny przycisk (`GPIO 9`) wybudzają ekran i zmieniają strony menu.
- **Tryb Regulatora LDO SX1262**: Włączono `-D SX126X_USE_REGULATOR_LDO=true`, co eliminuje zawieszanie się modułu radiowego na starcie.
