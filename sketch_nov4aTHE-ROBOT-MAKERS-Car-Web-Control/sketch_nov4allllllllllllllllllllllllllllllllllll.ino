#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi credentials
const char* ssid = "**********";
const char* password = "********";

// Motor driver pins (L298N / HW-130)
#define IN1 12
#define IN2 13
#define IN3 14
#define IN4 15

WebServer controlServer(80);
WiFiServer streamServer(81);

// ESP32-CAM AI Thinker pin config
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ===== Motor Control =====
void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void turnLeft() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

// ===== Web Control UI =====
void handleRoot() {
  controlServer.send(200, "text/html", R"rawliteral(
    <html><head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>Surveillance Car by THE ROBOT MAKERS</title>
      <style>
        button { width: 80px; height: 50px; font-size: 16px; margin: 5px; }
        .btns { display: flex; flex-wrap: wrap; justify-content: center; }
        img { width: 100%; max-width: 480px; }
      </style>
    </head><body>
      <h2 align="center">THE ROBOT MAKERS</h2>
      <div align="center">
        <img src="http://)rawliteral" + WiFi.localIP().toString() + R"rawliteral(:81/stream">
      </div>
      <div class="btns">
        <button onclick="send('F')">Forward</button><br>
        <button onclick="send('L')">Left</button>
        <button onclick="send('S')">Stop</button>
        <button onclick="send('R')">Right</button><br>
        <button onclick="send('B')">Backward</button>
      </div>
      <script>
        function send(cmd) {
          fetch("/control?move=" + cmd);
        }
      </script>
    </body></html>
  )rawliteral");
}

// ===== Command Handler =====
void handleControl() {
  String move = controlServer.arg("move");
  Serial.println("Received command: " + move);

  if (move == "F") moveForward();
  else if (move == "B") moveBackward();
  else if (move == "L") turnLeft();
  else if (move == "R") turnRight();
  else stopMotors();

  controlServer.send(204, "", "");//sngier
}
 
// ===== Camera Setup (Grayscale) =====
void startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM; config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.frame_size = FRAMESIZE_CIF; // 352x288
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    while (true);
  }
}

// ===== Streaming Task =====
void streamVideo() {
  WiFiClient client = streamServer.available();
  if (!client) return;

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println();

  while (client.connected()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) continue;

    uint8_t* jpg_buf = NULL;
    size_t jpg_len = 0;
    bool converted = frame2jpg(fb, 40, &jpg_buf, &jpg_len);

    if (converted) {
      client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", jpg_len);
      client.write(jpg_buf, jpg_len);
      client.println();
      free(jpg_buf);
    }

    esp_camera_fb_return(fb);
    delay(10); // frame delay
  }
}

// Task to run streamVideo() independently
void streamTask(void* pvParameters) {
  while (true) {
    streamVideo();
    vTaskDelay(1); // avoid watchdog timeout
  }
}

void setup() {
  Serial.begin(115200);

  // Motor pin setup
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  stopMotors();

  // Connect Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("Control page: http://"); Serial.println(WiFi.localIP());
  Serial.print("Stream page:  http://"); Serial.print(WiFi.localIP()); Serial.println(":81/stream");

  // Camera + servers
  startCamera();
  controlServer.on("/", handleRoot);
  controlServer.on("/control", handleControl);
  controlServer.begin();
  streamServer.begin();

  // Launch camera stream task on core 0
  xTaskCreatePinnedToCore(
    streamTask,       // Function
    "streamTask",     // Name
    8192,             // Stack size
    NULL,             // Param
    1,                // Priority
    NULL,             // Handle
    0                 // Core 0
  );
}

void loop() {
  controlServer.handleClient();  // only handles motor control
}