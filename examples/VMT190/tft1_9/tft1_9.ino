/* Heltec Automation LoRaWAN communication example
 *
 * Function:
 * 1. Drive GXHTV3 temperature and humidity sensor;
 * 2. Display current temperature and humidity on the TFT display;
 * 3. Get network time and weather.
 *
 * Description:
 * 1. This example requires connecting the GXHTV3 sensor to the SH2.0-4P interface.
 *
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * www.heltec.org
 *
 * */

#include "LoRaWan_APP.h"
#include "HT_ST7789spi.h"
#include <Adafruit_GFX.h> // Core graphics library
#include "pic.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "time.h" //时间的处理
#include <HTTPClient.h>
#include "SPI.h"
#include "img_data.h"
#include "Wire.h"
#include "GXHTC.h"

#define st7789_CS_Pin 39
#define st7789_REST_Pin 40
#define st7789_DC_Pin 47
#define st7789_SCLK_Pin 38
#define st7789_MOSI_Pin 48
#define st7789_LED_K_Pin 17
#define st7789_VTFT_CTRL_Pin 7
static HT_ST7789 *st7789 = NULL;  // 
static SPIClass *gspi_lcd = NULL; // 
WiFiClient client;

GXHTC gxhtc;
 char buffer[256];

// Network time and weather provider
const char *ntpServer = "ntp.aliyun.com";

const long gmtOffset_sec = 28800; // 时间偏移

const int daylightOffset_sec = 0;
char *ssid = "Heltec-RD";
char *password = "hunter_3120";

const char *host = "api.seniverse.com";
const char *privateKey = "Sfv1t8EYrow1Zi-5o";
const char *city = "chengdu";
const char *language = "en";

struct WetherData
{
  char city[32];
  char weather[64];
  char code[32];
  char high[32];
  char low[32];
  char humi[32];
};

void GetWeather()
{
  if (!client.connect(host, 80))
  {
    Serial.println("Connect host failed!");
    return;
  }
  Serial.println("host Conected!");

  String getUrl = "/v3/weather/daily.json?key=";
  getUrl += privateKey;
  getUrl += "&location=";
  getUrl += city;
  getUrl += "&language=";
  getUrl += language;
  client.print(String("GET ") + getUrl + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  Serial.println("Get send");

  char endOfHeaders[] = "\r\n\r\n";
  bool ok = client.find(endOfHeaders);
  if (!ok)
  {
    Serial.println("No response or invalid response!");
  }
  Serial.println("Skip headers");

  String line = "";

  line += client.readStringUntil('\n');

  Serial.println(line);

  DynamicJsonDocument doc(1400);

  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.println("deserialize json failed");
    return;
  }
  Serial.println("deserialize json success");

  struct WetherData weatherdata = {0};

  strcpy(weatherdata.city, doc["results"][0]["location"]["name"].as<const char *>());
  strcpy(weatherdata.weather, doc["results"][0]["daily"][0]["text_day"].as<const char *>());
  strcpy(weatherdata.code, doc["results"][0]["daily"][0]["code_day"].as<const char *>());
  uint8_t code_day = strtol(doc["results"][0]["daily"][0]["code_day"].as<const char *>(), NULL, 10);
  strcpy(weatherdata.high, doc["results"][0]["daily"][0]["high"].as<const char *>());
  strcpy(weatherdata.low, doc["results"][0]["daily"][0]["low"].as<const char *>());
  strcpy(weatherdata.humi, doc["results"][0]["daily"][0]["humidity"].as<const char *>());

  Serial.println("City");
  delay(10);
  Serial.println(weatherdata.city);
  // testdrawtext(120, 10, weatherdata.city, ST7789_WHITE);

  Serial.println("code");
  Serial.println(weatherdata.code);
  select(code_day);
  st7789->setTextSize(3);
  gxhtc.begin(2, 1);
  gxhtc.read_data();
  Serial.print("Temperature:");
  Serial.print(gxhtc.g_temperature);
  Serial.print("  Humidity:");
  Serial.println(gxhtc.g_humidity);
  Serial.println("temp high");
  Serial.println(weatherdata.high);
    sprintf(buffer, "%.2f", gxhtc.g_temperature);
  // testdrawtext(280,100,'s',ST77XX_BLUE,ST77XX_BLACK,10);
  st7789->drawRGBBitmap(0, 100, temp_0, 32, 32);
  // testdrawtext(10, 100, "T :", ST7789_WHITE);
  testdrawtext(50, 100, buffer, ST7789_RED);
  testdrawtext(90, 100, "C", ST7789_WHITE);

  Serial.println("temp low");
  Serial.println(weatherdata.low);
  Serial.println("humi");
  Serial.println(weatherdata.humi);
      sprintf(buffer, "%.2f", gxhtc.g_humidity);

  st7789->drawRGBBitmap(5, 135, humidity_0, 32, 32);
  testdrawtext(50, 140,buffer, ST7789_BLUE);
  testdrawtext(90, 140, "%", ST7789_WHITE);

  Serial.println("read json success");
  Serial.println();
  Serial.println("closing connection");
  client.stop();
  st7789->setTextSize(2);
}
void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  st7789->setTextSize(5);

  // Serial.println(&timeinfo, "%F %T %A");  // 格式化输出
  // const char * str = to_string(timeinfo.tm_hour);
  Serial.println(timeinfo.tm_hour); // 格式化输出
  sprintf(buffer, "%d", timeinfo.tm_hour);
  testdrawtext(140, 50, buffer, ST7789_WHITE);
  // sprintf(buffer,"%d",timeinfo.tm_hour);
  testdrawtext(200, 50, ": ", ST7789_YELLOW);
  sprintf(buffer, "%d", timeinfo.tm_min);
  Serial.println(timeinfo.tm_min); // 格式化输出
  testdrawtext(230, 50, buffer, ST7789_WHITE);
  st7789->setTextSize(2);

  Serial.println(timeinfo.tm_mon); // 格式化输出
  // sprintf(buffer, "%d", timeinfo.tm_mon);
  switch (timeinfo.tm_mon)
  {
  case 0:
    testdrawtext(140, 30, "Jan", ST7789_GREEN);
    break;
  case 1:
    testdrawtext(140, 30, "Feb", ST7789_GREEN);
    break;
  case 2:
    testdrawtext(140, 30, "Mar", ST7789_GREEN);
    break;
  case 3:
    testdrawtext(140, 30, "Apr", ST7789_GREEN);
    break;
  case 4:
    testdrawtext(140, 30, "May", ST7789_GREEN);
    break;
  case 5:
    testdrawtext(140, 30, "Jun", ST7789_GREEN);
    break;
  case 6:
    testdrawtext(140, 30, "Jul", ST7789_GREEN);
    break;
  case 7:
    testdrawtext(140, 30, "Aug", ST7789_GREEN);
    break;
  case 8:
    testdrawtext(140, 30, "Sep", ST7789_GREEN);
    break;
  case 9:
    testdrawtext(140, 30, "Oct", ST7789_GREEN);
    break;
  case 10:
    testdrawtext(140, 30, "Nov", ST7789_GREEN);
    break;
  case 11:
    testdrawtext(140, 20, "Dec", ST7789_GREEN);
    break;
  }
  sprintf(buffer, "%d", timeinfo.tm_mday);
  Serial.println(timeinfo.tm_mday); // 格式化输出
  testdrawtext(180, 30, buffer, ST7789_WHITE);
  // testdrawtext(180, 110, buffer, Font_16x26, ST7789_WHITE, ST7789_BLACK);
}
void GetNetTime()
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}
void select(uint8_t code)
{
  uint8_t now_weather_code_temp = code;
  switch (now_weather_code_temp)
  {
  case 0:
    // showMyFonts(X, Y, "晴", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p0_5151, 51, 51);
    break;
  case 1:
    // showMyFonts(X, Y, "夜晚晴", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p1_5152, 51, 52);
    break;
  case 2:
    // showMyFonts(X, Y, "晴", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p0_5151, 51, 51);
    break;
  case 3:
    // showMyFonts(X, Y, "夜晚晴", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p1_5152, 51, 52);
    break;
  case 4:
    // showMyFonts(X, Y, "多云", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p4_6047, 60, 47);
    break;
  case 5:
    // showMyFonts(X, Y, "晴间多云", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p5_6044, 60, 44);
    break;
  case 6:
    // showMyFonts(X, Y, "晴间多云", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p6_6051, 60, 51);
    break;
  case 7:
    // showMyFonts(X, Y, "大部多云", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p7_6042, 60, 42);
    break;
  case 8:
    // showMyFonts(X, Y, "大部多云", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p8_5649, 56, 49);
    break;
  case 9:
    // showMyFonts(X, Y, "阴", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p9_6040, 60, 40);
    break;
  case 10:
    // showMyFonts(X, Y, "阵雨", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p10_6059, 60, 59);
    break;
  case 11:
    // showMyFonts(X, Y, "雷阵雨", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p11_5656, 56, 56);
    break;
  case 12:
  {
    break;
  }
  case 13:
    // showMyFonts(X, Y, "小雨", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p13_5654, 56, 54);
    break;
  case 14:
    // showMyFonts(X, Y, "中雨", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p14_5654, 56, 54);
    break;
  case 15:
    // showMyFonts(X, Y, "大雨", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p15_5654, 56, 54);
    break;
  case 16:
    // showMyFonts(X, Y, "暴雨", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p16_5654, 56, 54);
    break;
  case 17:
    // showMyFonts(X, Y, "大暴雨", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p17_5754, 57, 54);
    break;
  case 18:
    // showMyFonts(X, Y, "特大暴雨", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p18_5754, 57, 54);
    break;
  case 19:
    // showMyFonts(X, Y, "冻雨", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p19_5657, 56, 57);
    break;
  case 20:
    // showMyFonts(X, Y, "雨夹雪", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p20_5655, 56, 55);
    break;
  case 21:
    // showMyFonts(X, Y, "阵雪", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p21_5656, 56, 56);
    break;
  case 22:
    // showMyFonts(X, Y, "小雪", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p22_5653, 56, 53);
    break;
  case 23:
    // showMyFonts(X, Y, "中雪", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p23_5653, 56, 53);
    break;
  case 24:
    // showMyFonts(X, Y, "大雪", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p24_5653, 56, 53);
    break;
  case 25:
    // showMyFonts(X, Y, "暴雪", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p25_5656, 56, 56);
    break;
  case 26:
    // showMyFonts(X, Y, "浮尘", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p26_5345, 53, 45);
    break;
  case 27:
    // showMyFonts(X, Y, "扬沙", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p26_5345, 53, 45);
    break;
  case 28:
    // showMyFonts(X, Y, "沙尘暴", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p28_5834, 58, 34);
    break;
  case 29:
    // showMyFonts(X, Y, "强沙尘暴", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p28_5834, 58, 34);
    break;
  case 30:
    // showMyFonts(X, Y, "雾", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p30_5450, 54, 50);
    break;
  case 31:
    // showMyFonts(X, Y, "霾", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p31_5650, 56, 50);
    break;
  case 32:
    // showMyFonts(X, Y, "风", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p32_5644, 56, 44);
    break;
  case 33:
    // showMyFonts(X, Y, "大风", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p32_5644, 56, 44);
    break;
  case 34:
    // showMyFonts(X, Y, "飓风", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p34_5656, 56, 56);
    break;
  case 35:
    // showMyFonts(X, Y, "热带风暴", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p34_5656, 56, 56);
    break;
  case 36:
    // showMyFonts(X, Y, "龙卷风", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p36_5655, 56, 55);
    break;
  case 37:
    // showMyFonts(X, Y, "冷", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p37_5158, 51, 58);
    break;
  case 38:
    // showMyFonts(X, Y, "热", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p38_5151, 51, 51);
    break;
  case 99:
    // showMyFonts(X, Y, "未知", TFT_GREEN);
    st7789->drawRGBBitmap(20, 30, p99_5323, 53, 23);
    break;
  default:
    break;
  }
}

void testdrawtext(uint16_t x, uint16_t y, char *text, uint16_t color) {
  st7789->setCursor(x, y);
  st7789->setTextColor(color);
  st7789->setTextWrap(true);
  st7789->print(text);
}

void setup()
{
  Serial.begin(115200);
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  delay(20);
  gspi_lcd = new SPIClass(HSPI);
  st7789 = new HT_ST7789(240, 320, gspi_lcd, st7789_CS_Pin, st7789_DC_Pin, st7789_REST_Pin);
  gspi_lcd->begin(st7789_SCLK_Pin, -1, st7789_MOSI_Pin, st7789_CS_Pin);
  // set up slave select pins as outputs as the Arduino API
  pinMode(gspi_lcd->pinSS(), OUTPUT);
  st7789->init(170, 320);

  Serial.printf("Ready!\r\n");
  st7789->setRotation(1);
  st7789->fillScreen(ST7789_BLACK);
  testdrawtext(0, 0, "init >>> ", ST7789_WHITE);

  pinMode(17, OUTPUT);
  digitalWrite(17, HIGH);
  // ledcSetup(0, 1000, 8);
  // attach the channel to the GPIO to be controlled
  // ledcAttachPin(17, 0);  //将 LEDC 通道绑定到指定 IO 口上以实现输出
  // ledcWrite(0, 180);     //指定通道输出一定占空比波形
  Serial.print("Connecting to ");
  testdrawtext(0, 30, "Connecting to ... ", ST7789_WHITE);

  Serial.println(ssid);
  testdrawtext(120, 30, ssid, ST7789_WHITE);
  WiFi.begin(ssid, password);
  uint16_t i = 0, j = 60;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (i <= 320)
    {
      testdrawtext(i, j, ".", ST7789_WHITE);
      i = i + 10;
    }
    if (i > 320)
    {
      testdrawtext(i, j, ".", ST7789_WHITE);
      i = 0;
      j = j + 10;
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  testdrawtext(0, 90, "WiFi connected", ST7789_WHITE);
  Serial.println("IP address: ");
  testdrawtext(0, 120, "IP address: ", ST7789_WHITE);
  testdrawtext(0, 150, (char *)(WiFi.localIP().toString().c_str()), ST7789_WHITE);

  Serial.println(WiFi.localIP());
  Serial.println("");
  Serial.println("WiFi Conected!");

  st7789->fillScreen(ST7789_BLACK);

  xTaskCreate(
      tasktwo,   /* Task function. */
      "Tasktwo", /* String with name of task. */
      5000,      /* Stack size in bytes. */
      NULL,      /* Parameter passed as input of the task */
      2,         /* Priority of the task. */
      NULL);     /* Task handle. */
  delay(1000);

  xTaskCreate(
      taskOne,   /* Task function. */
      "TaskOne", /* String with name of task. */
      5000,      /* Stack size in bytes. */
      NULL,      /* Parameter passed as input of the task */
      1,         /* Priority of the task. */
      NULL);     /* Task handle. */
}

void taskOne(void *parameter)
{
  while (1)
  {
    // st7789->drawRGBBitmap(150, 121, img_3_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_4_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_5_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_6_0, 122, 50);
    // st7789->drawRGBBitmap(150, 121, img_6_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_7_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_8_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_9_0, 122, 50);
    // st7789->drawRGBBitmap(0, 121, img_10_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_11_0, 122, 50);
    // st7789->drawRGBBitmap(150, 121, img_12_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_13_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_14_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_15_0, 122, 50);
    // st7789->drawRGBBitmap(150, 121, img_16_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_17_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_18_0, 122, 50);
    st7789->drawRGBBitmap(150, 121, img_19_0, 122, 50);
  }
  vTaskDelete(NULL);
}
void tasktwo(void *parameter)
{
  while (1)
  {
    st7789->fillRect(135, 50, 180, 51, ST7789_BLACK);             
    GetNetTime();
    // st7789->drawRGBBitmap(0, 0, wifi_0, 16, 16);
    st7789->setTextSize(1);
    testdrawtext(0, 0, ssid, 0x07FF);
    // st7789->drawRGBBitmap(0, 20, logo_0, 319, 133);
    GetWeather();
    st7789->drawFastHLine(0, 20, 320, ST7789_WHITE);
    st7789->setTextSize(2);
    testdrawtext(140, 100, "I am a bicycle", 0x07FF);
    st7789->fillRect(120, 20, 10, 50, ST7789_GREEN);
    st7789->fillRect(120, 70, 10, 50, ST7789_RED);
    st7789->fillRect(120, 120, 10, 50, ST7789_BLUE);

    st7789->drawFastVLine(130, 20, 170, ST7789_ORANGE);
     st7789->drawFastVLine(120, 20, 170, ST7789_ORANGE);
    delay(1000 * 60);
  }
  vTaskDelete(NULL);
}

void loop()
{
  delay(1000);
}
