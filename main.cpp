#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include "GxEPD2_display_selection_new_style.h"

uint8_t base_y = 0;

void setup()
{
  Serial.begin(115200);
  display.init(115200, true, 2, false);
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);
  display.fillScreen(GxEPD_WHITE);
  display.firstPage();
  display.setTextWrap(true);

  // Setup
  int16_t boundX, boundY;
  uint16_t boundW, boundH;
  char helloWorld[] = "Hello World";

  // ***LINE 1***
  display.setFont(&FreeSans12pt7b);
  display.getTextBounds(helloWorld, 0, base_y, &boundX, &boundY, &boundW, &boundH);
  base_y = -boundY;
  display.setCursor(0, base_y);
  display.println(helloWorld);
  display.drawRect(boundX, 0, boundW, boundH, GxEPD_BLACK);
  base_y = base_y + 12; // Since the previous baseline was the bottom, not top, we're already at the lower bound. +12 for the height of the newline.

  // ***LINE 2***
  display.setFont(&FreeSansBold12pt7b);
  display.getTextBounds(helloWorld, 0, base_y, &boundX, &boundY, &boundW, &boundH);
  display.println(helloWorld);
  display.drawRect(boundX, base_y, boundW, boundH, GxEPD_BLACK);
  base_y = base_y + 12 + boundH - 1;

  // ***LINE 3***
  display.setFont(&FreeSans9pt7b);
  char line3[] = "abc...xyz 123...789";
  display.getTextBounds(line3, 0, base_y, &boundX, &boundY, &boundW, &boundH);
  display.println(line3);
  display.drawRect(boundX, base_y, boundW, boundH, GxEPD_BLACK);
  base_y = base_y + 12 + boundH - 1;

  // ***LINE4***
  display.getTextBounds(helloWorld, 0, base_y, &boundX, &boundY, &boundW, &boundH);
  display.println(helloWorld);
  display.drawRect(boundX, base_y, boundW, boundH, GxEPD_BLACK);
  base_y = base_y + 9 + boundH - 1;

  display.nextPage();
}

void loop() {}

// TODO:
/**
 * base_y initial value: 0
 * 
 * First line of text:
 *    1. Get the bounds with the cursor at (0,base_y), then set base_y to the negative boundY.
 *    2. Set the cursor to (0, base_y).
 *    3. display.println()
 *    4. add the font point (eg. 12) to base_y to set the UPPER LEFT of the next line.
 * Next line of text:
 *    1. Get the bounds with the cursor at (0, base_y)
 *    2. display.println()
 *    3. add the font point (eg. 12) plus the boundH minus 1 (ie. base_y + 12 + boundH - 1) to base_y to set the UPPER LEFT of the next line.
 * Next line of text:
 *    
 */