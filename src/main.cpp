#include <Arduino.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <GxEPD2_BW.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "GxEPD2_display_selection_new_style.h"

HTTPClient http;
WiFiClient client;

#include "secrets.h"
#include "system_time.h"
#include "events.h"

// 0x0000 = black
// 0xFFFF = white

// Define dynamic areas.
uint16_t dateCursorX, dateCursorY, dateWidth, dateHeight;
uint16_t happeningNowRectX, happeningNowRectY;
uint16_t upNextRectX, upNextRectY;
uint16_t eventRectWidth, eventRectHeight;
int16_t fontBoundY, fontBoundYWithPadding;
time_t nextScreenUpdate = 0;

void setup()
{
  Serial.begin(115200);
  display.init(115200, true, 2, false);
  display.setRotation(1);
  display.setTextColor(0x0000);
  display.firstPage();

  Serial.print("\nStarting wifi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  String localIp = WiFi.localIP().toString();
  Serial.printf(" complete. IP Address: %s\n", localIp.c_str());

  setSyncProvider(&getApiTime);

  // Bootstrap the static part of the screen
  do {
    display.fillScreen(0xFFFF);
    display.setFont(&FreeSans9pt7b);
    int16_t boundX, boundY;
    uint16_t boundW, boundH;

    char TodayIs[] = "Today is ";
    display.getTextBounds(TodayIs, 0, 0, &boundX, &boundY, &boundW, &boundH);
    // Save these values for later - we'll need them on the events areas.
    fontBoundY = -boundY;
    fontBoundYWithPadding = fontBoundY + 5;

    display.setCursor(0, -boundY + 5);
    display.print(TodayIs);
    int today_is_x = display.getCursorX();
    // The (x,y) cursor is at the bottom left of where the next bit of text would appear.
    // X is correct but Y needs to set to -boundY + 5
    // display.drawRect(today_is_x, 5, display.width() - today_is_x - 1, boundH, 0x0000);
    dateCursorX = today_is_x;
    dateCursorY = -boundY + 5;
    dateWidth = display.width() - today_is_x - 1;
    dateHeight = boundH;

    // Draw a 3px horizontal line and a 3px vertical line
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
    happeningNowRectX = 5;
    happeningNowRectY = cursor_y + 12;
    upNextRectX = (display.width() / 2) + 7;
    upNextRectY = cursor_y + 12;
    eventRectWidth = (display.width() / 2) - 12;
    eventRectHeight = display.height() - (cursor_y + 10) - 5;
  } while (display.nextPage());
}

void loop()
{
  time_t rightNow = now(); // This will handle updating the NTP client every 5m by default
  if (rightNow >= nextScreenUpdate)
  {
    char date[11];
    sprintf(date, "%02d/%02d/%04d", month(), day(), year());
    display.setCursor(dateCursorX, dateCursorY);
    display.setFont(&FreeSans9pt7b);

    // Set Date
    display.setPartialWindow(dateCursorX, 0, dateWidth, dateHeight);
    display.firstPage();
    do {
      display.fillScreen(0xFFFF);
      display.print(date);
    } while (display.nextPage());

    // Fetch events
    StaticJsonDocument<512> events;
    get_events(&events);
    JsonArray current = events["current"].as<JsonArray>();
    JsonArray future = events["future"].as<JsonArray>();

    char current_event_time[20];
    char future_event_time[20];
    char current_event_title[15];
    char future_event_title[15];

    JsonObject current_object = current[0].as<JsonObject>();
    parseEvent(current_object, current_event_time, sizeof(current_event_time), current_event_title, sizeof(current_event_title));

    JsonObject future_object = future[0].as<JsonObject>();
    parseEvent(future_object, future_event_time, sizeof(future_event_time), future_event_title, sizeof(future_event_title));

    // Show "Happening Now"
    display.setPartialWindow(happeningNowRectX, happeningNowRectY, eventRectWidth, eventRectHeight);
    display.firstPage();
    do {
      display.fillScreen(0xFFFF);
      display.setCursor(happeningNowRectX, happeningNowRectY + fontBoundY);
      display.println(current_event_time);
      uint16_t new_y = display.getCursorY();
      display.setCursor(happeningNowRectX, new_y);
      display.println(current_event_title);
    } while (display.nextPage());

    // Show "Up Next"
    display.setPartialWindow(upNextRectX, upNextRectY, eventRectWidth, eventRectHeight);
    display.firstPage();
    do {
      display.fillScreen(0xFFFF);
      display.setCursor(upNextRectX, upNextRectY + fontBoundY);
      display.println(future_event_time);
      uint16_t new_y = display.getCursorY();
      display.setCursor(upNextRectX, new_y);
      display.println(future_event_title);
    } while (display.nextPage());

    // At the end, get a new now() and set a 5m update window
    nextScreenUpdate = now() + 300;
    Serial.printf("Next update at %lli\n", nextScreenUpdate);
    display.hibernate();
  }
}