void get_events(StaticJsonDocument<512> *events)
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
    char *eventTime,
    size_t eventTimeSize,
    char *eventTitle,
    size_t eventTitleSize)
{
  if (!(jsonObject.isNull() || jsonObject.size() == 0))
  {
    const char *startTime = jsonObject["startTime"].as<const char *>();
    const char *endTime = jsonObject["endTime"].as<const char *>();
    const char *originalTitle = jsonObject["summary"].as<const char *>();

    char eventTitleBuffer[eventTitleSize];
    // Truncate or copy the event title based on the maximum size
    if (strlen(originalTitle) <= eventTitleSize)
    {
      strcpy(eventTitleBuffer, originalTitle); // No truncation needed
    }
    else
    {
      strncpy(eventTitleBuffer, originalTitle, eventTitleSize - 1);
      eventTitleBuffer[eventTitleSize - 1] = '\0'; // Ensure null termination
    }

    strcpy(eventTitle, eventTitleBuffer);

    time_t startTimestamp = getTimeFromString(startTime);
    time_t endTimestamp = getTimeFromString(endTime);
    assembleTimeSummary(startTimestamp, endTimestamp, eventTime, eventTimeSize);
  }
  else
  {
    // eventTitle = "";
    strcpy(eventTitle, "");
    snprintf(eventTime, eventTimeSize, "%s", ""); // Ensure null termination
  }
}
