#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>

#ifndef WIFI_SSID
#define WIFI_SSID "your_ssid"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_password"
#endif

WebServer server(80);

void handleRoot() {
  String message = "<h1>Hello from ESP32-S3!</h1>";
  message += "<p>Uptime: " + String(millis() / 1000) + " seconds</p>";
  server.send(200, "text/html; charset=utf-8", message);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // 烧录后在串口监视器查看这个 IP

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient(); // 处理网页请求
}