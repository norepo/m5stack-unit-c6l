# MeshCore 1.18 — Repeater

[English](#english) | [Polski](#polski)

---

<a id="english"></a>
## English

Headless packet relay role for **M5Stack Unit C6L** that forwards packets across the mesh network without requiring Bluetooth or a smartphone connection.

### Binary Details
- **Build Target**: `M5Stack_Unit_C6L_repeater`
- **Firmware Size**: ~544 KB
- **RAM Usage**: 9.1% (29.7 KB / 328 KB)
- **Flash Usage**: 27.7% (544 KB / 1,966 KB)
- **Default Advert Name**: `"Unit C6L Repeater"`
- **Default Admin Password**: `"password"`
- **Max Neighbours**: `50`
- **Flash Mode**: `DIO` (80 MHz)

### Files
- `firmware.bin` — Main repeater application image (flash offset `0x10000`)
- `bootloader.bin` — ESP32-C6 bootloader (flash offset `0x0`)
- `partitions.bin` — Partition table (flash offset `0x8000`)

### Flashing Instructions

#### Standard Flash (Retains settings)
Double-click `scripts\flash-repeater.bat` or run:
```powershell
..\..\scripts\flash-repeater.ps1
```

#### Clean Flash (Recommended — Erases old SPIFFS / settings first)
Double-click `scripts\flash-repeater-clean.bat` or run:
```powershell
..\..\scripts\flash-repeater.ps1 -Erase
```

#### Direct Flashing via `esptool.py`
```bash
# Clean wipe and flash
python esptool.py --chip esp32c6 --baud 921600 erase_flash
python esptool.py --chip esp32c6 --baud 921600 write_flash 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

### Features & Hardware Integration
- **Zero-UI & Headless**: No display rendering or Bluetooth overhead; uses only ~30 KB RAM (~9%), leaving abundant memory for neighbor tables and packet routing.
- **Autonomous LoRa Routing**: Listens and retransmits packets across the mesh network without requiring a phone or computer connection.
- **SX1262 LDO Regulator Mode & Safe Power Default**: Configured with `-D SX126X_USE_REGULATOR_LDO=true` matching the Stamp C6LoRa power topology, defaulting to 20 dBm TX power to eliminate USB brownout resets (configurable up to 22 dBm via `set radio.txpower 22`).
- **PI4IO Expander Control**: Automates active antenna switching (`P6`) and LNA enable (`P5`).
- **Audible Startup & LED Alert**: Emits a startup tone on buzzer (`GPIO 11`) and monitors radio initialization status on the RGB LED (`GPIO 2`).
- **Serial Boot Diagnostics**: Automatically prints hardware reset reason (`esp_reset_reason()`) and available free heap over Serial at 115200 baud upon every restart.
- **Diagnostics & Troubleshooting**: Full guide on boot reset reasons, antenna matching, and serial logging in [TROUBLESHOOTING.md](../../docs/TROUBLESHOOTING.md).

---

<a id="polski"></a>
## Polski

Autonomiczny przekaźnik pakietów (repeater) dla **M5Stack Unit C6L**, który przekazuje pakiety w sieci mesh bez konieczności połączenia z telefonem lub używania Bluetooth.

### Szczegóły Oprogramowania
- **Środowisko kompilacji**: `M5Stack_Unit_C6L_repeater`
- **Rozmiar binariów**: ~544 KB
- **Użycie pamięci RAM**: 9.1% (29.7 KB / 328 KB)
- **Użycie pamięci Flash**: 27.7% (544 KB / 1 966 KB)
- **Domyślna nazwa rozgłoszeniowa (Advert)**: `"Unit C6L Repeater"`
- **Domyślne hasło administratora**: `"password"`
- **Maksymalna liczba sąsiadów**: `50`
- **Tryb pamięci Flash**: `DIO` (80 MHz)

### Pliki
- `firmware.bin` — Główny obraz aplikacji przekaźnika (adres pamięci flash `0x10000`)
- `bootloader.bin` — Bootloader ESP32-C6 (adres pamięci flash `0x0`)
- `partitions.bin` — Tabela partycji (adres pamięci flash `0x8000`)

### Instrukcja Wgrywania (Flashing)

#### Standardowe Wgrywanie (Zachowuje ustawienia)
Kliknij dwukrotnie `scripts\flash-repeater.bat` lub uruchom:
```powershell
..\..\scripts\flash-repeater.ps1
```

#### Czyste Wgrywanie (Zalecane — Czyści pamięć SPIFFS i konfigurację)
Kliknij dwukrotnie `scripts\flash-repeater-clean.bat` lub uruchom:
```powershell
..\..\scripts\flash-repeater.ps1 -Erase
```

#### Bezpośrednie Wgrywanie za Pomocą `esptool.py`
```bash
# Czyszczenie pamięci i wgranie oprogramowania
python esptool.py --chip esp32c6 --baud 921600 erase_flash
python esptool.py --chip esp32c6 --baud 921600 write_flash 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

### Funkcje i Integracja Sprzętowa
- **Tryb Headless (Bez interfejsu graficznego)**: Brak obciążenia wyświetlaniem i stosem Bluetooth; oprogramowanie zużywa jedynie ~30 KB RAM (~9%), pozostawiając maksimum pamięci na tablice routingu i buforowanie pakietów.
- **Autonomiczny Routing LoRa**: Automatycznie odbiera i retransmituje pakiety w sieci mesh, rozszerzając zasięg sieci bez udziału telefonu.
- **Tryb Regulatora LDO SX1262 i Bezpieczna Moc Domyślna**: Włączono flagę `-D SX126X_USE_REGULATOR_LDO=true`, zapewniając stabilne zasilanie toru radiowego SX1262, z domyślną mocą 20 dBm eliminującą ryzyko spadków napięcia (możliwość zmiany na 22 dBm komendą `set radio.txpower 22`).
- **Sterowanie Ekspanderem PI4IO**: Automatyczne przełączanie toru anteny (`P6`) oraz zasilania wzmacniacza LNA (`P5`).
- **Sygnalizacja Dźwiękowa i Dioda LED**: Odtwarza sygnał startowy na brzęczyku (`GPIO 11`) oraz sygnalizuje stan inicjalizacji radia diodą RGB (`GPIO 2`).
- **Diagnostyka Startowa na Porcie Szeregowym**: Automatycznie wypisuje przyczynę restartu (`esp_reset_reason()`) oraz ilość wolnej pamięci RAM (heap) na porcie szeregowym 115200 baud po każdym restarcie.
- **Diagnostyka i Rozwiązywanie Problemów**: Pełny poradnik dotyczący przyczyn restartów, anten i logowania w [TROUBLESHOOTING.md](../../docs/TROUBLESHOOTING.md).
