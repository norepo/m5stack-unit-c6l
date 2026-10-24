# M5Stack Unit C6L Hardware & Pinout Reference

The **M5Stack Unit C6L** integrates Espressif's **ESP32-C6** MCU (single-core 32-bit RISC-V at 160MHz) with Semtech's **SX1262** LoRa transceiver, a 0.66" monochrome SSD1306 SPI OLED display, and a PI4IOE5V6408 I2C I/O expander.

---

## Technical Specifications

| Parameter | Specification |
|---|---|
| **MCU** | ESP32-C6FH4 (RISC-V 160 MHz, 16 MB Flash, 512 KB SRAM) |
| **LoRa RF** | Semtech SX1262 (+22 dBm TX power, -148 dBm RX sensitivity) |
| **Frequency** | 868 ~ 923 MHz |
| **LNA** | SGM13005L4 Low Noise Amplifier (Enabled via PI4IO E0.P5) |
| **RF Switch** | Antenna Switch (Controlled via PI4IO E0.P6 & SX1262 DIO2) |
| **I/O Expander** | PI4IOE5V6408 @ I2C 0x43 (SDA: GPIO 10, SCL: GPIO 8) |
| **Display** | 0.66-inch SSD1306 SPI OLED (64 x 48 resolution) |
| **LED** | 1x WS2812C RGB LED (GPIO 2) |
| **Buzzer** | Passive Buzzer (GPIO 11) |
| **Buttons** | Side BOOT Button (GPIO 9), Front Key (PI4IO P0) |
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
| RESET | Expander E0.P7 | Controlled via PI4IOE5V6408 I/O Expander |
| RF Switch | Expander E0.P6 | SX_ANT_SW enable |
| LNA Enable | Expander E0.P5 | SX_LNA_EN enable |
| TCXO Voltage | 3.0V | Driven by SX1262 DIO3 |

### OLED Display (0.66" SSD1306 64x48 SPI)
| Function | GPIO | Notes |
|---|---|---|
| SPI SCLK | GPIO 20 | Shared SPI bus |
| SPI MOSI | GPIO 21 | Shared SPI bus |
| CS | GPIO 6 | Display Chip Select |
| DC / RS | GPIO 18 | Data / Command Select |
| RST | GPIO 15 | Hardware Reset (Must be held HIGH in operation) |

### Onboard Peripherals & I/O Expander (PI4IOE5V6408 @ 0x43)
| Function | GPIO | Notes |
|---|---|---|
| I2C SDA | GPIO 10 | Onboard bus for PI4IOE expander |
| I2C SCL | GPIO 8 | Onboard bus for PI4IOE expander |
| Expander INT | GPIO 3 | Interrupt line from expander |
| Front Button | Expander P0 | SYS_KEY1 input (Active LOW) |
| BOOT Button | GPIO 9 | Native MCU Button (Active LOW) |
| RGB LED | GPIO 2 | WS2812C data line |
| Buzzer | GPIO 11 | Alert tone generator |

### External Expansion (Grove HY2.0-4P Port)
| Function | GPIO | Notes |
|---|---|---|
| Grove TX | GPIO 16 | External Serial / GPS |
| Grove RX | GPIO 17 | External Serial / GPS |
