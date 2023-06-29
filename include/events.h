/**
 * Get events from the microservice API.
 * @param events The pointer to the place we're storing events.
 * @returns Whether or not the GET succeeded.
 */
bool get_events(StaticJsonDocument<768> *events)
{
  // Assume it didn't work
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
      } else {
        return true;
      }
    }
  }
  return false;
}
