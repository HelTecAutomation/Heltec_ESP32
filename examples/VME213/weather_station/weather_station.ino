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
#include "HT_E0213A367.h"
#include "images.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include "time.h"  //Handle time
#include <HTTPClient.h>

const char *ntpServer = "pool.ntp.org";
const char* tzInfo    = "NZST-12NZDT,M9.5.0,M4.1.0/3";
const char *location  = "Auckland";
const char *ssid = "Your Wi-Fi SSID";
const char *password = "Your Wi-Fi password";

// Initialize the display
HT_E0213A367 display(3, 2, 5, 1, 4, 6, -1, 6000000);  // rst,dc,cs,busy,sck,mosi,miso,frequency
struct tm timeinfo;

/* screen rotation
 * ANGLE_0_DEGREE
 * ANGLE_90_DEGREE
 * ANGLE_180_DEGREE
 * ANGLE_270_DEGREE
 */
#define VBAT_PIN 7  
#define DIRECTION ANGLE_0_DEGREE
#define Resolution 0.000244140625 
#define battery_in 3.3
#define coefficient 1.03
int width, height;
int demoMode = 0;

// ==== OPEN-METEO SETTINGS ====
const char* openMeteoUrl =
  "http://api.open-meteo.com/v1/forecast"
  "?latitude=-36.8509"
  "&longitude=174.7645"
  "&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset,precipitation_sum,uv_index_max,wind_direction_10m_dominant,weather_code"
  "&hourly=temperature_2m,relative_humidity_2m,apparent_temperature,precipitation_probability,precipitation,rain,showers,cloud_cover,wind_speed_10m,wind_direction_10m"
  "&current=temperature_2m,relative_humidity_2m,apparent_temperature,precipitation,rain,showers,weather_code,wind_speed_10m,wind_direction_10m,cloud_cover"
  "&timezone=Pacific%2FAuckland";

// =======================
// Forecast widget support
// =======================

struct DayForecast {
  String label;        // e.g. "Today Mon" or "Tue"
  uint8_t weatherCode; // Open-Meteo WMO code
  int tMin;            // min temp (°C)
  int tMax;            // max temp (°C)
};

const char* weekdayShortName(int wday) {
  // tm_wday: 0=Sun ... 6=Sat
  static const char* names[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  if (wday < 0 || wday > 6) return "?";
  return names[wday];
}
/*************************************************
*                                                *
*               eINK Display Pins                *
*                                                *
**************************************************/
void VextON(void) {
  pinMode(18, OUTPUT);
  digitalWrite(18, HIGH);
}

void VextOFF(void)  // Vext default OFF
{
  pinMode(18, OUTPUT);
  digitalWrite(18, LOW);
}

/*************************************************************************
*                                                                        *
*              Draw Weather Icons (stored in images.h)                   *
*                                                                        *
**************************************************************************/
void drawWeatherIcon(uint8_t wmoCode, int x, int y) {
  const unsigned char* iconBits = nullptr;
  int w = 48;
  int h = 48;

  // Map Open-Meteo WMO codes to icons
  switch (wmoCode) {
    // 0: clear sky
    case 0:
      iconBits = icon_sunny_code_0;
      break;

    // 1, 2: mainly clear / partly cloudy
    case 1:
    case 2:
      iconBits = icon_overcast_code_1_2;   // or a partly-cloudy icon if you have one
      break;

    // 3: overcast
    case 3:
      iconBits = icon_cloudy_code_3;
      break;
 
    // 45, 48: Fog
    case 45: case 48:
      iconBits = icon_fog_code_45_48;
      break;

    // Drizzle / rain / showers
    case 51: 
      iconBits = icon_drizzle_code_51;
      break;

    case 53: case 55:          // drizzle
      iconBits = icon_med_drizzle_code_53_55;
      break;

    case 56: case 57:          // drizzle
      iconBits = icon_freezing_drizzle_code_56_57;
      break;

    case 61: 
      iconBits = icon_rain_code_61;
      break;

    case 63: 
      iconBits = icon_rain_code_63;
      break;

    case 65:          // rain
      iconBits = icon_rain_code_65;
      break;

    case 66: case 67:         // freezing rain
      iconBits = icon_freezing_rain_code_66_67;
      break;

    case 71: case 72: case 75: case 85: case 86:          // snow
      iconBits = icon_snow_code_71_72_75_85_86;
      break;

    case 77:          // snow flakes
      iconBits = icon_snow_grains_code_77;
      break;

    case 80:        // light rain
      iconBits = icon_light_rain_code_80;
      break;

    case 81:      // medium rain
      iconBits = icon_med_rain_code_81;
      break;

    case 82:          // violent rain
      iconBits = icon_heavy_rain_code_82;
      break;

    // Thunderstorm
    case 95: 
      iconBits = icon_thunder_code_95;
      break;

    case 96: case 99: 
     iconBits = icon_thunder_code_96_99;
      break;

    // Fallback
    default:
      iconBits = icon_cloudy_code_3;
      break;
  }

  if (iconBits != nullptr) {
    display.drawXbm(x, y, w, h, iconBits);
  }
}


/*********************************************************
*                Emojis for Serial printout              *
**********************************************************/
String getWeatherEmoji(uint8_t code) {
  switch (code) {
    case 0:  return "☀️";   // Clear

    case 1:  return "🌤️";  // Mainly clear
    case 2:  return "⛅";   // Partly cloudy
    case 3:  return "☁️";   // Overcast

    case 45:
    case 48: return "🌫️";  // Fog

    case 51:
    case 53:
    case 55: return "🌦️";  // Drizzle

    case 56:
    case 57: return "🌧️❄️"; // Freezing drizzle

    case 61:
    case 63:
    case 65: return "🌧️";   // Rain

    case 66:
    case 67: return "🌧️❄️"; // Freezing rain

    case 71:
    case 73:
    case 75:
    case 77: return "🌨️";   // Snow

    case 80:
    case 81:
    case 82: return "🌦️";   // Showers

    case 85:
    case 86: return "🌨️";   // Snow showers

    case 95:
    case 96:
    case 99: return "⛈️";   // Thunderstorm

    default: return "❓";
  }
}
/**********************************************
*                                             *
*              Navigation Bar                 *
*                                             *
***********************************************/
void Navigation_bar() {
  display.setFont(ArialMT_Plain_10);
  display.drawLine(0, 15, 250, 15);
  display.drawXbm(5, -3, 20, 20, wifix_bitfis);
  display.drawString(27, 0, ssid);
  // Left side: location + date, e.g. "Auckland 25 Nov"
  struct tm nowTm;
  String header = location;
  if (getLocalTime(&nowTm)) {
    char dateBuf[12];   // "25 Nov"
    strftime(dateBuf, sizeof(dateBuf), "%d %b", &nowTm);
    header += " ";
    header += dateBuf;
  }

  // Center the text (x = screen midpoint)
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(125, 0, header);
/*****************************************************
 *                                                   *
 *              Battery Level Display                *
 *                                                   *
 *****************************************************/
  // Battery icon/text on right side (unchanged)
  display.setTextAlignment(TEXT_ALIGN_LEFT); // restore for battery()
  battery();
}

void battery()
{
    analogReadResolution(12);
    /*
    * battery / 4096 * 3.3 * coefficient * 4.9（）
    * coefficient is the battery calculation factor. 
    * If the calculated result shows a significant deviation from the actual battery level, 
    * please adjust the value of the coefficient
    */
    float battery_data = analogRead(VBAT_PIN)* Resolution * battery_in * coefficient* 4.9;
    Serial.printf("ADC analog value = %.2f V\r\n", battery_data );

    if (battery_data <=3.3)
    {
      display.drawString(188, 1, String(battery_data, 2)+"V");
    }
    else display.drawString(200, 1, String(battery_data, 2)+"V");

    if (battery_data <= 3.3)
    {
        display.drawString(230, 1, "N/A");
        display.drawXbm(215, 1, battery_w, battery_h, battery0);
        Serial.println("battery0");
    }
    else if (battery_data <= 3.4 && battery_data > 3.3)
    {
        display.drawXbm(230, 1, battery_w, battery_h, battery1);
        Serial.println("battery1");
    }
    else if (battery_data <= 3.5 && battery_data > 3.4)
    {
        display.drawXbm(230, 1, battery_w, battery_h, battery2);
        Serial.println("battery2");
    }
    else if (battery_data <= 3.6 && battery_data > 3.5)
    {
        display.drawXbm(230, 1, battery_w, battery_h, battery3);
        Serial.println("battery3");
    }
    else if (battery_data <= 3.8 && battery_data > 3.6)
    {
        display.drawXbm(230, 1, battery_w, battery_h, battery4);
        Serial.println("battery4");
    }
    else if (battery_data <= 3.9 && battery_data > 3.8)
    {
        display.drawXbm(230, 1, battery_w, battery_h, battery5);
        Serial.println("battery5");
    }
    else if (battery_data <= 4.1 && battery_data > 3.9)
    {
        display.drawXbm(230, 1, battery_w, battery_h, battery6);
        Serial.println("battery6");
    }
    else if (battery_data > 4.1)
    {
        display.drawXbm(230, 1, battery_w, battery_h, batteryfull);
        Serial.println("batteryfull");
    }
}
/***********************************************************************
*                                                                      *
*                  Next Row down, Today's Weather                      *
*                                                                      *
************************************************************************/
void drawTodayBlock(const DayForecast& today, int yStart) {
  const int SCREEN_W = 250;
  const int ICON_W   = 48;
  const int ICON_H   = 48;
  display.drawLine(0, 53, 250, 53);
  display.drawLine(125, 15, 125, 53);
  // Place icon on the left
  int iconX = 2;
  int iconY = yStart;
  drawWeatherIcon(today.weatherCode, iconX, iconY);

  // Text area to the right of the icon
  int textX = iconX + ICON_W + 2;
  int textY = iconY+5;  // top of text block

  // "Today Mon"
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(textX, textY, today.label);

  // "min / max °C" just below it
  String tempLine = String(today.tMin) + " / " + String(today.tMax) + "°C";
  display.drawString(textX, textY + 14, tempLine);
  display.drawXbm(textX + 56, textY + 4, 10, 17, temp); //Draw the thermometer symbol to the right of the temps
  draw_colon();
}

/*********************************************************************
*                                                                    *
*      Strip of up to 5 days with day label, icon and max temp       *
*                                                                    *
**********************************************************************/
void drawFiveDayStrip(DayForecast days[], int count, int yStart) {
  const int SCREEN_W = 250;
  const int ICON_W   = 48;
  const int ICON_H   = 48;

  if (count <= 0) return;
  if (count > 5) count = 5;

  int colWidth = SCREEN_W / count;

  for (int i = 0; i < count; i++) {
    int colXCenter = colWidth * i + colWidth / 2;
    DayForecast& d = days[i];

    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(colXCenter, yStart + 1, d.label);

    int iconX = colXCenter - ICON_W / 2;
    int iconY = yStart + 4;
    drawWeatherIcon(d.weatherCode, iconX, iconY);

    String tStr = String(d.tMax) + "°";
    display.drawString(colXCenter, iconY + ICON_H , tStr);
  }
}

// High-level widget: draws nav bar and forecast into current buffer
void drawWeatherWidget(const String& location,
                       const String& timeStr,
                       const DayForecast& today,
                       DayForecast next5[], int next5Count) {
  // Clear buffer and draw existing navigation bar (SSID + WiFi + battery)
  display.clear();
  Navigation_bar();

  // Today block and 5-day strip
  int todayY = 14;
  drawTodayBlock(today, todayY);

  int stripY = todayY + 40;
  drawFiveDayStrip(next5, next5Count, stripY);
}

/***********************************************************************
*                                                                      *
*             MAIN WEATHER DATA COLLECT AND DISPLAY                    *
*                                                                      *
************************************************************************/
void fetchWeather() {
  Serial.println("\n=== Fetching weather from Open-Meteo ===");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  HTTPClient http;
  WiFiClient client;   // plain (non-secure) client

  // Start connection
  if (!http.begin(client, openMeteoUrl)) {
    Serial.println("HTTP begin() failed");
    return;
  }

  int httpCode = http.GET();
  if (httpCode <= 0) {
    Serial.print("HTTP GET failed, error: ");
    Serial.println(http.errorToString(httpCode));
    http.end();
    return;
  }

  Serial.print("HTTP code: ");
  Serial.println(httpCode);

  if (httpCode != HTTP_CODE_OK) {
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  // For debugging you can temporarily uncomment:
  // Serial.println(payload);

  // ArduinoJson 7+ style
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // ===== PROCESS CURRENT DATA =====
  JsonObject current = doc["current"];

  float current_temp      = current["temperature_2m"]        | NAN;
  float current_rh        = current["relative_humidity_2m"]  | NAN;
  float current_app       = current["apparent_temperature"]  | NAN;
  float current_precip    = current["precipitation"]         | NAN;
  float current_rain      = current["rain"]                  | NAN;
  float current_showers   = current["showers"]               | NAN;
  int   current_weather_code = current["weather_code"]       | -1;
  float current_wind_speed   = current["wind_speed_10m"]     | NAN;
  float current_wind_dir     = current["wind_direction_10m"] | NAN;
  float current_cloud_cover  = current["cloud_cover"]        | NAN;

  Serial.println("=== Current Data ===");
  Serial.print("Temp (°C): ");         Serial.println(current_temp);
  Serial.print("RH (%): ");            Serial.println(current_rh);
  Serial.print("App Temp (°C): ");     Serial.println(current_app);
  Serial.print("Precip (mm): ");       Serial.println(current_precip);
  Serial.print("Rain (mm): ");         Serial.println(current_rain);
  Serial.print("Showers (mm): ");      Serial.println(current_showers);
  Serial.print("Weather code: ");      Serial.println(current_weather_code);
  Serial.print("Wind speed 10m (m/s): "); Serial.println(current_wind_speed);
  Serial.print("Wind dir 10m (deg): ");   Serial.println(current_wind_dir);
  Serial.print("Cloud cover: ");   Serial.println(current_cloud_cover);

  // ===== PROCESS HOURLY DATA =====
  JsonObject hourly       = doc["hourly"];
  JsonArray hourly_time   = hourly["time"];

  JsonArray hourly_temp          = hourly["temperature_2m"];
  JsonArray hourly_rh            = hourly["relative_humidity_2m"];
  JsonArray hourly_app_temp      = hourly["apparent_temperature"];
  JsonArray hourly_precip_prob   = hourly["precipitation_probability"];
  JsonArray hourly_precip        = hourly["precipitation"];
  JsonArray hourly_rain          = hourly["rain"];
  JsonArray hourly_showers       = hourly["showers"];
  JsonArray hourly_cloud_cover   = hourly["cloud_cover"];
  JsonArray hourly_wind_speed    = hourly["wind_speed_10m"];
  JsonArray hourly_wind_dir      = hourly["wind_direction_10m"];

  Serial.println("\n=== Hourly Data (first 5 entries) ===");
  const size_t maxHourlyPrint = 5;
  size_t nHourly = hourly_time.size();
  size_t count   = (nHourly < maxHourlyPrint) ? nHourly : maxHourlyPrint;

  for (size_t i = 0; i < count; i++) {
    const char* t  = hourly_time[i]        | "";
    float temp     = hourly_temp[i]        | NAN;
    float rh       = hourly_rh[i]          | NAN;
    float appT     = hourly_app_temp[i]    | NAN;
    float pProb    = hourly_precip_prob[i] | NAN;
    float p        = hourly_precip[i]      | NAN;
    float rain     = hourly_rain[i]        | NAN;
    float showers  = hourly_showers[i]     | NAN;
    float cc       = hourly_cloud_cover[i] | NAN;
    float wSpeed   = hourly_wind_speed[i]  | NAN;
    float wDir     = hourly_wind_dir[i]    | NAN;

    Serial.println("-----------------------");
    Serial.print("Time: ");                Serial.println(t);
    Serial.print("Temp (°C): ");           Serial.println(temp);
    Serial.print("RH (%): ");              Serial.println(rh);
    Serial.print("App Temp (°C): ");       Serial.println(appT);
    Serial.print("Precip Prob (%): ");     Serial.println(pProb);
    Serial.print("Precip (mm): ");         Serial.println(p);
    Serial.print("Rain (mm): ");           Serial.println(rain);
    Serial.print("Showers (mm): ");        Serial.println(showers);
    Serial.print("Cloud cover (%): ");     Serial.println(cc);
    Serial.print("Wind speed (m/s): ");    Serial.println(wSpeed);
    Serial.print("Wind dir (deg): ");      Serial.println(wDir);
  }

// ===== PROCESS DAILY DATA =====
JsonObject daily          = doc["daily"];
JsonArray daily_time      = daily["time"];
JsonArray daily_temp_max  = daily["temperature_2m_max"];
JsonArray daily_temp_min  = daily["temperature_2m_min"];
JsonArray daily_sunrise   = daily["sunrise"];
JsonArray daily_sunset    = daily["sunset"];
JsonArray daily_precip_sum   = daily["precipitation_sum"];
JsonArray daily_uv_index_max = daily["uv_index_max"];
JsonArray daily_wind_dir_dom = daily["wind_direction_10m_dominant"];

Serial.println("\n=== Daily Data (first 5 days) ===");
const size_t maxDailyPrint = 5;
size_t nDaily = daily_time.size();
size_t dCount = (nDaily < maxDailyPrint) ? nDaily : maxDailyPrint;

for (size_t i = 0; i < dCount; i++) {
  const char* t  = daily_time[i]      | "";
  float tMax     = daily_temp_max[i]  | NAN;
  float tMin     = daily_temp_min[i]  | NAN;
  const char* sr = daily_sunrise[i]   | "";
  const char* ss = daily_sunset[i]    | "";
  float pSum     = daily_precip_sum[i]   | NAN;
  float uvMax    = daily_uv_index_max[i] | NAN;
  float wDirDom  = daily_wind_dir_dom[i] | NAN;

  Serial.println("-----------------------");
  Serial.print("Date: ");               Serial.println(t);
  Serial.print("Tmax / Tmin (°C): ");
  Serial.print(tMax); Serial.print(" / "); Serial.println(tMin);
  Serial.print("Sunrise: ");            Serial.println(sr);
  Serial.print("Sunset: ");             Serial.println(ss);
  Serial.print("Precip sum (mm): ");    Serial.println(pSum);
  Serial.print("UV index max: ");       Serial.println(uvMax);
  Serial.print("Dominant wind dir (deg): "); Serial.println(wDirDom);
}

// ---- Build DayForecast objects for today + next 5 days ----
const size_t numDays = daily_time.size();
if (numDays >= 6) {
  JsonArray daily_weather_code = daily["weather_code"];

  DayForecast today;
  today.weatherCode = (int)daily_weather_code[0];
  today.tMin        = round((float)daily_temp_min[0]);
  today.tMax        = round((float)daily_temp_max[0]);

  int baseWday = -1;
  // Label today with weekday name
  if (getLocalTime(&timeinfo)) {
    baseWday = timeinfo.tm_wday;   // 0=Sun..6=Sat
    today.label = String("Today ") + weekdayShortName(baseWday);
  } else {
    today.label = "Today";
  }

  DayForecast next5[5];
  for (int i = 0; i < 5; i++) {
    next5[i].weatherCode = (int)daily_weather_code[i + 1];
    next5[i].tMin        = round((float)daily_temp_min[i + 1]);
    next5[i].tMax        = round((float)daily_temp_max[i + 1]);
    Serial.print("Code: ");               Serial.print(next5[i].weatherCode);
    Serial.println(getWeatherEmoji(next5[i].weatherCode));

    if (baseWday >= 0) {
      int w = (baseWday + i + 1) % 7;
      next5[i].label = weekdayShortName(w);
    } else {
      next5[i].label = String("Day") + String(i + 1);
    }
  }

  // Build time string for nav bar
  String timeStr = "--:--";
  struct tm nowTm;
  if (getLocalTime(&nowTm)) {
    char buf[6];
    strftime(buf, sizeof(buf), "%H:%M", &nowTm);
    timeStr = String(buf);
  }

  // Draw full widget (nav bar + today + 5 days)
  drawWeatherWidget(location, timeStr, today, next5, 5);
} else {
  Serial.println("Not enough daily forecast data to draw widget.");
}
} // <- end of fetchWeather()


/***********************************************************************
*                                                                      *
*                  Time and Date Display functions                     *
*                                                                      *
************************************************************************/
void printLocalTime() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 80, "Simon's Weather App.");
  char buffer[256];
  display.setFont(ArialMT_Plain_24);
  Serial.println(timeinfo.tm_mon);
  display.setFont(ArialMT_Plain_10);
  switch (timeinfo.tm_mon) {
    case 0:
      display.drawString(0, 95, "Jan");
      break;
    case 1:
      display.drawString(0, 95, "Feb");
      break;
    case 2:
      display.drawString(0, 95, "Mar");
      break;
    case 3:
      display.drawString(0, 95, "Apr");
      break;
    case 4:
      display.drawString(0, 95, "May");
      break;
    case 5:
      display.drawString(0, 95, "Jun");
      break;
    case 6:
      display.drawString(0, 95, "Jul");
      break;
    case 7:
      display.drawString(0, 95, "Aug");
      break;
    case 8:
      display.drawString(0, 95, "Sep");
      break;
    case 9:
      display.drawString(0, 95, "Oct");
      break;
    case 10:
      display.drawString(0, 95, "Nov");
      break;
    case 11:
      display.drawString(0, 95, "Dec");
      break;
  }
  sprintf(buffer, "%d", timeinfo.tm_mday);
  Serial.println(timeinfo.tm_mday);
  display.drawString(20, 95, buffer);
  Serial.println(timeinfo.tm_mday);
  week(timeinfo.tm_wday);
  delay(1000);
}

void draw_colon() {
  // X=193 is exactly between digit 2 and digit 3
  display.drawXbm(193, 28, 3, 18, colon3x18);
}

void part_time(){
  time1_num(timeinfo.tm_hour / 10);
  time2_num(timeinfo.tm_hour % 10);
  time3_num(timeinfo.tm_min / 10);
  time4_num(timeinfo.tm_min % 10);
}

void time1_num(uint8_t num) {
  switch (num) {
    case 0:
      display.dis_img_Partial_Refresh(155, 20, 32, 14, num0,true,true);
      break;
    case 1:
      display.dis_img_Partial_Refresh(155, 20, 32, 14, num1,true,true);
      break;
    case 2:
      display.dis_img_Partial_Refresh(155, 20, 32, 14, num2,true,true);
      break;
    default:
      break;
  }
}

void time2_num(uint8_t num) {
  switch (num) {
    case 0:
      display.dis_img_Partial_Refresh(170, 20, 32, 14, num0,true,true);
      break;
    case 1:
      display.dis_img_Partial_Refresh(170, 20, 32, 14, num1,true,true);
      break;
    case 2:
      display.dis_img_Partial_Refresh(170, 20, 32, 14, num2,true,true);
      break;
    case 3:
      display.dis_img_Partial_Refresh(170, 20, 32, 14, num3,true,true);
      break;
    case 4:
      display.dis_img_Partial_Refresh(170, 20, 32, 14, num4,true,true);
      break;
    case 5:
      display.dis_img_Partial_Refresh(170, 20, 32, 14, num5,true,true);
      break;
    case 6:
      display.dis_img_Partial_Refresh(170, 20, 32, 14, num6,true,true);
      break;
    case 7:
      display.dis_img_Partial_Refresh(170, 20, 32, 14, num7,true,true);
      break;
    case 8:
      display.dis_img_Partial_Refresh(170, 20, 32, 14, num8,true,true);
      break;
    case 9:
      display.dis_img_Partial_Refresh(170, 20, 32, 14, num9,true,true);
      break;
    default:
      break;
  }
}

void time3_num(uint8_t num) {
  switch (num) {
    case 0:
      display.dis_img_Partial_Refresh(205, 20, 32, 14, num0,true,true);
      break;
    case 1:
      display.dis_img_Partial_Refresh(205, 20, 32, 14, num1,true,true);
      break;
    case 2:
      display.dis_img_Partial_Refresh(205, 20, 32, 14, num2,true,true);
      break;
    case 3:
      display.dis_img_Partial_Refresh(205, 20, 32, 14, num3,true,true);
      break;
    case 4:
      display.dis_img_Partial_Refresh(205, 20, 32, 14, num4,true,true);
      break;
    case 5:
      display.dis_img_Partial_Refresh(205, 20, 32, 14, num5,true,true);
      break;
    default:
      break;
  }
}

void time4_num(uint8_t num) {
  switch (num) {
    case 0:
      display.dis_img_Partial_Refresh(220, 20, 32, 14, num0,true,true);
      break;
    case 1:
      display.dis_img_Partial_Refresh(220, 20, 32, 14, num1,true,true);
      break;
    case 2:
      display.dis_img_Partial_Refresh(220, 20, 32, 14, num2,true,true);
      break;
    case 3:
      display.dis_img_Partial_Refresh(220, 20, 32, 14, num3,true,true);
      break;
    case 4:
      display.dis_img_Partial_Refresh(220, 20, 32, 14, num4,true,true);
      break;
    case 5:
      display.dis_img_Partial_Refresh(220, 20, 32, 14, num5,true,true);
      break;
    case 6:
      display.dis_img_Partial_Refresh(220, 20, 32, 14, num6,true,true);
      break;
    case 7:
      display.dis_img_Partial_Refresh(220, 20, 32, 14, num7,true,true);
      break;
    case 8:
      display.dis_img_Partial_Refresh(220, 20, 32, 14, num8,true,true);
      break;
    case 9:
      display.dis_img_Partial_Refresh(220, 20, 32, 14, num9,true,true);
      break;
    default:
      break;
  }
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
}

/***********************************************************************
*                                                                      *
*                  Initialize Real Time Clock                          *
*                                                                      *
************************************************************************/
void GetNetTime() {
  configTzTime(tzInfo, ntpServer);
  printLocalTime();
}

/***********************************************************************
*                                                                      *
*                         SETUP                                        *
*                                                                      *
************************************************************************/
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
  pinMode(46, OUTPUT); // Enable ADC_CTrl
  digitalWrite(46, HIGH);
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
  display.drawString(0, 110, "IP address: ");
  display.drawString(60, 110, WiFi.localIP().toString().c_str());
  Serial.println(WiFi.localIP());
  GetNetTime();
  Serial.println("");
  display.update(COLOR_BUFFER);
  display.display();
  delay(1000);
}

/***********************************************************************
*                                                                      *
*                         LOOP                                        *
*                                                                      *
************************************************************************/
void loop() {
  uint8_t part_times = 10; // e.g. 10 minutes of partial clock ticks
  while (part_times > 0) {
    // Get fresh data + draw full widget into buffer
    fetchWeather();         // Now also calls drawWeatherWidget()

    part_time();            // your partial refresh clock routine

    // Do a full refresh (strong black/white, clears ghosting)
    display.update(COLOR_BUFFER);  // your full-update call
    display.display();

    delay(1000 * 60);
    part_times--;
  }
}
