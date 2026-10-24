#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <helpers/ui/DisplayDriver.h>
#include "target.h"

// Color scheme for monochrome OLED display (SSD1306)
ColorVal UIColor::window_bkg = 0;   // Black
ColorVal UIColor::title_bkg = 0;    // Black
ColorVal UIColor::title_txt = 1;    // White
ColorVal UIColor::primary_txt = 1;  // White
ColorVal UIColor::secondary_txt = 1;// White
ColorVal UIColor::warning_txt = 1;  // White
ColorVal UIColor::popup_bkg = 0;    // Black
ColorVal UIColor::popup_txt = 1;    // White
ColorVal UIColor::corp_blue = 1;    // White

// PI4IOE5V6408 registers & address
#define PI4IO_M_ADDR 0x43
#define PI4IO_REG_CHIP_RESET 0x01
#define PI4IO_REG_IO_DIR 0x03
#define PI4IO_REG_OUT_SET 0x05
#define PI4IO_REG_OUT_H_IM 0x07
#define PI4IO_REG_IN_DEF_STA 0x09
#define PI4IO_REG_PULL_EN 0x0B
#define PI4IO_REG_PULL_SEL 0x0D
#define PI4IO_REG_IN_STA 0x0F
#define PI4IO_REG_INT_MASK 0x11
#define PI4IO_REG_IRQ_STA 0x13

static void pi4io_write(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(PI4IO_M_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

static uint8_t pi4io_read(uint8_t reg) {
  Wire.beginTransmission(PI4IO_M_ADDR);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)PI4IO_M_ADDR, (uint8_t)1);
  return Wire.available() ? Wire.read() : 0;
}

#if defined(P_LORA_SCLK)
  static SPIClass spi(0);
#endif

void UnitC6LBoard::begin() {
  ESP32Board::begin();

  // Set SPI chip selects HIGH before any SPI transactions occur to prevent bus contention
  pinMode(P_LORA_NSS, OUTPUT);
  digitalWrite(P_LORA_NSS, HIGH); // LoRa SX1262 CS (GPIO 23)
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);          // OLED SSD1306 CS (GPIO 6)

  // Release OLED Display from hardware reset (GPIO 15)
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);

  // Turn off NeoPixel LED on GPIO 2
  neopixelWrite(2, 0, 0, 0);

  // Onboard I2C is GPIO 10 (SDA), GPIO 8 (SCL)
  Wire.begin(10, 8);

  // Initialize PI4IOE5V6408 expander
  pi4io_write(PI4IO_REG_CHIP_RESET, 0xFF);
  delay(10);
  // P5 (SX_LNA_EN), P6 (SX_ANT_SW), P7 (SX_NRST) as outputs (1); P0..P4 as inputs (0)
  pi4io_write(PI4IO_REG_IO_DIR, 0b11100000);
  delay(10);
  pi4io_write(PI4IO_REG_OUT_H_IM, 0b00011100);
  delay(10);
  pi4io_write(PI4IO_REG_PULL_SEL, 0b11100011);
  delay(10);
  pi4io_write(PI4IO_REG_PULL_EN, 0b11100011);
  delay(10);
  pi4io_write(PI4IO_REG_IN_DEF_STA, 0b00000011);
  delay(10);
  pi4io_write(PI4IO_REG_INT_MASK, 0b11111100);
  delay(10);

  // Clean hardware reset of SX1262: hold P7 LOW for 100ms, then release HIGH with LNA & ANT_SW active
  pi4io_write(PI4IO_REG_OUT_SET, 0b01100000); // P7 LOW (in reset), P6=1 (ANT_SW), P5=1 (LNA_EN)
  delay(100);
  pi4io_write(PI4IO_REG_OUT_SET, 0b11100000); // P7 HIGH (release reset), P6=1, P5=1
  delay(50);
  pi4io_read(PI4IO_REG_IRQ_STA); // clear any pending IRQ

  // Initialize shared SPI pins early so display and radio both have bus access
#if defined(P_LORA_SCLK)
  spi.begin(P_LORA_SCLK, P_LORA_MISO, P_LORA_MOSI);
#endif

  // Starting audio beep on Buzzer (GPIO 11)
#ifdef PIN_BUZZER
  pinMode(PIN_BUZZER, OUTPUT);
  tone(PIN_BUZZER, 2000, 80);
  delay(100);
  tone(PIN_BUZZER, 3000, 120);
#endif
}

UnitC6LBoard board;

#if defined(P_LORA_SCLK)
  RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, spi);
#else
  RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY);
#endif

WRAPPER_CLASS radio_driver(radio, board);

ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);
SensorManager sensors;

#ifdef DISPLAY_CLASS
  UnitC6LDisplay display(&spi);

  class UnitC6LButton : public MomentaryButton {
    unsigned long _down_at;
    bool _prev_pressed;
    bool _pending_click;
    unsigned long _last_click_time;
    uint8_t _click_count;

  public:
    UnitC6LButton() : MomentaryButton(9, 1000, true, true),
      _down_at(0), _prev_pressed(false), _pending_click(false),
      _last_click_time(0), _click_count(0) {}

    void begin() override {
      pinMode(9, INPUT_PULLUP);
      pinMode(3, INPUT_PULLUP);
    }

    bool isPressed() const {
      if (digitalRead(9) == LOW) return true;
      static uint32_t lastCheck = 0;
      static bool p0_down = false;
      uint32_t now = millis();
      if (now - lastCheck >= 20) {
        lastCheck = now;
        uint8_t sta = pi4io_read(PI4IO_REG_IN_STA);
        p0_down = ((sta & 0x01) == 0);
      }
      return p0_down;
    }

    int check(bool repeat_click = false) override {
      bool pressed = isPressed();
      int event = BUTTON_EVENT_NONE;
      uint32_t now = millis();

      if (pressed != _prev_pressed) {
        if (pressed) {
          _down_at = now;
        } else {
          if (_down_at > 0 && (now - _down_at < 1000)) {
            _click_count++;
            _last_click_time = now;
            _pending_click = true;
          }
          _down_at = 0;
        }
        _prev_pressed = pressed;
      }

      if (_down_at > 0 && (now - _down_at >= 1000)) {
        event = BUTTON_EVENT_LONG_PRESS;
        _down_at = 0;
        _click_count = 0;
        _pending_click = false;
      } else if (_pending_click && (now - _last_click_time >= 280)) {
        if (_click_count == 1) event = BUTTON_EVENT_CLICK;
        else if (_click_count == 2) event = BUTTON_EVENT_DOUBLE_CLICK;
        else if (_click_count >= 3) event = BUTTON_EVENT_TRIPLE_CLICK;
        _click_count = 0;
        _pending_click = false;
      }
      return event;
    }
  };

  static UnitC6LButton _impl_user_btn;
  MomentaryButton& user_btn = _impl_user_btn;
#endif

bool radio_init() {
  fallback_clock.begin();
  rtc_clock.begin(Wire);

#if defined(P_LORA_SCLK)
  spi.begin(P_LORA_SCLK, P_LORA_MISO, P_LORA_MOSI);
  bool ok = radio.std_init(&spi);
#else
  bool ok = radio.std_init();
#endif

  if (!ok) {
    neopixelWrite(2, 64, 0, 0); // Red LED alert to indicate radio init failure
#ifdef DISPLAY_CLASS
    display.startFrame();
    display.setTextSize(1);
    display.drawTextCentered(display.width() / 2, 12, "RADIO INIT");
    display.drawTextCentered(display.width() / 2, 28, "FAILED!");
    display.endFrame();
#endif
  }
  return ok;
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);
}
