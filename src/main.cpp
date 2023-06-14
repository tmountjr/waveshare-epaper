#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include "GxEPD2_display_selection_new_style.h"

// 0x0000 = black
// 0xFFFF = white

// Define dynamic areas.
uint16_t dateCursorX, dateCursorY;
uint16_t happeningNowRectX, happeningNowRectY;
uint16_t upNextRectX, upNextRectY;
uint16_t eventRectWidth, eventRectHeight;

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
    int16_t boundX, boundY;
    uint16_t boundW, boundH;

    char TodayIs[] = "Today is ";
    display.getTextBounds(TodayIs, 0, 0, &boundX, &boundY, &boundW, &boundH);

    display.setCursor(0, -boundY + 5);
    display.print(TodayIs);
    int today_is_x = display.getCursorX();
    // The (x,y) cursor is at the bottom left of where the next bit of text would appear.
    // X is correct but Y needs to set to -boundY + 5
    // display.drawRect(today_is_x, 5, display.width() - today_is_x - 1, boundH, 0x0000);
    dateCursorX = today_is_x;
    dateCursorY = -boundY + 5;

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
    line_x -= 1;

    // Current
    display.setCursor(0, line_y);
    display.setFont(&FreeSansBold9pt7b);
    display.print("Happening Now:");

    // Next
    uint16_t x_start = line_x + 1; // because exact midpoint is a line, and another line next to it (+1), and then things start (+1)
    char UpNext[] = "Up Next:";
    display.getTextBounds(UpNext, x_start, 45, &boundX, &boundY, &boundW, &boundH);
    // 3/4 point of the screen = display.width() * 0.75
    // text box width = boundW
    // starting X point = three-quarter-point - (boundW / 2)
    uint16_t text_x = (display.width() * 0.75) - (boundW / 2);
    display.setCursor(text_x, 45);
    display.print("Up Next:");
    
    int16_t cursor_y = display.getCursorY();

    // Draw some rectangles to define dynamic areas
    /**
     * So we need to calculate the following:
     *   * left rect x is always 5 (or whatever the padding we want is, in this case 5)
     *   * right rect x is midpoint (148) + 1 (extra line) + 5 (padding) + 1 (algebra)
     *   * left rect y and right rect y are always cursor_y + 10 (arbitrary padding to allow for under-baseline stuff)
     *   * width (136) is calculated as...display.width() [296] / 2 [148] - 10 [2x padding, 138] - 1 [extra line, 137] - 1 [algebra, 136]
     *   * height (68) is calculated as...display.height() [128] - (cursor_y + 10) [55, so 73] - 5 [bottom padding, 68]
     */
    // int16_t rect_width = (display.width() / 2) - 12;
    // int16_t rect_height = display.height() - (cursor_y + 10) - 5;
    // int16_t right_rect_x = (display.width() / 2) + 7;
    // display.drawRect(5, cursor_y + 10, rect_width, rect_height, 0x0000);  // left rectangle
    // display.drawRect(right_rect_x, cursor_y + 10, rect_width, rect_height, 0x0000); // right rectangle
    happeningNowRectX = 5;
    happeningNowRectY = cursor_y + 10;
    upNextRectX = (display.width() / 2) + 7;
    upNextRectY = cursor_y + 10;
    eventRectWidth = (display.width() / 2) - 12;
    eventRectHeight = display.height() - (cursor_y + 10) - 5;
  } while (display.nextPage());

}

void loop()
{

}