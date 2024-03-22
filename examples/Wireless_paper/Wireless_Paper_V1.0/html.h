//An array for storing web pages 
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
        var filePic = file.files[0];            //Selected file content - images
        var fileType = filePath.slice(filePath.indexOf("."));   //Choose the format of the file
        var fileSize = file.files[0].size;            //选Choose the size of the file
        var reader = new FileReader();
        reader.readAsDataURL(filePic);
        reader.onload = function (e) {
            var data = e.target.result;
            //Load image to obtain the true width and height of the image
            var image = new Image();
            image.onload = function () {
                var width = image.width;
                var height = image.height;
                if (width == 255 || height == 122) {  //Determine file pixels
                    //upload pictures
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

      

      var xbmData;
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
       xbmCode =  xbmData;
      console.log(xbmCode);
    };

    img.src = reader.result;
  };

  reader.readAsDataURL(file);
}

  function set() {
    var payload =  xbmCode; // What needs to be sent



    var a=payload.split(",").map(v=> parseInt(v, 16)).reduce((pre, cur) => pre + "," + cur, "").slice(1)
    // Give via get request /set
//   .replaceAll("+", "-").replaceAll("/", "_").replaceAll("0x", "").map(v=> parseInt(v, 16))
     var xhr = new XMLHttpRequest();
    // XMLHttpRequest
    xhr.open("GET", "/set?value=" + a, true);
    xhr.send();
  }
  
</script>)rawliteral";

