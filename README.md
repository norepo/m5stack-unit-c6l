# M5Stack Unit C6L — MeshCore Firmware & Tools

[![Hardware](https://img.shields.io/badge/Hardware-M5Stack%20Unit%20C6L-orange.svg)](https://m5stack.com)
[![MCU](https://img.shields.io/badge/MCU-ESP32--C6%20(RISC--V)-blue.svg)](https://espressif.com)
[![Radio](https://img.shields.io/badge/LoRa-Semtech%20SX1262%20(+22dBm)-green.svg)](https://semtech.com)
[![MeshCore](https://img.shields.io/badge/MeshCore-v1.18.0-purple.svg)](https://github.com/meshcore-dev/MeshCore)

Project repository containing precompiled **MeshCore 1.18** firmware, flashing tools, hardware configurations, and deployment guides for the **M5Stack Unit C6L**.

---

## Directory Organization

The project is structured logically so each role and tool has its dedicated folder:

```
m5stack-unit-c6l/
├── firmware/
│   ├── 01-companion-radio-ble/     # Bluetooth Companion Radio (Phone App)
│   │   ├── firmware.bin            # App image (~1,478 KB)
│   │   ├── bootloader.bin          # ESP32-C6 bootloader
│   │   ├── partitions.bin          # min_spiffs partition table
│   │   └── README.md               # BLE role documentation
│   └── 02-repeater/                # Standalone Relay / Repeater
│       ├── firmware.bin            # App image (~583 KB)
│       ├── bootloader.bin          # ESP32-C6 bootloader
│       ├── partitions.bin          # min_spiffs partition table
│       └── README.md               # Repeater role documentation
├── scripts/
│   ├── flash-companion.ps1 / .bat  # Flash Companion Radio (Slot 0)
│   ├── flash-repeater.ps1 / .bat   # Flash Repeater (Primary or Slot 1)
│   └── monitor-serial.bat          # 115200 baud serial log monitor
├── config/
│   ├── platformio.ini              # Build environment configuration
│   ├── target.h                    # Target definition & radio driver bindings
│   ├── UnitC6LBoard.cpp            # I/O expander & peripheral initialization
│   └── UnitC6LBoard.h              # Board class header
└── docs/
    ├── hardware-pinout.md          # Complete GPIO & schematic pinout
    └── dual-boot-ota.md            # Dual-boot slot switching guide
```

---

## Firmware Roles

### 1. Companion Radio (BLE)
- **Folder**: [`firmware/01-companion-radio-ble`](firmware/01-companion-radio-ble)
- **Role**: Connects to the MeshCore mobile application via Bluetooth Low Energy (PIN: `123456`).
- **Memory**: 50.1% RAM (164 KB), 73.9% Flash (1,453 KB).

### 2. Repeater (Relay)
- **Folder**: [`firmware/02-repeater`](firmware/02-repeater)
- **Role**: Autonomous solar or USB powered packet relay. Extends mesh range without phone connection.
- **Memory**: 9.1% RAM (30 KB), 28.1% Flash (552 KB).
- **Default Advert**: `"Unit C6L Repeater"` / Admin Pass: `"password"`

---

## Quick Start — Flashing

### Flash Companion Radio (BLE)
Double-click `scripts\flash-companion.bat` or run:
```powershell
.\scripts\flash-companion.ps1
```

### Flash Repeater
Double-click `scripts\flash-repeater.bat` or run:
```powershell
.\scripts\flash-repeater.ps1
```

### Advanced: Dual-Boot (Both Roles on One Device)
You can flash both roles simultaneously!
1. Flash Companion to Primary Slot: `.\scripts\flash-companion.ps1`
2. Flash Repeater to Secondary Slot: `.\scripts\flash-repeater.ps1 -SecondarySlot`
3. Toggle between them by holding the **BOOT** button for >500ms on startup. See [docs/dual-boot-ota.md](docs/dual-boot-ota.md) for full details.

---

## Hardware Reference
See [docs/hardware-pinout.md](docs/hardware-pinout.md) for full specifications.
- **Important**: Must use `flash_mode = dio` (ESP32-C6 `qio` causes crash loops).
- **SX1262 Reset & LNA**: Controlled through onboard PI4IOE5V6408 I2C expander.
