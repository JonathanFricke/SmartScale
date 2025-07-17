#include "internetClient.h"
#include "config.h"
#include "secret.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include "SSLClient.h"
#include <ArduinoHttpClient.h>

// Layers stack
static WiFiClient wifi_transpor_layer;
// static SSLClient secure_presentation_layer(&wifi_transpor_layer);
// static HttpClient httpClient = HttpClient(secure_presentation_layer, SERVER_IP, HTTPS_PORT);

InternetClient::InternetClient() {}

volatile bool InternetClient::postSuccess = false;  // Used as return for async postWeightTask (Not perfect)
volatile bool InternetClient::postFinished = false; // Used as return for async postWeightTask (Not perfect)
bool InternetClient::isPosting = false;

void InternetClient::reset() {
  InternetClient::postSuccess = false;
  InternetClient::postFinished = false;
  InternetClient::isPosting = false;
}

InternetClient &InternetClient::getInstance() {
  static InternetClient instance;
  return instance;
}

void InternetClient::connectWifi(const char *ssid, const char *password) {
  static unsigned long previousAttemptTime = 0;

  if (WiFi.status() != WL_CONNECTED) {
    if (!isWifiConneting) {
      WiFi.begin(ssid, password);
      Serial.print("Connecting to WiFi");
      isWifiConneting = true;
    }
    if (millis() - previousAttemptTime > WIFI_RETRY_DELAY) {
      previousAttemptTime = millis();
      Serial.print(".");
    }
  } else {
    if (isWifiConneting) {
      Serial.println("\nConnected to WiFi");
      isWifiConneting = false;
    }
  }
}

bool InternetClient::postWeight(int weight) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("POST wifi not connected!");
    return false;
  }

  if (InternetClient::isPosting)
    return false;
  InternetClient::isPosting = true;
  Serial.println("POST");

  // Allocate the weight on the heap so it survives outside this function
  int *weightPtr = new int(weight);

  xTaskCreatePinnedToCore(postWeightTask,   // Task function
                          "PostWeightTask", // Name
                          4096,             // Stack size in bytes
                          weightPtr,        // Parameter passed to task
                          1,                // Priority (1 is low; higher = more important)
                          NULL,             // Task handle (we don't need it)
                          0                 // Core 0;
  );

  return false;
}

void InternetClient::postWeightTask(void *param) {
  int weight = *((int *)param);
  delete (int *)param; // Clean up dynamic allocation

  StaticJsonDocument<JSON_DOC_SIZE> jsonData;
  jsonData["weight"] = float(weight) / WEIGHT_SCALE_FACTOR;
  //   jsonData["datetime"] = "2025-06-12T08:30:00Z";
  jsonData["datetime"] = getInstance().getTimestampUTC();
  String payload;
  serializeJson(jsonData, payload);

  SSLClient sslClient(&wifi_transpor_layer);
  if (SSL_INSECURE) {
    sslClient.setInsecure();
  } else {
    sslClient.setCACert(ROOT_CA);
  }
  HttpClient httpClient(sslClient, SERVER_IP, HTTPS_PORT);
  httpClient.setHttpResponseTimeout(5000);

  int attempt = 0;
  int statusCode = -1;

  while (attempt < MAX_POST_ATTEMPTS) {
    httpClient.beginRequest();
    httpClient.post(SERVER_POST_PATH);
    httpClient.sendHeader("Content-Type", "application/json");
    httpClient.sendHeader("authentication", API_KEY);
    httpClient.sendHeader("Content-Length", payload.length());
    httpClient.beginBody();
    httpClient.print(payload);
    httpClient.endRequest();

    statusCode = httpClient.responseStatusCode();
    Serial.print("statusCode");
    Serial.println(statusCode);
    Serial.print("responseBody");
    String response = httpClient.responseBody();
    Serial.println(response);

    if (statusCode == 200 || response.indexOf("\"status\":\"success\"") != -1)
      break; // Success

    if (statusCode < 0) {
      Serial.print("HTTP POST failed with error: ");
      Serial.println(httpClient.getWriteError());
    }

    attempt++;
    delay(POST_RETRY_DELAY_MS);
  }

  InternetClient::postSuccess = statusCode == 200;
  InternetClient::postFinished = true;

  vTaskDelete(NULL); // Cleanly end the task
}

String InternetClient::getTimestampUTC() {
  time_t now;
  struct tm timeinfo;

  time(&now);
  gmtime_r(&now, &timeinfo); // Get UTC time

  char buf[30];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &timeinfo); // Z = UTC
  return String(buf);                                          // e.g., "2025-06-12T08:30:00Z"
}
