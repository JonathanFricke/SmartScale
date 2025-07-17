#pragma once

#include <Arduino.h>
// #include <WiFiClient.h>

class InternetClient {
public:
  static InternetClient &getInstance();
  void connectWifi(const char *ssid, const char *password);
  bool postWeight(int weight);
  static volatile bool postFinished; // TODO: Replace with callback function
  static volatile bool postSuccess;  // TODO: Replace with callback function
  static bool isPosting;
  static void reset();

private:
  InternetClient();                                           // Private constructor for Singleton
  InternetClient(const InternetClient &) = delete;            // Prevent copy constructor (Renderer x = y will not work)
  InternetClient &operator=(const InternetClient &) = delete; // Prevent assignment (x = y will not work)

  static void postWeightTask(void *param);

  String getTimestampUTC();
  bool isWifiConneting = false;
};
