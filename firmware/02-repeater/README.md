# MeshCore 1.18 — Repeater

Headless packet relay role for **M5Stack Unit C6L** that forwards packets across the mesh network without requiring Bluetooth or a smartphone connection.

## Binary Details
- **Build Target**: `M5Stack_Unit_C6L_repeater`
- **Firmware Size**: ~583 KB
- **RAM Usage**: 9.1% (30 KB / 328 KB)
- **Flash Usage**: 28.1% (552 KB / 1,966 KB)
- **Default Advert Name**: `"Unit C6L Repeater"`
- **Default Admin Password**: `"password"`
- **Max Neighbours**: `50`
- **Flash Mode**: `DIO` (80 MHz)

## Files
- `firmware.bin` — Main repeater application image
- `bootloader.bin` — ESP32-C6 bootloader (flash offset `0x0`)
- `partitions.bin` — Partition table (flash offset `0x8000`)

## Flashing Options

### 1. Flash as Primary Firmware (Slot 0)
Replaces the primary slot at `0x10000`:
```powershell
..\..\scripts\flash-repeater.ps1
```

### 2. Dual-Boot / Secondary OTA Slot (Slot 1)
Flash as secondary firmware to OTA slot 1 at `0x1F0000` while keeping Companion Radio on slot 0:
```powershell
..\..\scripts\flash-repeater.ps1 -SecondarySlot
```
To toggle between Companion and Repeater at boot: hold the **BOOT** button for >500ms while powering on or resetting.
