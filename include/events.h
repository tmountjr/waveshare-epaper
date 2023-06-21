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
