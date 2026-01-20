#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

class MqttManager {
private:
    WiFiClient espClient;
    PubSubClient client;
    const char* server;
    int port;
    const char* user;
    const char* pass;
    
    // 定义收到消息时的回调类型
    typedef std::function<void(char*, uint8_t*, unsigned int)> MqttCallback;

public:
    MqttManager(const char* _server, int _port, const char* _user, const char* _pass) 
        : client(espClient), server(_server), port(_port), user(_user), pass(_pass) {}

    void begin(MqttCallback callback) {
        client.setServer(server, port);
        client.setCallback(callback); // 设置收到指令时的处理函数

        // 默认只有 256，发长 JSON 会失败
        client.setBufferSize(512);

        // 新增：设置心跳为 60 秒 (默认是 15 秒)
        // 这样服务器会更宽容，允许最长 90 秒的静默
        client.setKeepAlive(60);
    }

    void connect() {
        // 如果已经连接，直接返回
        if (client.connected()) return;

        Serial.println("[MQTT] Connecting...");
        // 生成唯一的 Client ID
        String clientId = "ESP32-Watering-" + String(random(0xffff), HEX);

        // 尝试连接
        if (client.connect(clientId.c_str(), user, pass)) {
            Serial.println("Connected!");
            // 连接成功后，重新订阅指令 Topic
            #ifdef MQTT_TOPIC_CMD
                client.subscribe(MQTT_TOPIC_CMD);
                Serial.println("[MQTT] Subscribed to: " MQTT_TOPIC_CMD);
            #else
                client.subscribe("watering/cmd");
                Serial.println("[MQTT] Subscribed to: watering/cmd");
            #endif
            #ifdef MQTT_TOPIC_STATUS
                publish(MQTT_TOPIC_STATUS, "Online");
            #else
                publish("watering/status", "Online");
            #endif
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // 这里不阻塞，由 main loop 控制重试频率
        }
    }

    void loop() {
        client.loop(); // 必须频繁调用以保持心跳
    }

    bool isConnected() {
        return client.connected();
    }

    void publish(const char* topic, const char* payload) {
        if (client.connected()) {
            client.publish(topic, payload);
            Serial.printf("[MQTT] Send [%s]: %s\n", topic, payload);
        }
    }
};

#endif