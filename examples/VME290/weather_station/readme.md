# WEATHER STATION

This example demonstrates how to obtain weather, time, etc. from the internet and display them on an ink screen.

# Supported Targets

At present, this example only supports Vison Master Eink213, which uses the ESP32-S3 chip.

## How to Use Example

Flash this example and observe the screen display. 

### Configure the Project

Change `SSID` and `password` to connect to your WiFi.
Default values will allow you to use this example without any changes. If you want to use your own Weather server and you don't have one already follow these steps:

* Create an account on [Weather server](https://www.seniverse.com/).
* After logging in, click on the "immediate use" button to create a new product for your data. You can create a free or paid product, then obtain its private key and fill it in the 'privateKey' field.
* This example uses a free API to obtain weather parameters for 3 days. The weather parameters for one week should have an interface. The author's city of residence does not have special weather conditions, and currently only supports three common weather conditions: sunny, rainy, and cloudy. Other weather interfaces are reserved.
* This instance obtains the time from the [time server](ntp.aliyun.com), and you can modify 'ntpServe' to use a server that is convenient for you to use

#### Config example:

You can find the data to be changed at the top of the file:

```cpp
const char *ntpServer = "ntp.aliyun.com";// time server
const long gmtOffset_sec = 28800;  // time offset，UTC/GMT +8
const int daylightOffset_sec = 0;// Timing started in 1990
const char *ssid = "Your Wi-Fi SSID";
const char *password = "Your Wi-Fi password";

const char *host = "api.seniverse.com";//Weather server 
const char *privateKey = "Sfv1t8EYrow1Zi-5o";//Product privateKey
const char *city = "chengdu";//The city to be queried
const char *language = "en";//language preference
//The following is the API splicing method
  String getUrl = "/v3/weather/daily.json?key=";
  getUrl += privateKey;
  getUrl += "&location=";
  getUrl += city;
  getUrl += "&language=";
  getUrl += language;
  client.print(String("GET ") + getUrl + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

#### Using Arduino IDE

To get more information about the Espressif boards see [heltec.org](https://heltec.org).

* Before Compile/Verify, select the correct board: `Tools -> Board`.
* Select the COM port: `Tools -> Port: xxx` where the `xxx` is the detected COM port.

