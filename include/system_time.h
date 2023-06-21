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
      }
    }
    http.end();
  }
  return toReturn;
}
