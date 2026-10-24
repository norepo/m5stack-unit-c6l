# MeshCore 1.18 — Repeater

Headless packet relay role for **M5Stack Unit C6L** that forwards packets across the mesh network without requiring Bluetooth or a smartphone connection.

## Binary Details
- **Build Target**: `M5Stack_Unit_C6L_repeater`
- **Firmware Size**: ~590 KB
- **RAM Usage**: 9.1% (29.7 KB / 328 KB)
- **Flash Usage**: 27.7% (544 KB / 1,966 KB)
- **Default Advert Name**: `"Unit C6L Repeater"`
- **Default Admin Password**: `"password"`
- **Max Neighbours**: `50`
- **Flash Mode**: `DIO` (80 MHz)

## Files
- `firmware.bin` — Main repeater application image (flash offset `0x10000`)
- `bootloader.bin` — ESP32-C6 bootloader (flash offset `0x0`)
- `partitions.bin` — Partition table (flash offset `0x8000`)

## Flashing

### Standard Flash (Retains settings)
Double-click `scripts\flash-repeater.bat` or run:
```powershell
..\..\scripts\flash-repeater.ps1
```

### Clean Flash (Recommended — Erases old SPIFFS / settings first)
Double-click `scripts\flash-repeater-clean.bat` or run:
```powershell
..\..\scripts\flash-repeater.ps1 -Erase
```

Or directly using `esptool.py`:
```bash
# Clean wipe and flash
python esptool.py --chip esp32c6 --baud 921600 erase_flash
python esptool.py --chip esp32c6 --baud 921600 write_flash 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

## Features & Hardware Integration
- **Zero-UI & Headless**: No display rendering or Bluetooth overhead; uses only ~30 KB RAM (~9%), leaving abundant memory for neighbor tables and packet routing.
- **Autonomous LoRa Routing**: Listens and retransmits packets across the mesh network without requiring a phone or computer connection.
- **SX1262 LDO Regulator Mode**: Configured with `-D SX126X_USE_REGULATOR_LDO=true` matching the Stamp C6LoRa power topology.
- **PI4IO Expander Control**: Automates active antenna switching (`P6`) and LNA enable (`P5`).
- **Audible Startup & LED Alert**: Emits a startup tone on buzzer (`GPIO 11`) and monitors radio initialization status on the RGB LED (`GPIO 2`).
