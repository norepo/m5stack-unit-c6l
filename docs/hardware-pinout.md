# M5Stack Unit C6L Hardware & Pinout Reference

The **M5Stack Unit C6L** integrates Espressif's **ESP32-C6** MCU (single-core 32-bit RISC-V at 160MHz) with Semtech's **SX1262** LoRa transceiver, a 0.66" monochrome SSD1306 OLED display, and an I2C I/O expander.

---

## Technical Specifications

| Parameter | Specification |
|---|---|
| **MCU** | ESP32-C6FH4 (RISC-V 160 MHz, 16 MB Flash, 512 KB SRAM) |
| **LoRa RF** | Semtech SX1262 (+22 dBm TX power, -148 dBm RX sensitivity) |
| **Frequency** | 868 ~ 923 MHz |
| **LNA** | SGM13005L4 Low Noise Amplifier |
| **I/O Expander** | PI4IOE5V6408 (controls radio reset, LNA enable, antenna switch) |
| **Display** | 0.66-inch SSD1306 OLED (64 x 48 resolution) |
| **LED** | WS2812C RGB LED (GPIO 15) |
| **Buzzer** | Passive Buzzer (GPIO 11) |
| **Flash Mode** | **DIO** (Do not use QIO; QIO causes boot crash loops) |

---

## GPIO Pin Mapping

### LoRa Radio (Semtech SX1262)
| Function | GPIO | Notes |
|---|---|---|
| SPI SCLK | GPIO 20 | Shared SPI bus |
| SPI MISO | GPIO 22 | Shared SPI bus |
| SPI MOSI | GPIO 21 | Shared SPI bus |
| NSS / CS | GPIO 23 | Radio Chip Select |
| DIO1 | GPIO 7 | Radio Interrupt |
| BUSY | GPIO 19 | Radio Busy line |
| RXEN | GPIO 5 | RX Enable |
| RESET | -1 | Controlled via PI4IOE5V6408 I/O Expander |
| RF Switch | DIO2 | Controlled automatically by SX1262 DIO2 |
| TCXO Voltage | 1.8V | Driven by SX1262 DIO3 |

### Peripherals
| Function | GPIO | Notes |
|---|---|---|
| I2C SDA | GPIO 16 | Onboard bus for Expander & OLED |
| I2C SCL | GPIO 17 | Onboard bus for Expander & OLED |
| RGB LED | GPIO 15 | Status / TX indicator |
| Buzzer | GPIO 11 | Alert tone generator |
| GPS RX | GPIO 4 | Optional external GPS |
| GPS TX | GPIO 5 | Optional external GPS |
