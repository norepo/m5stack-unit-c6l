# MeshCore 1.18 — Repeater + Observer (4x MQTT)

[English](#english) | [Polski](#polski)

---

<a id="english"></a>
## English

Hybrid standalone packet relay and multi-broker **MQTT Observer** firmware for the **M5Stack Unit C6L** (ESP32-C6 + SX1262 LoRa).

Functions simultaneously as a mesh repeater (forwarding packets over RF) and an internet-connected observer node forwarding intercepted packets, telemetry, and link metrics (SNR/RSSI) to up to **4 concurrent MQTT brokers** over Wi-Fi 6 (port 1883).

### Binary Details
- **Build Target**: `M5Stack_Unit_C6L_repeater_observer`
- **Firmware Size**: ~1,276 KB
- **RAM Usage**: 19.5% (63.9 KB / 327.6 KB) — **>263 KB of free heap**
- **Flash Usage**: 63.6% (1,250 KB / 1,966 KB with `min_spiffs.csv`)
- **Default Advert Name**: `"Unit C6L Observer"`
- **Default Admin Password**: `"password"`
- **Default Region IATA**: `"WAW"`
- **Flash Mode**: `DIO` (80 MHz)

### Files
- `firmware.bin` — Main application image (flash offset `0x10000`)
- `bootloader.bin` — ESP32-C6 bootloader (flash offset `0x0`)
- `partitions.bin` — Partition table (flash offset `0x8000`)

### Flashing Instructions

#### Standard Flash (Retains settings)
Double-click `scripts\flash-observer.bat` or run in PowerShell:
```powershell
.\scripts\flash-observer.ps1
```

#### Clean Flash (Recommended — Erases old SPIFFS / settings first)
Double-click `scripts\flash-observer-clean.bat` or run:
```powershell
.\scripts\flash-observer.ps1 -Erase
```

---

### CLI Configuration Guide

Connect via Serial monitor (115200 baud) or the MeshCore Web Serial console.

#### 1. Wi-Fi Configuration
```text
set wifi.ssid YourSSID
set wifi.pwd YourPassword
set wifi.enabled 1
get wifi.status
```

#### 2. Global MQTT Configuration
```text
set mqtt.enabled 1
set mqtt.iata WAW
set mqtt.status 1
set mqtt.packets 1
set mqtt.interval 60
get mqtt.status
```

#### 3. Broker Slots Configuration (4 Concurrent Queues)

- **Slot 1 (MeshMapper Analyzer)**:
  ```text
  set mqtt1.preset meshmapper
  set mqtt1.enabled 1
  ```
- **Slot 2 (CoreScope Analyzer)**:
  ```text
  set mqtt2.preset corescope
  set mqtt2.enabled 1
  ```
- **Slot 3 (Regional Analyzer)**:
  ```text
  set mqtt3.server mqtt.regionalmesh.org
  set mqtt3.port 1883
  set mqtt3.user myuser
  set mqtt3.pass mypass
  set mqtt3.enabled 1
  ```
- **Slot 4 (Custom Server / Personal Experiments)**:
  ```text
  set mqtt4.server 192.168.1.150
  set mqtt4.port 1883
  set mqtt4.topic mylab
  set mqtt4.enabled 1
  ```

#### 4. Checking Status
```text
get mqtt.status
get mqtt1
get mqtt2
get mqtt3
get mqtt4
```

---

<a id="polski"></a>
## Polski

Hybrydowe oprogramowanie łączące autonomiczny przekaźnik pakietów (repeater) oraz **MQTT Observer** dla modułu **M5Stack Unit C6L** (ESP32-C6 + SX1262 LoRa).

Urządzenie działa jednocześnie jako repeater sieci mesh (retransmitując pakiety drogą radiową) oraz jako internetowy nasłuchiwacz (observer), przesyłając przechwycone ramki, metryki jakości łącza (SNR/RSSI) oraz telemetrię do maksymalnie **4 równoległych brokerów MQTT** przez sieć Wi-Fi 6 (port 1883).

### Szczegóły Oprogramowania
- **Środowisko kompilacji**: `M5Stack_Unit_C6L_repeater_observer`
- **Rozmiar binariów**: ~1 276 KB
- **Użycie pamięci RAM**: 19.5% (63.9 KB / 327.6 KB) — **>263 KB wolnego RAM-u (sterty)**
- **Użycie pamięci Flash**: 63.6% (1 250 KB / 1 966 KB)
- **Domyślna nazwa rozgłoszeniowa (Advert)**: `"Unit C6L Observer"`
- **Domyślne hasło administratora**: `"password"`
- **Domyślny kod IATA**: `"WAW"`
- **Tryb pamięci Flash**: `DIO` (80 MHz)

### Pliki
- `firmware.bin` — Obraz aplikacji Repeater+Observer (adres flash `0x10000`)
- `bootloader.bin` — Bootloader ESP32-C6 (adres flash `0x0`)
- `partitions.bin` — Tabela partycji (adres flash `0x8000`)

### Instrukcja Wgrywania (Flashing)

#### Standardowe Wgrywanie (Zachowuje ustawienia w pamięci)
Kliknij dwukrotnie `scripts\flash-observer.bat` lub uruchom w PowerShell:
```powershell
.\scripts\flash-observer.ps1
```

#### Czyste Wgrywanie (Zalecane — Czyści pamięć Flash i konfigurację)
Kliknij dwukrotnie `scripts\flash-observer-clean.bat` lub uruchom:
```powershell
.\scripts\flash-observer.ps1 -Erase
```

---

### Przewodnik po Komendach CLI

Połącz się z urządzeniem przez port szeregowy (115200 baud) lub konsolę Web Serial.

#### 1. Konfiguracja Połączenia Wi-Fi
```text
set wifi.ssid TwojaNazwaWiFi
set wifi.pwd TwojeHasloWiFi
set wifi.enabled 1
get wifi.status
```

#### 2. Konfiguracja Globalna MQTT
```text
set mqtt.enabled 1
set mqtt.iata WAW
set mqtt.status 1
set mqtt.packets 1
set mqtt.interval 60
get mqtt.status
```

#### 3. Konfiguracja 4 Równoległych Slotów Brokerów

- **Slot 1 (MeshMapper Analyzer)**:
  ```text
  set mqtt1.preset meshmapper
  set mqtt1.enabled 1
  ```
- **Slot 2 (CoreScope Analyzer)**:
  ```text
  set mqtt2.preset corescope
  set mqtt2.enabled 1
  ```
- **Slot 3 (Analyzer Regionalny)**:
  ```text
  set mqtt3.server mqtt.regionalmesh.org
  set mqtt3.port 1883
  set mqtt3.user myuser
  set mqtt3.pass mypass
  set mqtt3.enabled 1
  ```
- **Slot 4 (Własny Serwer / Eksperymenty)**:
  ```text
  set mqtt4.server 192.168.1.150
  set mqtt4.port 1883
  set mqtt4.topic mylab
  set mqtt4.enabled 1
  ```

#### 4. Weryfikacja Działania i Diagnostyka
```text
get mqtt.status
get mqtt1
get mqtt2
get mqtt3
get mqtt4
```
