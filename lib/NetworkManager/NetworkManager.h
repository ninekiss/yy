#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

class NetworkManager {
private:
    const char* ssid;
    const char* password;

public:
    NetworkManager(const char* _ssid, const char* _pwd) : ssid(_ssid), password(_pwd) {}

    void connect() {
        if (WiFi.status() == WL_CONNECTED) return;

        Serial.printf("[Network] Connecting to %s ", ssid);
        WiFi.begin(ssid, password);
        
        // 尝试连接 10 秒，连不上就不阻塞了，交给 loop 去重试
        int timeout = 20; 
        while (WiFi.status() != WL_CONNECTED && timeout > 0) {
            delay(500);
            Serial.print(".");
            timeout--;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n[Network] Connected!");
        } else {
            Serial.println("\n[Network] Connection failed (timeout).");
        }
    }

    bool isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }
};

#endif