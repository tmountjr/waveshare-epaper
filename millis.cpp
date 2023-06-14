#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "GxEPD2_display_selection_new_style.h"

void setup()
{
  // Wipe the screen.
  display.init(115200, true, 2, false);
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);

  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(20, 80);
    display.print("Millis: ");
  } while (display.nextPage());
}

void loop()
{
  display.setPartialWindow(115, 50, 200, 40);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(120, 80);
    display.print(millis());
  } while (display.nextPage());
}
