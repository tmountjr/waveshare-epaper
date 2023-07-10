/**
 * Get events from the microservice API.
 * @param events The pointer to the place we're storing events.
 * @returns Whether or not the GET succeeded.
 */
bool get_events(StaticJsonDocument<768> *events, float voltage)
{
  char finalUrl[46], v[5];
  strcpy(finalUrl, MICROSERVICE_PATH);
  sprintf(v, "%.2f", voltage);
  strcat(finalUrl, v);

  // Assume it didn't work
  if (http.begin(client, finalUrl))
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
