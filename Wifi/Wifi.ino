#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// ESP32 WiFi舵机控制程序
// 该程序创建一个WiFi热点，允许用户通过网页控制舵机角

// 创建舵机对象
Servo myServo;

// WiFi热点配置
const char* ssid = "ESP32-Servo-Control";     // 热点名称
const char* password = "12345678";            // 热点密码（至少8位）

// 网络配置
IPAddress local_IP(192, 168, 137, 1);        // ESP32的IP地址
IPAddress gateway(192, 168, 137, 1);         // 网关
IPAddress subnet(255, 255, 255, 0);          // 子网掩码

// 创建Web服务器对象
WebServer server(80);

// 舵机配置
const int servoPin = 18;                     // 舵机连接到GPIO18
int servoPosition = 90;                      // 初始位置（90度）

// HTML网页内容
const char* htmlPage = R"rawstring(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 舵机控制</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 20px;
            background-color: #f0f0f0;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background-color: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            margin-bottom: 30px;
        }
        .servo-control {
            margin: 20px 0;
        }
        input[type="range"] {
            width: 100%;
            height: 10px;
            margin: 20px 0;
            background: #ddd;
            outline: none;
            border-radius: 5px;
        }
        input[type="range"]::-webkit-slider-thumb {
            appearance: none;
            width: 25px;
            height: 25px;
            background: #04AA6D;
            cursor: pointer;
            border-radius: 50%;
        }
        button {
            background-color: #04AA6D;
            color: white;
            padding: 15px 30px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
            margin: 10px;
        }
        button:hover {
            background-color: #45a049;
        }
        .position-display {
            font-size: 24px;
            font-weight: bold;
            color: #333;
            margin: 20px 0;
        }
        .preset-buttons {
            margin: 20px 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 舵机控制面板</h1>
        
        <div class="position-display">
            当前角度: <span id="currentAngle">90</span>度
        </div>
        
        <div class="servo-control">
            <input type="range" id="servoSlider" min="0" max="180" value="90" oninput="updateServo(this.value)">
            <div>0度                    90度                    180度</div>
        </div>
        
        <div class="preset-buttons">
            <h3>预设位置:</h3>
            <button onclick="setPosition(0)">0度</button>
            <button onclick="setPosition(45)">45度</button>
            <button onclick="setPosition(90)">90度</button>
            <button onclick="setPosition(135)">135度</button>
            <button onclick="setPosition(180)">180度</button>
        </div>
        
        <div style="margin-top: 30px; color: #666; font-size: 14px;">
            <p>连接到: ESP32-Servo-Control</p>
            <p>IP地址: 192.168.137.1</p>
        </div>
    </div>

    <script>
        function updateServo(angle) {
            document.getElementById('currentAngle').innerText = angle;
            
            fetch('/servo?angle=' + angle)
                .then(response => response.text())
                .then(data => console.log('舵机角度已设置为: ' + angle + '度'))
                .catch(error => console.error('错误:', error));
        }
        
        function setPosition(angle) {
            document.getElementById('servoSlider').value = angle;
            updateServo(angle);
        }
        
        window.onload = function() {
            fetch('/position')
                .then(response => response.text())
                .then(data => {
                    document.getElementById('currentAngle').innerText = data;
                    document.getElementById('servoSlider').value = data;
                })
                .catch(error => console.error('获取位置错误:', error));
        }
    </script>
</body>
</html>
)rawstring";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("启动ESP32舵机控制程序...");
  
  // 初始化舵机
  myServo.attach(servoPin);
  myServo.write(servoPosition);
  Serial.println("舵机初始化完成，初始位置: 90度");
  
  // 配置WiFi热点
  Serial.print("正在配置WiFi热点...");
  
  // 先停止可能存在的WiFi连接
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_AP);
  delay(100);
  
  // 配置静态IP
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("静态IP配置失败");
  }
  
  // 启动热点 (强制使用2.4GHz频段)
  if (WiFi.softAP(ssid, password, 1, 0, 4)) {  // channel=1, hidden=0, max_connection=4
    Serial.println("WiFi热点启动成功！");
    Serial.print("热点名称: ");
    Serial.println(ssid);
    Serial.print("密码: ");
    Serial.println(password);
    Serial.print("IP地址: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("频道: 1 (2.4GHz)");
    Serial.println();
    
    // 显示连接的设备数量
    Serial.print("当前连接设备数: ");
    Serial.println(WiFi.softAPgetStationNum());
  } else {
    Serial.println("WiFi热点启动失败！");
  }
  
  // 配置Web服务器路由
  server.on("/", handleRoot);
  server.on("/servo", handleServo);
  server.on("/position", handlePosition);
  
  // 启动Web服务器
  server.begin();
  Serial.println("Web服务器已启动");
  Serial.println("========================================");
  Serial.println("设置完成！");
  Serial.println("1. 连接WiFi热点: ESP32-Servo-Control");
  Serial.println("2. 密码: 12345678");
  Serial.println("3. 打开浏览器访问: http://192.168.137.1");
  Serial.println("========================================");
}

void loop() {
  server.handleClient();  // 处理客户端请求
  
  // 每10秒显示一次状态信息
  static unsigned long lastStatusTime = 0;
  if (millis() - lastStatusTime > 10000) {
    lastStatusTime = millis();
    Serial.print("热点状态 - 连接设备数: ");
    Serial.print(WiFi.softAPgetStationNum());
    Serial.print(", IP: ");
    Serial.println(WiFi.softAPIP());
  }
  
  delay(10);
}

// 处理根路径请求（显示主页）
void handleRoot() {
  server.send(200, "text/html", htmlPage);
  Serial.println("主页已加载");
}

// 处理舵机控制请求
void handleServo() {
  if (server.hasArg("angle")) {
    String angleStr = server.arg("angle");
    int angle = angleStr.toInt();
    
    // 限制角度范围
    if (angle >= 0 && angle <= 180) {
      servoPosition = angle;
      myServo.write(servoPosition);
      
      server.send(200, "text/plain", "OK");
      Serial.print("舵机角度设置为: ");
      Serial.print(angle);
      Serial.println("度");
    } else {
      server.send(400, "text/plain", "角度超出范围 (0-180)");
      Serial.println("错误: 角度超出范围");
    }
  } else {
    server.send(400, "text/plain", "缺少角度参数");
    Serial.println("错误: 缺少角度参数");
  }
}

// 处理位置查询请求
void handlePosition() {
  server.send(200, "text/plain", String(servoPosition));
}