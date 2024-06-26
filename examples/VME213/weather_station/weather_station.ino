/* Heltec Automation weather_station example
 *
 * Function:
 * 1. This example demonstrates how to obtain weather, time, etc. and display on the E-ink display.
 * 2. Time updates use part refresh, while others information update use global refresh
 * 3. Using the weather API provided by senderse.com.
 * 
 * Description:
 * 1. This example needs to work with `ArduinoJson` library.
 * 2. Get weather information via http.
 *
 * Library url: https://github.com/HelTecAutomation/Heltec_ESP32
 * Support: support@heltec.cn
 * 
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * https://www.heltec.org
 *
 * */
#include "HT_lCMEN2R13EFC1.h"
#include "images.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include "time.h"  //Handle time
#include <HTTPClient.h>
// Initialize the display
HT_ICMEN2R13EFC1 display(3, 2, 5, 1, 4, 6, -1, 6000000);  // rst,dc,cs,busy,sck,mosi,miso,frequency
struct tm timeinfo;
typedef void (*Demo)(void);

/* screen rotation
 * ANGLE_0_DEGREE
 * ANGLE_90_DEGREE
 * ANGLE_180_DEGREE
 * ANGLE_270_DEGREE
 */
#define DIRECTION ANGLE_0_DEGREE
#define Resolution 0.000244140625 
#define battary_in 3.3
#define coefficient 1.03
int width, height;
int demoMode = 0;

// get http
const char *ntpServer = "ntp.aliyun.com";

const long gmtOffset_sec = 28800;  //Time offset

const int daylightOffset_sec = 0;
const char *ssid = "Your Wi-Fi SSID";
const char *password = "Your Wi-Fi password";
const char *host = "api.seniverse.com"; //Weather API provider
// Users should apply for their own private key
// For demonstration only, not for commercial purposes
const char *privateKey = "Sfv1t8EYrow1Zi-5o";
const char *city = "chengdu";
const char *language = "en";

struct WetherData {
  char city[32];
  char weather[64];
  char weather_1[64];
  char weather_2[64];
  char code[32];
  char high[32];
  char humi[32];
};

uint8_t code_tday, code_pday;
WiFiClient client;

struct WetherData weatherdata = { 0 };

void setup() {
  Serial.begin(115200);
  if (DIRECTION == ANGLE_0_DEGREE || DIRECTION == ANGLE_180_DEGREE) {
    width = display._width;
    height = display._height;
  } else {
    width = display._height;
    height = display._width;
  }
  VextON();
  delay(100);
  // Initialising the UI will init the display too.
  display.init();
  display.screenRotate(DIRECTION);
  display.clear();
  display.update(COLOR_BUFFER);
  display.drawString(0, 0, "init >>> ");
  Serial.print("Connecting to ");
  display.drawString(0, 20, "Connecting to ... ");
  Serial.println(ssid);
  display.drawString(100, 20, ssid);
  WiFi.begin(ssid, password);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED) {
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

void Navigation_bar() {
  display.setFont(ArialMT_Plain_10);
  display.drawLine(0, 15, 250, 15);
  display.drawXbm(5, -3, 20, 20, wifix_bitfis);
  display.drawString(25, 0, ssid);
  battery();
}

void battery()
{
    analogReadResolution(12);
    int battery_levl = analogRead(7)* Resolution * battary_in * coefficient;//battary/4096*3.3*coefficient
    float battery_one = 0.4125;
    Serial.printf("ADC analog value = %.2f\n", battery_levl );
    if (battery_levl < battery_one)
    {
        display.drawString(230, 0, "N/A");
        display.drawXbm(215, 0, battery_w, battery_h, battery0);
    }
    else if (battery_levl < 2 * battery_one && battery_levl > battery_one)
    {
        display.drawXbm(230, 0, battery_w, battery_h, battery1);
    }
    else if (battery_levl < 3 * battery_one && battery_levl > 2 * battery_one)
    {
        display.drawXbm(230, 0, battery_w, battery_h, battery2);
    }
    else if (battery_levl < 4 * battery_one && battery_levl > 3 * battery_one)
    {
        display.drawXbm(230, 0, battery_w, battery_h, battery3);
    }
    else if (battery_levl < 5 * battery_one && battery_levl > 4 * battery_one)
    {
        display.drawXbm(230, 0, battery_w, battery_h, battery4);
    }
    else if (battery_levl < 6 * battery_one && battery_levl > 5 * battery_one)
    {
        display.drawXbm(230, 0, battery_w, battery_h, battery5);
    }
    else if (battery_levl < 7 * battery_one && battery_levl > 6 * battery_one)
    {
        display.drawXbm(230, 0, battery_w, battery_h, battery6);
    }
    else if (battery_levl < 7 * battery_one && battery_levl > 6 * battery_one)
    {
        display.drawXbm(230, 0, battery_w, battery_h, batteryfull);
    }
}

void drawImageDemo() {
  display.setFont(ArialMT_Plain_16);
  GetWeather();
  display.drawLine(125, 15, 125, 80);
  display.drawLine(0, 80, 250, 80);
}

void GetWeather() {
  if (!client.connect(host, 80)) {
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
  if (!ok) {
    Serial.println("No response or invalid response!");
  }
  Serial.println("Skip headers");
  String line = "";
  line += client.readStringUntil('\n');
  Serial.println(line);
  DynamicJsonDocument doc(1400);
  DeserializationError error = deserializeJson(doc, line);
  if (error) {
    Serial.println("deserialize json failed");
    return;
  }
  Serial.println("deserialize json success");
  strcpy(weatherdata.city, doc["results"][0]["location"]["name"].as<const char *>());
  strcpy(weatherdata.weather, doc["results"][0]["daily"][0]["text_day"].as<const char *>());
  strcpy(weatherdata.code, doc["results"][0]["daily"][0]["code_day"].as<const char *>());
  uint8_t code_day = strtol(doc["results"][0]["daily"][0]["code_day"].as<const char *>(), NULL, 10);
  strcpy(weatherdata.high, doc["results"][0]["daily"][0]["high"].as<const char *>());
  strcpy(weatherdata.humi, doc["results"][0]["daily"][0]["humidity"].as<const char *>());
  code_tday = strtol(doc["results"][0]["daily"][1]["code_day"].as<const char *>(), NULL, 10);
  code_pday = strtol(doc["results"][0]["daily"][2]["code_day"].as<const char *>(), NULL, 10);
  Serial.println("City");
  delay(10);
  Serial.println(weatherdata.city);
  Serial.println("code");
  Serial.println(weatherdata.code);
  weatherdata_select64(code_day);
  Serial.println("temp high");
  Serial.println(weatherdata.high);
  display.drawXbm(70, 25, 10, 17, temp);
  display.drawString(85, 25, weatherdata.high);
  display.drawString(105, 25, "C");
  Serial.println("humi");
  Serial.println(weatherdata.humi);
  display.drawXbm(70, 50, 11, 16, hum);
  display.drawString(85, 50, weatherdata.humi);
  display.drawString(105, 50, "%");
  Serial.println("read json success");
  Serial.println();
  Serial.println("closing connection");
  client.stop();
}

void weatherdata_select64(uint8_t code_day) {

  switch (code_day) {
    case 0:
      display.drawXbm(5, 10, 64, 64, sunny6464);
      break;
    case 1:
      display.drawXbm(5, 10, 64, 64, sunny6464);
      break;
    case 2:
      display.drawXbm(5, 10, 64, 64, sunny6464);
      break;
    case 3:
      display.drawXbm(5, 10, 64, 64, sunny6464);
      break;
    case 4:
      display.drawXbm(5, 10, 64, 64, cloudy6464);
      break;
    case 5:
      display.drawXbm(5, 10, 64, 64, cloudy6464);
      break;
    case 6:
      display.drawXbm(5, 10, 64, 64, cloudy6464);
      break;
    case 7:
      display.drawXbm(5, 10, 64, 64, cloudy6464);
      break;
    case 8:
      display.drawXbm(5, 10, 64, 64, cloudy6464);
      break;
    case 9:
      display.drawXbm(5, 10, 64, 64, cloudy6464);
      break;
    case 10:
      display.drawXbm(5, 10, 64, 64, rain6464xbm);
      break;
    case 11:
      display.drawXbm(5, 10, 64, 64, rain6464xbm);
      break;
    case 12:
      {
        break;
      }
    case 13:
      display.drawXbm(5, 10, 64, 64, rain6464xbm);
      break;
    case 14:
      display.drawXbm(5, 10, 64, 64, rain6464xbm);
      break;
    case 15:
      display.drawXbm(5, 10, 64, 64, rain6464xbm);
      break;
    case 16:
      display.drawXbm(5, 10, 64, 64, rain6464xbm);
      break;
    case 17:
      display.drawXbm(5, 10, 64, 64, rain6464xbm);
      break;
    case 18:
      display.drawXbm(5, 10, 64, 64, rain6464xbm);
      break;
    case 19:
      display.drawXbm(5, 10, 64, 64, rain6464xbm);
      break;
    case 20:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 21:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 22:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 23:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 24:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 25:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 26:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 27:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 28:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 29:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 30:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 31:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 32:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 33:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 34:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 35:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 36:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 37:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 38:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 99:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    default:
      break;
  }
}

void weatherdata_select30(uint8_t x, uint8_t y, uint8_t code_day) {
  switch (code_day) {
    case 0:
      display.drawXbm(x, 94, 30, 30, sun1616);
      break;
    case 1:
      display.drawXbm(x, 94, 30, 30, sun1616);
      break;
    case 2:
      display.drawXbm(x, 94, 30, 30, sun1616);
      break;
    case 3:
      display.drawXbm(x, 94, 30, 30, sun1616);
      break;
    case 4:
      display.drawXbm(x, 95, 30, 22, cloud1616);
      break;
    case 5:
      display.drawXbm(x, 95, 30, 22, cloud1616);
      break;
    case 6:
      display.drawXbm(x, 95, 30, 22, cloud1616);
      break;
    case 7:
      display.drawXbm(x, 95, 30, 22, cloud1616);
      break;
    case 8:
      display.drawXbm(x, 95, 30, 22, cloud1616);
      break;
    case 9:
      display.drawXbm(x, 95, 30, 22, cloud1616);
      break;
    case 10:
      display.drawXbm(x, 97, 30, 29, rain1616);
      break;
    case 11:
      display.drawXbm(x, 97, 30, 29, rain1616);
      break;
    case 12:
      {
        break;
      }
    case 13:
      display.drawXbm(x, 97, 30, 29, rain1616);
      break;
    case 14:
      display.drawXbm(x, 97, 30, 29, rain1616);
      break;
    case 15:
      display.drawXbm(x, 97, 30, 29, rain1616);
      break;
    case 16:
      display.drawXbm(x, 97, 30, 29, rain1616);
      break;
    case 17:
      display.drawXbm(x, 97, 30, 29, rain1616);
      break;
    case 18:
      display.drawXbm(x, 97, 30, 29, rain1616);
      break;
    case 19:
      display.drawXbm(x, 97, 30, 29, rain1616);
      break;
    case 20:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 21:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 22:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 23:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 24:
      display.drawString(5, 30, "N/A");
      break;
    case 25:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 26:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 27:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 28:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 29:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 30:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 31:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 32:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 33:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 34:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 35:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 36:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 37:
      display.drawString(5, 30, "N/A");
      break;
    case 38:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    case 99:
      display.setFont(ArialMT_Plain_24);
      display.drawString(5, 30, "N/A");
      break;
    default:
      break;
  }
}

void time1_num(uint8_t num) {
  switch (num) {
    case 0:
      display.dis_img_Partial_Refresh(150, 32, 14, 4, num0);
      break;
    case 1:
      display.dis_img_Partial_Refresh(150, 32, 14, 4, num1);
      break;
    case 2:
      display.dis_img_Partial_Refresh(150, 32, 14, 4, num2);
      break;
    default:
      break;
  }
}

void time2_num(uint8_t num) {
  switch (num) {
    case 0:
      display.dis_img_Partial_Refresh(164, 32, 14, 4, num0);
      break;
    case 1:
      display.dis_img_Partial_Refresh(164, 32, 14, 4, num1);
      break;
    case 2:
      display.dis_img_Partial_Refresh(164, 32, 14, 4, num2);
      break;
    case 3:
      display.dis_img_Partial_Refresh(164, 32, 14, 4, num3);
      break;
    case 4:
      display.dis_img_Partial_Refresh(164, 32, 14, 4, num4);
      break;
    case 5:
      display.dis_img_Partial_Refresh(164, 32, 14, 4, num5);
      break;
    case 6:
      display.dis_img_Partial_Refresh(164, 32, 14, 4, num6);
      break;
    case 7:
      display.dis_img_Partial_Refresh(164, 32, 14, 4, num7);
      break;
    case 8:
      display.dis_img_Partial_Refresh(164, 32, 14, 4, num8);
      break;
    case 9:
      display.dis_img_Partial_Refresh(164, 32, 14, 4, num9);
      break;
    default:
      break;
  }
}

void time3_num(uint8_t num) {
  switch (num) {
    case 0:
      display.dis_img_Partial_Refresh(210, 32, 14, 4, num0);
      break;
    case 1:
      display.dis_img_Partial_Refresh(210, 32, 14, 4, num1);
      break;
    case 2:
      display.dis_img_Partial_Refresh(210, 32, 14, 4, num2);
      break;
    case 3:
      display.dis_img_Partial_Refresh(210, 32, 14, 4, num3);
      break;
    case 4:
      display.dis_img_Partial_Refresh(210, 32, 14, 4, num4);
      break;
    case 5:
      display.dis_img_Partial_Refresh(210, 32, 14, 4, num5);
      break;
    default:
      break;
  }
}

void time4_num(uint8_t num) {
  switch (num) {
    case 0:
      display.dis_img_Partial_Refresh(224, 32, 14, 4, num0);
      break;
    case 1:
      display.dis_img_Partial_Refresh(224, 32, 14, 4, num1);
      break;
    case 2:
      display.dis_img_Partial_Refresh(224, 32, 14, 4, num2);
      break;
    case 3:
      display.dis_img_Partial_Refresh(224, 32, 14, 4, num3);
      break;
    case 4:
      display.dis_img_Partial_Refresh(224, 32, 14, 4, num4);
      break;
    case 5:
      display.dis_img_Partial_Refresh(224, 32, 14, 4, num5);
      break;
    case 6:
      display.dis_img_Partial_Refresh(224, 32, 14, 4, num6);
      break;
    case 7:
      display.dis_img_Partial_Refresh(224, 32, 14, 4, num7);
      break;
    case 8:
      display.dis_img_Partial_Refresh(224, 32, 14, 4, num8);
      break;
    case 9:
      display.dis_img_Partial_Refresh(224, 32, 14, 4, num9);
      break;
    default:
      break;
  }
}

void printLocalTime() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  display.setFont(ArialMT_Plain_10);
  display.drawString(130, 60, "Variety is the spice of life.");
  char buffer[256];
  display.setFont(ArialMT_Plain_24);
  display.drawString(190, 33, ": ");
  Serial.println(timeinfo.tm_mon);
  display.setFont(ArialMT_Plain_10);
  switch (timeinfo.tm_mon) {
    case 0:
      display.drawString(130, 22, "Jan");
      break;
    case 1:
      display.drawString(130, 22, "Feb");
      break;
    case 2:
      display.drawString(130, 22, "Mar");
      break;
    case 3:
      display.drawString(130, 22, "Apr");
      break;
    case 4:
      display.drawString(130, 22, "May");
      break;
    case 5:
      display.drawString(130, 22, "Jun");
      break;
    case 6:
      display.drawString(130, 22, "Jul");
      break;
    case 7:
      display.drawString(130, 22, "Aug");
      break;
    case 8:
      display.drawString(130, 22, "Sep");
      break;
    case 9:
      display.drawString(130, 22, "Oct");
      break;
    case 10:
      display.drawString(130, 22, "Nov");
      break;
    case 11:
      display.drawString(130, 22, "Dec");
      break;
  }
  sprintf(buffer, "%d", timeinfo.tm_mday);
  Serial.println(timeinfo.tm_mday);
  display.drawString(155, 22, buffer);
  Serial.println(timeinfo.tm_mday);
  week(timeinfo.tm_wday);
  delay(1000);
}

void part_time(){
  display.INIT_JD79656_mcu();
  time1_num(timeinfo.tm_hour / 10);
  time2_num(timeinfo.tm_hour % 10);
  time3_num(timeinfo.tm_min / 10);
  time4_num(timeinfo.tm_min % 10);
}

void GetNetTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void week(uint8_t day) {
  char *next_week[7] = {
    "Sun",
    "Mon",
    "Tues",
    "Wed",
    "Thur",
    "Fri",
    "Sat",
  };
  display.setFont(ArialMT_Plain_10);
  display.drawString(15, 80, next_week[(day + 1) % 7]);
  weatherdata_select30(10, 90, code_tday);
  display.drawString(53, 80, next_week[(day + 2) % 7]);
  weatherdata_select30(50, 90, code_pday);
  display.drawString(98, 80, next_week[(day + 3) % 7]);
  display.drawXbm(90, 95, 30, 22, cloud1616);
  display.drawString(132, 80, next_week[(day + 4) % 7]);
  display.drawXbm(130, 94, 30, 29, rain1616);
  display.drawString(176, 80, next_week[(day + 5) % 7]);
  display.drawXbm(170, 94, 30, 30, sun1616);
  display.drawString(215, 80, next_week[(day + 6) % 7]);
  display.drawXbm(210, 94, 30, 29, rain1616);
}

void VextON(void) {
  pinMode(18, OUTPUT);
  digitalWrite(18, HIGH);
}

void VextOFF(void)  // Vext default OFF
{
  pinMode(18, OUTPUT);
  digitalWrite(18, LOW);
}

void loop() {
  display.clear();
  display.update(COLOR_BUFFER);
  display.clear();
  Navigation_bar();
  drawImageDemo();
  GetNetTime();
  display.update(COLOR_BUFFER);
  display.display();
  uint8_t part_times = 10;// Partial refresh of 10, full refresh once
  while (part_times > 0) {
    part_time();
    delay(1000 * 60);
    part_times--;
  }
}
