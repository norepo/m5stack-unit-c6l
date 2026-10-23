# Dual-Boot OTA Switching Guide

The ESP32-C6 partition layout (`min_spiffs.csv`) allocates two symmetric 1,920 KB OTA application slots:
- **`ota_0`**: Offset `0x10000` (Default / Primary slot)
- **`ota_1`**: Offset `0x1F0000` (Secondary slot)

This allows running **both Companion Radio (BLE)** and **Repeater** on the same device without re-flashing!

---

## Partition Map Layout

```
# Name,   Type, SubType, Offset,   Size,     Flags
nvs,      data, nvs,     0x9000,   0x5000,
otadata,  data, ota,     0xe000,   0x2000,
app0,     app,  ota_0,   0x10000,  0x1E0000,
app1,     app,  ota_1,   0x1F0000, 0x1E0000,
spiffs,   data, spiffs,  0x3D0000, 0x20000,
coredump, data, coredump,0x3F0000, 0x10000,
```

---

## Setup Steps

1. **Flash Companion Radio to Slot 0**:
   ```powershell
   .\scripts\flash-companion.ps1
   ```
2. **Flash Repeater to Slot 1**:
   ```powershell
   .\scripts\flash-repeater.ps1 -SecondarySlot
   ```

---

## Switching Between Roles
When powering on or pressing reset:
- **Default Boot**: Boots into the active partition (Companion Radio).
- **Hold BOOT Button**: Holding the BOOT button for >500 ms during power-on / reset toggles between `ota_0` and `ota_1`.
