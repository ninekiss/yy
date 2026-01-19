#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include "time.h"

class TimeManager {
private:
    const char* ntpServer;
    long gmtOffset;
    int daylightOffset;

public:
    TimeManager(const char* server = "ntp.aliyun.com", long offset = 28800) 
        : ntpServer(server), gmtOffset(offset), daylightOffset(0) {}

    void begin() {
        Serial.println("[Time] Configuring NTP...");
        configTime(gmtOffset, daylightOffset, ntpServer);
    }

    // 尝试获取时间，成功返回 true，失败返回 false
    // 这里的 t 是引用传递，会将获取到的时间填入 t
    bool getTime(struct tm &t) {
        if(!getLocalTime(&t)){
            return false;
        }
        return true;
    }
    
    // 阻塞直到时间同步成功 (用于 setup)
    void waitForSync() {
        Serial.print("[Time] Syncing");
        struct tm t;
        while (!getTime(t)) {
            Serial.print(".");
            delay(500);
        }
        Serial.println("\n[Time] Synced!");
        Serial.printf("[Time] Current time: %04d-%02d-%02d %02d:%02d:%02d\n", 
            t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, 
            t.tm_hour, t.tm_min, t.tm_sec);
    }
};

#endif