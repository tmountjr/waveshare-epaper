#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include "GxEPD2_display_selection_new_style.h"

// 0x0000 = black
// 0xFFFF = white

void setup()
{
  Serial.begin(115200);
  display.init(115200, true, 2, false);
  display.setRotation(1);
  display.setTextColor(0x0000);
  display.firstPage();
  do {
    // Just write out shit hardcoded for now
    display.fillScreen(0xFFFF);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(0, 17);
    display.println("Today is 06/14/2023");
    int16_t line_y = 25;
    int16_t line_x = (display.width() / 2) - 1;
    for (int i = 0; i < 3; i++)
    {
      display.drawFastHLine(0, line_y, display.width(), 0x0000);
      display.drawFastVLine(line_x, 25, display.height() - 25, 0x0000);
      line_y += 1;
      line_x += 1;
    }
    
    line_y = 45;
    int16_t boundX, boundY;
    uint16_t boundW, boundH;

    // Current
    display.setCursor(0, line_y);
    display.setFont(&FreeSansBold9pt7b);
    display.print("Happening Now:");

    // Next
    uint16_t x_start = line_x + 1; // because exact midpoint is a line, and another line next to it (+1), and then things start (+1)
    Serial.printf("x_start: %d\n", x_start);
    char UpNext[] = "Up Next:";
    display.getTextBounds(UpNext, x_start, 45, &boundX, &boundY, &boundW, &boundH);
    // 3/4 point of the screen = display.width() * 0.75
    // text box width = boundW
    // starting X point = three-quarter-point - (boundW / 2)
    uint16_t text_x = (display.width() * 0.75) - (boundW / 2);
    display.setCursor(text_x, 45);
    display.print("Up Next:");
    
    int16_t cursor_x = display.getCursorX();
    int16_t cursor_y = display.getCursorY();
    Serial.printf("cursorX: %d, cursor_y: %d\n", cursor_x, cursor_y);

    // Draw some rectangles to define dynamic areas
    display.drawRect(5, cursor_y + 10, 136, 68, 0x0000);
    display.drawRect(154, cursor_y + 10, 136, 68, 0x0000);

    // TODO: remove date and add in box outlining date area
  } while (display.nextPage());

}



void loop()
{

}