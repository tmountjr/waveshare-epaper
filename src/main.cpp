#include <string.h>
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <user_interface.h>
#include <ESP8266HTTPClient.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "secrets.h"
#include "battery.h"

HTTPClient http;
WiFiClient client;

#include "events.h"
#include "GxEPD2_display_selection_new_style.h"
// PINOUT:
// 3V3          = VCC   = grey
// GND          = GND   = brown
// D7 = GPIO 13 = DIN   = blue
// D5 = GPIO 14 = CLK   = yellow
// D8 = GPIO 15 = CS    = orange
// D2 = GPIO 4  = DC    = green
// D1 = GPIO 5  = RST   = white
// D6 = GPIO 12 = BUSY  = purple
//
// OTHER PINS:
// 1k pullup between 3V3 and RST
// 100k * 2 voltage dividers between Vin and GND, going to A0
// jumper between D0/GPIO16 and RST on the board to wake up on a timer

#define BAT_PIN A0
#define ADJUSTMENT 0.25
#define STARTUP_LED_PIN D4

/**
 * Get the voltage from the onboard ADC. Returns the actual voltage.
 */
float voltage()
{
  return (((analogRead(BAT_PIN) * 3.3) / 1024) * 2) + ADJUSTMENT;
}

/**
 * Equivalent of Arduino map() function but for floats.
 */
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

struct
{
  uint8_t startup_count;
  bool clean_start;
} startup_data;

uint32_t sleep_interval = 300e6;                                          // 5 minutes
uint16_t max_starts_between_refresh = (60 * 60) / (sleep_interval / 1e6); // 1 hours

// Define dynamic areas.
struct
{
  uint16_t dateCursorX, dateCursorY, dateWidth, dateHeight;
  uint16_t happeningNowRectX, happeningNowRectY;
  uint16_t upNextRectX, upNextRectY;
  uint16_t eventRectWidth, eventRectHeight;
  int16_t fontBoundY, fontBoundYWithPadding;
  uint16_t batteryX;
  uint16_t batteryY = 1;
  uint8_t batteryW = 16;
  uint8_t batteryH = 8;
  time_t nextScreenUpdate = 0;
} dimensions;

bool dimensions_calculated = false;

void setup()
{
  pinMode(D0, WAKEUP_PULLUP);
  pinMode(BAT_PIN, INPUT);
  pinMode(STARTUP_LED_PIN, OUTPUT);

  // Turn on the ESP12E LED while we boot, bearing in mind it's reversed.
  digitalWrite(STARTUP_LED_PIN, LOW);

  Serial.begin(74880);
  Serial.setTimeout(2000);
  while (!Serial)
  {
  }

  // Read in startup_data
  ESP.rtcUserMemoryRead(0, reinterpret_cast<uint32_t *>(&startup_data), sizeof(startup_data));
  rst_info *r_info = ESP.getResetInfoPtr();

  if ((*r_info).reason == 6 || startup_data.startup_count >= max_starts_between_refresh)
  {
    startup_data.startup_count = 1;
    startup_data.clean_start = true;
  }

  // ***WIFI INIT***
  Serial.print("\nStarting wifi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }
  String local_ip = WiFi.localIP().toString();
  Serial.printf(" complete. IP Address: %s\n", local_ip.c_str());
  // ***END WIFI INIT***

  // Init the display based on startup_data.clean_start
  display.init(74880, startup_data.clean_start, 2, false);
  display.setRotation(1);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(GxEPD_BLACK);

  if (startup_data.clean_start)
  {
    // Only if we're doing a clean start, clear and then redraw the entire display.
    // Once the display has been redrawn and the boundary points calculated, store them in rtc memory.
    display.firstPage();
    int16_t bX, bY;
    uint16_t bW, bH;
    do
    {
      display.fillScreen(GxEPD_WHITE);

      // *** START TODAY IS ***
      char TodayIs[] = "Today is ";
      display.getTextBounds(TodayIs, 0, 0, &bX, &bY, &bW, &bH);
      display.setCursor(bX, -bY + 5);
      display.print(TodayIs);

      // Get some dimensions
      dimensions.fontBoundY = -bY;
      dimensions.fontBoundYWithPadding = dimensions.fontBoundY + 5;
      int today_is_x = display.getCursorX();
      dimensions.dateCursorX = today_is_x;
      dimensions.dateCursorY = -bY + 5;
      dimensions.dateWidth = display.width() - today_is_x - 20; // give room for the battery
      dimensions.dateHeight = bH;
      // *** END TODAY IS ***

      // *** START HORIZONTAL/VERTICAL LINES ***
      int16_t line_y = 25, line_x = (display.width() / 2) - 1;
      for (int i = 0; i < 3; i++)
      {
        display.drawFastHLine(0, line_y, display.width(), GxEPD_BLACK);
        display.drawFastVLine(line_x, 25, display.height() - 25, GxEPD_BLACK);
        line_y += 1;
        line_x += 1;
      }
      // *** END HORIZONTAL/VERTICAL LINES ***

      // Do some resets for the next drawings
      display.setFont(&FreeSansBold9pt7b);
      line_y = 45;
      line_x -= 1;

      // *** START HAPPENING NOW ***
      display.setCursor(0, line_y);
      char HappeningNow[] = "Happening Now:";
      display.print(HappeningNow);
      // *** END HAPPENING NOW ***

      // *** START UP NEXT ***
      uint16_t x_start = line_x + 1;
      char UpNext[] = "Up Next:";
      display.getTextBounds(UpNext, x_start, line_y, &bX, &bY, &bW, &bH);
      // 3/4 point of the screen = display.width() * 0.75
      // text box width = bW
      // starting X point = three-quarter-point - (bW / 2)
      uint16_t text_x = (display.width() * 0.75) - (bW / 2);
      display.setCursor(text_x, line_y);
      display.print(UpNext);
      // *** END UP NEXT ***

      // Last bits of calculation
      /**
       * So we need to calculate the following:
       *   * left rect x is always 5 (or whatever the padding we want is, in this case 5)
       *   * right rect x is midpoint (148) + 1 (extra line) + 5 (padding) + 1 (algebra)
       *   * left rect y and right rect y are always cursor_y + 10 (arbitrary padding to allow for under-baseline stuff)
       *   * width (136) is calculated as...display.width() [296] / 2 [148] - 10 [2x padding, 138] - 1 [extra line, 137] - 1 [algebra, 136]
       *   * height (68) is calculated as...display.height() [128] - (cursor_y + 10) [55, so 73] - 5 [bottom padding, 68]
       */
      int16_t cursor_y = display.getCursorY();
      dimensions.happeningNowRectX = 5;
      dimensions.happeningNowRectY = cursor_y + 12;
      dimensions.upNextRectX = (display.width() / 2) + 7;
      dimensions.upNextRectY = dimensions.happeningNowRectY;
      dimensions.eventRectWidth = (display.width() / 2) - 12;
      dimensions.eventRectHeight = display.height() - (cursor_y + 10) - 5;
    } while (display.nextPage());

    // Persist this data back to rtc memory for later use.
    ESP.rtcUserMemoryWrite(sizeof(startup_data), reinterpret_cast<uint32_t *>(&dimensions), sizeof(dimensions));
    dimensions_calculated = true;
  }

  // *** START BATTERY ***
  dimensions.batteryX = display.width() - 16 - 1;
  const uint8_t *BatteryIcon;
  float battery_voltage = voltage();
  float battery_percent = mapFloat(battery_voltage, 2.8, 4.2, 0, 100);
  Serial.printf("Battery voltage: %.2f; percent: %.2f\n", battery_voltage, battery_percent);
  
  if (battery_percent >= 66) BatteryIcon = BatteryFull;
  else if (battery_percent >= 33) BatteryIcon = BatteryMid;
  else BatteryIcon = BatteryLow;

  display.setPartialWindow(dimensions.batteryX, dimensions.batteryY, dimensions.batteryW, dimensions.batteryH);
  display.firstPage();
  do
  {
    display.drawBitmap(dimensions.batteryX, dimensions.batteryY, BatteryIcon, dimensions.batteryW, dimensions.batteryH, GxEPD_BLACK);
  } while (display.nextPage());
  // *** END BATTERY ***

  // Pull the boundary points if we haven't calculated them already.
  if (!dimensions_calculated)
  {
    ESP.rtcUserMemoryRead(sizeof(startup_data), reinterpret_cast<uint32_t *>(&dimensions), sizeof(dimensions));
  }

  // Connect to the events API, download everything, and draw the updates on every single refresh.
  StaticJsonDocument<768> events;
  get_events(&events);
  JsonArray current = events["current"].as<JsonArray>();
  JsonArray future = events["future"].as<JsonArray>();
  const char *date = events["formattedDate"];
  JsonObject current_object = current[0].as<JsonObject>();
  JsonObject future_object = future[0].as<JsonObject>();

  // *** START DATE PAINT ***
  display.setPartialWindow(dimensions.dateCursorX, 0, dimensions.dateWidth, dimensions.dateHeight);
  display.setFont(&FreeSans9pt7b);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(dimensions.dateCursorX, dimensions.dateCursorY);
    display.print(date);
  } while (display.nextPage());
  // *** END DATE PAINT

  // *** START HAPPENING NOW DATA ***
  display.setPartialWindow(dimensions.happeningNowRectX, dimensions.happeningNowRectY, dimensions.eventRectWidth, dimensions.eventRectHeight);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(dimensions.happeningNowRectX, dimensions.happeningNowRectY + dimensions.fontBoundY);
    display.println(current_object["timeWindow"].as<const char *>());

    uint16_t new_y = display.getCursorY();
    display.setCursor(dimensions.happeningNowRectX, new_y);
    display.println(current_object["summary_small"].as<const char *>());

    new_y = display.getCursorY();
    display.setCursor(dimensions.happeningNowRectX, new_y);
    display.println(current_object["meetingAudience"].as<const char *>());
  } while (display.nextPage());
  // *** END HAPPENING NOW DATA ***

  // *** START UP NEXT DATA ***
  display.setPartialWindow(dimensions.upNextRectX, dimensions.upNextRectY, dimensions.eventRectWidth, dimensions.eventRectHeight);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(dimensions.upNextRectX, dimensions.upNextRectY + dimensions.fontBoundY);
    display.println(future_object["timeWindow"].as<const char *>());

    uint16_t new_y = display.getCursorY();
    display.setCursor(dimensions.upNextRectX, new_y);
    display.println(future_object["summary_small"].as<const char *>());

    new_y = display.getCursorY();
    display.setCursor(dimensions.upNextRectX, new_y);
    display.println(future_object["meetingAudience"].as<const char *>());
  } while (display.nextPage());
  // *** END UP NEXT DATA ***

  // Cleanup - hibernate the paper, then deep sleep the ESP
  // 1. increment startup counter, reset clean start flag, and push startup_data to user memory.
  startup_data.startup_count += 1;
  startup_data.clean_start = false;
  ESP.rtcUserMemoryWrite(0, reinterpret_cast<uint32_t *>(&startup_data), sizeof(startup_data));

  // 2. hibernate the epaper
  display.hibernate();

  // 2.5 turn off the LED (though it should get pulled low when sleep starts?)
  digitalWrite(STARTUP_LED_PIN, HIGH);

  // 3. give the mcu a second to finish cleaning up, then deep sleep.
  delay(1000);
  ESP.deepSleep(sleep_interval);
}

// Nothing in loop since we're not staying awake once we wake up and update the display.
void loop() {}