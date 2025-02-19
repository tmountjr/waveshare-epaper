/**
 * Get events from the microservice API.
 * @param events The pointer to the place we're storing events.
 * @returns Whether or not the GET succeeded.
 */
bool get_events(StaticJsonDocument<768> *events, float voltage, const char* ip_addr)
{
  const int maxUrlLength = 200;
  char finalUrl[maxUrlLength];
  char baseUrl[100] = "";
  strcat(baseUrl, "http://");
  strcat(baseUrl, ip_addr);
  strcat(baseUrl, ":8080/events?voltage=");

  char v[10];
  dtostrf(voltage, 1, 2, v);

  int len = snprintf(finalUrl, maxUrlLength, "%s%s", baseUrl, v);
  if (len >= maxUrlLength) {
    Serial.println("URL too long");
    return false;
  }

  String finalUrlString = String(finalUrl);

  // Assume it didn't work
  if (http.begin(client, finalUrlString))
  {
    http.setTimeout(30000);
    unsigned long ms_start = millis();
    int httpCode = http.GET();
    unsigned long ms_end = millis();
    Serial.print("Request time: ");
    Serial.print(ms_end - ms_start);
    Serial.println("ms");
    if (httpCode == HTTPC_ERROR_READ_TIMEOUT)
    {
      Serial.print("HTTP Read Timeout");
      return false;
    }
    else if (httpCode == 200)
    {
      DeserializationError error = deserializeJson(*events, http.getStream());
      if (error)
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
      } else {
        return true;
      }
    } else {
      Serial.print("HTTP Connection error: ");
      Serial.println(httpCode);
      return false;
    }
  }
  return false;
}
