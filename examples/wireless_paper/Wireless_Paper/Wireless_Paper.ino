
#include "ESPAsyncWebSrv.h"
#include "HT_QYEG0213RWS800_BWR.h"
#include "images.h"

QYEG0213RWS800_BWR   display(6, 5, 4, 7, 3, 2, -1, 6000000); //rst,dc,cs,busy,sck,mosi,miso,frequency


int width, height;
String HTTP_Payload ;
//using namespace base64;
//epaper_class epaper;
AsyncWebServer server(80);        // 创建WebServer对象, 端口号80
// 使用端口号80可以直接输入IP访问，使用其它端口需要输入IP:端口号访问
// 一个储存网页的数组
const char *ssid = "Heltec-RD";
const char *password = "hunter_3120";
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="utf-8">
</head>
<body>
  <h2>wireless_paper</h2>
  <!-- Create an ID bit dht box to display the obtained data -->
  <div id="dht">
  </div>
<canvas id="canvasElem" width="1" height="1"></canvas>
  <input type="file" id="avatar" name="avatar" style="display:none" accept="image/bmp,image/jpg,image/png" onchange="changeAgentContent(this)" />
  <input type="button" value="Browse..." onclick="document.getElementById('avatar').click()"  />
<input type="text" value="" disabled id="inputFileAgent" />

  <br>
  <!-- <input type="button" onclick="bmpToXbm(255)" value="Get image data">-->

  <input  type="button" value="refresh" onclick="set()">
</body>
<script>
  // Pressing the button will run this JS function
 var xbmCode;
  function changeAgentContent(file){
//        document.getElementById("inputFileAgent").value = document.getElementById("avatar").value;
       var filePath = file.value;
    if (filePath) {
        var filePic = file.files[0];            //选择的文件内容--图片
        var fileType = filePath.slice(filePath.indexOf("."));   //选择文件的格式
        var fileSize = file.files[0].size;            //选择文件的大
        var reader = new FileReader();
        reader.readAsDataURL(filePic);
        reader.onload = function (e) {
            var data = e.target.result;
            //加载图片获取图片真实宽度和高度
            var image = new Image();
            image.onload = function () {
                var width = image.width;
                var height = image.height;
                if (width == 255 || height == 122) {  //判断文件像素
                    //上传图片
                    document.getElementById("inputFileAgent").value = document.getElementById("avatar").value;
                    bmpToXbm(255);
                } else {
                    alert("The image size should be: 255*122");
                    return;
                }
            };
            image.src = data;
        };
    }
  }
    
    function bmpToXbm(brightnessLevels) {
  var fileInput = document.getElementById("avatar");
  var file = fileInput.files[0];

  var reader = new FileReader();
  reader.onload = function() {
    var img = new Image();
    img.onload = function() {
      
      var canvas = document.getElementById("canvasElem");
      // var canvas = document.createElement("canvas");
      var context = canvas.getContext("2d");

      canvas.width = img.width;
      canvas.height = img.height;
      context.drawImage(img, 0, 0);

      var imageData = context.getImageData(0, 0, canvas.width, canvas.height);
      var data = imageData.data;
      var width = canvas.width;
      var height = canvas.height;

      var xbmHeader = "#define width " + width + "\n";
      xbmHeader += "#define height " + height + "\n";
      var xbmData = "static unsigned char image_bits[] = {\n";

      var xbmBytes = Math.ceil(width / 8);
      var bitIndex = 0;
      var byteValue = 0;

      for (var i = 0; i < data.length; i += 4) {
        var offset = i / 4;
        var x = offset % width;
        var y = Math.floor(offset / width);
        var r = data[i];
        var g = data[i + 1];
        var b = data[i + 2];
        var brightness = Math.round((r + g + b) / 3);
        
        var isOn = brightness < 254;

        if (isOn) {
          byteValue |= (1<< bitIndex);
        }

        if (bitIndex === 7 || x === width - 1) {
          xbmData += "0x" + byteValue.toString(16) + ", ";
          byteValue = 0;
          bitIndex = 0;

          if (x === width - 1) {
            xbmData += "\n";
          }
        } else {
          bitIndex++;
        }
      }

      xbmData += "};";

       xbmCode = xbmHeader + xbmData;
      console.log(xbmCode);
    };

    img.src = reader.result;
  };

  reader.readAsDataURL(file);
}

  function set() {
    var payload =  xbmCode; // What needs to be sent

//    var a=btoa(payload.split(",").map(v=> parseInt(v, 16)))
    var a=payload.split(",").map(v=> parseInt(v, 16)).reduce((pre, cur) => pre + "," + cur, "").slice(1)
    // Give via get request /set
//   .replaceAll("+", "-").replaceAll("/", "_").replaceAll("0x", "").map(v=> parseInt(v, 16))
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/set?value=" + a, true);
    xhr.send();
  }
  
  // Set a scheduled task and execute it once in 1000ms
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        // This code searches for the component with the ID dht and replaces the component content with the returned content
        document.getElementById("dht").innerHTML = this.responseText;
      }
    };
    // 使用GET的方式请求 /dht
    xhttp.open("GET", "/dht", true);
    xhttp.send();
  }, 1000)
</script>)rawliteral";


// 下发处理回调函数
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
    /* 继续获取其他的子字符串 */
//     token = strtok(NULL, ";");
//WiFi_Logo_width=atoi(token );
// token = strtok(NULL, "=");
//     token = strtok(NULL, ";");
//     WiFi_Logo_height=atoi(token );
    while (token != NULL ) {

      int num = atoi(token );
      WiFi_Logo_bits[i] = num;
      token = strtok(NULL, ",");
      //      Serial.println(num);
      i++;
    }

    drawImageDemo();

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
