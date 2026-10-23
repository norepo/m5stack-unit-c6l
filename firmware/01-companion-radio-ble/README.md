# MeshCore 1.18 — Companion Radio (BLE)

Firmware role for **M5Stack Unit C6L** that connects over Bluetooth Low Energy (BLE) to mobile apps (e.g. MeshCore App / Web Flasher).

## Binary Details
- **Build Target**: `M5Stack_Unit_C6L_companion_radio_ble`
- **Firmware Size**: ~1,478 KB
- **RAM Usage**: 50.1% (164 KB / 328 KB)
- **Flash Usage**: 73.9% (1,453 KB / 1,966 KB)
- **Bluetooth PIN**: `123456`
- **Flash Mode**: `DIO` (80 MHz)

## Files
- `firmware.bin` — Main application image (flash offset `0x10000`)
- `bootloader.bin` — ESP32-C6 bootloader (flash offset `0x0`)
- `partitions.bin` — Partition table (flash offset `0x8000`)

## Flashing
Run the PowerShell script:
```powershell
..\..\scripts\flash-companion.ps1
```
Or directly using `esptool.py`:
```bash
python esptool.py --chip esp32c6 --baud 921600 write_flash 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```
