              ESP32-CAM Surveillance Robot Car
Live Video Streaming + WiFi Web Control.

Created by THE ROBOT MAKERS


Project Overview:

 This project turns the ESP32-CAM AI Thinker into a surveillance robot car that streams live video over WiFi while allowing the user to control its movement (Forward, Backward, Left, Right, Stop) through a   built- in Web Control Panel.

The robot acts as a small internet-controlled rover for:

-Home surveillance

-Remote monitoring

-Robotics learning

-Wireless FPV car


Features: 

- WiFi control (no Bluetooth needed)

- Live MJPEG video stream at http://IP:81/stream

- Web-based control UI (Forward, Left, Right, Stop, Backward)

- Clean UI interface (HTML + JavaScript)

- L298N/HW-130 motor driver control

- Motor + camera run in parallel using FreeRTOS tasks

- Camera runs in grayscale mode for improved speed & stability

Components Used:
| Component                     | Quantity | Purpose                    |
| ----------------------------- | -------- | -------------------------- |
| ESP32-CAM AI Thinker          | 1        | Camera + WiFi + controller |
| FTDI programmer               | 1        | For uploading code to ESP32-CAM AI Thinker         |
| Motor Driver (L298N)          | 1        | Control motors             |
| TT Gear Motors                | 4        | Movement                   |
| Robot chassis                 | 1        | Body                       |
| 18650 Battery / Li-ion        | 2 (10v-12v) | Power                      |
| Jumper wires                  | Several  | Connections                |



Pin Connections (ESP32-CAM → Motor Driver):
| ESP32-CAM Pin | Motor Driver Function |
| ------------- | --------------------- |
| GPIO 12       | IN1                   |
| GPIO 13       | IN2                   |
| GPIO 14       | IN3                   |
| GPIO 15       | IN4                   |




Accessing the Robot:

1. Power the car

2. Open Serial Monitor (115200 baud)

3. It prints something like:
 
  Control page: http://192.168.0.102

  Stream page:  http://192.168.0.102:81/stream


Full Source Code:

Already included in the github.



Future Improvements:

1. Add obstacle avoidance.

2. Add voice control via server.

3. Add AI image detection (face/object).

4. Add Android app for control.

5. Add battery level monitor.


Credits:

Made by MANSUR ISAH (SmartSense)

Team: THE ROBOT MAKERS.
