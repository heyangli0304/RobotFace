#include <ESP32Servo.h>

Servo myServo1;  // 第一个舵机
Servo myServo2;  // 第二个舵机

const int servoPin1 = 33;  // 第一个舵机连接到引脚33
const int servoPin2 = 22;  // 第二个舵机连接到引脚32

void setup() {
    Serial.begin(115200);
    // 等待串口连接
    while (!Serial) {
        delay(10);
    }

    // 初始化第一个舵机
    myServo1.setPeriodHertz(50);  // 设置第一个舵机的频率
    myServo1.attach(servoPin1, 500, 2500);  // 附加第一个舵机的引脚和PWM范围

    // 初始化第二个舵机
    myServo2.setPeriodHertz(50);  // 设置第二个舵机的频率
    myServo2.attach(servoPin2, 500, 2500);  // 附加第二个舵机的引脚和PWM范围

    // 默认位置归零
    myServo1.write(90);  
    myServo2.write(90);  
    
    Serial.println("两个舵机控制测试已启动，输入角度（0-180）：");
    Serial.println("输入格式：angle1,angle2  (例如: 90,45)");
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');  // 读取串口输入
        input.trim();  // 去除多余的换行符和空格

        // 查找输入中的逗号
        int commaIndex = input.indexOf(',');
        if (commaIndex != -1) {
            String angle1Str = input.substring(0, commaIndex);  // 获取第一个舵机角度
            String angle2Str = input.substring(commaIndex + 1);  // 获取第二个舵机角度

            int angle1 = angle1Str.toInt();  // 将第一个角度转换为整数
            int angle2 = angle2Str.toInt();  // 将第二个角度转换为整数

            // 检查角度有效性
            if (angle1 >= 0 && angle1 <= 180 && angle2 >= 0 && angle2 <= 180) {
                myServo1.write(angle1);  // 控制第一个舵机转到指定角度
                myServo2.write(angle2);  // 控制第二个舵机转到指定角度

                Serial.printf("第一个舵机已移动到角度：%d\n", angle1);
                Serial.printf("第二个舵机已移动到角度：%d\n", angle2);
            } else {
                Serial.println("角度无效，请输入0到180之间的角度");
            }
        } else {
            Serial.println("输入格式错误，请使用：angle1,angle2");
        }
    }
}
