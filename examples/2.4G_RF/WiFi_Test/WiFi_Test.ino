/*
 * Heltec WiFi Kit 32 - WiFi Connection & RSSI Display Example
 * Function: Connect to a router and display signal strength (RSSI) via serial monitor
 * Compatible: Heltec WiFi Kit 32 (both V2 and V3)
 */

#include <WiFi.h>

// ========== Modify with your network credentials ==========
const char* ssid     = "your_SSID";
const char* password = "your_PASSWORD";
// ===========================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("===================================");
  Serial.println("Heltec WiFi Kit 32 WiFi Connection Example");
  Serial.println("===================================");

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("================================");
  Serial.println("WiFi connected successfully!");
  Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  Serial.println("================================");
  Serial.println();
}

void loop() {
  long rssi = WiFi.RSSI();

  Serial.print("Signal Strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");

  if (rssi > -50) {
    Serial.println("   -> Excellent signal");
  } else if (rssi > -60) {
    Serial.println("   -> Good signal");
  } else if (rssi > -70) {
    Serial.println("   -> Fair signal");
  } else {
    Serial.println("   -> Weak signal");
  }

  Serial.println("---");
  delay(2000);
}