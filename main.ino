#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Adafruit_NeoPixel.h>
#include "time.h"

// NeoPixel settings
#define PIN        48
#define NUMPIXELS  1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500

// WiFi credentials
const char* ssid = "USERNAME-AP";
const char* password = "PASSWORD-AP";

// NTP Server for local time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 12600; // GMT +3:30 (Iran)
const int   daylightOffset_sec = 0;

// API endpoint that provides time (JSON response)
String serverName = "http://192.168.1.100:44555/time";

// Interval between checks
unsigned long lastTime = 0;
unsigned long timerDelay = 1000; // 1 second

// Time received from API
int api_hour = -1;
int api_minute = -1;

// Warning threshold (in minutes)
const int ALERT_BEFORE_MINUTES = 2;

// Initial setup
void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    blinkColor(pixels.Color(127, 0, 0)); // Red = connecting
    Serial.print(".");
  }

  Serial.println("\n✅ Connected to WiFi");

  // Enable NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    lastTime = millis();

    if (WiFi.status() != WL_CONNECTED) {
      blinkColor(pixels.Color(127, 0, 0)); // Blinking red
      return;
    }

    // Get time from API
    String response = httpGETRequest(serverName);
    if (response.length() == 0) {
      blinkColor(pixels.Color(126, 35, 1.5)); // Orange = failed to receive
      delay(100);
      blinkColor(pixels.Color(0, 0, 0));
      delay(100);
      blinkColor(pixels.Color(126, 35, 1.5));
      delay(100);
      blinkColor(pixels.Color(0, 0, 0));
      return;
    }

    JSONVar data = JSON.parse(response);
    if (JSON.typeof(data) == "undefined") {
      Serial.println("❌ JSON Parse Failed");
      return;
    }

    api_hour = int(data["hour"]);
    api_minute = int(data["minute"]);

    Serial.print("🕒 API Time: ");
    Serial.print(api_hour);
    Serial.print(":");
    Serial.println(api_minute);

    // Compare current time with API time
    if (isTimeClose(api_hour, api_minute)) {
      blinkColor(pixels.Color(127, 127, 0)); // Blinking yellow = near scheduled time
    } else {
      setColor(pixels.Color(0, 100, 0)); // Solid green = success
      delay(500);
      setColor(pixels.Color(0, 0, 0)); // Turn off
    }
  }
}

// Compare current local time with target time
bool isTimeClose(int targetHour, int targetMinute) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("❌ Failed to obtain local time");
    return false;
  }

  int nowMin = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  int targetMin = targetHour * 60 + targetMinute;

  Serial.print("⏱️ Local Time: ");
  Serial.print(timeinfo.tm_hour);
  Serial.print(":");
  Serial.println(timeinfo.tm_min);

  return abs(nowMin - targetMin + 2) <= ALERT_BEFORE_MINUTES;
}

// Blink NeoPixel with specific color
void blinkColor(uint32_t color) {
  pixels.clear();
  pixels.setPixelColor(0, color);
  pixels.show();
  delay(DELAYVAL);
  pixels.clear();
  pixels.show();
  delay(DELAYVAL);
}

// Set NeoPixel to solid color
void setColor(uint32_t color) {
  pixels.clear();
  pixels.setPixelColor(0, color);
  pixels.show();
}

// Send HTTP GET request
String httpGETRequest(String server) {
  HTTPClient http;
  http.begin(server);
  int httpResponseCode = http.GET();
  String payload = "";

  if (httpResponseCode > 0) {
    payload = http.getString();
  } else {
    Serial.print("❌ HTTP GET failed: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return payload;
}
