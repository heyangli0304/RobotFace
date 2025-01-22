#include <WiFi.h>
#include <ESP32Servo.h>

const char* ssid = "MOJOY";
const char* password = "123456789.";
WiFiServer server(12345);
/*眼睛y轴引脚32
  眼睛x轴引脚23
  左眼皮引脚33
  右眼皮引脚21
  左眉毛26
  右眉毛18
  嘴左侧13
  嘴右侧4
  张闭口15
  扩展引脚17，14
*/
Servo myServo[11];
const int servoPins[] = {32,23,33,21,26,14,18,17,13,4,15};
const int numServos = 11;
//初始舵机角度120度
int initAngles[] = {120,95,168,130,140,65,90,73,90,90,25};  // 每个舵机的角度预设

// 预设角度范围
int xMin = 60, xMax = 130;   // x轴舵机的角度范围
int yMin = 106, yMax = 134;   // y轴舵机的角度范围
int eyeLeftMin = 140, eyeLeftMax = 168;   // 左上眼皮角度范围
int eyeRightMin = 130, eyeRightMax = 165; // 右上眼皮角度范围
int mouthLeftMin = 70, mouthLeftMax = 90;  // 嘴巴左侧角度范围
int mouthRightMin = 80, mouthRightMax = 110; // 嘴巴右侧角度范围

// 时间间隔控制闭眼睁眼
unsigned long lastEyeTime = 0;
bool eyeState = false; // false为睁眼，true为闭眼

// 初始化舵机
void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("连接中...");
    }

    Serial.println("已连接到WiFi");
    Serial.println(WiFi.localIP());

    for (int i = 0; i < numServos; i++) {
        myServo[i].setPeriodHertz(50);  
        myServo[i].attach(servoPins[i], 500, 2500);  // 设置舵机频率和最大最小脉宽
        myServo[i].write(initAngles[i]);  // 设置初始角度
    }

    server.begin();
    Serial.println("服务器已启动");
}

// 转换坐标到角度
int mapToAngle(int value, int minValue, int maxValue, int minAngle, int maxAngle) {
    return map(value, minValue, maxValue, minAngle, maxAngle);
}

// 设置舵机角度
void setServoAngles(int emotionIndex) {
    switch (emotionIndex) {
        case 0:  // neutral
            myServo[4].write(140); // 左眉毛
            myServo[6].write(90); // 右眉毛
            myServo[5].write(71); // 扩展眉毛
            myServo[7].write(73); // 扩展眉毛
            myServo[8].write(90); // 嘴巴左侧
            myServo[9].write(90); // 嘴巴右侧
            myServo[10].write(25);  // 下巴
            break;
        case 1:  // happy
            myServo[4].write(150); // 左眉毛
            myServo[6].write(80); // 右眉毛
            myServo[5].write(90); // 扩展眉毛
            myServo[7].write(90); // 扩展眉毛
            myServo[8].write(100); // 嘴巴左侧
            myServo[9].write(80); // 嘴巴右侧
            myServo[10].write(37);  // 下巴
            break;
        case 2:  // sad
            myServo[4].write(180); // 左眉毛
            myServo[6].write(50); // 右眉毛
            myServo[5].write(45); // 扩展眉毛
            myServo[7].write(45); // 扩展眉毛
            myServo[8].write(50); // 嘴巴左侧
            myServo[9].write(120); // 嘴巴右侧
            myServo[10].write(25);  // 下巴
            break;
        case 3:  // angry
            myServo[4].write(100); // 左眉毛
            myServo[6].write(130); // 右眉毛
            myServo[5].write(90); // 扩展眉毛
            myServo[7].write(90); // 扩展眉毛
            myServo[8].write(50); // 嘴巴左侧
            myServo[9].write(120); // 嘴巴右侧
            myServo[10].write(25);  // 下巴
            break;
        case 4:  // surprise
            myServo[4].write(155); // 左眉毛
            myServo[6].write(75); // 右眉毛
            myServo[8].write(90); // 扩展眉毛
            myServo[7].write(90); // 扩展眉毛
            myServo[10].write(57);  // 下巴
            break;
        default:
            break;
    }
}

unsigned long lastBlinkTime = 0; // 记录上次眨眼的时间
const unsigned long blinkInterval = 10000; // 每10秒眨眼一次
const unsigned long blinkDuration = 1000; // 眨眼时长为1秒

void blinkEyes() {
    // 眨眼过程
    myServo[2].write(130); // 右上眼皮闭眼
    myServo[3].write(168); // 左上眼皮闭眼
    delay(blinkDuration / 2); // 闭眼的时间
    myServo[2].write(165); // 右上眼皮睁眼
    myServo[3].write(140); // 左上眼皮睁眼
    delay(blinkDuration / 2); // 睁眼的时间
}
void loop() {
    WiFiClient client = server.available();


    if (client) {
        Serial.println("客户端已连接");
        while (client.connected()) {
            if (client.available()) {
                String receivedData = client.readStringUntil('\n');
                receivedData.trim();
                Serial.println("接收到数据: " + receivedData);

                int firstComma = receivedData.indexOf(',');
                int secondComma = receivedData.indexOf(',', firstComma + 1);
                if (firstComma == -1 || secondComma == -1) {
                    Serial.println("数据格式错误");
                    continue;
                }

                String emotion = receivedData.substring(0, firstComma);
                int x = receivedData.substring(firstComma + 1, secondComma).toInt();
                int y = receivedData.substring(secondComma + 1).toInt();

                // 根据坐标计算对应的角度
                int angleX = mapToAngle(x, 5, 450, xMin, xMax);
                int angleY = mapToAngle(y, 5, 300, yMax, yMin);  //y轴反向映射

                // 设置舵机角度
                myServo[1].write(angleX); // x轴舵机
                myServo[0].write(angleY); // y轴舵机

                // 情绪对应的舵机设置
                int emotionIndex = 0;
                if (emotion == "happy") emotionIndex = 1;
                else if (emotion == "sad") emotionIndex = 2;
                else if (emotion == "angry") emotionIndex = 3;
                else if (emotion == "surprise") emotionIndex = 4;
                else if (emotion == "neutral") emotionIndex = 0;

                setServoAngles(emotionIndex);
                    // 检查是否需要眨眼
                unsigned long currentTime = millis();
                if (currentTime - lastBlinkTime >= blinkInterval) {
                    blinkEyes();
                    lastBlinkTime = currentTime; // 更新上次眨眼时间
                }
             
                client.println("数据接收成功");
            }
        }
        client.stop();
        Serial.println("客户端断开连接");
    }
   
}
