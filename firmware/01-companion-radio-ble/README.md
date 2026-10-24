# MeshCore 1.18 — Companion Radio (BLE)

Firmware role for **M5Stack Unit C6L** that connects over Bluetooth Low Energy (BLE) to mobile apps (e.g. MeshCore App / Web Flasher).

## Binary Details
- **Build Target**: `M5Stack_Unit_C6L_companion_radio_ble`
- **Firmware Size**: ~1,500 KB
- **RAM Usage**: 50.3% (164.8 KB / 328 KB)
- **Flash Usage**: 74.8% (1,470 KB / 1,966 KB)
- **Bluetooth PIN**: Random 6-digit PIN on boot (displayed on screen)
- **Flash Mode**: `DIO` (80 MHz)

## Files
- `firmware.bin` — Main application image (flash offset `0x10000`)
- `bootloader.bin` — ESP32-C6 bootloader (flash offset `0x0`)
- `partitions.bin` — Partition table (flash offset `0x8000`)

## Flashing

### Standard Flash (Retains settings)
Double-click `scripts\flash-companion.bat` or run:
```powershell
..\..\scripts\flash-companion.ps1
```

### Clean Flash (Recommended — Erases old SPIFFS / pairings first)
Double-click `scripts\flash-companion-clean.bat` or run:
```powershell
..\..\scripts\flash-companion.ps1 -Erase
```

### Full Flash Erase Only
Double-click `scripts\erase-flash.bat` or run:
```powershell
..\..\scripts\erase-flash.ps1
```

Or directly using `esptool.py`:
```bash
# 1. (Optional clean wipe)
python esptool.py --chip esp32c6 --baud 921600 erase_flash

# 2. Flash firmware
python esptool.py --chip esp32c6 --baud 921600 write_flash 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

## Changelog & Hardware Fixes

### Latest Release Fixes
- **Phantom GPS Screen Eliminated (`-UENV_INCLUDE_GPS`)**: The M5Stack Unit C6L has no onboard GPS hardware. Previously, `ENV_INCLUDE_GPS=1` was inherited from base configurations, creating a broken GPS page in the menu cycle that attempted to print both `"gps off"` and `"Can't access GPS"` at the identical coordinate `y=8`, causing the jumbled `Gps ' df 1 cces` artifact. Adding `-UENV_INCLUDE_GPS` and removing unused GPS pins completely removes this screen from the menu cycle.
- **Prominent Bluetooth PIN on First Screen**:
  - Replaced the unused battery voltage on the first (`FIRST`) screen with the random 6-digit Bluetooth PIN (`PIN:%06d` at `y=23`, centered).
  - Line 3 (`y=35`) now clearly indicates the connection state: `BLE Ready` (waiting for pairing), `Connected` (Bluetooth connected), `USB Link` (USB serial active), or `BLE: OFF` (Bluetooth disabled).
  - The PIN is always visible on the main screen upon boot, allowing quick and effortless pairing with mobile apps or MeshCore Web.
- **Battery Info Relocated to Radio Screen**: Since the Unit C6L is powered via external 5V (USB or Grove connector) and has no internal battery by default, `BAT: 0.00V` has been removed from the main screen and relocated to line 5 of the `RADIO` screen (`BAT:%.2fV` at `y=40`). It remains accessible for external power diagnostics without taking up valuable space on the main screen.
- **Accurate Bluetooth Connection Detection**: Fixed `ArduinoSerialInterface::isConnected()` under `ARDUINO_USB_CDC_ON_BOOT` and introduced `isBluetoothConnected()` to differentiate between USB serial link and active Bluetooth connections. Previously, being connected via USB caused `hasConnection()` to return true, which masked the Bluetooth PIN and falsely showed `"Connected"` on the Bluetooth screen even when BLE was still unpaired and awaiting connection. The PIN is now correctly displayed on both the main screen and the `BLUETOOTH` screen until an actual Bluetooth client connects.
- **Correct XBM Rendering (`drawXBitmap`)**: Switched `UnitC6LDisplay::drawXbm` to use `drawXBitmap` instead of `drawBitmap`. XBM files store pixel data LSB-first. Using MSB-first `drawBitmap` caused every 8-pixel byte to be mirrored horizontally, resulting in a backwards MeshCore splash logo, an inverted "fish-like" Bluetooth icon, inverted advert waves `)X(`, and an unrecognizable power button. All icons and splash logo are now crisp, upright, and non-mirrored.
- **Action Labels on Icon Screens**: Added intuitive text prompts directly below the 32x32 icons:
  - `BLUETOOTH`: Displays `PIN: <random>` when waiting for pairing, `Connected` when paired, or `BLE: OFF` when disabled.
  - `ADVERT`: Displays `HOLD: SEND` below the broadcast antenna icon.
  - `SHUTDOWN`: Displays `HOLD: OFF` below the standard IEC Power icon, and updates to `Turning off...` while initiating shutdown.
- **Dynamic Random BLE PIN**: Removed static PIN override so the device generates a fresh 6-digit random PIN each session via `StdRNG` (between `100000` and `999999`), clearly shown on both the main screen (`FIRST`) and the `BLUETOOTH` screen for secure, straightforward pairing without hardcoded credentials.
- **OLED Text Wrap & Overwrite Fix**: Disabled text wrap (`display.setTextWrap(false)`) in `UnitC6LDisplay.h`. Adafruit_GFX defaults to `wrap = true`, which caused the 58-character scrolling status bar (`PL-test-m5stack-c6l | 0.00V | ...`) to wrap 6 times down the entire height of the 64x48 OLED every frame, overwriting all screens and tricking `getTextBounds` into thinking the text was static. Text now cleanly scrolls along line 0 (`y = 0..7`) without touching lower rows.
- **64x48 OLED UI Layout Optimization**: Adapted `ui-tiny/UITask.cpp` for the compact 64x48 screen:
  - `FIRST` page: Changed `setTextSize(2)` to `setTextSize(1)` so `MSG: %d`, `PIN:%06d`, and connection status fit within the 64-pixel width without clipping or vertical collisions.
  - `RADIO` page: Reorganized frequency, SF, bandwidth, CR, power, noise floor, and battery voltage into 5 clean 8px lines without horizontal character overlap.
  - `RECENT` page: Scaled vertical line pitch (`y += 9`) so up to 4 recent contact adverts fit cleanly between y=10 and y=44.
- **Splash Screen Isolation**: Excluded status bar rendering while `SplashScreen` is active (`curr == splash`), allowing the MeshCore logo (centered) and build date to display cleanly on boot.
- **Simultaneous USB + BLE Support**: Enabled `-D ENABLE_USB_INTERFACE` in the BLE environment. Both USB cable connection (Web Serial on https://app.meshcore.nz/) and Bluetooth work simultaneously.
- **Dual-Button Navigation & Wake**: Added `UnitC6LButton` to read both the front button (`P0` on PI4IO expander) and the side button (`GPIO 9` BOOT). Pressing either button wakes the screen from sleep and cycles menu pages.
- **OLED USB Power Retention**: Added `-D KEEP_DISPLAY_ON_USB=1` and overridden `isExternalPowered() = true`, preventing the 15-second display auto-off when powered over USB.
- **Clean Flash Tools**: Added `-Erase` switch to `flash-companion.ps1`, `flash-companion-clean.bat`, and `erase-flash.bat` to eliminate leftover flash/SPIFFS data from previous firmware.
- **SX1262 LDO Regulator Mode**: Enabled `-D SX126X_USE_REGULATOR_LDO=true` in `platformio.ini`. The Unit C6L (Stamp C6LoRa) routes SX1262 power through the internal LDO regulator rather than DC-DC; without this, `radio_init()` failed and halted the MCU in `while(1);` on boot.
- **SPI Bus CS Isolation**: Set LoRa NSS (`GPIO 23`) and OLED CS (`GPIO 6`) as `OUTPUT HIGH` before any SPI traffic begins. Prevents the SX1262 from receiving display initialization and drawing bytes over the shared SPI bus (`SCLK=20`, `MOSI=21`, `MISO=22`).
- **SX1262 Reset Pulse Timing**: Implemented clean 100ms active-low reset pulse on expander `P7` (`SX_NRST`) followed by release `HIGH` with LNA (`P5`) and antenna switch (`P6`) enabled.
- **OLED Display Orientation**: Added `display.setRotation(2)` (180° rotation) so that UI text and icons are right-side up when device antennas point upward.
- **USB CDC Configuration**: Added `-D ARDUINO_USB_CDC_ON_BOOT=1` and `-D ARDUINO_USB_MODE=1` to ensure reliable Web Serial and USB log monitoring.
- **Boot Fault Diagnostics**: Added visual fault feedback (RGB LED turns RED on `GPIO 2` and display outputs `"RADIO INIT / FAILED!"` if radio hardware initialization fails).



