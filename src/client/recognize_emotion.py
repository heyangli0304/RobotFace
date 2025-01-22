import cv2
from fer import FER
import sys
import time
import socket
from collections import Counter

# 打印 Python 路径
print("Python Executable: ", sys.executable)

# 创建情绪识别器
emotion_detector = FER()

# 打开摄像头
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Could not open video.")
    sys.exit()

# 获取摄像头分辨率
frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

# 连接 ESP32 服务器
server_ip = '192.168.111.107'
server_port = 12345
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    sock.connect((server_ip, server_port))
except Exception as e:
    print(f"Error: {e}")
    sys.exit()

# 发送间隔控制
send_interval = 0.2  # 每0.2秒发送一次
last_send_time = time.time()

# 主循环
while True:
    ret, frame = cap.read()
    # 将视频帧水平翻转
    frame = cv2.flip(frame, 1)  # 1 表示水平翻转
    if not ret:
        print("Error: Unable to read frame.")
        break

    # FER进行情绪识别
    emotions = emotion_detector.detect_emotions(frame)
    if emotions:
        # 获取最大的人脸
        max_area = 0
        dominant_emotion = None
        for emotion in emotions:
            box = emotion['box']
            area = box[2] * box[3]
            if area > max_area:
                max_area = area
                dominant_emotion = max(emotion["emotions"], key=emotion["emotions"].get)
                x, y, w, h = box

        # 计算角度
       # angle_x = int((x + w / 2) / frame_width * 180)
        #angle_y = int((y + h / 2) / frame_height * 180)

        # 数据打包
        data_to_send = f"{dominant_emotion},{x},{y}"

        # 控制数据发送频率
        if time.time() - last_send_time >= send_interval:
            try:
                sock.sendall((data_to_send + '\n').encode('utf-8'))
                print(f"Sent: {data_to_send}")
                last_send_time = time.time()
            except Exception as e:
                print(f"Error sending data: {e}")
                break

        # 在图像上绘制识别框和情绪标签
        cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)
        cv2.putText(frame, f"{dominant_emotion}", (x, y - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.9, (255, 255, 255), 2)

    # 显示图像
    cv2.imshow("Emotion Recognition", frame)

    # 按 'q' 退出
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# 释放资源
cap.release()
sock.close()
cv2.destroyAllWindows()
