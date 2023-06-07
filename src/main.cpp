#include <Arduino.h>
#include <GxEPD2_BW.h>
// #include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSerifBoldItalic24pt7b.h>
#include "GxEPD2_display_selection_new_style.h"
// Changed the include to use the correct pins: CS=D8, DC=D2, RST=D1, BUSY=D0
// DIN=D7 (HMOSI), CLK=D5 (HSCLK)

const char HelloWorld[] = "FOOD";

void helloWorld()
{
  display.setRotation(1);
  display.setFont(&FreeSerifBoldItalic24pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(HelloWorld);
  } while (display.nextPage());
}

void setup() {
  Serial.begin(115200);
  display.init(115200, true, 2, false);
  Serial.println("Init display");
  helloWorld();
  Serial.println("Hello World");
  display.hibernate();
  Serial.println("Hibernate");
}

void loop() {
}
