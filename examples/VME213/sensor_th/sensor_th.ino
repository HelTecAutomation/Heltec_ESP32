/* Heltec Automation  GXHTC Sensors test example
 *
 * Function:
 * Read the temperature, humidity, and time displayed on the screen
 * 
 * Description:
 * 1. This example use a quick link cable to connect gxhtv3 and read temperature and humidity through iic
 * 
 * Library url: https://github.com/HelTecAutomation/Heltec_ESP32
 * Support: support@heltec.cn
 *
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * https://www.heltec.org
 */
#include "Wire.h"
#include "GXHTC.h"
#include "HT_lCMEN2R13EFC1.h"
#include "img.h"
#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>
// Initialize the display
HT_ICMEN2R13EFC1 display(3, 2, 5, 1, 4, 6, -1, 6000000); // rst,dc,cs,busy,sck,mosi,miso,frequency
GXHTC gxhtc;

char buffer[256];
const char *ntpServer = "ntp.aliyun.com";
const long gmtOffset_sec = 28800; // time offset
const int daylightOffset_sec = 0;
const char *ssid = "Your Wi-Fi SSID";
const char *password = "Your Wi-Fi password";

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  display.setFont(ArialMT_Plain_10);
  char buffer[256];
  display.setFont(ArialMT_Plain_10);
  Serial.println(timeinfo.tm_hour);
  sprintf(buffer, "%d", timeinfo.tm_hour);
  display.drawString(10, 0, buffer);
  display.drawString(25, 0, ": ");
  sprintf(buffer, "%d", timeinfo.tm_min);
  Serial.println(timeinfo.tm_min);
  display.drawString(30, 0, buffer);
  Serial.println(timeinfo.tm_mon);
  display.setFont(ArialMT_Plain_16);
  sprintf(buffer, "%d", timeinfo.tm_year + 1900);
  display.drawString(160, 100, buffer);
  display.drawString(195, 100, "/");
  sprintf(buffer, "%d", timeinfo.tm_mon + 1);
  display.drawString(200, 100, buffer);
  display.drawString(210, 100, "/");
  sprintf(buffer, "%d", timeinfo.tm_mday);
  Serial.println(timeinfo.tm_mday); 
  display.drawString(215, 100, buffer);
  Serial.println(timeinfo.tm_mday); 
}

void GetNetTime()
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void read_th()
{
  gxhtc.read_data();
  Serial.print("Temperature:");
  Serial.print(gxhtc.g_temperature);
  display.setFont(ArialMT_Plain_24);
  sprintf(buffer, "%.2f", gxhtc.g_temperature);
  display.drawXbm(18, 55, 10, 17, temp);
  display.drawString(30, 50, "28.2 C");
  Serial.print("  Humidity:");
  Serial.println(gxhtc.g_humidity);
  sprintf(buffer, "%.2f", gxhtc.g_humidity);
  display.drawXbm(128, 55, 11, 16, hum);
  display.drawString(142, 50, "69 %");
  Serial.printf("id = %X\r\n", gxhtc.read_id());
}

void setup()
{
  Serial.begin(115200);
  pinMode(18, OUTPUT);
  digitalWrite(18, HIGH);
  gxhtc.begin(39, 38);
  display.init();
  display.clear();
  display.update(COLOR_BUFFER);
  display.drawString(0, 0, "init >>> ");
  Serial.print("Connecting to ");
  display.drawString(0, 20, "Connecting to ... ");
  Serial.println(ssid);
  display.drawString(100, 20, ssid);
  WiFi.begin(ssid, password);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    display.drawString(i, 40, ".");
    i = i + 10;
  }
  Serial.println("");
  Serial.println("WiFi connected");
  display.drawString(0, 60, "WiFi connected");
  Serial.println("IP address: ");
  display.drawString(0, 90, "IP address: ");
  display.drawString(60, 90, WiFi.localIP().toString().c_str());
  Serial.println(WiFi.localIP());
  Serial.println("");
  Serial.println("WiFi Conected!");
  display.update(COLOR_BUFFER);
  display.display();
  delay(1000);
}

void loop()
{
  display.clear();
  display.drawLine(0, 15, 250, 15);
  read_th();
  GetNetTime();
  display.update(COLOR_BUFFER);
  display.display();
  delay(30000);
}
