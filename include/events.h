void get_events(StaticJsonDocument<768> *events)
{
  if (http.begin(client, MICROSERVICE_PATH))
  {
    int httpCode = http.GET();
    if (httpCode == 200)
    {
      DeserializationError error = deserializeJson(*events, http.getStream());
      if (error)
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
      }
    }
  }
}

void parseEvent(
    const JsonObject &jsonObject,
    char eventTime[],
    size_t eventTimeSize,
    char eventTitle[],
    size_t eventTitleSize)
{
  if (!(jsonObject.isNull() || jsonObject.size() == 0))
  {
    const char *t = jsonObject["summary_small"].as<const char *>();
    strncpy(eventTitle, t, eventTitleSize);
    Serial.printf("[parseEvent] eventTitle = %s\n", eventTitle);

    const char *window = jsonObject["timeWindow"].as<const char*>();
    strncpy(eventTime, window, eventTimeSize);
  }
  else
  {
    snprintf(eventTitle, eventTitleSize, "%s", "");
    snprintf(eventTime, eventTimeSize, "%s", ""); // Ensure null termination
  }
}
