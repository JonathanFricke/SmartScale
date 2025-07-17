#include <Arduino.h>

// === CONSTANTS ===
const int WIFI_RETRY_DELAY = 1000;
const int NTP_SYNC_DELAY_MS = 500;
const int HTTP_SUCCESS_CODE = 200;
const int JSON_DOC_SIZE = 200;
const int WEIGHT_SCALE = 100;
const float INVALID_WEIGHT = -1.0;

void setupWifi() {
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi..");

  while (WiFi.status() != WL_CONNECTED) {
    delay(WIFI_RETRY_DELAY);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected to WiFi");
}

void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.print("Waiting for NTP time sync...");

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(NTP_SYNC_DELAY_MS);
  }
  Serial.println("\nTime synchronized:");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// HTTPClient http;
// WiFiClientSecure client;

ESP_SSLClient ssl_client;
WiFiClient basic_client;

int postWeight(int weight) {
  Serial.println("postWeight");

  ssl_client.setInsecure();
  /** Call setDebugLevel(level) to set the debug
   * esp_ssl_debug_none = 0
   * esp_ssl_debug_error = 1
   * esp_ssl_debug_warn = 2
   * esp_ssl_debug_info = 3
   * esp_ssl_debug_dump = 4
   */
  ssl_client.setDebugLevel(3);
  // In case ESP32 WiFiClient, the session timeout should be set,
  // if the TCP session was kept alive because it was unable to detect the
  // server disconnection.
  ssl_client.setSessionTimeout(10); // Set the timeout in seconds (>=10 seconds)
                                    // Assign the basic client
  // Due to the basic_client pointer is assigned, to avoid dangling pointer,
  // basic_client should be existed as long as it was used by ssl_client for
  // transportation.
  ssl_client.setClient(&basic_client);

  String payload = "{\"title\":\"hello\"}";

  if (ssl_client.connect("reqres.in", 443)) {
    Serial.println(" ok");
    Serial.println("Send POST request...");
    ssl_client.print("POST /api/users HTTP/1.1\r\n");
    ssl_client.print("Host: reqres.in\r\n");
    ssl_client.print("Content-Type: application/json\r\n");
    ssl_client.print("x-api-key: reqres-free-v1\r\n");
    ssl_client.print("Content-Length: ");
    ssl_client.print(payload.length());
    ssl_client.print("\r\n\r\n");
    ssl_client.print(payload);

    Serial.print("Read response...");

    unsigned long ms = millis();
    while (!ssl_client.available() && millis() - ms < 3000) {
      delay(0);
    }
    Serial.println();
    while (ssl_client.available()) {
      Serial.print((char)ssl_client.read());
    }
    Serial.println();
  } else {
    Serial.println(" failed\n");
    return 400;
  }

  ssl_client.stop();

  Serial.println("Return Success");
  return 200;
}

int getWeight() { return 0; }

String getTimestampUTC() {
  time_t now;
  struct tm timeinfo;

  time(&now);
  gmtime_r(&now, &timeinfo); // Get UTC time

  char buf[30];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &timeinfo); // Z = UTC
  return String(buf);                                          // e.g., "2025-06-12T08:30:00Z"
}
