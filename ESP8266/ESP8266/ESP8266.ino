#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h> // Thư viện để xử lý JSON

#define SensorDHT11 4
#define Control_1 14
DHT dht(SensorDHT11, DHT11);

ESP8266WebServer server(80);
const char *ssid = "ESP8266_wifi";
const char *password = "82668266";

float temperature;
float humidity;
bool Control_1_status;

String dataSEND;
String Web_Address;

void setup() {
  Serial.begin(9600);
  delay(100);

  WiFi.softAP(ssid, password);
  dht.begin();

  Control_1_status = true;
  pinMode(Control_1, OUTPUT); 
  digitalWrite(Control_1, HIGH);// Chỉ định chân Control_1 là chân OUTPUT

  Web_Address = WiFi.softAPIP().toString(); // Chuyển địa chỉ IP thành chuỗi

  server.on("/", HTTP_GET, UploadWeb);
  server.on("/data", HTTP_GET, DataCenter); // Thêm route để trả về JSON
  server.on("/control", HTTP_GET, ControlCenter); // Thêm route để điều khiển LED
  server.begin();
}

void loop() {
  server.handleClient();
  CollectDataSensor();
}

void CollectDataSensor() {
  delay(100);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) {
    dataSEND = String(0) + "|" + String(0) + "|" + Web_Address + "|" + String(Control_1_status);
    Serial.println(dataSEND);
  } else {
    dataSEND = String(temperature) + "|" + String(humidity) + "|" + Web_Address + "|" + String(Control_1_status);
    Serial.println(dataSEND);
  }
  delay(1000);
}

void DataCenter() {
  String json = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + ",\"Control_1_status\":" + String(Control_1_status) + "}";
  server.send(200, "application/json", json);
  //Serial.println(json);
}

void ControlCenter() {
  String DataGet = server.arg("sign");
  //Serial.println(DataGet);
  if (DataGet == "click") 
    Control_1_status = !Control_1_status;
  if (Control_1_status == true) 
    digitalWrite(Control_1, HIGH); // Bật LED
  else
    digitalWrite(Control_1, LOW); // tắt LED
  server.send(200, "text/plain", "OK");
}

void UploadWeb() {
  String html = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body {
      font-family: Arial, sans-serif;
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
      background-color: #f0f0f0;
    }

    .container {
      text-align: center;
      border: 1px solid #ccc;
      padding: 20px;
      border-radius: 10px;
      background-color: #4CAF50;
      transition: background-color 0.3s, box-shadow 0.3s;
      margin: 10px;
      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    }

    .yellow-box {
      background-color: yellow;
      border-radius: 10px;
      padding: 10px;
      margin-bottom: 20px;
      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    }

    .blue-box {
      background-color: blue;
      border-radius: 10px;
      padding: 10px;
      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    }

    #temperature {
      color: red;
      text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5); /* Hiệu ứng đổ bóng */
    }

    #humidity {
      color: white;
      text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5); /* Hiệu ứng đổ bóng */
    }

    #Control1Button {
      cursor: pointer;
      background-color: white;
      color: black;
      border-radius: 50%; /* Make it circular */
      padding: 10px 20px;
      margin-bottom: 20px;
      width: 60px; /* Set the width and height to make it circular */
      height: 60px;
      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
      justify-content: center; /* Center horizontally */
      align-items: center; /* Center vertically */
    }

    h1,h2 {
      color: black;
      text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5); /* Hiệu ứng đổ bóng */
    }
  </style>
</head>
<body>
  <h2>Đang lấy dữ liệu ...</h2>
  <div class="container">
    <h1>WebServer ESP8266</h1>
    <h1>by PHÓNG-THÔNG</h1>
    <div class="yellow-box">
      <span id="temperature">Nhiệt độ: --°C</span><br>
    </div>
    <div class="blue-box">
      <span id="humidity">Độ ẩm: --%</span><br>
    </div>
  </div>
  <h1>Control</h1>
  <button id="Control1Button" onclick="toggleLED()">1</button>
  <script>
    let intervalID; // Biến lưu trữ ID của hẹn giờ
    let button1Disabled = false; // Biến kiểm soát trạng thái của nút 1

    async function updateData() {
      try {
        const response = await fetch("/data");
        if (response.ok) {
          const data = await response.json();
          document.getElementById("temperature").textContent = `Nhiệt độ: ${data.temperature} °C`;
          document.getElementById("humidity").textContent = `Độ ẩm: ${data.humidity} %`;
        
          //Cập nhật trạng thái BUTTON
          if(data.Control_1_status == '0')
            document.getElementById("Control1Button").style.backgroundColor = `white`;
          else
            document.getElementById("Control1Button").style.backgroundColor = `red`;
          
          const h2 = document.querySelector('h2');
          h2.textContent = '';
        }
      } catch (error) {
        console.error("Lỗi khi cập nhật dữ liệu:", error);
      }
    }

    async function toggleLED() {
      if (button1Disabled) {
        return; // Nếu nút đã bị vô hiệu hóa, không làm gì cả
      }
      const h2 = document.querySelector('h2');
      h2.textContent = 'Cập nhật...';
      // Vô hiệu hóa nút 1
      button1Disabled = true;
      try {
        const response = await fetch("/control?sign=click");
        if (response.ok) {
          // Sau khi điều khiển thiết bị, gọi ngay lập tức hàm updateData()
          updateData();
          
          // Kiểm tra nếu hẹn giờ đang hoạt động thì hủy nó
          if (intervalID) {
            clearInterval(intervalID);
          }

          // Thiết lập hẹn giờ mới sau khi điều khiển thiết bị
          intervalID = setInterval(updateData, 10000);

          // Mở lại nút 1
          button1Disabled = false;
        } else {
          console.error("Lỗi khi điều khiển đèn LED.");
        }
      } catch (error) {
        console.error("Lỗi khi điều khiển LED:", error);
      }
    }

    updateData();

    // Cập nhật dữ liệu mỗi 10 giây
    // Bắt đầu hẹn giờ ban đầu
    intervalID = setInterval(updateData, 10000);
  </script>
</body>
</html>
  )HTML";

  server.send(200, "text/html", html);
}
