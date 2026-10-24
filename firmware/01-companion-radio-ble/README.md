# MeshCore 1.18 — Companion Radio (BLE)

Firmware role for **M5Stack Unit C6L** that connects over Bluetooth Low Energy (BLE) to mobile apps (e.g. MeshCore App / Web Flasher).

## Binary Details
- **Build Target**: `M5Stack_Unit_C6L_companion_radio_ble`
- **Firmware Size**: ~1,500 KB
- **RAM Usage**: 50.3% (164.8 KB / 328 KB)
- **Flash Usage**: 74.8% (1,470 KB / 1,966 KB)
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

## Changelog & Hardware Fixes

### Latest Release Fixes
- **SX1262 LDO Regulator Mode**: Enabled `-D SX126X_USE_REGULATOR_LDO=true` in `platformio.ini`. The Unit C6L (Stamp C6LoRa) routes SX1262 power through the internal LDO regulator rather than DC-DC; without this, `radio_init()` failed and halted the MCU in `while(1);` on boot.
- **SPI Bus CS Isolation**: Set LoRa NSS (`GPIO 23`) and OLED CS (`GPIO 6`) as `OUTPUT HIGH` before any SPI traffic begins. Prevents the SX1262 from receiving display initialization and drawing bytes over the shared SPI bus (`SCLK=20`, `MOSI=21`, `MISO=22`).
- **SX1262 Reset Pulse Timing**: Implemented clean 100ms active-low reset pulse on expander `P7` (`SX_NRST`) followed by release `HIGH` with LNA (`P5`) and antenna switch (`P6`) enabled.
- **OLED Display Orientation**: Added `display.setRotation(2)` (180° rotation) so that UI text and icons are right-side up when device antennas point upward.
- **USB CDC Configuration**: Added `-D ARDUINO_USB_CDC_ON_BOOT=1` and `-D ARDUINO_USB_MODE=1` to ensure reliable Web Serial and USB log monitoring.
- **Boot Fault Diagnostics**: Added visual fault feedback (RGB LED turns RED on `GPIO 2` and display outputs `"RADIO INIT / FAILED!"` if radio hardware initialization fails).

