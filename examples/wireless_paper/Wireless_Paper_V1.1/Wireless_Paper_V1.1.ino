/* Heltec Automation  Wireless_Paper_V1.0 Sensors test example
 *
 * Function:
 * Refresh e-paper through BLE, WIFI, and LORAWAN (currently unavailable)
 * 
 * HelTec AutoMation, Chengdu, China
 * www.heltec.org
 * Operation instance:
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
 */



#include "../../../src/webser/ESPAsyncWebSrv.h"
#include "HT_lCMEN2R13EFC1.h"
#include "images.h"
#include "html.h"
HT_ICMEN2R13EFC1   display(6, 5, 4, 7, 3, 2, -1, 6000000); //rst,dc,cs,busy,sck,mosi,miso,frequency


int width, height;
String HTTP_Payload ;

AsyncWebServer server(80);        // Create WebServer object with port number 80

//By using port number 80, you can directly enter the IP address for access, while using other ports requires entering the IP: port number for access

//An array for storing web pages
const char *ssid = "Heltec-RD";
const char *password = "hunter_3120";


// Distribute processing callback function
void Config_Callback(AsyncWebServerRequest *request)
{

   
  if (request->hasParam("value")) // If there is a value, it will be delivered
  {
    String Payload;
    const char*   buff;

    Payload = request->getParam("value")->value();    // Obtain the data delivered
    buff = Payload.c_str();

    delay(100);
    int i = 0;
    char *token;
    token = strtok((char*)buff, ",");

    while (token != NULL ) {
      int num = atoi(token );
      WiFi_Logo_bits[i] = num;
      token = strtok(NULL, ",");
       Serial.print(WiFi_Logo_bits[i]);
      i++;
    }

    drawImageDemo();

 Serial.print(".hhhh");
  }
  request->send(200, "text/plain", "OK"); 
}
void setup()
{
 
  Serial.begin(115200);
  Serial.println();
  //disableCore0WDT();
  VextON();
  delay(100);
  display.init();

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/set", HTTP_GET, Config_Callback);  // Bind the handler that is configured to deliver the function
  server.begin();  


//  drawImageDemo();

}
void drawImageDemo() {
  // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
  // on how to create xbm files
  display.clear();
  display.update(BLACK_BUFFER);

  display.clear();
  int x = width / 2 - WiFi_Logo_width / 2;
  int y = height / 2 - WiFi_Logo_height / 2;
  display.drawXbm(0 , 0  , WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display.update(BLACK_BUFFER);
  display.display();
   Serial.print(".555");
}
void VextON(void)
{
  pinMode(45, OUTPUT);
  digitalWrite(45, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(45, OUTPUT);
  digitalWrite(45, HIGH);
}
void loop() {
  //  server.handleClient(); //Handle requests from clients
  vTaskDelay(10000);

}
