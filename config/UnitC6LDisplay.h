#pragma once

#include <helpers/ui/DisplayDriver.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#define SSD1306_NO_SPLASH
#include <Adafruit_SSD1306.h>

#define OLED_CS    6
#define OLED_DC    18
#define OLED_RST   15
#define OLED_WIDTH 64
#define OLED_HEIGHT 48

class UnitC6LDisplay : public DisplayDriver {
  Adafruit_SSD1306 display;
  bool _isOn;
  uint8_t _color;

public:
  UnitC6LDisplay(SPIClass* spi) : DisplayDriver(OLED_WIDTH, OLED_HEIGHT),
      display(OLED_WIDTH, OLED_HEIGHT, spi, OLED_DC, OLED_RST, OLED_CS)
  {
    _isOn = false;
    _color = SSD1306_WHITE;
  }

  bool begin() {
    _isOn = true;
    // periphBegin = false because SPIClass::begin() is already called
    bool ok = display.begin(SSD1306_SWITCHCAPVCC, 0, true, false);
    if (ok) {
      display.setRotation(2); // Flip 180 deg so screen is right-side up with antenna pointing up
      display.ssd1306_command(SSD1306_SETCOMPINS);
      display.ssd1306_command(0x12);
      display.ssd1306_command(SSD1306_SETCONTRAST);
      display.ssd1306_command(0xCF);
      display.clearDisplay();
      display.display();
    }
    return ok;
  }

  bool isOn() override { return _isOn; }

  void turnOn() override {
    _isOn = true;
    display.ssd1306_command(SSD1306_DISPLAYON);
  }

  void turnOff() override {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    _isOn = false;
  }

  void clear() override {
    display.clearDisplay();
    display.display();
  }

  void startFrame(ColorVal bkg = UIColor::window_bkg) override {
    display.clearDisplay();
    _color = SSD1306_WHITE;
    display.setTextColor(_color);
    display.setTextSize(1);
    display.cp437(true);
  }

  void setTextSize(int sz) override {
    display.setTextSize(sz);
  }

  void setColor(ColorVal c) override {
    _color = c;
    display.setTextColor(_color);
  }

  void setCursor(int x, int y) override {
    display.setCursor(x, y);
  }

  void print(const char* str) override {
    display.print(str);
  }

  void fillRect(int x, int y, int w, int h) override {
    display.fillRect(x, y, w, h, _color);
  }

  void drawRect(int x, int y, int w, int h) override {
    display.drawRect(x, y, w, h, _color);
  }

  void drawXbm(int x, int y, const uint8_t* bits, int w, int h) override {
    display.drawBitmap(x, y, bits, w, h, _color);
  }

  uint16_t getTextWidth(const char* str) override {
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    return w;
  }

  void endFrame() override {
    display.display();
  }
};
