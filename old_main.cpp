#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "GxEPD2_display_selection_new_style.h"
// Changed the include to use the correct pins: CS=D8, DC=D2, RST=D1, BUSY=D0
// DIN=D7 (HMOSI), CLK=D5 (HSCLK)

const char HelloWorld[] = "FOOD";

void helloWorld()
{
  display.setRotation(1);
  display.setFont(&FreeSans9pt7b);
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

uint16_t base_y = 0;

void headline() {
  char dateStr[11];
  sprintf(dateStr, "%02d/%02d/%04d", 6, 14, 2023);
  char headline[] = "Today is ";
  char headlineWithDate[sizeof(headline) + sizeof(dateStr)];
  strcpy(headlineWithDate, headline);
  strcat(headlineWithDate, dateStr);

  Serial.println(headlineWithDate);

  // Print at 0,0
  // Cursor is bottom left, not top left, so if the top left is 0x0, subtract tbx and tby to get the bottom left
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(headlineWithDate, 0, base_y, &tbx, &tby, &tbw, &tbh);
  base_y = base_y + -tby;

  display.setCursor(0, base_y);
  display.print(headlineWithDate);

  // Draw a 3px thick line 10px below the baseline of the text.
  base_y = base_y + 10;
  for (uint8_t i = 0; i < 3; i++) {
    display.drawFastHLine(0, base_y, display.width() - 1, GxEPD_BLACK);
    base_y = base_y + 1;
  }

  Serial.printf("New base y: %d\n", base_y);
}

void happeningNow() {
  base_y = base_y + 10;  // top padding
  Serial.printf("Initial Base Y from happeningNow(): %d\n", base_y);
  char HappeningNow[] = "Happening Now:";
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.setFont(&FreeSansBold9pt7b);
  display.getTextBounds(HappeningNow, 0, base_y, &tbx, &tby, &tbw, &tbh);
  base_y = base_y + tby;
  display.setCursor(0, base_y);
  display.print(HappeningNow);

  base_y = base_y + 5;
  char event[] = "11am-12pm: FGT";
  display.setFont(&FreeSans9pt7b);
  display.getTextBounds(event, 0, base_y, &tbx, &tby, &tbw, &tbh);
  base_y = base_y + tby;
  display.setCursor(0, base_y);
  display.print(event);
}


void setup() {
  Serial.begin(115200);
  display.init(115200, true, 2, false);
  display.setRotation(1);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.fillScreen(GxEPD_WHITE);
  display.firstPage();

  // Show the headline
  Serial.println("headline");
  headline();
  happeningNow();
  display.nextPage();
  display.hibernate();
}

void loop() {
}
