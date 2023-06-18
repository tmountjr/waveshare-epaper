signed long tz_offset = 0L;
time_t nextUpdateTime = 0;

/**
 * Custom time provider.
 * @returns A unix timestamp.
 */
time_t getApiTime()
{
  time_t toReturn = 0;
  if (http.begin(client, "http://worldtimeapi.org/api/timezone/America/New_York"))
  {
    int httpCode = http.GET();
    if (httpCode == 200)
    {
      DynamicJsonDocument apiResp(2048);
      String resp = http.getString();
      DeserializationError e = deserializeJson(apiResp, resp);
      if (!e)
      {
        tz_offset = apiResp["raw_offset"].as<signed long>(); // eg for new york, might be -18000 = -5h
        time_t unixtime = apiResp["unixtime"].as<time_t>();
        bool is_dst = apiResp["dst"].as<bool>();
        if (is_dst)
        {
          signed int dst_offset = apiResp["dst_offset"].as<signed int>();
          tz_offset = tz_offset + dst_offset;
        }
        toReturn = unixtime + tz_offset;
        nextUpdateTime = toReturn - 1;
        Serial.printf("Setting unix time to %i\n", (int)toReturn);
      }
    }
    http.end();
  }
  return toReturn;
}

time_t getTimeFromString(const char* timeString)
{
  tmElements_t tm;
  int year, month, day, hour, minute, second, tz_hour, tz_minute;
  sscanf(timeString, "%d-%d-%dT%d:%d:%d-%d:%d",
    &year,
    &month,
    &day,
    &hour,
    &minute,
    &second,
    &tz_hour,
    &tz_minute);

  tm.Year = static_cast<uint8_t>(CalendarYrToTm(year));
  tm.Month = static_cast<uint8_t>(month);
  tm.Day = static_cast<uint8_t>(day);
  tm.Hour = static_cast<uint8_t>(hour);
  tm.Minute = static_cast<uint8_t>(minute);
  tm.Second = static_cast<uint8_t>(second);
  return makeTime(tm);
}

void assembleTimeSummary(time_t startTimestamp, time_t endTimestamp, char* summary, size_t summarySize)
{
  tmElements_t startTm;
  tmElements_t endTm;

  breakTime(startTimestamp, startTm);
  breakTime(endTimestamp, endTm);
  char startTime[9], endTime[9];
  sprintf(startTime, "%02d:%02d%c", startTm.Hour % 12, startTm.Minute, (startTm.Hour < 12) ? 'a' : 'p');
  sprintf(endTime, "%02d:%02d%c", endTm.Hour % 12, endTm.Minute, (endTm.Hour < 12) ? 'a' : 'p');
  snprintf(summary, summarySize, "%s-%s", startTime, endTime);
}