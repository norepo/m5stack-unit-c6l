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
│   ├── 01-companion-radio-ble/          # Bluetooth Companion Radio (Phone App)
│   │   ├── firmware.bin                 # Main application image (~1,472 KB)
│   │   ├── bootloader.bin               # ESP32-C6 bootloader
│   │   ├── partitions.bin               # Partition table (min_spiffs.csv)
│   │   └── README.md                    # Bilingual companion documentation
│   ├── 02-repeater/                     # Standalone Relay / Repeater
│   │   ├── firmware.bin                 # Main repeater image (~544 KB)
│   │   ├── bootloader.bin               # ESP32-C6 bootloader
│   │   ├── partitions.bin               # Partition table
│   │   └── README.md                    # Bilingual repeater documentation
│   └── 03-repeater-observer/            # Standalone Relay + 4x MQTT Observer (Wi-Fi 6)
│       ├── firmware.bin                 # Repeater+Observer image (~1,276 KB)
│       ├── bootloader.bin               # ESP32-C6 bootloader
│       ├── partitions.bin               # Partition table
│       └── README.md                    # Bilingual observer & CLI documentation
├── scripts/
│   ├── flash-companion.bat / .ps1       # Standard flash Companion Radio
│   ├── flash-companion-clean.bat        # Clean flash Companion (wipes SPIFFS & old data)
│   ├── flash-repeater.bat / .ps1        # Standard flash Repeater
│   ├── flash-repeater-clean.bat         # Clean flash Repeater (wipes SPIFFS & old data)
│   ├── flash-observer.bat / .ps1        # Standard flash Repeater+Observer
│   ├── flash-observer-clean.bat         # Clean flash Repeater+Observer (wipes SPIFFS & old data)
│   ├── erase-flash.bat / .ps1           # Full chip flash erase tool
│   ├── monitor-serial.bat               # 115200 baud serial log monitor
│   ├── package-release.bat / .ps1       # Package standalone ZIP release bundles
│   └── publish-github-release.ps1       # Automated GitHub Release publisher
├── config/
│   ├── platformio.ini                   # Build environment configuration
│   ├── target.h                         # Target definition & radio driver bindings
│   ├── UnitC6LBoard.cpp                 # Expander & peripheral initialization
│   ├── UnitC6LBoard.h                   # Board class header
│   ├── UnitC6LDisplay.h                 # OLED driver (rotation & drawXBitmap)
│   ├── MqttObserverManager.h            # Multi-broker MQTT observer manager header
│   └── MqttObserverManager.cpp          # Multi-broker MQTT observer manager implementation
├── patches/
│   ├── 01-meshcore-uitask-and-ble-pin.patch      # Upstream MeshCore UI & dynamic PIN patch
│   └── 02-meshcore-repeater-mqtt-observer.patch  # Repeater MQTT observer patch
└── docs/
    ├── hardware-pinout.md               # Complete GPIO & schematic pinout
    ├── dual-boot-ota.md                 # Dual-boot slot switching guide
    └── TROUBLESHOOTING.md               # Diagnostics, antenna, brownout & release guide
```

### Firmware Roles

#### 1. Companion Radio (BLE)
- **Folder**: [`firmware/01-companion-radio-ble`](firmware/01-companion-radio-ble)
- **Role**: Connects to the MeshCore mobile application via Bluetooth Low Energy or USB Web Serial with dynamic random PIN pairing.
- **Display**: 64x48 OLED with scrolling status bar, message counter, real-time connection status (BLE/USB), and RF telemetry.
- **Memory**: 50.4% RAM (165 KB / 328 KB), 74.9% Flash (1,472 KB / 1,966 KB with `min_spiffs.csv`).
- **Wi-Fi Architecture**: The ESP32-C6 has 320 KB internal SRAM and no external PSRAM. Running both the complete NimBLE 5.3 stack and Wi-Fi 6 TCP stack simultaneously causes heap exhaustion. To ensure 100% stability, Wi-Fi is preserved in code behind the `-D ENABLE_WIFI_INTERFACE` build flag but disabled by default in the companion release.

#### 2. Repeater (Relay)
- **Folder**: [`firmware/02-repeater`](firmware/02-repeater)
- **Role**: Autonomous solar or USB powered packet relay. Extends mesh range without phone connection.
- **Mode**: Headless (Zero-UI overhead), reserving maximal RAM for routing tables and packet buffering.
- **Memory**: 9.1% RAM (29.7 KB / 328 KB), 27.7% Flash (544 KB / 1,966 KB).
- **Default Advert**: `"Unit C6L Repeater"` / Admin Pass: `"password"`

#### 3. Repeater + Observer (4x MQTT)
- **Folder**: [`firmware/03-repeater-observer`](firmware/03-repeater-observer)
- **Role**: Hybrid packet relay and internet-connected observer node. Retransmits packets across the mesh while streaming overheard packets, telemetry, and link metrics (SNR/RSSI) to up to **4 concurrent MQTT brokers** over Wi-Fi 6 (port 1883).
- **CLI Configuration**: Fully configurable via Serial / Web Serial (`set wifi.*`, `set mqtt.*`, `set mqtt1..4.*`).
- **Memory**: 19.5% RAM (63.9 KB / 328 KB, >263 KB free heap), 63.6% Flash (1,250 KB / 1,966 KB).
- **Default Advert**: `"Unit C6L Observer"` / Default IATA: `"WAW"`

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

#### Flash Repeater + Observer (4x MQTT)
- **Clean Flash (Recommended)**: Double-click `scripts\flash-observer-clean.bat` or run:
  ```powershell
  .\scripts\flash-observer.ps1 -Erase
  ```
- **Standard Flash**: Double-click `scripts\flash-observer.bat` or run:
  ```powershell
  .\scripts\flash-observer.ps1
  ```

#### Full Chip Erase
To wipe all flash partitions completely, double-click `scripts\erase-flash.bat` or run:
```powershell
.\scripts\erase-flash.ps1
```

### Key Hardware & UI Enhancements
- **4x Concurrent MQTT Broker Observer**: Native uplink streaming to MeshMapper, CoreScope, regional analyzers, and personal custom servers via Wi-Fi 6 with non-blocking single-serialization architecture.
- **Dynamic CLI Configuration**: Full runtime control over Wi-Fi (`set wifi.ssid`, `set wifi.pwd`, `set wifi.enabled`) and MQTT broker parameters (`set mqtt.iata`, `set mqtt1..4.server`, `set mqtt1..4.preset`).
- **Wi-Fi 6 TCP Interface & Dedicated Screen**: Integrated MeshCore Wi-Fi interface listening on TCP port `5000` with dynamic CLI configuration.
- **Partition Table Expansion (`huge_app.csv` / `min_spiffs.csv`)**: Partition layout allocating 1.92–3.0 MB to accommodate Wi-Fi 6 and multi-client networking.
- **Phantom GPS Screen Eliminated (`-UENV_INCLUDE_GPS`)**: Removed inherited GPS configuration that produced scrambled text on the GPS-less Unit C6L.
- **Prominent Random BLE PIN**: The 6-digit session PIN (`PIN:%06d`) is generated via `StdRNG` and displayed directly on line 2 of the main screen upon boot.
- **SX1262 LDO Regulator Mode**: Configured `-D SX126X_USE_REGULATOR_LDO=true` matching the Stamp C6LoRa power topology.

---

<a id="polski"></a>
## Polski

Repozytorium projektu zawierające skompilowane oprogramowanie **MeshCore 1.18**, narzędzia do wgrywania, konfiguracje sprzętowe oraz instrukcje wdrożeniowe dla **M5Stack Unit C6L**.

### Struktura Katalogów

```
m5stack-unit-c6l/
├── firmware/
│   ├── 01-companion-radio-ble/          # Radio towarzyszące Bluetooth (aplikacja na telefon)
│   │   ├── firmware.bin                 # Obraz aplikacji (~1 472 KB)
│   │   ├── bootloader.bin               # Bootloader ESP32-C6
│   │   ├── partitions.bin               # Tabela partycji (min_spiffs.csv)
│   │   └── README.md                    # Dwujęzyczna dokumentacja radia towarzyszącego
│   ├── 02-repeater/                     # Autonomiczny przekaźnik / repeater
│   │   ├── firmware.bin                 # Obraz aplikacji przekaźnika (~544 KB)
│   │   ├── bootloader.bin               # Bootloader ESP32-C6
│   │   ├── partitions.bin               # Tabela partycji
│   │   └── README.md                    # Dwujęzyczna dokumentacja repeatera
│   └── 03-repeater-observer/            # Przekaźnik + 4x MQTT Observer (Wi-Fi 6)
│       ├── firmware.bin                 # Obraz aplikacji Repeater+Observer (~1 276 KB)
│       ├── bootloader.bin               # Bootloader ESP32-C6
│       ├── partitions.bin               # Tabela partycji
│       └── README.md                    # Dwujęzyczna dokumentacja observera i komend CLI
├── scripts/
│   ├── flash-companion.bat / .ps1       # Standardowe wgrywanie radia towarzyszącego
│   ├── flash-companion-clean.bat        # Czyste wgrywanie radia (usuwa stary SPIFFS i konfigurację)
│   ├── flash-repeater.bat / .ps1        # Standardowe wgrywanie przekaźnika
│   ├── flash-repeater-clean.bat         # Czyste wgrywanie przekaźnika (usuwa stary SPIFFS)
│   ├── flash-observer.bat / .ps1        # Standardowe wgrywanie przekaźnika z MQTT Observerem
│   ├── flash-observer-clean.bat         # Czyste wgrywanie przekaźnika z MQTT Observerem
│   ├── erase-flash.bat / .ps1           # Narzędzie pełnego czyszczenia pamięci Flash
│   ├── monitor-serial.bat               # Monitor logów szeregowych (115200 baud)
│   ├── package-release.bat / .ps1       # Tworzenie paczek wydań ZIP do pobrania
│   └── publish-github-release.ps1       # Automatyczna publikacja wydań na GitHubie
├── config/
│   ├── platformio.ini                   # Konfiguracja środowiska kompilacji
│   ├── target.h                         # Definicje sprzętowe i sterownik radia
│   ├── UnitC6LBoard.cpp                 # Inicjalizacja ekspandera i peryferiów
│   ├── UnitC6LBoard.h                   # Nagłówek klasy płytki
│   ├── UnitC6LDisplay.h                 # Sterownik OLED (obrót i funkcja drawXBitmap)
│   ├── MqttObserverManager.h            # Nagłówek menedżera multi-broker MQTT
│   └── MqttObserverManager.cpp          # Implementacja menedżera multi-broker MQTT
├── patches/
│   ├── 01-meshcore-uitask-and-ble-pin.patch      # Łatka z kodem interfejsu i dynamicznego PINu
│   └── 02-meshcore-repeater-mqtt-observer.patch  # Łatka dodająca obsługę MQTT Observera
└── docs/
    ├── hardware-pinout.md               # Pełna rozpiska wyprowadzeń GPIO i schematów
    ├── dual-boot-ota.md                 # Poradnik konfiguracji Dual-Boot
    └── TROUBLESHOOTING.md               # Diagnostyka restartów, anteny i wersjonowanie
```

### Role Oprogramowania

#### 1. Radio Towarzyszące — Companion Radio (BLE)
- **Katalog**: [`firmware/01-companion-radio-ble`](firmware/01-companion-radio-ble)
- **Rola**: Łączy się z aplikacją mobilną MeshCore przez Bluetooth Low Energy lub port szeregowy USB z wykorzystaniem dynamicznie generowanego kodu PIN dla BLE.
- **Wyświetlacz**: Ekran 64x48 OLED ze statusem, licznikiem wiadomości i telemetrią radiową.
- **Pamięć**: 50.4% RAM (165 KB / 328 KB), 74.9% Flash (1 472 KB / 1 966 KB z `min_spiffs.csv`).

#### 2. Przekaźnik — Repeater (Relay)
- **Katalog**: [`firmware/02-repeater`](firmware/02-repeater)
- **Rola**: Autonomiczny przekaźnik pakietów zasilany z portu USB lub panelu słonecznego. Rozszerza zasięg sieci mesh bez potrzeby połączenia z telefonem.
- **Tryb**: Headless (brak narzutu interfejsu graficznego), co zapewnia maksymalną ilość wolnego RAM-u na tablice routingu.
- **Pamięć**: 9.1% RAM (29.7 KB / 328 KB), 27.7% Flash (544 KB / 1 966 KB).
- **Domyślna nazwa Advert**: `"Unit C6L Repeater"` / Hasło administratora: `"password"`

#### 3. Przekaźnik + Observer — Repeater + Observer (4x MQTT)
- **Katalog**: [`firmware/03-repeater-observer`](firmware/03-repeater-observer)
- **Rola**: Hybrydowy węzeł łączący retransmisję pakietów w sieci mesh z jednoczesnym wysyłaniem przechwyconych ramek radiowych, telemetrii i jakości łącza (SNR/RSSI) do maksymalnie **4 równoległych brokerów MQTT** przez sieć Wi-Fi 6 (port 1883).
- **Konfiguracja CLI**: Pełne zarządzanie przez port szeregowy (`set wifi.*`, `set mqtt.*`, `set mqtt1..4.*`).
- **Pamięć**: 19.5% RAM (63.9 KB / 328 KB, >263 KB wolnego RAM-u), 63.6% Flash (1 250 KB / 1 966 KB).
- **Domyślna nazwa Advert**: `"Unit C6L Observer"` / Domyślny IATA: `"WAW"`

### Szybki Start — Wgrywanie Oprogramowania

#### Wgrywanie Radia Towarzyszącego (BLE)
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

#### Wgrywanie Przekaźnika z Observerem (Repeater + 4x MQTT)
- **Czyste Wgrywanie (Zalecane)**: Kliknij dwukrotnie `scripts\flash-observer-clean.bat` lub uruchom:
  ```powershell
  .\scripts\flash-observer.ps1 -Erase
  ```
- **Standardowe Wgrywanie**: Kliknij dwukrotnie `scripts\flash-observer.bat` lub uruchom:
  ```powershell
  .\scripts\flash-observer.ps1
  ```

#### Pełne Czyszczenie Pamięci Urządzenia
Aby całkowicie wymazać wszystkie partycje pamięci Flash, kliknij dwukrotnie `scripts\erase-flash.bat` lub uruchom:
```powershell
.\scripts\erase-flash.ps1
```

### Kluczowe Usprawnienia i Nowości
- **4 Równoległe Połączenia MQTT Observer**: Natywne strumieniowanie danych do MeshMapper, CoreScope, serwerów regionalnych oraz własnego serwera testowego przez Wi-Fi 6 z asynchronicznym kolejkowaniem pakietów.
- **Dynamiczna Konfiguracja CLI**: Pełna edycja parametrów Wi-Fi (`set wifi.ssid`, `set wifi.pwd`, `set wifi.enabled`) oraz slotów brokerów MQTT (`set mqtt.iata`, `set mqtt1..4.server`, `set mqtt1..4.preset`).
- **Niski Narzut Pamięciowy**: W trybie Observera zajęte jest zaledwie 19.5% pamięci RAM, co pozostawia ponad 263 KB wolnej sterty na tablice routingu mesh i bufory sieciowe.
- **Tryb Regulatora LDO SX1262**: Włączono `-D SX126X_USE_REGULATOR_LDO=true`, co eliminuje zawieszanie się modułu radiowego na starcie.
